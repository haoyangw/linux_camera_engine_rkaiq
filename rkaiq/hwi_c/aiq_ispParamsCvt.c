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

#if RKAIQ_HAVE_DUMPSYS
#include "dumpcam_server/info/include/rk_info_utils.h"
#include "dumpcam_server/info/include/st_string.h"
#include "hwi_c/info/aiq_ispParamsCvtInfo.h"
#endif

void AiqIspParamsCvt_setCamPhyId(AiqIspParamsCvt_t* pCvt, int phyId) {
    pCvt->_CamPhyId = phyId;
}

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

static void AiqIspParamsCvt_checkModuleEnable(AiqIspParamsCvt_t* pCvt, AiqList_t* results) {
    aiq_params_base_t* ynrResult =
        AiqIspParamsCvt_get_3a_result(pCvt, results, RESULT_TYPE_YNR_PARAM);
    aiq_params_base_t* cnrResult =
        AiqIspParamsCvt_get_3a_result(pCvt, results, RESULT_TYPE_UVNR_PARAM);
    aiq_params_base_t* sharpResult =
        AiqIspParamsCvt_get_3a_result(pCvt, results, RESULT_TYPE_SHARPEN_PARAM);
#if ISP_HW_V33
    aiq_params_base_t* enhResult =
        AiqIspParamsCvt_get_3a_result(pCvt, results, RESULT_TYPE_ENH_PARAM);
#endif
    bool update_vaild = false;
#if ISP_HW_V39
    if (ynrResult!= NULL && cnrResult != NULL && sharpResult != NULL) {
        if (ynrResult->en == cnrResult->en && ynrResult->en == sharpResult->en) {
            pCvt->mCommonCvtInfo.cnr_path_en = ynrResult->en;
            update_vaild = true;
        }
        else {
            pCvt->mCommonCvtInfo.cnr_path_en = true;
        }
    }
#elif ISP_HW_V33
    if (ynrResult != NULL && cnrResult != NULL && sharpResult != NULL && enhResult != NULL) {
        if (ynrResult->en == cnrResult->en && ynrResult->en == sharpResult->en && sharpResult->en == enhResult->en) {
            pCvt->mCommonCvtInfo.cnr_path_en = ynrResult->en;
            update_vaild = true;
        }
        else {
            pCvt->mCommonCvtInfo.cnr_path_en = true;
        }
    }
#endif
    pCvt->mCommonCvtInfo.warning_signal = false;
#if ISP_HW_V39
    if (!update_vaild) {
        if (pCvt->mCommonCvtInfo.cnr_path_en) {
            if (ynrResult != NULL && !ynrResult->en) {
                ynrResult->en = true;
                ynrResult->bypass = true;
                pCvt->mCommonCvtInfo.warning_signal = true;
                LOGW_ANR("ynr must be disabled together with cnr and sharp. Force switch to bypass");
            }
            if (sharpResult != NULL && !sharpResult->en) {
                sharpResult->en = true;
                sharpResult->bypass = true;
                pCvt->mCommonCvtInfo.warning_signal = true;
                LOGW_ANR("sharp must be disabled together with cnr and ynr. Force switch to bypass");
            }
            if (cnrResult != NULL && !cnrResult->en) {
                cnrResult->en = true;
                LOGW_ANR("cnr must be disabled together with ynr and sharp. So you can set cnr.hiNr_bifilt.bifiltOut_alpha instead.");
            }
        }
        else {
            if (ynrResult != NULL && ynrResult->en) {
                ynrResult->en = false;
                pCvt->mCommonCvtInfo.warning_signal = true;
                LOGE_ANR("ynr must be enabled together with cnr and sharp.");
            }
            if (sharpResult != NULL && sharpResult->en) {
                sharpResult->en = false;
                pCvt->mCommonCvtInfo.warning_signal = true;
                LOGE_ANR("sharp must be enabled together with cnr and ynr.");
            }
            if (cnrResult != NULL && cnrResult->en) {
                cnrResult->en = false;
                LOGW_ANR("cnr must be enabled together with ynr and sharp.");
            }
        }
    }
    if (pCvt->mCommonCvtInfo.warning_signal) {
        pCvt->mCommonCvtInfo.warning_count++;
        if (pCvt->mCommonCvtInfo.warning_count > 0 && pCvt->mCommonCvtInfo.warning_count < 5)
            LOGW_ANR("ynr, cnr, and sharp cannot be enabled/disabled individually. they must be enabled together");
        else if (pCvt->mCommonCvtInfo.warning_count> 0 && pCvt->mCommonCvtInfo.warning_count % 300 == 0) {
            LOGW_ANR("ynr, cnr, and sharp cannot be enabled/disabled individually. they must be enabled together");
        }
    }
#elif ISP_HW_V33
    if (!update_vaild) {
        if (pCvt->mCommonCvtInfo.cnr_path_en) {
            if (ynrResult != NULL && !ynrResult->en) {
                ynrResult->en = true;
                ynrResult->bypass = true;
                pCvt->mCommonCvtInfo.warning_signal = true;
                LOGW_ANR("ynr must be disabled together with enh, cnr and sharp. Force switch to bypass");
            }
            if (sharpResult != NULL && !sharpResult->en) {
                sharpResult->en = true;
                sharpResult->bypass = true;
                pCvt->mCommonCvtInfo.warning_signal = true;
                LOGW_ANR("sharp must be disabled together with enh, cnr and ynr. Force switch to bypass");
            }
            if (enhResult != NULL && !enhResult->en) {
                enhResult->en = true;
                enhResult->bypass = true;
                pCvt->mCommonCvtInfo.warning_signal = true;
                LOGW_ANR("enh must be disabled together with cnr sharp and ynr. Force switch to bypass");
            }
            if (cnrResult != NULL && !cnrResult->en) {
                cnrResult->en = true;
                LOGW_ANR("cnr must be disabled together with enh, ynr and sharp. So you can set cnr.hiNr_bifilt.bifiltOut_alpha instead.");
            }
        }
        else {
            if (ynrResult != NULL && ynrResult->en) {
                ynrResult->en = false;
                pCvt->mCommonCvtInfo.warning_signal = true;
                LOGE_ANR("ynr must be enabled together with enh, cnr and sharp.");
            }
            if (sharpResult != NULL && sharpResult->en) {
                sharpResult->en = false;
                pCvt->mCommonCvtInfo.warning_signal = true;
                LOGE_ANR("sharp must be enabled together with enh, cnr and ynr.");
            }
            if (enhResult != NULL && enhResult->en) {
                enhResult->en = false;
                pCvt->mCommonCvtInfo.warning_signal = true;
                LOGE_ANR("enh must be enabled together with cnr sharp and ynr. Force switch to bypass");
            }
            if (cnrResult != NULL && cnrResult->en) {
                cnrResult->en = false;
                LOGW_ANR("cnr must be enabled together with enh, ynr and sharp.");
            }
        }
    }
    if (pCvt->mCommonCvtInfo.warning_signal) {
        pCvt->mCommonCvtInfo.warning_count++;
        if (pCvt->mCommonCvtInfo.warning_count > 0 && pCvt->mCommonCvtInfo.warning_count < 5)
            LOGW_ANR("enh, ynr, cnr, and sharp cannot be enabled/disabled individually. they must be enabled together");
        else if (pCvt->mCommonCvtInfo.warning_count> 0 && pCvt->mCommonCvtInfo.warning_count % 300 == 0) {
            LOGW_ANR("enh, ynr, cnr, and sharp cannot be enabled/disabled individually. they must be enabled together");
        }
    }
#endif
}

void AiqIspParamsCvt_getCommonCvtInfo(AiqIspParamsCvt_t* pCvt, AiqList_t* results, bool use_aiisp) {
    pCvt->mCommonCvtInfo.isGrayMode   = false;
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
    if (frameId == 0 && !pCvt->mCommonCvtInfo.isFirstFrame) {
        pCvt->mCommonCvtInfo.isFirstFrame = true;
    }
    else {
        pCvt->mCommonCvtInfo.isFirstFrame = false;
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
            for (int i = 0; i < 2; i++) {
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

    AiqIspParamsCvt_checkModuleEnable(pCvt, results);

#if RKAIQ_HAVE_DEHAZE_V14
    aiq_params_base_t* ynrResult =
        AiqIspParamsCvt_get_3a_result(pCvt, results, RESULT_TYPE_YNR_PARAM);
    if (ynrResult != NULL) {
        ynr_param_t* ynr_param = (ynr_param_t*)ynrResult->_data;
        for (int i = 0; i < YNR_ISO_CURVE_POINT_NUM; i++)
            pCvt->mCommonCvtInfo.ynr_sigma[i] = ynr_param->dyn.hw_ynrC_luma2Sigma_curve.val[i];
    }
#endif
#if RKAIQ_HAVE_YNR_V40
    aiq_params_base_t* ynrResult =
        AiqIspParamsCvt_get_3a_result(pCvt, results, RESULT_TYPE_YNR_PARAM);
    if (ynrResult != NULL) {
        ynr_param_t* ynr_param = (ynr_param_t*)ynrResult->_data;
        for (int i = 0; i < 17; i++)
            pCvt->mCommonCvtInfo.ynr_sigma[i] = ynr_param->dyn.sigmaEnv.hw_ynrC_luma2Sigma_curve.val[i];
    }
#endif

    aiq_params_base_t* tnrResult =
        AiqIspParamsCvt_get_3a_result(pCvt, results, RESULT_TYPE_TNR_PARAM);
    if (tnrResult != NULL) {
        btnr_param_t* btnr_param = (btnr_param_t*)tnrResult->_data;
        btnr_params_static_t* psta = &btnr_param->sta;
#if ISP_HW_V33
        btnr_other_dyn_t* pdyn = &btnr_param->spNrDyn;
        pCvt->mCommonCvtInfo.sw_btnrT_outFrmBase_mode = pdyn->sw_btnrT_outFrmBase_mode;
#endif
        pCvt->mCommonCvtInfo.btnrCfg_pixDomain_mode = psta->hw_btnrCfg_pixDomain_mode;
        pCvt->mCommonCvtInfo.btnr_en = tnrResult->en;
    }
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

#if RKAIQ_HAVE_DUMPSYS
    if (pCvt->mIspParamsCvtOps.update) {
#if defined(ISP_HW_V39)
        pCvt->mIspParamsCvtOps.update(isp_cfg, pCvt->mCvtedIsp39Prams);
#elif defined(ISP_HW_V33)
        pCvt->mIspParamsCvtOps.update(isp_cfg, pCvt->mCvtedIsp33Prams);
#endif
    }
#endif

    return XCAM_RETURN_NO_ERROR;
}

inline void AiqIspParamsCvt_set_working_mode(AiqIspParamsCvt_t* pCvt, int mode) {
    if (!pCvt) return;

    pCvt->_working_mode = mode;
}

void AiqIspParamsCvt_setCalib(AiqIspParamsCvt_t* pCvt, const CamCalibDbV2Context_t* calibv2) {
#if (defined(ISP_HW_V39) || defined(ISP_HW_V33)) && (USE_NEWSTRUCT)
    btnr_api_attrib_t * btnr_attrib = (btnr_api_attrib_t*)(CALIBDBV2_GET_MODULE_PTR((void *)calibv2, bayertnr));
    btnr_cvt_info_t *btnr_info = &pCvt->mBtnrInfo;
    pCvt->btnr_attrib = btnr_attrib;
#endif
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
#if RKAIQ_HAVE_DUMPSYS
    pCvt->mCvtedIsp39Prams =
        (struct isp39_isp_params_cfg*)aiq_mallocz(sizeof(*pCvt->mCvtedIsp39Prams));
    pCvt->mIspParamsCvtOps.update = AiqIspParamsCvt_updIsp39Params;
#endif
#elif defined(ISP_HW_V33)
    pCvt->mIspParamsCvtOps.Convert3aResultsToIspCfg = Convert3aResultsToIsp33Cfg;
#if RKAIQ_HAVE_DUMPSYS
    pCvt->mCvtedIsp33Prams =
        (struct isp33_isp_params_cfg*)aiq_mallocz(sizeof(*pCvt->mCvtedIsp33Prams));
    pCvt->mIspParamsCvtOps.update = AiqIspParamsCvt_updIsp33Params;
#endif
#else
    pCvt->mIspParamsCvtOps.Convert3aResultsToIspCfg = NULL;
    pCvt->mIspParamsCvtOps.update                   = NULL;
    pCvt->mCvtedIsp39Prams                          = NULL;
#endif

    aiq_memset(&pCvt->AntiTmoFlicker, 0, sizeof(pCvt->AntiTmoFlicker));

    return XCAM_RETURN_NO_ERROR;
}

void AiqIspParamsCvt_deinit(AiqIspParamsCvt_t* pCvt) {
#if defined(ISP_HW_V39)
    for (int i = 0; i < pCvt->mCacInfo.current_lut_size; i++) {
        aiq_free(pCvt->mCacInfo.current_lut_[i]);
        pCvt->mCacInfo.current_lut_[i] = NULL;
    }
    pCvt->mCacInfo.current_lut_size = 0;
    if (pCvt->mCacInfo.lut_manger_) {
        LutBufferManagerDeinit(&pCvt->mCacInfo, pCvt->mCacInfo.lut_manger_);
        aiq_free(pCvt->mCacInfo.lut_manger_);
        pCvt->mCacInfo.lut_manger_ = NULL;
    }
#endif

    if (pCvt->mCvtedIsp39Prams) {
        aiq_free(pCvt->mCvtedIsp39Prams);
        pCvt->mCvtedIsp39Prams = NULL;
    }
}

#if RKAIQ_HAVE_DUMPSYS
int AiqIspParamsCvt_dump(void* dumper, st_string* result, int argc, void* argv[]) {
    cvt_isp_params_dump((AiqIspParamsCvt_t*)dumper, result, argc, argv);
    return 0;
}
#endif
