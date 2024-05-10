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


#include "isp/rk_aiq_isp_ldch22.h"
#include "uAPI2/rk_aiq_user_api2_ldch.h"
#include "aiq_core_c/algo_handlers/RkAiqLdchHandler.h"
#include "RkAiqGlobalParamsManager_c.h"

RKAIQ_BEGIN_DECLARE

#ifdef RK_SIMULATOR_HW
#define CHECK_USER_API_ENABLE
#endif

#if RKAIQ_HAVE_LDCH_V21 && USE_NEWSTRUCT
static XCamReturn
_ldch_SetAttrib(const rk_aiq_sys_ctx_t* sys_ctx, ldch_api_attrib_t* attr)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    if (attr->opMode == RK_AIQ_OP_MODE_AUTO) {
        rk_aiq_global_params_wrap_t params;
        params.opMode = attr->opMode;
        params.bypass = attr->bypass;
        params.en = attr->en;
        params.type = RESULT_TYPE_LDCH_PARAM;
        params.aut_param_size = sizeof(ldch_param_auto_t);
        params.aut_param_ptr = &attr->stAuto;
        ret = GlobalParamsManager_set(&sys_ctx->_rkAiqManager->mGlobalParamsManager, &params);
    } else {
        LOGE_AGIC("wrong mode %d !", attr->opMode);
        return XCAM_RETURN_ERROR_PARAM;
    }

    return ret;
}

XCamReturn
rk_aiq_user_api2_ldch_SetAttrib(const rk_aiq_sys_ctx_t* sys_ctx, ldch_api_attrib_t* attr)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
#if USE_NEWSTRUCT
    CHECK_USER_API_ENABLE2(sys_ctx);
    CHECK_USER_API_ENABLE(RK_AIQ_ALGO_TYPE_ALDCH);
    RKAIQ_API_SMART_LOCK(sys_ctx);
    if (sys_ctx->cam_type == RK_AIQ_CAM_TYPE_GROUP) {
#ifdef RKAIQ_ENABLE_CAMGROUP
        const rk_aiq_camgroup_ctx_t* camgroup_ctx = (rk_aiq_camgroup_ctx_t *)sys_ctx;
		rk_aiq_sys_ctx_t* camCtx = NULL;
        for (int i = 0; i < RK_AIQ_CAM_GROUP_MAX_CAMS; i++) {
			camCtx = camgroup_ctx->cam_ctxs_array[i];
            if (!camCtx)
                continue;
            ret = _ldch_SetAttrib(camCtx, attr);
        }
        return ret;
#else
        return XCAM_RETURN_ERROR_FAILED;
#endif
    } else {
        return _ldch_SetAttrib(sys_ctx, attr);
    }
#else
    return XCAM_RETURN_ERROR_UNKNOWN;
#endif
}

static XCamReturn
_ldch_GetAttrib(const rk_aiq_sys_ctx_t* sys_ctx, ldch_api_attrib_t* attr)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqLdchHandleInt* algo_handle =
        algoHandle<RkAiqLdchHandleInt>(sys_ctx, RK_AIQ_ALGO_TYPE_ALDCH);

    if (algo_handle) {
        algo_handle->getAttrib(attr);
    }

    return ret;
}

XCamReturn
rk_aiq_user_api2_ldch_GetAttrib(const rk_aiq_sys_ctx_t* sys_ctx, ldch_api_attrib_t* attr)
{
#if USE_NEWSTRUCT
    CHECK_USER_API_ENABLE2(sys_ctx);
    CHECK_USER_API_ENABLE(RK_AIQ_ALGO_TYPE_ALDCH);
    RKAIQ_API_SMART_LOCK(sys_ctx);
    if (sys_ctx->cam_type == RK_AIQ_CAM_TYPE_GROUP) {
#ifdef RKAIQ_ENABLE_CAMGROUP
        const rk_aiq_camgroup_ctx_t* camgroup_ctx = (rk_aiq_camgroup_ctx_t *)sys_ctx;
		rk_aiq_sys_ctx_t* camCtx = NULL;
        for (int i = 0; i < RK_AIQ_CAM_GROUP_MAX_CAMS; i++) {
			camCtx = camgroup_ctx->cam_ctxs_array[i];
            if (!camCtx)
                continue;
            // return the first cam's attrib
            return _ldch_GetAttrib(camCtx , attr);
        }
#else
        return XCAM_RETURN_ERROR_FAILED;
#endif
    } else {
        return _ldch_GetAttrib(sys_ctx, attr);
    }
#else
    return XCAM_RETURN_ERROR_UNKNOWN;
#endif
    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn
_ldch_QueryStatus(const rk_aiq_sys_ctx_t* sys_ctx, ldch_status_t* status)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    RkAiqLdchHandleInt* algo_handle =
        algoHandle<RkAiqLdchHandleInt>(sys_ctx, RK_AIQ_ALGO_TYPE_ALDCH);

    if (algo_handle) {
        return algo_handle->queryStatus(status);
    }

    return ret;
}

XCamReturn
rk_aiq_user_api2_ldch_QueryStatus(const rk_aiq_sys_ctx_t* sys_ctx, ldch_status_t* status)
{
#if USE_NEWSTRUCT
    CHECK_USER_API_ENABLE2(sys_ctx);
    CHECK_USER_API_ENABLE(RK_AIQ_ALGO_TYPE_ALDCH);
    RKAIQ_API_SMART_LOCK(sys_ctx);
    if (sys_ctx->cam_type == RK_AIQ_CAM_TYPE_GROUP) {
#ifdef RKAIQ_ENABLE_CAMGROUP
        const rk_aiq_camgroup_ctx_t* camgroup_ctx = (rk_aiq_camgroup_ctx_t *)sys_ctx;
		rk_aiq_sys_ctx_t* camCtx = NULL;
        for (int i = 0; i < RK_AIQ_CAM_GROUP_MAX_CAMS; i++) {
			camCtx = camgroup_ctx->cam_ctxs_array[i];
            if (!camCtx)
                continue;
            // return the first cam's attrib
            return _ldch_QueryStatus(camCtx , status);
        }
#else
        return XCAM_RETURN_ERROR_FAILED;
#endif
    } else {
        return _ldch_QueryStatus(sys_ctx, status);
    }
#else
    return XCAM_RETURN_ERROR_UNKNOWN;
#endif
   return XCAM_RETURN_NO_ERROR;
}
#else
XCamReturn
rk_aiq_user_api2_ldch_SetAttrib(const rk_aiq_sys_ctx_t* sys_ctx, ldch_api_attrib_t* attr)
{
    return XCAM_RETURN_ERROR_UNKNOWN;
}

XCamReturn
rk_aiq_user_api2_ldch_GetAttrib(const rk_aiq_sys_ctx_t* sys_ctx, ldch_api_attrib_t* attr)
{
    return XCAM_RETURN_ERROR_UNKNOWN;
}

XCamReturn
rk_aiq_user_api2_ldch_QueryStatus(const rk_aiq_sys_ctx_t* sys_ctx, ldch_status_t* status)
{
   return XCAM_RETURN_ERROR_UNKNOWN;
}
#endif

RKAIQ_END_DECLARE
