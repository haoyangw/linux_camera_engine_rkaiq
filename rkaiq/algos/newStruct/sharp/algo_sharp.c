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
#include "sharp_types_prvt.h"
#include "xcam_log.h"

#include "algo_types_priv.h"
#include "RkAiqCalibDbTypes.h"
#include "RkAiqCalibDbTypesV2.h"
#include "RkAiqCalibDbV2Helper.h"
#include "interpolation.h"
#include "c_base/aiq_base.h"
#include "newStruct/algo_common.h"

#if RKAIQ_HAVE_SHARP_V33
XCamReturn SharpSelectParam
(
    sharp_param_auto_t *paut,
    sharp_param_t* out,
    int iso)
{
    if(paut == NULL || out == NULL) {
        LOGE_ASHARP("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return XCAM_RETURN_ERROR_PARAM;
    }

    int i = 0;
    int iso_low = 0, iso_high = 0, ilow = 0, ihigh = 0;
    float ratio = 0.0f;
    uint16_t uratio;
    pre_interp(iso, NULL, 0, &ilow, &ihigh, &ratio);
    uratio = ratio * (1 << RATIO_FIXBIT);

    return XCAM_RETURN_NO_ERROR;
}
#endif

#if RKAIQ_HAVE_SHARP_V34
XCamReturn SharpSelectParam
(
    SharpContext_t *pSharpCtx,
    sharp_param_t* out,
    int iso)
{
    sharp_param_auto_t *paut = &pSharpCtx->sharp_attrib->stAuto;

    if(paut == NULL || out == NULL) {
        LOGE_ASHARP("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return XCAM_RETURN_ERROR_PARAM;
    }

    int i = 0;
    int iso_low = 0, iso_high = 0, ilow = 0, ihigh = 0, inear = 0;
    float ratio = 0.0f;
    uint16_t uratio;
    pre_interp(iso, NULL, 0, &ilow, &ihigh, &ratio);
    uratio = ratio * (1 << RATIO_FIXBIT);

    if (ratio > 0.5)
        inear = ihigh;
    else
        inear = ilow;

    for (i=0; i<8; i++) {
        out->dyn.hfExtra_sgmEnv.sw_sharpC_luma2Sigma_curve.idx[i] = paut->dyn[inear].hfExtra_sgmEnv.sw_sharpC_luma2Sigma_curve.idx[i];
    }
    for (i=0; i<8; i++) {
        out->dyn.hfExtra_sgmEnv.sw_sharpC_luma2Sigma_curve.val[i] = interpolation_u16(
            paut->dyn[ilow].hfExtra_sgmEnv.sw_sharpC_luma2Sigma_curve.val[i], paut->dyn[ihigh].hfExtra_sgmEnv.sw_sharpC_luma2Sigma_curve.val[i], uratio);
    }
    out->dyn.hfExtra_preBifilt.sw_sharpT_filtCfg_mode = paut->dyn[inear].hfExtra_preBifilt.sw_sharpT_filtCfg_mode;
    out->dyn.hfExtra_preBifilt.sw_sharpT_filtSpatial_strg = interpolation_f32(
        paut->dyn[ilow].hfExtra_preBifilt.sw_sharpT_filtSpatial_strg, paut->dyn[ihigh].hfExtra_preBifilt.sw_sharpT_filtSpatial_strg, ratio);
    for (i=0; i<3; i++) {
        out->dyn.hfExtra_preBifilt.hw_sharpT_filtSpatial_wgt[i] = interpolation_f32(
            paut->dyn[ilow].hfExtra_preBifilt.hw_sharpT_filtSpatial_wgt[i], paut->dyn[ihigh].hfExtra_preBifilt.hw_sharpT_filtSpatial_wgt[i], ratio);
    }
    out->dyn.hfExtra_preBifilt.sw_sharpT_rgeSgm_scale = interpolation_f32(
        paut->dyn[ilow].hfExtra_preBifilt.sw_sharpT_rgeSgm_scale, paut->dyn[ihigh].hfExtra_preBifilt.sw_sharpT_rgeSgm_scale, ratio);
    out->dyn.hfExtra_preBifilt.sw_sharpT_rgeSgm_offset = interpolation_u8(
        paut->dyn[ilow].hfExtra_preBifilt.sw_sharpT_rgeSgm_offset, paut->dyn[ihigh].hfExtra_preBifilt.sw_sharpT_rgeSgm_offset, uratio);
    out->dyn.hfExtra_preBifilt.hw_sharpT_bifiltOut_alpha = interpolation_f32(
        paut->dyn[ilow].hfExtra_preBifilt.hw_sharpT_bifiltOut_alpha, paut->dyn[ihigh].hfExtra_preBifilt.hw_sharpT_bifiltOut_alpha, ratio);
    out->dyn.hfExtra_lpf.sw_sharpT_filtCfg_mode = paut->dyn[inear].hfExtra_lpf.sw_sharpT_filtCfg_mode;
    out->dyn.hfExtra_lpf.sw_sharpT_hfHi_strg = interpolation_f32(
        paut->dyn[ilow].hfExtra_lpf.sw_sharpT_hfHi_strg, paut->dyn[ihigh].hfExtra_lpf.sw_sharpT_hfHi_strg, ratio);
    out->dyn.hfExtra_lpf.sw_sharpT_hfMid_strg = interpolation_f32(
        paut->dyn[ilow].hfExtra_lpf.sw_sharpT_hfMid_strg, paut->dyn[ihigh].hfExtra_lpf.sw_sharpT_hfMid_strg, ratio);
    for (i=0; i<6; i++) {
        out->dyn.hfExtra_lpf.hw_sharpT_lpf_wgt[i] = interpolation_f32(
            paut->dyn[ilow].hfExtra_lpf.hw_sharpT_lpf_wgt[i], paut->dyn[ihigh].hfExtra_lpf.hw_sharpT_lpf_wgt[i], ratio);
    }
    out->dyn.hfExtra_lpf.hw_sharpT_lpfOut_alpha = interpolation_f32(
        paut->dyn[ilow].hfExtra_lpf.hw_sharpT_lpfOut_alpha, paut->dyn[ihigh].hfExtra_lpf.hw_sharpT_lpfOut_alpha, ratio);
    out->dyn.hfExtra_hfBifilt.sw_sharpT_filtCfg_mode = paut->dyn[inear].hfExtra_hfBifilt.sw_sharpT_filtCfg_mode;
    out->dyn.hfExtra_hfBifilt.sw_sharpT_filtSpatial_strg = interpolation_f32(
        paut->dyn[ilow].hfExtra_hfBifilt.sw_sharpT_filtSpatial_strg, paut->dyn[ihigh].hfExtra_hfBifilt.sw_sharpT_filtSpatial_strg, ratio);
    for (i=0; i<3; i++) {
        out->dyn.hfExtra_hfBifilt.hw_sharpT_filtSpatial_wgt[i] = interpolation_f32(
            paut->dyn[ilow].hfExtra_hfBifilt.hw_sharpT_filtSpatial_wgt[i], paut->dyn[ihigh].hfExtra_hfBifilt.hw_sharpT_filtSpatial_wgt[i], ratio);
    }
    out->dyn.hfExtra_hfBifilt.sw_sharpT_rgeSgm_scale = interpolation_f32(
        paut->dyn[ilow].hfExtra_hfBifilt.sw_sharpT_rgeSgm_scale, paut->dyn[ihigh].hfExtra_hfBifilt.sw_sharpT_rgeSgm_scale, ratio);
    out->dyn.hfExtra_hfBifilt.sw_sharpT_rgeSgm_offset = interpolation_u8(
        paut->dyn[ilow].hfExtra_hfBifilt.sw_sharpT_rgeSgm_offset, paut->dyn[ihigh].hfExtra_hfBifilt.sw_sharpT_rgeSgm_offset, uratio);
    out->dyn.hfExtra_hfBifilt.hw_sharpT_biFiltOut_alpha = interpolation_f32(
        paut->dyn[ilow].hfExtra_hfBifilt.hw_sharpT_biFiltOut_alpha, paut->dyn[ihigh].hfExtra_hfBifilt.hw_sharpT_biFiltOut_alpha, ratio);
    for (i=0; i<8; i++) {
        out->dyn.shpScl_hf.hw_sharpT_luma2hfScl_val[i] = interpolation_f32(
            paut->dyn[ilow].shpScl_hf.hw_sharpT_luma2hfScl_val[i], paut->dyn[ihigh].shpScl_hf.hw_sharpT_luma2hfScl_val[i], ratio);
    }
    for (i=0; i<17; i++) {
        out->dyn.sharpOpt.hw_sharpT_hfScl2ShpScl_val[i] = interpolation_u16(
            paut->dyn[ilow].sharpOpt.hw_sharpT_hfScl2ShpScl_val[i], paut->dyn[ihigh].sharpOpt.hw_sharpT_hfScl2ShpScl_val[i], uratio);
    }
    out->dyn.shpScl_locSgmStrg.hw_sharpT_locSgmStrg_mode = paut->dyn[inear].shpScl_locSgmStrg.hw_sharpT_locSgmStrg_mode;
    out->dyn.shpScl_locSgmStrg.hw_sharpT_glbSgmStrg_val = interpolation_f32(
        paut->dyn[ilow].shpScl_locSgmStrg.hw_sharpT_glbSgmStrg_val, paut->dyn[ihigh].shpScl_locSgmStrg.hw_sharpT_glbSgmStrg_val, ratio);
    out->dyn.shpScl_locSgmStrg.hw_sharpT_glbSgmStrg_alpha = interpolation_f32(
        paut->dyn[ilow].shpScl_locSgmStrg.hw_sharpT_glbSgmStrg_alpha, paut->dyn[ihigh].shpScl_locSgmStrg.hw_sharpT_glbSgmStrg_alpha, ratio);
    out->dyn.shpScl_locSgmStrg.hw_sharpT_localSgmStrg_scale = interpolation_f32(
        paut->dyn[ilow].shpScl_locSgmStrg.hw_sharpT_localSgmStrg_scale, paut->dyn[ihigh].shpScl_locSgmStrg.hw_sharpT_localSgmStrg_scale, ratio);
    out->dyn.shpScl_texDetect.hw_sharpT_estNsFilt_mode = paut->dyn[inear].shpScl_texDetect.hw_sharpT_estNsFilt_mode;
    out->dyn.shpScl_texDetect.hw_sharpT_estNsClip_mode = paut->dyn[inear].shpScl_texDetect.hw_sharpT_estNsClip_mode;
    out->dyn.shpScl_texDetect.hw_sharpT_estNsManual_maxLimit = interpolation_u16(
        paut->dyn[ilow].shpScl_texDetect.hw_sharpT_estNsManual_maxLimit, paut->dyn[ihigh].shpScl_texDetect.hw_sharpT_estNsManual_maxLimit, uratio);
    out->dyn.shpScl_texDetect.hw_sharpT_estNs_scale = interpolation_f32(
        paut->dyn[ilow].shpScl_texDetect.hw_sharpT_estNs_scale, paut->dyn[ihigh].shpScl_texDetect.hw_sharpT_estNs_scale, ratio);
    out->dyn.shpScl_texDetect.hw_sharpT_estNsNorize_shift = paut->dyn[inear].shpScl_texDetect.hw_sharpT_estNsNorize_shift;
    out->dyn.sharpOpt.hw_sharpT_shpSrc_mode = paut->dyn[inear].sharpOpt.hw_sharpT_shpSrc_mode;
    out->dyn.sharpOpt.hw_sharpT_shpOpt_mode = paut->dyn[inear].sharpOpt.hw_sharpT_shpOpt_mode;
    out->dyn.sharpOpt.hw_sharpT_hfHiGlbShpScl_val = interpolation_f32(
        paut->dyn[ilow].sharpOpt.hw_sharpT_hfHiGlbShpScl_val, paut->dyn[ihigh].sharpOpt.hw_sharpT_hfHiGlbShpScl_val, ratio);
    out->dyn.sharpOpt.hw_sharpT_hfMidGlbShpScl_val = interpolation_f32(
        paut->dyn[ilow].sharpOpt.hw_sharpT_hfMidGlbShpScl_val, paut->dyn[ihigh].sharpOpt.hw_sharpT_hfMidGlbShpScl_val, ratio);
    for (i=0; i<14; i++) {
        out->dyn.sharpOpt.hw_sharpT_locSgmStrg2ShpScl_val[i] = interpolation_f32(
            paut->dyn[ilow].sharpOpt.hw_sharpT_locSgmStrg2ShpScl_val[i], paut->dyn[ihigh].sharpOpt.hw_sharpT_locSgmStrg2ShpScl_val[i], ratio);
    }
    for (i=0; i<22; i++) {
        out->dyn.sharpOpt.hw_sharpT_radiDist2ShpScl_val[i] = interpolation_f32(
            paut->dyn[ilow].sharpOpt.hw_sharpT_radiDist2ShpScl_val[i], paut->dyn[ihigh].sharpOpt.hw_sharpT_radiDist2ShpScl_val[i], ratio);
    }
    out->dyn.sharpOpt.hw_sharpT_tex2ShpScl_scale = interpolation_f32(
        paut->dyn[ilow].sharpOpt.hw_sharpT_tex2ShpScl_scale, paut->dyn[ihigh].sharpOpt.hw_sharpT_tex2ShpScl_scale, ratio);
    out->dyn.sharpOpt.hw_sharpT_texShpSclRemap_en = paut->dyn[inear].sharpOpt.hw_sharpT_texShpSclRemap_en;
    for (i=0; i<17; i++) {
        out->dyn.sharpOpt.hw_sharpT_texShpSclRemap_val[i] = interpolation_u16(
            paut->dyn[ilow].sharpOpt.hw_sharpT_texShpSclRemap_val[i], paut->dyn[ihigh].sharpOpt.hw_sharpT_texShpSclRemap_val[i], uratio);
    }
    out->dyn.sharpOpt.sw_sharpT_bwEdgClipIdx_mode = paut->dyn[inear].sharpOpt.sw_sharpT_bwEdgClipIdx_mode;
    for (i=0; i<8; i++) {
        out->dyn.sharpOpt.hw_sharpT_luma2WhtEdg_maxLimit[i] = interpolation_u16(
            paut->dyn[ilow].sharpOpt.hw_sharpT_luma2WhtEdg_maxLimit[i], paut->dyn[ihigh].sharpOpt.hw_sharpT_luma2WhtEdg_maxLimit[i], uratio);
    }
    for (i=0; i<8; i++) {
        out->dyn.sharpOpt.hw_sharpT_luma2BkEdg_maxLimit[i] = interpolation_u16(
            paut->dyn[ilow].sharpOpt.hw_sharpT_luma2BkEdg_maxLimit[i], paut->dyn[ihigh].sharpOpt.hw_sharpT_luma2BkEdg_maxLimit[i], uratio);
    }
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn SharpApplyStrength
(
    SharpContext_t *pSharpCtx,
    sharp_param_t* out)
{
    if(pSharpCtx == NULL || out == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return XCAM_RETURN_ERROR_PARAM;
    }

    if (pSharpCtx->strength_en) {
        float fPercent = algo_strength_to_percent(pSharpCtx->fStrength);

        out->dyn.hfExtra_preBifilt.hw_sharpT_bifiltOut_alpha /= fPercent;
        out->dyn.hfExtra_lpf.hw_sharpT_lpfOut_alpha /= fPercent;
        out->dyn.hfExtra_hfBifilt.hw_sharpT_biFiltOut_alpha /= fPercent;
        out->dyn.sharpOpt.hw_sharpT_hfHiGlbShpScl_val *= fPercent;
        out->dyn.sharpOpt.hw_sharpT_hfMidGlbShpScl_val *= fPercent;

        for (int i = 0; i < RK_SHARP_V34_LUMA_POINT_NUM; i++) {
            out->dyn.sharpOpt.hw_sharpT_luma2WhtEdg_maxLimit[i] *= fPercent;
        }
        for (int i = 0; i < RK_SHARP_V34_LUMA_POINT_NUM; i++) {
            out->dyn.sharpOpt.hw_sharpT_luma2BkEdg_maxLimit[i] *= fPercent;
        }

        LOGD_ANR("SharpApplyStrength: fStrength %f, fPercent %f\n", pSharpCtx->fStrength, fPercent);
    }

    return XCAM_RETURN_NO_ERROR;
}
#endif

static XCamReturn
create_context(RkAiqAlgoContext **context, const AlgoCtxInstanceCfg* cfg)
{
    XCamReturn result = XCAM_RETURN_NO_ERROR;
    CamCalibDbV2Context_t *pCalibDbV2 = cfg->calibv2;

    SharpContext_t* ctx = aiq_mallocz(sizeof(SharpContext_t));

    if (ctx == NULL) {
        LOGE_ASHARP("%s create context failed!", __func__);
        return XCAM_RETURN_ERROR_MEM;
    }
    ctx->isReCal_ = true;
    ctx->sharp_attrib =
        (sharp_api_attrib_t*)(CALIBDBV2_GET_MODULE_PTR(pCalibDbV2, sharp));

    *context = (RkAiqAlgoContext* )ctx;
    LOGV_ASHARP("%s: Sharp (exit)\n", __FUNCTION__ );

    *context = (RkAiqAlgoContext*)ctx;
    return result;
}

static XCamReturn
destroy_context(RkAiqAlgoContext *context)
{
    XCamReturn result = XCAM_RETURN_NO_ERROR;
    SharpContext_t* pSharpCtx = (SharpContext_t*)context;
    aiq_free(pSharpCtx);
    return result;
}

static XCamReturn
prepare(RkAiqAlgoCom* params)
{
    XCamReturn result = XCAM_RETURN_NO_ERROR;
    SharpContext_t* pSharpCtx = (SharpContext_t *)params->ctx;

	if(!!(params->u.prepare.conf_type & RK_AIQ_ALGO_CONFTYPE_UPDATECALIB )){
        // just update calib ptr
        if (params->u.prepare.conf_type & RK_AIQ_ALGO_CONFTYPE_UPDATECALIB_PTR) {
            pSharpCtx->sharp_attrib =
                (sharp_api_attrib_t*)(CALIBDBV2_GET_MODULE_PTR(params->u.prepare.calibv2, sharp));
            return XCAM_RETURN_NO_ERROR;
        }
    }

    pSharpCtx->working_mode = params->u.prepare.working_mode;
    pSharpCtx->sharp_attrib =
        (sharp_api_attrib_t*)(CALIBDBV2_GET_MODULE_PTR(params->u.prepare.calibv2, sharp));
    pSharpCtx->isReCal_ = true;

    return result;
}

XCamReturn Asharp_processing(const RkAiqAlgoCom* inparams, RkAiqAlgoResCom* outparams, int iso)
{

    SharpContext_t* pCtx = (SharpContext_t *)inparams->ctx;
    sharp_api_attrib_t* pAttrib = pCtx->sharp_attrib;
    sharp_param_t* proc_res = outparams->algoRes;

    if (pAttrib->opMode != RK_AIQ_OP_MODE_AUTO) {
        LOGE_ANR("mode is %d, not auto mode, ignore", pAttrib->opMode);
        return XCAM_RETURN_NO_ERROR;
    }
    if (pAttrib->en == false || pAttrib->bypass == true) {
        if (inparams->u.proc.is_attrib_update) {
            outparams->cfg_update = true;
            outparams->en = pAttrib->en;
            outparams->bypass = pAttrib->bypass;
        }
        LOGD_ANR("SHARP not enable, skip!");
        return XCAM_RETURN_NO_ERROR;
    }

    bool need_recal = pCtx->isReCal_;
    bool init = inparams->u.proc.init;
    if (inparams->u.proc.is_attrib_update || inparams->u.proc.init) {
        need_recal = true;
    }

    int delta_iso = abs(iso - pCtx->pre_iso);
    if(delta_iso > 0.01 || init) {
        pCtx->pre_iso = iso;
        need_recal = true;
    }

    outparams->cfg_update = false;
    if (need_recal) {
        proc_res->sta = pCtx->sharp_attrib->stAuto.sta;
#if RKAIQ_HAVE_SHARP_V34
        SharpSelectParam(pCtx, proc_res, iso);
        SharpApplyStrength(pCtx, proc_res);
#endif
        outparams->cfg_update = true;
        outparams->en = pAttrib->en;
        outparams->bypass = pAttrib->bypass;
        LOGD_ANR("SHARP processing: iso %d, en:%d, bypass:%d", iso, outparams->en, outparams->bypass);

        pCtx->isReCal_ = false;
    }

    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn
processing(const RkAiqAlgoCom* inparams, RkAiqAlgoResCom* outparams)
{
    int iso = inparams->u.proc.iso;
    Asharp_processing(inparams, outparams, iso);

    LOGV_ASHARP("%s: Sharp (exit)\n", __FUNCTION__ );
    return XCAM_RETURN_NO_ERROR;
}

#if 0
XCamReturn
algo_sharp_SetAttrib(RkAiqAlgoContext *ctx, const sharp_api_attrib_t *attr)
{

    if(ctx == NULL || attr == NULL) {
        LOGE_ASHARP("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return XCAM_RETURN_ERROR_PARAM;
    }

    SharpContext_t* pSharpCtx = (SharpContext_t*)ctx;
    pSharpCtx->isReCal_ = true;

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
algo_sharp_GetAttrib(const RkAiqAlgoContext *ctx, sharp_api_attrib_t *attr)
{

    if(ctx == NULL || attr == NULL) {
        LOGE_ASHARP("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return XCAM_RETURN_ERROR_PARAM;
    }

    SharpContext_t* pSharpCtx = (SharpContext_t*)ctx;
    sharp_api_attrib_t* sharp_attrib = pSharpCtx->sharp_attrib;

    attr->opMode = sharp_attrib->opMode;
    attr->en = sharp_attrib->en;
    attr->bypass = sharp_attrib->bypass;
    memcpy(&attr->stAuto, &sharp_attrib->stAuto, sizeof(sharp_param_auto_t));

    return XCAM_RETURN_NO_ERROR;
}
#endif
XCamReturn
algo_sharp_SetStrength(RkAiqAlgoContext *ctx, float strg, bool strg_en)
{

    if(ctx == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return XCAM_RETURN_ERROR_PARAM;
    }

    SharpContext_t* pSharpCtx = (SharpContext_t*)ctx;

    pSharpCtx->fStrength = strg;
    pSharpCtx->strength_en = strg_en;
    pSharpCtx->isReCal_ = true;

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
algo_sharp_GetStrength(RkAiqAlgoContext *ctx, float *strg, bool *strg_en)
{

    if(ctx == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return XCAM_RETURN_ERROR_PARAM;
    }

    SharpContext_t* pSharpCtx = (SharpContext_t*)ctx;

    *strg = pSharpCtx->fStrength;
    *strg_en = pSharpCtx->strength_en;

    return XCAM_RETURN_NO_ERROR;
}

#define RKISP_ALGO_SHARP_VERSION     "v0.0.9"
#define RKISP_ALGO_SHARP_VENDOR      "Rockchip"
#define RKISP_ALGO_SHARP_DESCRIPTION "Rockchip sharp algo for ISP32"

RkAiqAlgoDescription g_RkIspAlgoDescSharp = {
    .common = {
        .version = RKISP_ALGO_SHARP_VERSION,
        .vendor  = RKISP_ALGO_SHARP_VENDOR,
        .description = RKISP_ALGO_SHARP_DESCRIPTION,
        .type    = RK_AIQ_ALGO_TYPE_ASHARP,
        .id      = 0,
        .create_context  = create_context,
        .destroy_context = destroy_context,
    },
    .prepare = prepare,
    .pre_process = NULL,
    .processing = processing,
    .post_process = NULL,
};

