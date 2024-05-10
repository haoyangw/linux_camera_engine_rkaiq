/*
 * Copyright (c) 2024 Rockchip Eletronics Co., Ltd.
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
 */

#include "aiq_base.h"
#include "RkAiqCcmHandler.h"
#include "aiq_core.h"
#include "RkAiqGlobalParamsManager_c.h"
#include "newStruct/algo_common.h"

static void _handlerCcm_init(AiqAlgoHandler_t* pHdl) {
    ENTER_ANALYZER_FUNCTION();

    AiqAlgoHandler_deinit(pHdl);
    pHdl->mConfig       = (RkAiqAlgoCom*)(aiq_mallocz(sizeof(RkAiqAlgoCom)));
    pHdl->mProcInParam  = (RkAiqAlgoCom*)(aiq_mallocz(sizeof(RkAiqAlgoProcCcm)));
    pHdl->mProcOutParam = (RkAiqAlgoResCom*)(aiq_mallocz(sizeof(RkAiqAlgoResCom)));

    pHdl->mResultType = RESULT_TYPE_CCM_PARAM;
    pHdl->mResultSize = sizeof(ccm_param_t);

    EXIT_ANALYZER_FUNCTION();
}

static XCamReturn _handlerCcm_prepare(AiqAlgoHandler_t* pAlgoHandler) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    ret = AiqAlgoHandler_prepare(pAlgoHandler);
    RKAIQCORE_CHECK_RET(ret, "ccm handle prepare failed");

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)pAlgoHandler->mDes;
    ret                       = des->prepare(pAlgoHandler->mConfig);
    RKAIQCORE_CHECK_RET(ret, "ccm algo prepare failed");

    EXIT_ANALYZER_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn _handlerCcm_processing(AiqAlgoHandler_t* pAlgoHandler) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqAlgoResCom* proc_res = pAlgoHandler->mProcOutParam;
    //RkAiqAlgoProcResCcm* ccm_proc_res_int = (RkAiqAlgoProcResCcm*)pAlgoHandler->mProcOutParam;
    RkAiqAlgosGroupShared_t* shared =
        (RkAiqAlgosGroupShared_t*)(pAlgoHandler->mAlogsGroupSharedParams);
    RkAiqAlgosComShared_t* sharedCom = &pAlgoHandler->mAiqCore->mAlogsComSharedParams;

    ret = AiqAlgoHandler_processing(pAlgoHandler);
    if (ret) {
        RKAIQCORE_CHECK_RET(ret, "ccm handle processing failed");
    }

    aiq_params_base_t* pBase = shared->fullParams->pParamsArray[RESULT_TYPE_CCM_PARAM];
    if (!pBase) {
        LOGW_ACCM("no ccm params buf !");
        return XCAM_RETURN_BYPASS;
    }

    proc_res->algoRes =  (rk_aiq_isp_ccm_params_t*)pBase->_data;

    RkAiqAlgoProcCcm* proc_int        = (RkAiqAlgoProcCcm*)pAlgoHandler->mProcInParam;
    RKAiqAecExpInfo_t* pCurExp = &shared->curExp;
    RkAiqAlgoProcResAwbShared_t* awb_res = NULL;
    AlgoRstShared_t* awb_proc_res = shared->res_comb.awb_proc_res_c;
    if (awb_proc_res) {
        awb_res = (RkAiqAlgoProcResAwbShared_t*)awb_proc_res->_data;
    }
    get_illu_estm_info(&proc_int->illu_info, awb_res, pCurExp, sharedCom->working_mode);

    ret = AiqAlgoHandler_do_processing_common(pAlgoHandler);

    RKAIQCORE_CHECK_RET(ret, "adebayer algo processing failed");

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

AiqAlgoHandler_t* AiqAlgoHandlerCcm_constructor(RkAiqAlgoDesComm* des, AiqCore_t* aiqCore) {
    AiqAlgoHandler_t* pHdl = (AiqAlgoHandler_t*)aiq_mallocz(sizeof(AiqAlgoHandlerCcm_t));
    if (!pHdl)
		return NULL;
	AiqAlgoHandler_constructor(pHdl, des, aiqCore);

    pHdl->processing   = _handlerCcm_processing;
    pHdl->genIspResult = AiqAlgoHandler_genIspResult_common;
    pHdl->prepare      = _handlerCcm_prepare;
    pHdl->init         = _handlerCcm_init;
	return pHdl;
}

#if 0
XCamReturn AiqAlgoHandlerCcm_setAttrib(AiqAlgoHandlerCcm_t* pHdlCcm, ccm_api_attrib_t* attr) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    aiqMutex_lock(&pHdlCcm->mCfgMutex);
    ret = algo_ccm_SetAttrib(pHdlCcm->mAlgoCtx, attr);
    aiqMutex_unlock(&pHdlCcm->mCfgMutex);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn AiqAlgoHandlerCcm_getAttrib(AiqAlgoHandlerCcm_t* pHdlCcm, ccm_api_attrib_t* attr) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    aiqMutex_lock(&pHdlCcm->mCfgMutex);

    ret = algo_ccm_GetAttrib(pHdlCcm->mAlgoCtx, attr);

    aiqMutex_unlock(&pHdlCcm->mCfgMutex);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn AiqAlgoHandlerCcm_queryStatus(AiqAlgoHandlerCcm_t* pHdlCcm, ccm_status_t* status) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    aiqMutex_lock(&pHdlCcm->mCfgMutex);

    AiqAlgoHandler_t* pHdl = (AiqAlgoHandler_t*)pHdlCcm;
    aiq_params_base_t* pCurBase =
        pHdl->mAiqCore->mAiqCurParams->pParamsArray[RESULT_TYPE_CCM_PARAM];

    if (pCurBase) {
        rk_aiq_isp_ccm_params_t* ccm_param = (rk_aiq_isp_ccm_params_t*)pCurBase->_data;
        if (ccm_param) {
            status->stMan = *ccm_param; 
            status->en     = pCurBase->en;
            status->bypass = pCurBase->bypass;
            status->opMode = pHdlCcm->mOpMode;
        } else {
            ret = XCAM_RETURN_ERROR_FAILED;
            LOGE_ACCM("have no status info !");
        }
    } else {
        ret = XCAM_RETURN_ERROR_FAILED;
        LOGE_ACCM("have no status info !");
    }

    aiqMutex_unlock(&pHdlCcm->mCfgMutex);
    EXIT_ANALYZER_FUNCTION();
    return ret;
}
#endif
