/*
 *  Copyright (c) 2024 Rockchip Corporation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include "algo_types_priv.h"
#include "hsv_types_prvt.h"
#include "xcam_log.h"

#include "RkAiqCalibDbTypes.h"
#include "RkAiqCalibDbTypesV2.h"
#include "RkAiqCalibDbV2Helper.h"

#include "interpolation.h"
#include "c_base/aiq_base.h"

static int illu_estm_once(ahsv_param_illuLink_t* illuLinks, uint8_t illuLink_len, float awbGain[2]) {
    int ret = -1;
    uint8_t case_id = 0;
    if (illuLink_len == 0)
        return -1;

    float minDist = 9999999;
    float dist;
    float nRG = awbGain[0]; //current R/G gain
    float nBG = awbGain[1]; //current B/G gain

    int i;
    for (i=0; i<illuLink_len; i++) {
        ahsv_param_illuLink_t *pIlluCase = &illuLinks[i];
        float refRG = pIlluCase->sw_hsvC_wbGainR_val;
        float refBG = pIlluCase->sw_hsvC_wbGainB_val;
        dist = sqrt((nRG - refRG) * (nRG -  refRG) + (nBG -  refBG) * (nBG -  refBG));
        if(dist < minDist) {
            minDist = dist;
            ret = i;
        }
    }
    LOGD_AHSV("ret %d, minDist %f, %s", ret, minDist, illuLinks[ret].sw_hsvC_illu_name);
    return ret;
}

static float get_alpha_by_gain(float idx[4], float val[4], float gain) {
    float alpha = 0.0;
    interpolation_f(idx, val, 4, gain, &alpha);
    return alpha;
}

static int get_mesh_by_name(HsvContext_t* pHsvCtx, char* name) {
    ahsv_hsvCalib_t* calibdb = &pHsvCtx->hsv_calib->calibdb;
    int table_len = calibdb->sw_hsvCfg_tblAll_len;

    int i;
    for (i=0; i<table_len; i++) {
        ahsv_tableAll_t *pTable = &calibdb->tableAll[i];
        if (strncmp(name, pTable->sw_hsvC_illu_name, AHSV_ILLUM_NAME_LEN) == 0) {
            return i;
        }
    }

    return -1;
}

static void interpolate_lut1d_alpha(float alpha, hsv_lut1d_dyn_t* in, int16_t* out) {
    int32_t f1 = (int32_t)(alpha * 128.0f);
    int32_t f2 = 128 - f1;
    int32_t lut1d0;

    if (in->hw_hsvT_lut1d_mode < 4) {
        for(int i = 0; i < HSV_1DLUT_NUM; i++) {
            out[i] = (int16_t)((f1 * (int32_t)in->hw_hsvT_lut1d_val[i] + (1 << 6)) >> 7);
        }
    } else {
        for(int i = 0; i < HSV_1DLUT_NUM; i++) {
            lut1d0 = (i << 4) & 0x3ff;
            out[i] = (int16_t)((f1 * (int32_t)in->hw_hsvT_lut1d_val[i] + f2 * lut1d0 + (1 << 6)) >> 7);
        }
    }
}
    
static void interpolate_lut2d_alpha(float alpha, hsv_lut2d_dyn_t* in, int16_t* out) {
    int32_t f1 = (int32_t)(alpha * 128.0f);
    int32_t f2 = 128 - f1;
    int32_t lut2d0;
    int lut_idx = 0;

    if (in->hw_hsvT_lut2d_mode > 2) {
        for (int i = 0; i < HSV_2DLUT_ROW; i++) {
            lut2d0 = (i << 6) & 0x3ff;
            for (int j = 0; j < HSV_2DLUT_COL; j++) {
                lut_idx = i * HSV_2DLUT_COL + j;
                out[lut_idx] = 
                    (int16_t)((f1 * (int32_t)in->hw_hsvT_lut2d_val[lut_idx] + f2 * lut2d0 + (1 << 6)) >> 7);
            }
        }
    } else {
        for (int j = 0; j < HSV_2DLUT_COL; j++) {
            lut2d0 = (j << 6) & 0x3ff;
            for (int i = 0; i < HSV_2DLUT_ROW; i++) {
                lut_idx = i * HSV_2DLUT_ROW + j;
                out[lut_idx] = 
                    (int16_t)((f1 * (int32_t)in->hw_hsvT_lut2d_val[lut_idx] + f2 * lut2d0 + (1 << 6)) >> 7);
            }
        }
    }
}

static void array_weight_sum(int16_t* in, int16_t* out, int32_t wei0, int32_t num, int bit, int32_t* sum) 
{
    int32_t wei1 = (1 >> bit) - wei0;
    for (int i = 0; i < num; i++) {
        out[i] =
            (int16_t)((wei0 * (int32_t) out[i] + wei1 * (int32_t)in[i])>>bit);
        
        *sum += abs(out[i]); 
    }
}
static void Damping(HsvContext_t* pHsvCtx, float damp) 
{
    int32_t f1 = (int32_t)(damp * 128.0f);
    int32_t lutSum[3] = {0};

    hsv_meshGain_t *pDamped = &pHsvCtx->damped_lut;
    hsv_meshGain_t *pUndamped = &pHsvCtx->undamped_lut;
    hsv_calib_attrib_t* hsv_calib = pHsvCtx->hsv_calib;
    ahsv_param_static_t* sta = &hsv_calib->tunning.stAuto.sta;

    /* calc. damped lut */
    if (sta->hsvCfg.hw_hsvT_lut0_en) {
        array_weight_sum(pDamped->lut0, pUndamped->lut0, f1, 
                         HSV_1DLUT_NUM, 7, &lutSum[0]);
        LOGD_AHSV("lut0: Damping %f, lutsum0 %d -> %d\n", 
                  damp, lutSum[0], pHsvCtx->pre_lutSum[0]);
    } else {
        array_weight_sum(pDamped->lut0, pUndamped->lut0, 128, 
                         HSV_1DLUT_NUM, 7, &lutSum[0]);
        LOGD_AHSV("lut1: Damping 0, lutsum0 %d -> %d\n", 
                  lutSum[0], pHsvCtx->pre_lutSum[0]);
    }
    if (sta->hsvCfg.hw_hsvT_lut2_en) {
        array_weight_sum(pDamped->lut1, pUndamped->lut1, f1, 
                         HSV_1DLUT_NUM, 7, &lutSum[1]);
        LOGD_AHSV("lut1: Damping %f, lutsum1 %d -> %d\n", 
                  damp, lutSum[1], pHsvCtx->pre_lutSum[1]);
    } else {
        array_weight_sum(pDamped->lut1, pUndamped->lut1, 128, 
                         HSV_1DLUT_NUM, 7, &lutSum[1]);
        LOGD_AHSV("lut1: Damping 0, lutsum1 %d -> %d\n", 
                  lutSum[1], pHsvCtx->pre_lutSum[1]);
    }
    if (sta->hsvCfg.hw_hsvT_lut2_en) {
        array_weight_sum(pDamped->lut2, pUndamped->lut2, f1, 
                         HSV_2DLUT_NUM, 7, &lutSum[2]);
        LOGD_AHSV("lut2: Damping %f, lutsum2 %d -> %d\n", 
                  damp, lutSum[2], pHsvCtx->pre_lutSum[2]);
    } else {
        array_weight_sum(pDamped->lut2, pUndamped->lut2, 128, 
                         HSV_2DLUT_NUM, 7, &lutSum[2]);
        LOGD_AHSV("lut2: Damping 0, lutsum2 %d -> %d\n", 
                  lutSum[2], pHsvCtx->pre_lutSum[2]);
    }

    pHsvCtx->damp_converged = !((lutSum[0] - pHsvCtx->pre_lutSum[0]) > 0 ||
                                  (lutSum[1] - pHsvCtx->pre_lutSum[1]) > 0 ||
                                  (lutSum[2] - pHsvCtx->pre_lutSum[2]) > 0);
    LOGD_AHSV("lut converged %d \n", pHsvCtx->damp_converged);
    pHsvCtx->pre_lutSum[0] = lutSum[0];
    pHsvCtx->pre_lutSum[1] = lutSum[1];
    pHsvCtx->pre_lutSum[2] = lutSum[2];
}

XCamReturn Ahsv_prepare(RkAiqAlgoCom* params)
{
    HsvContext_t* pHsvCtx = (HsvContext_t *)params->ctx;
    hsv_calib_attrib_t* pcalib =
        (hsv_calib_attrib_t*)(CALIBDBV2_GET_MODULE_PTR(params->u.prepare.calibv2, hsv));

    pHsvCtx->hsv_calib = pcalib;

    pHsvCtx->pre_gain = -1;
    pHsvCtx->pre_wbgain[0] = -1;
    pHsvCtx->pre_wbgain[1] = -1;
    pHsvCtx->pre_illu_idx = INVALID_ILLU_IDX;
    pHsvCtx->pre_mode[0] = INVALID_ILLU_IDX;
    pHsvCtx->pre_mode[1] = INVALID_ILLU_IDX;
    pHsvCtx->pre_mode[2] = INVALID_ILLU_IDX;
    pHsvCtx->pre_lutSum[0] = -1;
    pHsvCtx->pre_lutSum[1] = -1;
    pHsvCtx->pre_lutSum[2] = -1;
    pHsvCtx->damp_converged = false;
    pHsvCtx->is_calib_update = false;

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn Ahsv_processing(const RkAiqAlgoCom* inparams, RkAiqAlgoResCom* outparams, illu_estm_info_t* swinfo)
{
    HsvContext_t* pHsvCtx = (HsvContext_t*)inparams->ctx;
    hsv_calib_attrib_t* hsv_calib = pHsvCtx->hsv_calib;
    hsv_api_attrib_t* tunning = &hsv_calib->tunning;
    ahsv_hsvCalib_t* calibdb = &hsv_calib->calibdb;
    ahsv_param_dyn_t* pdyn = &tunning->stAuto.dyn;

    bool need_recal = false;

    bool isReCal_ = inparams->u.proc.is_attrib_update || inparams->u.proc.init;
    if (isReCal_ || pHsvCtx->is_calib_update) {
        need_recal = true;
    }

    float delta_wbg = sqrt((swinfo->awbGain[0] - pHsvCtx->pre_wbgain[0]) * 
                           (swinfo->awbGain[0] - pHsvCtx->pre_wbgain[0]) + 
                           (swinfo->awbGain[1] - pHsvCtx->pre_wbgain[1]) * 
                           (swinfo->awbGain[1] - pHsvCtx->pre_wbgain[1]));
    //(1) estimate idx
    int illu_idx = -1;
    if (delta_wbg > tunning->stAuto.sta.sw_hsvT_wbgainDiff_th || isReCal_) {
        illu_idx = illu_estm_once(pdyn->illuLink, pdyn->sw_hsvCfg_illuLink_len, swinfo->awbGain);
        if (illu_idx < 0) {
            LOGE_AHSV("illu_estm_once failed!");
            return XCAM_RETURN_ERROR_PARAM;
        }
    }
    if (illu_idx < 0) {
        illu_idx = pHsvCtx->pre_illu_idx;
    }

    ahsv_param_illuLink_t* pIlluCase = &pdyn->illuLink[illu_idx];    

    if (illu_idx != pHsvCtx->pre_illu_idx || pHsvCtx->is_calib_update) {
        pHsvCtx->pre_illu_idx  = illu_idx;
        need_recal = true;

        LOGD_AHSV("HSV illu change --> %s",
                pdyn->illuLink[illu_idx].sw_hsvC_illu_name);

        int mesh_idx = get_mesh_by_name(pHsvCtx, pIlluCase->sw_hsvC_illu_name);
        if (mesh_idx  < 0) {
            LOGE_AHSV("get_mesh_by_name failed!");
            return XCAM_RETURN_ERROR_PARAM;
        }

        ahsv_tableAll_t* pTable = &calibdb->tableAll[mesh_idx];
        
        pHsvCtx->calib_lut = &pTable->meshGain;
    }

    //(2) interpolate alpha
    float alpha = pHsvCtx->pre_alpha;
    float delta_gain = fabs(swinfo->sensorGain - pHsvCtx->pre_gain);
    if (delta_gain > tunning->stAuto.sta.sw_hsvT_gainDiff_th || isReCal_) {
        alpha = get_alpha_by_gain(
            pIlluCase->gain2StrgCurve.sw_hsvT_isoIdx_val, pIlluCase->gain2StrgCurve.sw_hsvT_alpha_val, swinfo->sensorGain);
    }
    
    if (fabs(alpha - pHsvCtx->pre_alpha) > DIVMIN) {
        LOGD_AHSV("HSV alpha changed %f --> %f", pHsvCtx->pre_alpha, alpha);

        pHsvCtx->pre_alpha  = alpha;
        need_recal = true;
    }

    //(3) lut = alpha*lutfile + (1-alpha)*lut0
    if (need_recal || pHsvCtx->is_calib_update) {
        pHsvCtx->is_calib_update = false;
        LOGD_AHSV("HSV interpolate lut by alpha");
        if (tunning->stAuto.sta.hsvCfg.hw_hsvT_lut0_en)
            interpolate_lut1d_alpha(alpha, &pHsvCtx->calib_lut->lut0, pHsvCtx->undamped_lut.lut0);
        else
            memcpy(pHsvCtx->undamped_lut.lut0, 
                   pHsvCtx->calib_lut->lut0.hw_hsvT_lut1d_val,
                   sizeof(pHsvCtx->calib_lut->lut0.hw_hsvT_lut1d_val));
        if (tunning->stAuto.sta.hsvCfg.hw_hsvT_lut1_en)
            interpolate_lut1d_alpha(alpha, &pHsvCtx->calib_lut->lut1, pHsvCtx->undamped_lut.lut1);
        else
            memcpy(pHsvCtx->undamped_lut.lut1, 
                   pHsvCtx->calib_lut->lut1.hw_hsvT_lut1d_val,
                   sizeof(pHsvCtx->calib_lut->lut1.hw_hsvT_lut1d_val));
        if (tunning->stAuto.sta.hsvCfg.hw_hsvT_lut2_en)
            interpolate_lut2d_alpha(alpha, &pHsvCtx->calib_lut->lut2, pHsvCtx->undamped_lut.lut2);
        else
            memcpy(pHsvCtx->undamped_lut.lut2, 
                   pHsvCtx->calib_lut->lut2.hw_hsvT_lut2d_val,
                   sizeof(pHsvCtx->calib_lut->lut2.hw_hsvT_lut2d_val));
    }

    //(4) damp
    bool mode_keep = 
            (pHsvCtx->calib_lut->lut0.hw_hsvT_lut1d_mode == pHsvCtx->pre_mode[0]) &&
            (pHsvCtx->calib_lut->lut1.hw_hsvT_lut1d_mode == pHsvCtx->pre_mode[1]) && 
            (pHsvCtx->calib_lut->lut2.hw_hsvT_lut2d_mode == pHsvCtx->pre_mode[2]);
    
    if (!mode_keep) {
        pHsvCtx->pre_mode[0] = pHsvCtx->calib_lut->lut0.hw_hsvT_lut1d_mode;
        pHsvCtx->pre_mode[1] = pHsvCtx->calib_lut->lut1.hw_hsvT_lut1d_mode;
        pHsvCtx->pre_mode[2] = pHsvCtx->calib_lut->lut2.hw_hsvT_lut2d_mode;
    }

    bool damp_en = tunning->stAuto.sta.sw_hsvT_damp_en && mode_keep;
    if (damp_en) {
        if (need_recal || !pHsvCtx->damp_converged) {
            float dampCoef = swinfo->awbIIRDampCoef;
            Damping(pHsvCtx, dampCoef);
            need_recal = true;
        }
    }

    outparams->cfg_update = false;
    hsv_param_t* hsvRes = outparams->algoRes;
    if (isReCal_) {
        hsvRes->dyn.lut0.hw_hsvT_lut1d_mode = pHsvCtx->calib_lut->lut0.hw_hsvT_lut1d_mode;
        hsvRes->dyn.lut1.hw_hsvT_lut1d_mode = pHsvCtx->calib_lut->lut1.hw_hsvT_lut1d_mode;
        hsvRes->dyn.lut2.hw_hsvT_lut2d_mode = pHsvCtx->calib_lut->lut2.hw_hsvT_lut2d_mode;
    }
    if (need_recal) {
        hsvRes->sta = tunning->stAuto.sta.hsvCfg;
        if (damp_en) {
            memcpy(hsvRes->dyn.lut0.hw_hsvT_lut1d_val, 
                   pHsvCtx->damped_lut.lut0, 
                   sizeof(pHsvCtx->damped_lut.lut0));
            memcpy(hsvRes->dyn.lut1.hw_hsvT_lut1d_val, 
                   pHsvCtx->damped_lut.lut1, 
                   sizeof(pHsvCtx->damped_lut.lut1));
            memcpy(hsvRes->dyn.lut2.hw_hsvT_lut2d_val, 
                   pHsvCtx->damped_lut.lut2, 
                   sizeof(pHsvCtx->damped_lut.lut2));
        } else {
            memcpy(hsvRes->dyn.lut0.hw_hsvT_lut1d_val, 
                   pHsvCtx->undamped_lut.lut0, 
                   sizeof(pHsvCtx->undamped_lut.lut0));
            memcpy(hsvRes->dyn.lut1.hw_hsvT_lut1d_val, 
                   pHsvCtx->undamped_lut.lut1, 
                   sizeof(pHsvCtx->undamped_lut.lut1));
            memcpy(hsvRes->dyn.lut2.hw_hsvT_lut2d_val, 
                   pHsvCtx->undamped_lut.lut2, 
                   sizeof(pHsvCtx->undamped_lut.lut2));
        }

        outparams->cfg_update = true;
        outparams->en = tunning->en;
        outparams->bypass = tunning->bypass;
    }

    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn
create_context
(
    RkAiqAlgoContext** context,
    const AlgoCtxInstanceCfg* cfg
)
{
    XCamReturn result = XCAM_RETURN_NO_ERROR;
    CamCalibDbV2Context_t *pCalibDbV2 = cfg->calibv2;
    LOG1_AHSV("%s: (enter)\n", __FUNCTION__);

    HsvContext_t *ctx = aiq_mallocz(sizeof(HsvContext_t));
    memset(ctx, 0, sizeof(HsvContext_t));

    *context = (RkAiqAlgoContext *)(ctx);

    LOG1_AHSV("%s: (exit)\n", __FUNCTION__);
    return result;
}

static XCamReturn
destroy_context
(
    RkAiqAlgoContext* context
)
{
    XCamReturn result = XCAM_RETURN_NO_ERROR;

    HsvContext_t* pHsvCtx = (HsvContext_t*)context;
    aiq_free(pHsvCtx);
    return result;
}

static XCamReturn
prepare
(
    RkAiqAlgoCom* params
)
{
    XCamReturn result = XCAM_RETURN_NO_ERROR;
    HsvContext_t* pHsvCtx = (HsvContext_t*)params->ctx;

    Ahsv_prepare(params);
    
    return result;
}

static XCamReturn
processing
(
    const RkAiqAlgoCom* inparams,
    RkAiqAlgoResCom* outparams
)
{
    XCamReturn result = XCAM_RETURN_NO_ERROR;
    LOG1_AHSV("%s: (enter)\n", __FUNCTION__ );

    RkAiqAlgoProcHsv* proc_param = (RkAiqAlgoProcHsv*)inparams;
    illu_estm_info_t *swinfo = &proc_param->illu_info;

    Ahsv_processing(inparams, outparams, swinfo);

    LOG1_AHSV("%s: (exit)\n", __FUNCTION__ );
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
algo_hsv_queryahsvStatus
(
    RkAiqAlgoContext* ctx,
    ahsv_status_t* status
)
{
    if(ctx == NULL || status == NULL) {
        LOGE_AHSV("have no ahsv status info !");
        return XCAM_RETURN_ERROR_PARAM;
    }

    HsvContext_t* pHsvCtx = (HsvContext_t*)ctx;
    hsv_param_auto_t* stAuto = &pHsvCtx->hsv_calib->tunning.stAuto;

    strncpy(status->sw_hsvC_illuUsed_name, 
            stAuto->dyn.illuLink[pHsvCtx->pre_illu_idx].sw_hsvC_illu_name, 
            AHSV_ILLUM_NAME_LEN - 1);
    
    status->sw_hsvT_alpha_val = pHsvCtx->pre_alpha;

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
algo_hsv_SetCalib
(
    RkAiqAlgoContext* ctx,
    ahsv_hsvCalib_t* calib
) {
    if(ctx == NULL || calib == NULL) {
        LOGE_AHSV("%s: null pointer\n", __FUNCTION__);
        return XCAM_RETURN_ERROR_PARAM;
    }

    HsvContext_t* pHsvCtx = (HsvContext_t*)ctx;
    hsv_api_attrib_t* hsv_attrib = &pHsvCtx->hsv_calib->tunning;
    ahsv_hsvCalib_t* ahsv_calib = &pHsvCtx->hsv_calib->calibdb;

    if (hsv_attrib->opMode != RK_AIQ_OP_MODE_AUTO) {
        LOGE_AHSV("not auto mode: %d", hsv_attrib->opMode);
        return XCAM_RETURN_ERROR_PARAM;
    }

    memcpy(ahsv_calib, calib, sizeof(ahsv_hsvCalib_t));
    pHsvCtx->is_calib_update = true;

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
algo_hsv_GetCalib
(
    RkAiqAlgoContext* ctx,
    ahsv_hsvCalib_t* calib
)
{
    if(ctx == NULL || calib == NULL) {
        LOGE_AHSV("%s: null pointer\n", __FUNCTION__);
        return XCAM_RETURN_ERROR_PARAM;
    }

    HsvContext_t* pHsvCtx = (HsvContext_t*)ctx;
    ahsv_hsvCalib_t* ahsv_calib = &pHsvCtx->hsv_calib->calibdb;

    memcpy(calib, ahsv_calib, sizeof(ahsv_hsvCalib_t));

    return XCAM_RETURN_NO_ERROR;
}

#define RKISP_ALGO_HSV_VERSION     "v0.1.0"
#define RKISP_ALGO_HSV_VENDOR      "Rockchip"
#define RKISP_ALGO_HSV_DESCRIPTION "Rockchip hsv algo for ISP2.0"

RkAiqAlgoDescription g_RkIspAlgoDescHsv = {
    .common = {
        .version = RKISP_ALGO_HSV_VERSION,
        .vendor  = RKISP_ALGO_HSV_VENDOR,
        .description = RKISP_ALGO_HSV_DESCRIPTION,
        .type    = RK_AIQ_ALGO_TYPE_AHSV,
        .id      = 0,
        .create_context  = create_context,
        .destroy_context = destroy_context,
    },
    .prepare = prepare,
    .pre_process = NULL,
    .processing = processing,
    .post_process = NULL,
};

//RKAIQ_END_DECLARE
