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
#include "RkAiqDrcHandler.h"
#include "aiq_core.h"
#include "RkAiqGlobalParamsManager_c.h"
#include "newStruct/drc/drc_types_prvt.h"

static void _handlerDrc_init(AiqAlgoHandler_t* pHdl) {
    ENTER_ANALYZER_FUNCTION();

    AiqAlgoHandler_deinit(pHdl);
    pHdl->mConfig       = (RkAiqAlgoCom*)(aiq_mallocz(sizeof(RkAiqAlgoConfigDrc)));
    pHdl->mProcInParam  = (RkAiqAlgoCom*)(aiq_mallocz(sizeof(RkAiqAlgoProcDrc)));
    pHdl->mProcOutParam = (RkAiqAlgoResCom*)(aiq_mallocz(sizeof(RkAiqAlgoProcResDrc)));

    pHdl->mResultType = RESULT_TYPE_DRC_PARAM ;
    pHdl->mResultSize = sizeof(drc_param_t);

    EXIT_ANALYZER_FUNCTION();
}

static XCamReturn _handlerDrc_prepare(AiqAlgoHandler_t* pAlgoHandler) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    ret = AiqAlgoHandler_prepare(pAlgoHandler);
    RKAIQCORE_CHECK_RET(ret, "drc handle prepare failed");

    RkAiqAlgoConfigDrc* adrc_config_int     = (RkAiqAlgoConfigDrc*)pAlgoHandler->mConfig;
    RkAiqAlgosComShared_t* sharedCom = &pAlgoHandler->mAiqCore->mAlogsComSharedParams;

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)pAlgoHandler->mDes;
    ret                       = des->prepare(pAlgoHandler->mConfig);
    RKAIQCORE_CHECK_RET(ret, "drc algo prepare failed");

    EXIT_ANALYZER_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

static void DrcProchelper(AiqAlgoHandler_t* pAlgoHandler, RkAiqAlgoProcDrc* drc_proc_param)
{
    RkAiqAlgosGroupShared_t* shared =
        (RkAiqAlgosGroupShared_t*)(pAlgoHandler->mAlogsGroupSharedParams);
    RkAiqAlgosComShared_t* sharedCom = &pAlgoHandler->mAiqCore->mAlogsComSharedParams;

    aiq_params_base_t* pBase = shared->fullParams->pParamsArray[RESULT_TYPE_DRC_PARAM];
    rk_aiq_isp_drc_v39_t* drcRes = (rk_aiq_isp_drc_params_t*)pBase->_data;

    FrameNumber_t FrameNumber;
    unsigned char compr_bit = sharedCom->snsDes.compr_bit;
    if (sharedCom->working_mode < RK_AIQ_WORKING_MODE_ISP_HDR2)
        FrameNumber = LINEAR_NUM;
    else if (sharedCom->working_mode < RK_AIQ_WORKING_MODE_ISP_HDR3 &&
             sharedCom->working_mode >= RK_AIQ_WORKING_MODE_ISP_HDR2)
        FrameNumber = HDR_2X_NUM;
    else
        FrameNumber = HDR_3X_NUM;
    if (compr_bit) {
        FrameNumber = SENSOR_MGE;
        if (compr_bit > ISP_HDR_BIT_NUM_MAX)
            LOGE_ATMO("%s:  SensorMgeBitNum(%d) > %d!!!\n", __FUNCTION__, compr_bit,
                      ISP_HDR_BIT_NUM_MAX);
        if (compr_bit < ISP_HDR_BIT_NUM_MIN)
            LOGE_ATMO("%s:  SensorMgeBitNum(%d) < %d!!!\n", __FUNCTION__, compr_bit,
                      ISP_HDR_BIT_NUM_MIN);
        compr_bit =
            CLIP(compr_bit, ISP_HDR_BIT_NUM_MAX, ISP_HDR_BIT_NUM_MIN);
    }
    drc_proc_param->FrameNumber = FrameNumber;
    drcRes->compr_bit = compr_bit;

    drc_proc_param->NextData.AEData.LongFrmMode = drc_proc_param->LongFrmMode;
    if (FrameNumber == LINEAR_NUM) {
        drc_proc_param->NextData.AEData.SExpo =
            drc_proc_param->com.u.proc.nxtExp->LinearExp.exp_real_params.analog_gain *
            drc_proc_param->com.u.proc.nxtExp->LinearExp.exp_real_params.digital_gain *
            drc_proc_param->com.u.proc.nxtExp->LinearExp.exp_real_params.isp_dgain *
            drc_proc_param->com.u.proc.nxtExp->LinearExp.exp_real_params.integration_time;
        if (drc_proc_param->NextData.AEData.SExpo < FLT_EPSILON) {
            drc_proc_param->NextData.AEData.SExpo =
                drc_proc_param->com.u.proc.curExp->LinearExp.exp_real_params.analog_gain *
                drc_proc_param->com.u.proc.curExp->LinearExp.exp_real_params.digital_gain *
                drc_proc_param->com.u.proc.curExp->LinearExp.exp_real_params.isp_dgain *
                drc_proc_param->com.u.proc.curExp->LinearExp.exp_real_params.integration_time;
        }
        drc_proc_param->NextData.AEData.MExpo = drc_proc_param->NextData.AEData.SExpo;
        drc_proc_param->NextData.AEData.LExpo = drc_proc_param->NextData.AEData.SExpo;
    }
    else if(FrameNumber == HDR_2X_NUM) {
        drc_proc_param->NextData.AEData.SExpo =
            drc_proc_param->com.u.proc.nxtExp->HdrExp[0].exp_real_params.analog_gain *
            drc_proc_param->com.u.proc.nxtExp->HdrExp[0].exp_real_params.digital_gain *
            drc_proc_param->com.u.proc.nxtExp->HdrExp[0].exp_real_params.isp_dgain *
            drc_proc_param->com.u.proc.nxtExp->HdrExp[0].exp_real_params.integration_time;
        drc_proc_param->NextData.AEData.MExpo =
            drc_proc_param->com.u.proc.nxtExp->HdrExp[1].exp_real_params.analog_gain *
            drc_proc_param->com.u.proc.nxtExp->HdrExp[1].exp_real_params.digital_gain *
            drc_proc_param->com.u.proc.nxtExp->HdrExp[1].exp_real_params.isp_dgain *
            drc_proc_param->com.u.proc.nxtExp->HdrExp[1].exp_real_params.integration_time;
        if (drc_proc_param->NextData.AEData.SExpo < FLT_EPSILON) {
            drc_proc_param->NextData.AEData.SExpo =
                drc_proc_param->com.u.proc.curExp->HdrExp[0].exp_real_params.analog_gain *
                drc_proc_param->com.u.proc.curExp->HdrExp[0].exp_real_params.digital_gain *
                drc_proc_param->com.u.proc.curExp->HdrExp[0].exp_real_params.isp_dgain *
                drc_proc_param->com.u.proc.curExp->HdrExp[0].exp_real_params.integration_time;
            drc_proc_param->NextData.AEData.MExpo =
                drc_proc_param->com.u.proc.curExp->HdrExp[1].exp_real_params.analog_gain *
                drc_proc_param->com.u.proc.curExp->HdrExp[1].exp_real_params.digital_gain *
                drc_proc_param->com.u.proc.curExp->HdrExp[1].exp_real_params.isp_dgain *
                drc_proc_param->com.u.proc.curExp->HdrExp[1].exp_real_params.integration_time;
        }
        drc_proc_param->NextData.AEData.LExpo = drc_proc_param->NextData.AEData.MExpo;
    }
    else if(FrameNumber == HDR_3X_NUM) {
        drc_proc_param->NextData.AEData.SExpo =
            drc_proc_param->com.u.proc.nxtExp->HdrExp[0].exp_real_params.analog_gain *
            drc_proc_param->com.u.proc.nxtExp->HdrExp[0].exp_real_params.digital_gain *
            drc_proc_param->com.u.proc.nxtExp->HdrExp[0].exp_real_params.isp_dgain *
            drc_proc_param->com.u.proc.nxtExp->HdrExp[0].exp_real_params.integration_time;
        drc_proc_param->NextData.AEData.MExpo =
            drc_proc_param->com.u.proc.nxtExp->HdrExp[1].exp_real_params.analog_gain *
            drc_proc_param->com.u.proc.nxtExp->HdrExp[1].exp_real_params.digital_gain *
            drc_proc_param->com.u.proc.nxtExp->HdrExp[1].exp_real_params.isp_dgain *
            drc_proc_param->com.u.proc.nxtExp->HdrExp[1].exp_real_params.integration_time;
        drc_proc_param->NextData.AEData.LExpo =
            drc_proc_param->com.u.proc.nxtExp->HdrExp[2].exp_real_params.analog_gain *
            drc_proc_param->com.u.proc.nxtExp->HdrExp[2].exp_real_params.digital_gain *
            drc_proc_param->com.u.proc.nxtExp->HdrExp[2].exp_real_params.isp_dgain *
            drc_proc_param->com.u.proc.nxtExp->HdrExp[2].exp_real_params.integration_time;
        if (drc_proc_param->NextData.AEData.SExpo < FLT_EPSILON) {
            drc_proc_param->NextData.AEData.SExpo =
                drc_proc_param->com.u.proc.curExp->HdrExp[0].exp_real_params.analog_gain *
                drc_proc_param->com.u.proc.curExp->HdrExp[0].exp_real_params.digital_gain *
                drc_proc_param->com.u.proc.curExp->HdrExp[0].exp_real_params.isp_dgain *
                drc_proc_param->com.u.proc.curExp->HdrExp[0].exp_real_params.integration_time;
            drc_proc_param->NextData.AEData.MExpo =
                drc_proc_param->com.u.proc.curExp->HdrExp[1].exp_real_params.analog_gain *
                drc_proc_param->com.u.proc.curExp->HdrExp[1].exp_real_params.digital_gain *
                drc_proc_param->com.u.proc.curExp->HdrExp[1].exp_real_params.isp_dgain *
                drc_proc_param->com.u.proc.curExp->HdrExp[1].exp_real_params.integration_time;
            drc_proc_param->NextData.AEData.LExpo =
                drc_proc_param->com.u.proc.curExp->HdrExp[2].exp_real_params.analog_gain *
                drc_proc_param->com.u.proc.curExp->HdrExp[2].exp_real_params.digital_gain *
                drc_proc_param->com.u.proc.curExp->HdrExp[2].exp_real_params.isp_dgain *
                drc_proc_param->com.u.proc.curExp->HdrExp[2].exp_real_params.integration_time;
        }
    } else if (FrameNumber == SENSOR_MGE) {
        drc_proc_param->NextData.AEData.MExpo =
            drc_proc_param->com.u.proc.nxtExp->LinearExp.exp_real_params.analog_gain *
            drc_proc_param->com.u.proc.nxtExp->LinearExp.exp_real_params.digital_gain *
            drc_proc_param->com.u.proc.nxtExp->LinearExp.exp_real_params.isp_dgain *
            drc_proc_param->com.u.proc.nxtExp->LinearExp.exp_real_params.integration_time;
        if (drc_proc_param->NextData.AEData.MExpo < FLT_EPSILON) {
            drc_proc_param->NextData.AEData.MExpo =
                drc_proc_param->com.u.proc.curExp->LinearExp.exp_real_params.analog_gain *
                drc_proc_param->com.u.proc.curExp->LinearExp.exp_real_params.digital_gain *
                drc_proc_param->com.u.proc.curExp->LinearExp.exp_real_params.isp_dgain *
                drc_proc_param->com.u.proc.curExp->LinearExp.exp_real_params.integration_time;
        }
        drc_proc_param->NextData.AEData.LExpo = drc_proc_param->NextData.AEData.MExpo;
        drc_proc_param->NextData.AEData.SExpo =
            drc_proc_param->NextData.AEData.MExpo /
            pow(2.0f, (float)(compr_bit - ISP_HDR_BIT_NUM_MIN));
    }
    if (drc_proc_param->NextData.AEData.SExpo > FLT_EPSILON) {
        drc_proc_param->NextData.AEData.L2S_Ratio =
            drc_proc_param->NextData.AEData.LExpo / drc_proc_param->NextData.AEData.SExpo;
        if (drc_proc_param->NextData.AEData.L2S_Ratio < RATIO_DEFAULT) {
            LOGE_ATMO("%s: Next L2S_Ratio:%f is less than 1.0x, clip to 1.0x!!!\n",
                        __FUNCTION__, drc_proc_param->NextData.AEData.L2S_Ratio);
            drc_proc_param->NextData.AEData.L2S_Ratio = RATIO_DEFAULT;
        }
        drc_proc_param->NextData.AEData.M2S_Ratio =
            drc_proc_param->NextData.AEData.MExpo / drc_proc_param->NextData.AEData.SExpo;
        if (drc_proc_param->NextData.AEData.M2S_Ratio < RATIO_DEFAULT) {
            LOGE_ATMO("%s: Next M2S_Ratio:%f is less than 1.0x, clip to 1.0x!!!\n",
                        __FUNCTION__, drc_proc_param->NextData.AEData.M2S_Ratio);
            drc_proc_param->NextData.AEData.M2S_Ratio = RATIO_DEFAULT;
        }
    }
    else
        LOGE_ATMO("%s: Next Short frame for drc expo sync is %f!!!\n", __FUNCTION__,
                    drc_proc_param->NextData.AEData.SExpo);
    if (drc_proc_param->NextData.AEData.MExpo > FLT_EPSILON) {
        drc_proc_param->NextData.AEData.L2M_Ratio =
            drc_proc_param->NextData.AEData.LExpo / drc_proc_param->NextData.AEData.MExpo;
        if (drc_proc_param->NextData.AEData.L2M_Ratio < RATIO_DEFAULT) {
            LOGE_ATMO("%s: Next L2M_Ratio:%f is less than 1.0x, clip to 1.0x!!!\n",
                        __FUNCTION__, drc_proc_param->NextData.AEData.L2M_Ratio);
            drc_proc_param->NextData.AEData.L2M_Ratio = RATIO_DEFAULT;
        }
    } else
        LOGE_ATMO("%s: Next Midlle frame for drc expo sync is %f!!!\n", __FUNCTION__,
                    drc_proc_param->NextData.AEData.MExpo);
    //clip for long frame mode
    if (drc_proc_param->NextData.AEData.LongFrmMode) {
        drc_proc_param->NextData.AEData.L2S_Ratio = LONG_FRAME_MODE_RATIO;
        drc_proc_param->NextData.AEData.M2S_Ratio = LONG_FRAME_MODE_RATIO;
        drc_proc_param->NextData.AEData.L2M_Ratio = LONG_FRAME_MODE_RATIO;
    }
    // clip L2M_ratio to 32x
    if (drc_proc_param->NextData.AEData.L2M_Ratio > AE_RATIO_L2M_MAX) {
        LOGE_ATMO("%s: Next L2M_ratio:%f out of range, clip to 32.0x!!!\n", __FUNCTION__,
                    drc_proc_param->NextData.AEData.L2M_Ratio);
        drc_proc_param->NextData.AEData.L2M_Ratio = AE_RATIO_L2M_MAX;
    }
    // clip L2S_ratio
    if (drc_proc_param->NextData.AEData.L2S_Ratio > AE_RATIO_MAX) {
        LOGE_ATMO("%s: Next L2S_Ratio:%f out of range, clip to 256.0x!!!\n", __FUNCTION__,
                    drc_proc_param->NextData.AEData.L2S_Ratio);
        drc_proc_param->NextData.AEData.L2S_Ratio = AE_RATIO_MAX;
    }
    // clip L2M_ratio and M2S_Ratio
    if (drc_proc_param->NextData.AEData.L2M_Ratio * drc_proc_param->NextData.AEData.M2S_Ratio >
        AE_RATIO_MAX) {
        LOGE_ATMO("%s: Next L2M_Ratio*M2S_Ratio:%f out of range, clip to 256.0x!!!\n",
                    __FUNCTION__,
                    drc_proc_param->NextData.AEData.L2M_Ratio * drc_proc_param->NextData.AEData.M2S_Ratio);
        drc_proc_param->NextData.AEData.M2S_Ratio =
            AE_RATIO_MAX / drc_proc_param->NextData.AEData.L2M_Ratio;
    }
    drcRes->L2S_Ratio = drc_proc_param->NextData.AEData.L2S_Ratio;
}

static XCamReturn _handlerDrc_processing(AiqAlgoHandler_t* pAlgoHandler) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqAlgosGroupShared_t* shared =
        (RkAiqAlgosGroupShared_t*)(pAlgoHandler->mAlogsGroupSharedParams);

    ret = AiqAlgoHandler_processing(pAlgoHandler);
    if (ret) {
        RKAIQCORE_CHECK_RET(ret, "drc handle processing failed");
    }

    aiq_params_base_t* pBase = shared->fullParams->pParamsArray[RESULT_TYPE_DRC_PARAM];
    if (!pBase) {
        LOGW_ATMO("no blc params buf !");
        return XCAM_RETURN_BYPASS;
    }

    RkAiqAlgoProcDrc* drc_proc_param = (RkAiqAlgoProcDrc*)pAlgoHandler->mProcInParam;
    drc_proc_param->isp_ob_predgain = 1.0;

    DrcProchelper(pAlgoHandler, drc_proc_param);

    AiqAlgoHandler_do_processing_common(pAlgoHandler);

    GlobalParamsManager_t * globalParamsManager = pAlgoHandler->mAiqCore->mGlobalParamsManger;

    rk_aiq_global_params_wrap_t params;
    trans_api_attrib_t trans_attr;
    params.type = RESULT_TYPE_TRANS_PARAM;
    params.man_param_size = sizeof(trans_param_t);
    params.man_param_ptr = &trans_attr.stMan;
    params.aut_param_ptr = NULL;
    ret = GlobalParamsManager_get(globalParamsManager, &params);
    trans_attr.en = params.en;
    trans_attr.bypass = params.bypass;
	XCamReturn ret1           = GlobalParamsManager_getAndClearPending(globalParamsManager, &params);
    if (pBase) {
		trans_api_attrib_t* trans_curAttr = &((rk_aiq_isp_drc_params_t*)pBase->_data)->trans_attr;
		*trans_curAttr = trans_attr;
	}
    if (ret1 == XCAM_RETURN_NO_ERROR) {
        pAlgoHandler->mProcOutParam->cfg_update = true;
        LOGD_ATMO("trans params update");
    }

    RKAIQCORE_CHECK_RET(ret, "drc algo processing failed");

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

AiqAlgoHandler_t* AiqAlgoHandlerDrc_constructor(RkAiqAlgoDesComm* des, AiqCore_t* aiqCore) {
    AiqAlgoHandler_t* pHdl = (AiqAlgoHandler_t*)aiq_mallocz(sizeof(AiqDrcHandler_t));
    if (!pHdl)
		return NULL;
	AiqAlgoHandler_constructor(pHdl, des, aiqCore);
    pHdl->processing   = _handlerDrc_processing;
    pHdl->genIspResult = AiqAlgoHandler_genIspResult_common;
    pHdl->prepare      = _handlerDrc_prepare;
    pHdl->init         = _handlerDrc_init;
	return pHdl;
}

#if 0
XCamReturn AiqDrcHandler_setAttrib(AiqDrcHandler_t* pHdlDrc, drc_api_attrib_t* attr)
{
    AiqAlgoHandler_t* pHdl = (AiqAlgoHandler_t*)pHdlDrc;

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    aiqMutex_lock(&pHdl->mCfgMutex);
    ret = algo_drc_SetAttrib(pHdl->mAlgoCtx, attr);
    aiqMutex_unlock(&pHdl->mCfgMutex);

    EXIT_ANALYZER_FUNCTION();
	return ret;
}

XCamReturn AiqDrcHandler_getAttrib(AiqDrcHandler_t* pHdlDrc, drc_api_attrib_t* attr)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    AiqAlgoHandler_t* pHdl = (AiqAlgoHandler_t*)pHdlDrc;

    aiqMutex_lock(&pHdl->mCfgMutex);

    ret = algo_drc_GetAttrib(pHdl->mAlgoCtx, attr);

    aiqMutex_unlock(&pHdl->mCfgMutex);

    EXIT_ANALYZER_FUNCTION();
    return ret;
}
#endif
XCamReturn AiqDrcHandler_queryStatus(AiqDrcHandler_t* pHdlDrc, drc_status_t* status)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    AiqAlgoHandler_t* pHdl = (AiqAlgoHandler_t*)pHdlDrc;

    aiqMutex_lock(&pHdl->mCfgMutex);

    aiq_params_base_t* pCurBase =
        pHdl->mAiqCore->mAiqCurParams->pParamsArray[RESULT_TYPE_DRC_PARAM];

    if (pCurBase) {
        rk_aiq_isp_drc_params_t* drc_param = (rk_aiq_isp_drc_params_t*)pCurBase->_data;
        if (drc_param) {
            status->stMan  = drc_param->drc_param;
            status->en     = pCurBase->en;
            status->bypass = pCurBase->bypass;
            status->opMode = pHdlDrc->_base.mOpMode;
        } else {
            ret = XCAM_RETURN_ERROR_FAILED;
            LOGE_ANR("have no status info !");
        }
    } else {
        ret = XCAM_RETURN_ERROR_FAILED;
        LOGE_ANR("have no status info !");
    }

    aiqMutex_unlock(&pHdl->mCfgMutex);
    EXIT_ANALYZER_FUNCTION();
    return ret;

}
