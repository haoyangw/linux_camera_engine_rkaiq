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

#include "hwi_c/aiq_ispParamsCvt.h"

#include "c_base/aiq_base.h"
#if defined(ISP_HW_V39)
#include "hwi_c/isp39/aiq_isp39ParamsCvt.h"
#elif defined(ISP_HW_V33)
#include "hwi_c/isp33/aiq_isp33ParamsCvt.h"
#endif

void AiqIspParamsCvt_setCamPhyId(AiqIspParamsCvt_t* pCvt, int phyId) { pCvt->_CamPhyId = phyId; }

aiq_params_base_t* AiqIspParamsCvt_get_3a_result(AiqIspParamsCvt_t* pCvt, AiqList_t* results,
                                                 int32_t type) {
    AiqListItem_t* pItem = NULL;
    bool rm              = false;
    AIQ_LIST_FOREACH(results, pItem, rm) {
        aiq_params_base_t* params = *(aiq_params_base_t**)(pItem->_pData);
        if (type == params->type) {
            return params;
        }
    }

    return NULL;
}

void AiqIspParamsCvt_getCommonCvtInfo(AiqIspParamsCvt_t* pCvt, AiqList_t* results, bool use_aiisp) {
    pCvt->mCommonCvtInfo.isGrayMode   = false;
    pCvt->mCommonCvtInfo.isFirstFrame = false;
    pCvt->mCommonCvtInfo.frameNum     = 1;
    pCvt->mCommonCvtInfo.ae_exp = NULL;
    pCvt->mCommonCvtInfo.use_aiisp    = use_aiisp;

    aiq_params_base_t* params = NULL;
    AiqListItem_t* pItem      = aiqList_get_item(results, NULL);
    if (!pItem) {
        LOGE("Fatal error !");
        return;
    }
    params           = *(aiq_params_base_t**)(pItem->_pData);

    uint32_t frameId             = params->frame_id;
    pCvt->mCommonCvtInfo.frameId = frameId;
    if (frameId == 0) {
		pCvt->mCommonCvtInfo.isFirstFrame = true;
        pCvt->mCommonCvtInfo.preDGain = 1.0;
        pCvt->mCommonCvtInfo.L2S_Ratio = 1.0;
#if RKAIQ_HAVE_DEHAZE_V14
        for (int i = 0; i < YNR_ISO_CURVE_POINT_NUM; i++)
            pCvt->mCommonCvtInfo.ynr_sigma[i] = 0.0f;
#endif
        pCvt->mCommonCvtInfo.ynr_count = 0;
        pCvt->mCommonCvtInfo.sharp_count = 0;
        pCvt->mCommonCvtInfo.cmps_on = false;
    }
	// NOTICE: from _expParamsPool of AiqSensorHw_t, type is AiqSensorExpInfo_t*
	// should be different from AiqAecExpInfoWrapper_t
    aiq_params_base_t* aeResult =
        AiqIspParamsCvt_get_3a_result(pCvt, results, RESULT_TYPE_EXPOSURE_PARAM);
    if (aeResult) {
        AiqSensorExpInfo_t* exp   = (AiqSensorExpInfo_t*)(aeResult);
        RKAiqAecExpInfo_t* ae_exp = &exp->aecExpInfo;
        pCvt->mCommonCvtInfo.ae_exp = ae_exp;

        if (pCvt->_working_mode == RK_AIQ_WORKING_MODE_NORMAL) {
            float isp_dgain = MAX(1.0f, ae_exp->LinearExp.exp_real_params.isp_dgain);
            float exptime   = MAX(1.0f, ae_exp->LinearExp.exp_real_params.integration_time);
            int iso         = 50 * exp->aecExpInfo.LinearExp.exp_real_params.analog_gain *
                      exp->aecExpInfo.LinearExp.exp_real_params.digital_gain *
                      exp->aecExpInfo.LinearExp.exp_real_params.isp_dgain;

            pCvt->mCommonCvtInfo.frameIso[0]   = iso;
            pCvt->mCommonCvtInfo.frameEt[0]    = exptime;
            pCvt->mCommonCvtInfo.frameDGain[0] = isp_dgain;
        } else {
            for (int i = 0;i < 2;i++) {
                int iso = 50 *
                ae_exp->HdrExp[i].exp_real_params.analog_gain *
                ae_exp->HdrExp[i].exp_real_params.digital_gain *
                ae_exp->HdrExp[i].exp_real_params.isp_dgain;

                pCvt->mCommonCvtInfo.frameIso[i] = iso;
                pCvt->mCommonCvtInfo.frameEt[i] = MAX(1.0f, ae_exp->HdrExp[i].exp_real_params.integration_time);
                pCvt->mCommonCvtInfo.frameDGain[i] = MAX(1.0f, ae_exp->HdrExp[i].exp_real_params.isp_dgain);
            }
        }
    }

    if (pCvt->_working_mode == RK_AIQ_WORKING_MODE_NORMAL) {
        pCvt->mCommonCvtInfo.frameNum = 1;
    } else if (pCvt->_working_mode == RK_AIQ_ISP_HDR_MODE_2_FRAME_HDR ||
               pCvt->_working_mode == RK_AIQ_ISP_HDR_MODE_2_LINE_HDR) {
        pCvt->mCommonCvtInfo.frameNum = 2;
    } else if (pCvt->_working_mode == RK_AIQ_ISP_HDR_MODE_3_FRAME_HDR ||
               pCvt->_working_mode == RK_AIQ_ISP_HDR_MODE_3_LINE_HDR) {
        pCvt->mCommonCvtInfo.frameNum = 3;
    }

    if (pCvt->mBlcResult) {
#if RKAIQ_HAVE_BLC_V32 && USE_NEWSTRUCT
        blc_param_t *blc_v32    = (blc_param_t*)pCvt->mBlcResult->_data;
        pCvt->mCommonCvtInfo.blc_res.en        = pCvt->mBlcResult->en;
        pCvt->mCommonCvtInfo.blc_res.obcPreTnr = blc_v32->dyn.obcPreTnr;
        pCvt->mCommonCvtInfo.blc_res.obcPostTnr = blc_v32->dyn.obcPostTnr;
#endif
    }

    pCvt->mCommonCvtInfo.cnr_ynr_sharp_same = 0;
    aiq_params_base_t* ynrResult =
        AiqIspParamsCvt_get_3a_result(pCvt, results, RESULT_TYPE_YNR_PARAM);
    aiq_params_base_t* cnResult =
        AiqIspParamsCvt_get_3a_result(pCvt, results, RESULT_TYPE_UVNR_PARAM);
    aiq_params_base_t* sharpResult =
        AiqIspParamsCvt_get_3a_result(pCvt, results, RESULT_TYPE_SHARPEN_PARAM);
    if (ynrResult!= NULL && cnResult != NULL && sharpResult != NULL) {
        if (ynrResult->en == cnResult->en && ynrResult->en == sharpResult->en) {
            pCvt->mCommonCvtInfo.cnr_ynr_sharp_same = 1;
        }
        else {
            pCvt->mCommonCvtInfo.cnr_ynr_sharp_same = 0;
        }
    }
    LOGD_CAMHW_SUBM(ISP20PARAM_SUBM, "%s: cnr_ynr_sharp_same = %d", __func__, 
                    pCvt->mCommonCvtInfo.cnr_ynr_sharp_same);
#if RKAIQ_HAVE_DEHAZE_V14
    if (ynrResult != NULL) {
        ynr_param_t* ynr_param = (ynr_param_t*)ynrResult->_data;
        for (int i = 0; i < YNR_ISO_CURVE_POINT_NUM; i++)
            pCvt->mCommonCvtInfo.ynr_sigma[i] = ynr_param->dyn.hw_ynrC_luma2Sigma_curve.val[i];
    }
#endif
}

XCamReturn AiqIspParamsCvt_merge_isp_results(AiqIspParamsCvt_t* pCvt, AiqList_t* results,
                                             void* isp_cfg, bool is_multi_isp, bool use_aiisp) {
    if (!results) return XCAM_RETURN_ERROR_PARAM;
#if defined(ISP_HW_V39)
    pCvt->isp_params.isp_cfg = (struct isp39_isp_params_cfg*)isp_cfg;
#elif defined(ISP_HW_V33)
    pCvt->isp_params.isp_cfg = (struct isp33_isp_params_cfg*)isp_cfg;
#endif
    pCvt->mBlcResult = AiqIspParamsCvt_get_3a_result(pCvt, results, RESULT_TYPE_BLC_PARAM);

#if USE_NEWSTRUCT
    AiqIspParamsCvt_getCommonCvtInfo(pCvt, results, use_aiisp);
#endif

    aiq_params_base_t* drc_params = AiqIspParamsCvt_get_3a_result(pCvt, results, RESULT_TYPE_DRC_PARAM);
	if (drc_params)
		pCvt->mIspParamsCvtOps.Convert3aResultsToIspCfg(pCvt, drc_params, isp_cfg, is_multi_isp);

    LOGD_CAMHW_SUBM(ISP20PARAM_SUBM, "%s, isp cam3a results size: %d\n", __FUNCTION__,
                    aiqList_size(results));
    AiqListItem_t* pItem = NULL;
    bool rm              = false;
    AIQ_LIST_FOREACH(results, pItem, rm) {
        aiq_params_base_t* params = *(aiq_params_base_t**)(pItem->_pData);
        if (params->type != RESULT_TYPE_DRC_PARAM)
			pCvt->mIspParamsCvtOps.Convert3aResultsToIspCfg(pCvt, params, isp_cfg, is_multi_isp);
        pItem = aiqList_erase_item_locked(results, pItem);
        rm    = true;
        AIQ_REF_BASE_UNREF(&params->_ref_base);
    }
    /* aiqList_deinit(results); */
    return XCAM_RETURN_NO_ERROR;
}

inline void AiqIspParamsCvt_set_working_mode(AiqIspParamsCvt_t* pCvt, int mode) {
    if (!pCvt) return;

    pCvt->_working_mode = mode;
}

XCamReturn AiqIspParamsCvt_init(AiqIspParamsCvt_t* pCvt) {
    if (!pCvt) return XCAM_RETURN_ERROR_PARAM;

    pCvt->_force_isp_module_ens = 0;
    pCvt->_force_module_flags   = 0;
    pCvt->_CamPhyId             = -1;
    pCvt->_lsc_en               = false;
    pCvt->_working_mode         = RK_AIQ_WORKING_MODE_ISP_HDR3;
#if defined(ISP_HW_V39)
    pCvt->mIspParamsCvtOps.Convert3aResultsToIspCfg = Convert3aResultsToIsp39Cfg;
#elif defined(ISP_HW_V33)
    pCvt->mIspParamsCvtOps.Convert3aResultsToIspCfg = Convert3aResultsToIsp33Cfg;
#endif

    aiq_memset(&pCvt->AntiTmoFlicker, 0, sizeof(pCvt->AntiTmoFlicker));

    return XCAM_RETURN_NO_ERROR;
}

void AiqIspParamsCvt_deinit(AiqIspParamsCvt_t* pCvt) {
}
