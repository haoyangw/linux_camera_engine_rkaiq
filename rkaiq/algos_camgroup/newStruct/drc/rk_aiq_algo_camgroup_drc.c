/*
 * rk_aiq_algo_camgroup_drc.c
 *
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

#include "rk_aiq_algo_camgroup_types.h"
#include "newStruct/drc/drc_types_prvt.h"
#include "c_base/aiq_base.h"
#include "newStruct/rk_aiq_algo_camgroup_common.h"

#define DEFAULT_RECALCULATE_DELTA_ISO (0.01)

typedef DrcContext_t DrcGroupContext_t;

static XCamReturn groupDrcProcessing(const RkAiqAlgoCom* inparams, RkAiqAlgoResCom* outparams)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    LOGD_ATMO("%s enter", __FUNCTION__ );

    DrcGroupContext_t* pDrcGroupCtx = (DrcGroupContext_t *)inparams->ctx;
    RkAiqAlgoCamGroupProcIn* procParaGroup = (RkAiqAlgoCamGroupProcIn*)inparams;
    pDrcGroupCtx->FrameID = inparams->frame_id;
    RkAiqAlgoCamGroupProcOut* procResParaGroup = (RkAiqAlgoCamGroupProcOut*)outparams;
    pDrcGroupCtx->blc_ob_enable = procParaGroup->stAblcV32_proc_res.blc_ob_enable;
    pDrcGroupCtx->isp_ob_predgain = procParaGroup->stAblcV32_proc_res.isp_ob_predgain;
    drc_api_attrib_t* drc_attrib = pDrcGroupCtx->drc_attrib;
	// TODO
	//pDrcGroupCtx->compr_bit = procParaGroup->camgroupParmasArray[0]->aec.compr_bit;
    //group empty
    if(procParaGroup == NULL || procParaGroup->camgroupParmasArray == NULL) {
        LOGE_ATMO("procParaGroup or camgroupParmasArray is null");
        return(XCAM_RETURN_ERROR_FAILED);
    }

    // skip group algo if in MANUAL mode
    if (pDrcGroupCtx->drc_attrib->opMode != RK_AIQ_OP_MODE_AUTO) {
        return XCAM_RETURN_NO_ERROR;
    }

    if (pDrcGroupCtx->isCapture) {
        LOGD_ATMO("%s: It's capturing, using pre frame params\n", __func__);
        pDrcGroupCtx->isCapture = false;
        return XCAM_RETURN_NO_ERROR;
    }

    if (procParaGroup->working_mode < RK_AIQ_WORKING_MODE_ISP_HDR2)
        pDrcGroupCtx->FrameNumber = LINEAR_NUM;
    else if (procParaGroup->working_mode < RK_AIQ_WORKING_MODE_ISP_HDR3 &&
             procParaGroup->working_mode >= RK_AIQ_WORKING_MODE_ISP_HDR2)
        pDrcGroupCtx->FrameNumber = HDR_2X_NUM;
    else
        pDrcGroupCtx->FrameNumber = HDR_3X_NUM;

    if (pDrcGroupCtx->FrameNumber == HDR_2X_NUM || pDrcGroupCtx->FrameNumber == HDR_3X_NUM ||
        pDrcGroupCtx->FrameNumber == SENSOR_MGE)
        outparams->en = true;
    else if (pDrcGroupCtx->FrameNumber == LINEAR_NUM) {
        if (pDrcGroupCtx->blc_ob_enable)
            outparams->en = true;
        else {
            if (drc_attrib->opMode == RK_AIQ_OP_MODE_AUTO) {
                outparams->en = drc_attrib->en;
            } else {
                outparams->en = false;
            }
        }
    }

    if (outparams->en) {
        RkAiqAlgoProcResAeShared_t* pAEProcRes = &procParaGroup->camgroupParmasArray[0]->aec._aeProcRes;
        pDrcGroupCtx->NextData.AEData.LongFrmMode = pAEProcRes->LongFrmMode;
    }

    if(pDrcGroupCtx->FrameNumber == LINEAR_NUM) {
        pDrcGroupCtx->NextData.AEData.SExpo =
            procParaGroup->camgroupParmasArray[0]->aec._effAecExpInfo.LinearExp.exp_real_params.analog_gain *
            procParaGroup->camgroupParmasArray[0]->aec._effAecExpInfo.LinearExp.exp_real_params.digital_gain *
            procParaGroup->camgroupParmasArray[0]->aec._effAecExpInfo.LinearExp.exp_real_params.isp_dgain *
            procParaGroup->camgroupParmasArray[0]->aec._effAecExpInfo.LinearExp.exp_real_params.integration_time;
        pDrcGroupCtx->NextData.AEData.MExpo = pDrcGroupCtx->NextData.AEData.SExpo;
        pDrcGroupCtx->NextData.AEData.LExpo = pDrcGroupCtx->NextData.AEData.SExpo;
    }
    else if(pDrcGroupCtx->FrameNumber == HDR_2X_NUM) {
        pDrcGroupCtx->NextData.AEData.SExpo =
            procParaGroup->camgroupParmasArray[0]->aec._effAecExpInfo.HdrExp[0].exp_real_params.analog_gain *
            procParaGroup->camgroupParmasArray[0]->aec._effAecExpInfo.HdrExp[0].exp_real_params.digital_gain *
            procParaGroup->camgroupParmasArray[0]->aec._effAecExpInfo.HdrExp[0].exp_real_params.isp_dgain *
            procParaGroup->camgroupParmasArray[0]->aec._effAecExpInfo.HdrExp[0].exp_real_params.integration_time;
        pDrcGroupCtx->NextData.AEData.MExpo =
            procParaGroup->camgroupParmasArray[0]->aec._effAecExpInfo.HdrExp[1].exp_real_params.analog_gain *
            procParaGroup->camgroupParmasArray[0]->aec._effAecExpInfo.HdrExp[1].exp_real_params.digital_gain *
            procParaGroup->camgroupParmasArray[0]->aec._effAecExpInfo.HdrExp[1].exp_real_params.isp_dgain *
            procParaGroup->camgroupParmasArray[0]->aec._effAecExpInfo.HdrExp[1].exp_real_params.integration_time;
        pDrcGroupCtx->NextData.AEData.LExpo = pDrcGroupCtx->NextData.AEData.MExpo;
    }
    else if(pDrcGroupCtx->FrameNumber == HDR_3X_NUM) {
        pDrcGroupCtx->NextData.AEData.SExpo =
            procParaGroup->camgroupParmasArray[0]->aec._effAecExpInfo.HdrExp[0].exp_real_params.analog_gain *
            procParaGroup->camgroupParmasArray[0]->aec._effAecExpInfo.HdrExp[0].exp_real_params.digital_gain *
            procParaGroup->camgroupParmasArray[0]->aec._effAecExpInfo.HdrExp[0].exp_real_params.isp_dgain *
            procParaGroup->camgroupParmasArray[0]->aec._effAecExpInfo.HdrExp[0].exp_real_params.integration_time;
        pDrcGroupCtx->NextData.AEData.MExpo =
            procParaGroup->camgroupParmasArray[0]->aec._effAecExpInfo.HdrExp[1].exp_real_params.analog_gain *
            procParaGroup->camgroupParmasArray[0]->aec._effAecExpInfo.HdrExp[1].exp_real_params.digital_gain *
            procParaGroup->camgroupParmasArray[0]->aec._effAecExpInfo.HdrExp[1].exp_real_params.isp_dgain *
            procParaGroup->camgroupParmasArray[0]->aec._effAecExpInfo.HdrExp[1].exp_real_params.integration_time;
        pDrcGroupCtx->NextData.AEData.LExpo =
            procParaGroup->camgroupParmasArray[0]->aec._effAecExpInfo.HdrExp[2].exp_real_params.analog_gain *
            procParaGroup->camgroupParmasArray[0]->aec._effAecExpInfo.HdrExp[2].exp_real_params.digital_gain *
            procParaGroup->camgroupParmasArray[0]->aec._effAecExpInfo.HdrExp[2].exp_real_params.isp_dgain *
            procParaGroup->camgroupParmasArray[0]->aec._effAecExpInfo.HdrExp[2].exp_real_params.integration_time;
    } else if (pDrcGroupCtx->FrameNumber == SENSOR_MGE) {
        pDrcGroupCtx->NextData.AEData.MExpo =
            procParaGroup->camgroupParmasArray[0]->aec._effAecExpInfo.LinearExp.exp_real_params.analog_gain *
            procParaGroup->camgroupParmasArray[0]->aec._effAecExpInfo.LinearExp.exp_real_params.digital_gain *
            procParaGroup->camgroupParmasArray[0]->aec._effAecExpInfo.LinearExp.exp_real_params.isp_dgain *
            procParaGroup->camgroupParmasArray[0]->aec._effAecExpInfo.LinearExp.exp_real_params.integration_time;
        pDrcGroupCtx->NextData.AEData.LExpo = pDrcGroupCtx->NextData.AEData.MExpo;
        pDrcGroupCtx->NextData.AEData.SExpo =
            pDrcGroupCtx->NextData.AEData.MExpo /
            pow(2.0f, (float)(pDrcGroupCtx->compr_bit - ISP_HDR_BIT_NUM_MIN));
    }
    if (pDrcGroupCtx->NextData.AEData.SExpo > FLT_EPSILON) {
        pDrcGroupCtx->NextData.AEData.L2S_Ratio =
            pDrcGroupCtx->NextData.AEData.LExpo / pDrcGroupCtx->NextData.AEData.SExpo;
        if (pDrcGroupCtx->NextData.AEData.L2S_Ratio < RATIO_DEFAULT) {
            LOGE_ATMO("%s: Next L2S_Ratio:%f is less than 1.0x, clip to 1.0x!!!\n",
                        __FUNCTION__, pDrcGroupCtx->NextData.AEData.L2S_Ratio);
            pDrcGroupCtx->NextData.AEData.L2S_Ratio = RATIO_DEFAULT;
        }
        pDrcGroupCtx->NextData.AEData.M2S_Ratio =
            pDrcGroupCtx->NextData.AEData.MExpo / pDrcGroupCtx->NextData.AEData.SExpo;
        if (pDrcGroupCtx->NextData.AEData.M2S_Ratio < RATIO_DEFAULT) {
            LOGE_ATMO("%s: Next M2S_Ratio:%f is less than 1.0x, clip to 1.0x!!!\n",
                        __FUNCTION__, pDrcGroupCtx->NextData.AEData.M2S_Ratio);
            pDrcGroupCtx->NextData.AEData.M2S_Ratio = RATIO_DEFAULT;
        }
    }
    else
        LOGE_ATMO("%s: Next Short frame for drc expo sync is %f!!!\n", __FUNCTION__,
                    pDrcGroupCtx->NextData.AEData.SExpo);
    if (pDrcGroupCtx->NextData.AEData.MExpo > FLT_EPSILON) {
        pDrcGroupCtx->NextData.AEData.L2M_Ratio =
            pDrcGroupCtx->NextData.AEData.LExpo / pDrcGroupCtx->NextData.AEData.MExpo;
        if (pDrcGroupCtx->NextData.AEData.L2M_Ratio < RATIO_DEFAULT) {
            LOGE_ATMO("%s: Next L2M_Ratio:%f is less than 1.0x, clip to 1.0x!!!\n",
                        __FUNCTION__, pDrcGroupCtx->NextData.AEData.L2M_Ratio);
            pDrcGroupCtx->NextData.AEData.L2M_Ratio = RATIO_DEFAULT;
        }
    } else
        LOGE_ATMO("%s: Next Midlle frame for drc expo sync is %f!!!\n", __FUNCTION__,
                    pDrcGroupCtx->NextData.AEData.MExpo);
    //clip for long frame mode
    if (pDrcGroupCtx->NextData.AEData.LongFrmMode) {
        pDrcGroupCtx->NextData.AEData.L2S_Ratio = LONG_FRAME_MODE_RATIO;
        pDrcGroupCtx->NextData.AEData.M2S_Ratio = LONG_FRAME_MODE_RATIO;
        pDrcGroupCtx->NextData.AEData.L2M_Ratio = LONG_FRAME_MODE_RATIO;
    }
    // clip L2M_ratio to 32x
    if (pDrcGroupCtx->NextData.AEData.L2M_Ratio > AE_RATIO_L2M_MAX) {
        LOGE_ATMO("%s: Next L2M_ratio:%f out of range, clip to 32.0x!!!\n", __FUNCTION__,
                    pDrcGroupCtx->NextData.AEData.L2M_Ratio);
        pDrcGroupCtx->NextData.AEData.L2M_Ratio = AE_RATIO_L2M_MAX;
    }
    // clip L2S_ratio
    if (pDrcGroupCtx->NextData.AEData.L2S_Ratio > AE_RATIO_MAX) {
        LOGE_ATMO("%s: Next L2S_Ratio:%f out of range, clip to 256.0x!!!\n", __FUNCTION__,
                    pDrcGroupCtx->NextData.AEData.L2S_Ratio);
        pDrcGroupCtx->NextData.AEData.L2S_Ratio = AE_RATIO_MAX;
    }
    // clip L2M_ratio and M2S_Ratio
    if (pDrcGroupCtx->NextData.AEData.L2M_Ratio * pDrcGroupCtx->NextData.AEData.M2S_Ratio >
        AE_RATIO_MAX) {
        LOGE_ATMO("%s: Next L2M_Ratio*M2S_Ratio:%f out of range, clip to 256.0x!!!\n",
                    __FUNCTION__,
                    pDrcGroupCtx->NextData.AEData.L2M_Ratio * pDrcGroupCtx->NextData.AEData.M2S_Ratio);
        pDrcGroupCtx->NextData.AEData.M2S_Ratio =
            AE_RATIO_MAX / pDrcGroupCtx->NextData.AEData.L2M_Ratio;
    }

    int iso = pDrcGroupCtx->iso;
    float blc_ob_predgain = procParaGroup->stAblcV32_proc_res.isp_ob_predgain;
    rk_aiq_singlecam_3a_result_t* scam_3a_res = procParaGroup->camgroupParmasArray[0];
    if(scam_3a_res->aec._bEffAecExpValid) {
        RKAiqAecExpInfo_t* pCurExp = &scam_3a_res->aec._effAecExpInfo;
        if((rk_aiq_working_mode_t)procParaGroup->working_mode == RK_AIQ_WORKING_MODE_NORMAL) {
            iso = blc_ob_predgain * scam_3a_res->hdrIso;
        } else {
            iso = scam_3a_res->hdrIso;
        }
    }

    if (procParaGroup->attribUpdated) {
        LOGI("%s attribUpdated", __func__);
        pDrcGroupCtx->isReCal_ = true;
    }

    int delta_iso = abs(iso - pDrcGroupCtx->iso);
    if (delta_iso > DEFAULT_RECALCULATE_DELTA_ISO)
        pDrcGroupCtx->isReCal_ = true;

    bool bypass_expo_params = true;
    // get bypass_expo_params
    if(!pDrcGroupCtx->CurrData.AEData.LongFrmMode !=
                !pDrcGroupCtx->NextData.AEData.LongFrmMode)
        bypass_expo_params = false;
    else if ((pDrcGroupCtx->CurrData.AEData.L2M_Ratio - pDrcGroupCtx->NextData.AEData.L2M_Ratio) >
                    FLT_EPSILON ||
                (pDrcGroupCtx->CurrData.AEData.L2M_Ratio - pDrcGroupCtx->NextData.AEData.L2M_Ratio) <
                    -FLT_EPSILON ||
                (pDrcGroupCtx->CurrData.AEData.M2S_Ratio - pDrcGroupCtx->NextData.AEData.M2S_Ratio) >
                    FLT_EPSILON ||
                (pDrcGroupCtx->CurrData.AEData.M2S_Ratio - pDrcGroupCtx->NextData.AEData.M2S_Ratio) <
                    -FLT_EPSILON ||
                (pDrcGroupCtx->CurrData.AEData.L2S_Ratio - pDrcGroupCtx->NextData.AEData.L2S_Ratio) >
                    FLT_EPSILON ||
                (pDrcGroupCtx->CurrData.AEData.L2S_Ratio - pDrcGroupCtx->NextData.AEData.L2S_Ratio) <
                    -FLT_EPSILON)
        bypass_expo_params = false;
    else
        bypass_expo_params = true;

    rk_aiq_isp_drc_v39_t* drc_param = procResParaGroup->camgroupParmasArray[0]->drc;

    if (pDrcGroupCtx->isReCal_ || !bypass_expo_params) {
#if RKAIQ_HAVE_DRC_V12
        DrcSelectParam(pDrcGroupCtx, &drc_param->result, iso);
        DrcExpoParaProcessing(pDrcGroupCtx, &drc_param->result);
#endif
#if RKAIQ_HAVE_DRC_V20
        drc_param->L2S_Ratio = pDrcGroupCtx->NextData.AEData.L2S_Ratio;
        drc_param->compr_bit = pDrcGroupCtx->compr_bit;
        DrcSelectParam(pDrcGroupCtx, &drc_param->drc_param, iso);
#endif
		outparams->cfg_update = true;
	} else {
		outparams->cfg_update = false;
	}

	void* gp_ptrs[procResParaGroup->arraySize];
	int gp_size = sizeof(*procResParaGroup->camgroupParmasArray[0]->drc);
	for (int i = 0; i < procResParaGroup->arraySize; i++)
		gp_ptrs[i] = procResParaGroup->camgroupParmasArray[i]->drc;

	algo_camgroup_update_results(inparams, outparams, gp_ptrs, gp_size);

    pDrcGroupCtx->iso = iso;
    pDrcGroupCtx->isReCal_ = false;
    pDrcGroupCtx->CurrData.AEData.LongFrmMode = pDrcGroupCtx->NextData.AEData.LongFrmMode;
    pDrcGroupCtx->CurrData.AEData.L2M_Ratio   = pDrcGroupCtx->NextData.AEData.L2M_Ratio;
    pDrcGroupCtx->CurrData.AEData.M2S_Ratio   = pDrcGroupCtx->NextData.AEData.M2S_Ratio;
    pDrcGroupCtx->CurrData.AEData.L2S_Ratio   = pDrcGroupCtx->NextData.AEData.L2S_Ratio;
    LOGD_ATMO("%s exit\n", __FUNCTION__);
    return ret;
}

#define RKISP_ALGO_CAMGROUP_DRC_VERSION     "v0.0.1"
#define RKISP_ALGO_CAMGROUP_DRC_VENDOR      "Rockchip"
#define RKISP_ALGO_CAMGROUP_DRC_DESCRIPTION "Rockchip Drc camgroup algo for ISP2.0"

RkAiqAlgoDescription g_RkIspAlgoDescCamgroupDrc = {
    .common = {
        .version = RKISP_ALGO_CAMGROUP_DRC_VERSION,
        .vendor  = RKISP_ALGO_CAMGROUP_DRC_VENDOR,
        .description = RKISP_ALGO_CAMGROUP_DRC_DESCRIPTION,
        .type    = RK_AIQ_ALGO_TYPE_ADRC,
        .id      = 0,
        .create_context  = algo_camgroup_CreateCtx,
        .destroy_context = algo_camgroup_DestroyCtx,
    },
    .prepare = algo_camgroup_Prepare,
    .pre_process = NULL,
    .processing = groupDrcProcessing,
    .post_process = NULL,
};
