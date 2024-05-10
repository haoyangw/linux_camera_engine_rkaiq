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
#include "RkAiqSharpHandler.h"
#include "aiq_core.h"
#include "RkAiqGlobalParamsManager_c.h"

static void _handlerSharp_init(AiqAlgoHandler_t* pHdl) {
    ENTER_ANALYZER_FUNCTION();

    AiqAlgoHandler_deinit(pHdl);
    pHdl->mConfig       = (RkAiqAlgoCom*)(aiq_mallocz(sizeof(RkAiqAlgoCom)));
    pHdl->mProcInParam  = (RkAiqAlgoCom*)(aiq_mallocz(sizeof(RkAiqAlgoProcSharp)));
    pHdl->mProcOutParam = (RkAiqAlgoResCom*)(aiq_mallocz(sizeof(RkAiqAlgoProcResSharp)));

    pHdl->mResultType = RESULT_TYPE_SHARPEN_PARAM;
    pHdl->mResultSize = sizeof(sharp_param_t);

    EXIT_ANALYZER_FUNCTION();
}

static XCamReturn _handlerSharp_prepare(AiqAlgoHandler_t* pAlgoHandler) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    ret = AiqAlgoHandler_prepare(pAlgoHandler);
    RKAIQCORE_CHECK_RET(ret, "sharp handle prepare failed");

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)pAlgoHandler->mDes;
    ret                       = des->prepare(pAlgoHandler->mConfig);
    RKAIQCORE_CHECK_RET(ret, "sharp algo prepare failed");

    EXIT_ANALYZER_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn _handlerSharp_processing(AiqAlgoHandler_t* pAlgoHandler) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    ret = AiqAlgoHandler_processing(pAlgoHandler);
    if (ret) {
        RKAIQCORE_CHECK_RET(ret, "sharp handle processing failed");
    }

    RkAiqAlgoProcSharp* sharp_proc_param = (RkAiqAlgoProcSharp*)pAlgoHandler->mProcInParam;
    sharp_proc_param->blc_ob_predgain = 1.0;

    AiqAlgoHandler_do_processing_common(pAlgoHandler);

    RKAIQCORE_CHECK_RET(ret, "sharp algo processing failed");

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

AiqAlgoHandler_t* AiqAlgoHandlerSharp_constructor(RkAiqAlgoDesComm* des, AiqCore_t* aiqCore) {
    AiqAlgoHandler_t* pHdl = (AiqAlgoHandler_t*)aiq_mallocz(sizeof(AiqAlgoHandler_t));
    if (!pHdl)
		return NULL;
	AiqAlgoHandler_constructor(pHdl, des, aiqCore);
    pHdl->processing   = _handlerSharp_processing;
    pHdl->genIspResult = AiqAlgoHandler_genIspResult_common;
    pHdl->prepare      = _handlerSharp_prepare;
    pHdl->init         = _handlerSharp_init;
	return pHdl;
}

#if 0
XCamReturn AiqSharpHandler_setAttrib(AiqSharpHandler_t* pHdlSharp, sharp_api_attrib_t* attr)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    aiqMutex_lock(&pHdlSharp->mCfgMutex);
    ret = algo_sharp_SetAttrib(pHdlSharp->mAlgoCtx, attr);
    aiqMutex_unlock(&pHdlSharp->mCfgMutex);

    EXIT_ANALYZER_FUNCTION();
	return ret;
}

XCamReturn AiqSharpHandler_getAttrib(AiqSharpHandler_t* pHdlSharp, sharp_api_attrib_t* attr)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    aiqMutex_lock(&pHdlSharp->mCfgMutex);
    ret = algo_sharp_GetAttrib(pHdlSharp->mAlgoCtx, attr);
    aiqMutex_unlock(&pHdlSharp->mCfgMutex);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn AiqSharpHandler_queryStatus(AiqSharpHandler_t* pHdlSharp, sharp_status_t* status)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    aiqMutex_lock(&pHdlSharp->mCfgMutex);

    AiqAlgoHandler_t* pHdl = (AiqAlgoHandler_t*)pHdlSharp;
    aiq_params_base_t* pCurBase =
        pHdl->mAiqCore->mAiqCurParams->pParamsArray[RESULT_TYPE_SHARPEN_PARAM];

    if (pCurBase) {
        rk_aiq_isp_sharp_params_t* sharp_param = (rk_aiq_isp_sharp_params_t*)pCurBase->_data;
        if (sharp_param) {
            status->stMan  = *sharp_param;
            status->en     = pCurBase->en;
            status->bypass = pCurBase->bypass;
            status->opMode = pHdlSharp->mOpMode;
        } else {
            ret = XCAM_RETURN_ERROR_FAILED;
            LOGE_ASHARP("have no status info !");
        }
    } else {
        ret = XCAM_RETURN_ERROR_FAILED;
        LOGE_ASHARP("have no status info !");
    }

    aiqMutex_unlock(&pHdl->mCfgMutex);
    EXIT_ANALYZER_FUNCTION();
    return ret;
}
#endif

XCamReturn AiqSharpHandler_setStrength(AiqSharpHandler_t* pHdlSharp, asharp_strength_t* strg)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    aiqMutex_lock(&pHdlSharp->mCfgMutex);
    ret = algo_sharp_SetStrength(pHdlSharp->mAlgoCtx, strg->percent, strg->en);
    aiqMutex_unlock(&pHdlSharp->mCfgMutex);
    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn AiqSharpHandler_getStrength(AiqSharpHandler_t* pHdlSharp, asharp_strength_t* strg)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    aiqMutex_lock(&pHdlSharp->mCfgMutex);
    ret = algo_sharp_GetStrength(pHdlSharp->mAlgoCtx, &strg->percent, &strg->en);
    aiqMutex_unlock(&pHdlSharp->mCfgMutex);
    EXIT_ANALYZER_FUNCTION();
    return ret;
}
