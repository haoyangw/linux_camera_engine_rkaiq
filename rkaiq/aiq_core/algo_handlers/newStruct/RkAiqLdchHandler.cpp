/*
 * Copyright (c) 2019-2022 Rockchip Eletronics Co., Ltd.
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

#include "RkAiqLdchHandler.h"
#include "algos/algo_types_priv.h"
#include "newStruct/ldch/include/ldch_algo_api.h"
#include "RkAiqCore.h"

namespace RkCam {

DEFINE_HANDLE_REGISTER_TYPE(RkAiqLdchHandleInt);

void RkAiqLdchHandleInt::init() {
    ENTER_ANALYZER_FUNCTION();

    RkAiqHandle::deInit();
    mConfig       = (RkAiqAlgoCom*)(new RkAiqAlgoConfigLdch());
    mProcInParam  = (RkAiqAlgoCom*)(new RkAiqAlgoCom());
    mProcOutParam = (RkAiqAlgoResCom*)(new RkAiqAlgoProcResLdch());

    EXIT_ANALYZER_FUNCTION();
}

XCamReturn RkAiqLdchHandleInt::setAttrib(ldch_api_attrib_t* attr) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    mCfgMutex.lock();
    ret = algo_ldch_SetAttrib(mAlgoCtx, attr);
    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqLdchHandleInt::getAttrib(ldch_api_attrib_t* attr) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    mCfgMutex.lock();

    ret = algo_ldch_GetAttrib(mAlgoCtx, attr);

    mCfgMutex.unlock();

    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqLdchHandleInt::queryStatus(ldch_status_t* status) {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    mCfgMutex.lock();

    if (mAiqCore->mAiqCurParams->data().ptr() && mAiqCore->mAiqCurParams->data()->mLdchParams.ptr()) {
        rk_aiq_isp_ldch_params_t* ldch_param = mAiqCore->mAiqCurParams->data()->mLdchParams->data().ptr();
        if (ldch_param) {
            status->stMan = ldch_param->result; 
            status->en = ldch_param->en;
            status->bypass = ldch_param->bypass;
            status->opMode = RK_AIQ_OP_MODE_AUTO;
        } else {
            ret = XCAM_RETURN_ERROR_FAILED;
            LOGE_ALDCH("have no status info !");
        }
    } else {
        ret = XCAM_RETURN_ERROR_FAILED;
        LOGE_ALDCH("have no status info !");
    }

    mCfgMutex.unlock();
    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn RkAiqLdchHandleInt::prepare() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    ret = RkAiqHandle::prepare();
    RKAIQCORE_CHECK_RET(ret, "ldch handle prepare failed");

    RkAiqAlgoConfigLdch* ldch_config_int   = (RkAiqAlgoConfigLdch*)mConfig;
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;
    ldch_config_int->resource_path = sharedCom->resourcePath;
    ldch_config_int->mem_ops_ptr   = mAiqCore->mShareMemOps;
    ldch_config_int->is_multi_isp = sharedCom->is_multi_isp_mode;
    ldch_config_int->multi_isp_extended_pixel = sharedCom->multi_isp_extended_pixels;

    RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
    ret                       = des->prepare(mConfig);
    RKAIQCORE_CHECK_RET(ret, "ldch algo prepare failed");

    EXIT_ANALYZER_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn RkAiqLdchHandleInt::processing() {
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqAlgoProcResLdch* ldch_proc_res_int =
            (RkAiqAlgoProcResLdch*)mProcOutParam;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
            (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;

    ret = RkAiqHandle::processing();
    if (ret) {
        RKAIQCORE_CHECK_RET(ret, "ldch handle processing failed");
    }

    ldch_proc_res_int->ldchRes =  &shared->fullParams->mLdchParams->data()->result;

    GlobalParamsManager* globalParamsManager = mAiqCore->getGlobalParamsManager();

    if (globalParamsManager && !globalParamsManager->isFullManualMode() &&
        globalParamsManager->isManual(RESULT_TYPE_LDCH_PARAM)) {
        rk_aiq_global_params_wrap_t wrap_param;
        wrap_param.type = RESULT_TYPE_LDCH_PARAM;
        wrap_param.man_param_size = sizeof(ldch_param_t);
        wrap_param.man_param_ptr = ldch_proc_res_int->ldchRes;
        XCamReturn ret1 = globalParamsManager->getAndClearPending(&wrap_param);
        if (ret1 == XCAM_RETURN_NO_ERROR) {
            LOGK_ALDCH("get new manual params success !");
            ldch_proc_res_int->res_com.en = wrap_param.en;
            ldch_proc_res_int->res_com.bypass = wrap_param.bypass;
            ldch_proc_res_int->res_com.cfg_update = true;
        } else {
            ldch_proc_res_int->res_com.cfg_update = false;
        }
    } else {
        globalParamsManager->lockAlgoParam(RESULT_TYPE_LDCH_PARAM);
        mProcInParam->u.proc.is_attrib_update = globalParamsManager->getAndClearAlgoParamUpdateFlagLocked(RESULT_TYPE_LDCH_PARAM);

        RkAiqAlgoDescription* des = (RkAiqAlgoDescription*)mDes;
        ret                       = des->processing(mProcInParam, mProcOutParam);
        globalParamsManager->unlockAlgoParam(RESULT_TYPE_LDCH_PARAM);
    }

    RKAIQCORE_CHECK_RET(ret, "ldch algo processing failed");

    EXIT_ANALYZER_FUNCTION();
    return ret;
}


XCamReturn RkAiqLdchHandleInt::genIspResult(RkAiqFullParams* params,
        RkAiqFullParams* cur_params) {
    ENTER_ANALYZER_FUNCTION();
    XCamReturn ret                = XCAM_RETURN_NO_ERROR;
    RkAiqCore::RkAiqAlgosGroupShared_t* shared =
        (RkAiqCore::RkAiqAlgosGroupShared_t*)(getGroupShared());
    RkAiqCore::RkAiqAlgosComShared_t* sharedCom = &mAiqCore->mAlogsComSharedParams;
    RkAiqAlgoProcResLdch* ldch_res = (RkAiqAlgoProcResLdch*)mProcOutParam;

    rk_aiq_isp_ldch_params_t* ldch_param = params->mLdchParams->data().ptr();

    if (!ldch_res) {
        LOGD_ANALYZER("no ldch result");
        return XCAM_RETURN_NO_ERROR;
    }

    if (!this->getAlgoId()) {
        if (sharedCom->init) {
            ldch_param->frame_id = 0;
        } else {
            ldch_param->frame_id = shared->frameId;
        }

        if (ldch_res->res_com.cfg_update) {
            mSyncFlag = shared->frameId;
            ldch_param->sync_flag = mSyncFlag;
            ldch_param->en = ldch_res->res_com.en;
            ldch_param->bypass = ldch_res->res_com.bypass;
            // copy from algo result
            // set as the latest result
            cur_params->mLdchParams = params->mLdchParams;
            ldch_param->is_update = true;
            LOGD_ALDCH("[%d] params from algo", mSyncFlag);
        } else if (mSyncFlag != ldch_param->sync_flag) {
            ldch_param->sync_flag = mSyncFlag;
            // copy from latest result
            if (cur_params->mLdchParams.ptr()) {
                ldch_param->result = cur_params->mLdchParams->data()->result;
                ldch_param->en = cur_params->mLdchParams->data()->en;
                ldch_param->bypass = cur_params->mLdchParams->data()->bypass;
                ldch_param->is_update = true;
            } else {
                LOGE_ALDCH("no latest params !");
                ldch_param->is_update = false;
            }
            LOGD_ALDCH("[%d] params from latest [%d]", shared->frameId, mSyncFlag);
        } else {
            // do nothing, result in buf needn't update
            ldch_param->is_update = false;
            LOGD_ALDCH("[%d] params needn't update", shared->frameId);
        }
    }

    EXIT_ANALYZER_FUNCTION();

    return ret;
}

}  // namespace RkCam
