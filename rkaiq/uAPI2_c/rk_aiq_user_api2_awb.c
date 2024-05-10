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
#include "uAPI2/rk_aiq_user_api2_awb.h"

//#include "RkAiqCamGroupHandleInt.h"
#include "aiq_core_c/algo_handlers/RkAiqAwbHandler.h"
#include "aiq_core_c/algo_camgroup_handlers/RkAiqCamGroupAwbHandle.h"
#include "awb/rk_aiq_uapiv2_awb_int.h"

RKAIQ_BEGIN_DECLARE

#ifdef RK_SIMULATOR_HW
#define CHECK_USER_API_ENABLE
#endif


XCamReturn
rk_aiq_user_api2_awbV21_SetAllAttrib(const rk_aiq_sys_ctx_t* sys_ctx, rk_aiq_uapiV2_wbV21_attrib_t attr)
{
    return XCAM_RETURN_ERROR_UNKNOWN;
}

XCamReturn
rk_aiq_user_api2_awbV21_GetAllAttrib(const rk_aiq_sys_ctx_t* sys_ctx, rk_aiq_uapiV2_wbV21_attrib_t *attr)
{
    return XCAM_RETURN_ERROR_UNKNOWN;
}

XCamReturn
rk_aiq_user_api2_awbV30_SetAllAttrib(const rk_aiq_sys_ctx_t* sys_ctx, rk_aiq_uapiV2_wbV30_attrib_t attr)
{
    return XCAM_RETURN_ERROR_UNKNOWN;
}

XCamReturn
rk_aiq_user_api2_awbV30_GetAllAttrib(const rk_aiq_sys_ctx_t* sys_ctx, rk_aiq_uapiV2_wbV30_attrib_t *attr)
{
    return XCAM_RETURN_ERROR_UNKNOWN;
}

XCamReturn
rk_aiq_user_api2_awbV32_SetAllAttrib(const rk_aiq_sys_ctx_t* sys_ctx, rk_aiq_uapiV2_wbV32_attrib_t attr)
{
    return XCAM_RETURN_ERROR_UNKNOWN;
}

XCamReturn
rk_aiq_user_api2_awbV32_GetAllAttrib(const rk_aiq_sys_ctx_t* sys_ctx, rk_aiq_uapiV2_wbV32_attrib_t *attr)
{
    return XCAM_RETURN_ERROR_UNKNOWN;
}


XCamReturn
rk_aiq_user_api2_awbV39_SetAllAttrib(const rk_aiq_sys_ctx_t* sys_ctx, rk_aiq_uapiV2_wbV39_attrib_t attr)
{

#if RKAIQ_HAVE_AWB_V39// TODO

    CHECK_USER_API_ENABLE2(sys_ctx);
    CHECK_USER_API_ENABLE(RK_AIQ_ALGO_TYPE_AWB);
    RKAIQ_API_SMART_LOCK(sys_ctx);

    if (sys_ctx->cam_type == RK_AIQ_CAM_TYPE_GROUP) {
        LOGW_AWB("%s:bypass",__FUNCTION__);
        return XCAM_RETURN_NO_ERROR;//TODO
    } else {
        AiqAlgoHandlerAwb_t* algo_handle =
            (AiqAlgoHandlerAwb_t*)sys_ctx->_analyzer->mAlgoHandleMaps[RK_AIQ_ALGO_TYPE_AWB];

        if (algo_handle) {
            return AiqAlgoHandlerAwb_setWbV39Attrib(algo_handle, attr);
        }
    }
    return XCAM_RETURN_NO_ERROR;
#else
    return XCAM_RETURN_ERROR_UNKNOWN;
#endif
}

XCamReturn
rk_aiq_user_api2_awbV39_GetAllAttrib(const rk_aiq_sys_ctx_t* sys_ctx, rk_aiq_uapiV2_wbV39_attrib_t *attr)
{

#if RKAIQ_HAVE_AWB_V39

    RKAIQ_API_SMART_LOCK(sys_ctx);
    if (sys_ctx->cam_type == RK_AIQ_CAM_TYPE_GROUP) {
        LOGW_AWB("%s:bypass",__FUNCTION__);
        return XCAM_RETURN_NO_ERROR;//TODO
    } else {
        AiqAlgoHandlerAwb_t* algo_handle =
            (AiqAlgoHandlerAwb_t*)sys_ctx->_analyzer->mAlgoHandleMaps[RK_AIQ_ALGO_TYPE_AWB];

        if (algo_handle) {
            return AiqAlgoHandlerAwb_getWbV39Attrib(algo_handle, attr);
        }
         return XCAM_RETURN_NO_ERROR;
    }
#else
    return XCAM_RETURN_ERROR_UNKNOWN;
#endif
}



XCamReturn
rk_aiq_user_api2_awb_GetCCT(const rk_aiq_sys_ctx_t* sys_ctx, rk_aiq_wb_cct_t *cct)
{
    RKAIQ_API_SMART_LOCK(sys_ctx);

    if (sys_ctx->cam_type == RK_AIQ_CAM_TYPE_GROUP) {
#ifdef RKAIQ_ENABLE_CAMGROUP
        const rk_aiq_camgroup_ctx_t* camgroup_ctx = (rk_aiq_camgroup_ctx_t *)sys_ctx;
        AiqAlgoCamGroupAwbHandler_t* algo_handle =
            (AiqAlgoCamGroupAwbHandler_t*)camgroup_ctx->cam_group_manager->mDefAlgoHandleMap[RK_AIQ_ALGO_TYPE_AWB];

        if (algo_handle) {
            return AiqAlgoCamGroupAwbHandler_getCct(algo_handle, cct);
        } else {
			rk_aiq_sys_ctx_t* camCtx = NULL;
			for (int i = 0; i < RK_AIQ_CAM_GROUP_MAX_CAMS; i++) {
				camCtx = camgroup_ctx->cam_ctxs_array[i];
                if (!camCtx)
                    continue;

				AiqAlgoHandlerAwb_t* algo_handle =
					(AiqAlgoHandlerAwb_t*)camCtx->_analyzer->mAlgoHandleMaps[RK_AIQ_ALGO_TYPE_AWB];
                if (algo_handle)
					return AiqAlgoHandlerAwb_getCct(algo_handle, cct);
            }
        }
#else
        return XCAM_RETURN_ERROR_FAILED;
#endif
    } else {
        AiqAlgoHandlerAwb_t* algo_handle =
            (AiqAlgoHandlerAwb_t*)sys_ctx->_analyzer->mAlgoHandleMaps[RK_AIQ_ALGO_TYPE_AWB];

        if (algo_handle) {
            return AiqAlgoHandlerAwb_getCct(algo_handle, cct);
        }
    }
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
rk_aiq_user_api2_awb_QueryWBInfo(const rk_aiq_sys_ctx_t* sys_ctx, rk_aiq_wb_querry_info_t *wb_querry_info)
{
    RKAIQ_API_SMART_LOCK(sys_ctx);

    if (sys_ctx->cam_type == RK_AIQ_CAM_TYPE_GROUP) {
#ifdef RKAIQ_ENABLE_CAMGROUP
        const rk_aiq_camgroup_ctx_t* camgroup_ctx = (rk_aiq_camgroup_ctx_t *)sys_ctx;
        AiqAlgoCamGroupAwbHandler_t* algo_handle =
            (AiqAlgoCamGroupAwbHandler_t*)camgroup_ctx->cam_group_manager->mDefAlgoHandleMap[RK_AIQ_ALGO_TYPE_AWB];


        if (algo_handle) {
            return AiqAlgoCamGroupAwbHandler_queryWBInfo(algo_handle, wb_querry_info);
        } else {
			rk_aiq_sys_ctx_t* camCtx = NULL;
			for (int i = 0; i < RK_AIQ_CAM_GROUP_MAX_CAMS; i++) {
				camCtx = camgroup_ctx->cam_ctxs_array[i];
                if (!camCtx)
                    continue;

				AiqAlgoHandlerAwb_t* algo_handle =
					(AiqAlgoHandlerAwb_t*)camCtx->_analyzer->mAlgoHandleMaps[RK_AIQ_ALGO_TYPE_AWB];
                if (algo_handle)
					return AiqAlgoHandlerAwb_queryWBInfo(algo_handle, wb_querry_info);
            }
        }
#else
        return XCAM_RETURN_ERROR_FAILED;
#endif
    } else {
        AiqAlgoHandlerAwb_t* algo_handle =
            (AiqAlgoHandlerAwb_t*)sys_ctx->_analyzer->mAlgoHandleMaps[RK_AIQ_ALGO_TYPE_AWB];

        if (algo_handle) {
            return AiqAlgoHandlerAwb_queryWBInfo(algo_handle, wb_querry_info);
        }
    }
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
rk_aiq_user_api2_awb_Lock(const rk_aiq_sys_ctx_t* sys_ctx)
{
    RKAIQ_API_SMART_LOCK(sys_ctx);
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    if (sys_ctx->cam_type == RK_AIQ_CAM_TYPE_GROUP) {
#ifdef RKAIQ_ENABLE_CAMGROUP
        const rk_aiq_camgroup_ctx_t* camgroup_ctx = (rk_aiq_camgroup_ctx_t *)sys_ctx;
        AiqAlgoCamGroupAwbHandler_t* algo_handle =
            (AiqAlgoCamGroupAwbHandler_t*)camgroup_ctx->cam_group_manager->mDefAlgoHandleMap[RK_AIQ_ALGO_TYPE_AWB];


        if (algo_handle) {
            return AiqAlgoCamGroupAwbHandler_lock(algo_handle);
        } else {
			rk_aiq_sys_ctx_t* camCtx = NULL;
			for (int i = 0; i < RK_AIQ_CAM_GROUP_MAX_CAMS; i++) {
				camCtx = camgroup_ctx->cam_ctxs_array[i];
                if (!camCtx)
                    continue;

				AiqAlgoHandlerAwb_t* algo_handle =
					(AiqAlgoHandlerAwb_t*)camCtx->_analyzer->mAlgoHandleMaps[RK_AIQ_ALGO_TYPE_AWB];
                if (algo_handle)
					ret = AiqAlgoHandlerAwb_lock(algo_handle);
            }
        }
#else
        return XCAM_RETURN_ERROR_FAILED;
#endif
    } else {
        AiqAlgoHandlerAwb_t* algo_handle =
            (AiqAlgoHandlerAwb_t*)sys_ctx->_analyzer->mAlgoHandleMaps[RK_AIQ_ALGO_TYPE_AWB];

        if (algo_handle) {
            return AiqAlgoHandlerAwb_lock(algo_handle);
        }
    }
    return ret;
}

XCamReturn
rk_aiq_user_api2_awb_Unlock(const rk_aiq_sys_ctx_t* sys_ctx)
{
    RKAIQ_API_SMART_LOCK(sys_ctx);
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    if (sys_ctx->cam_type == RK_AIQ_CAM_TYPE_GROUP) {
#ifdef RKAIQ_ENABLE_CAMGROUP
        const rk_aiq_camgroup_ctx_t* camgroup_ctx = (rk_aiq_camgroup_ctx_t *)sys_ctx;
        AiqAlgoCamGroupAwbHandler_t* algo_handle =
            (AiqAlgoCamGroupAwbHandler_t*)camgroup_ctx->cam_group_manager->mDefAlgoHandleMap[RK_AIQ_ALGO_TYPE_AWB];


        if (algo_handle) {
            return AiqAlgoCamGroupAwbHandler_unlock(algo_handle);
        } else {
			rk_aiq_sys_ctx_t* camCtx = NULL;
			for (int i = 0; i < RK_AIQ_CAM_GROUP_MAX_CAMS; i++) {
				camCtx = camgroup_ctx->cam_ctxs_array[i];
                if (!camCtx)
                    continue;

				AiqAlgoHandlerAwb_t* algo_handle =
					(AiqAlgoHandlerAwb_t*)camCtx->_analyzer->mAlgoHandleMaps[RK_AIQ_ALGO_TYPE_AWB];
                if (algo_handle)
					ret = AiqAlgoHandlerAwb_unlock(algo_handle);
            }
        }
#else
        return XCAM_RETURN_ERROR_FAILED;
#endif
    } else {
        AiqAlgoHandlerAwb_t* algo_handle =
            (AiqAlgoHandlerAwb_t*)sys_ctx->_analyzer->mAlgoHandleMaps[RK_AIQ_ALGO_TYPE_AWB];

        if (algo_handle) {
            return AiqAlgoHandlerAwb_unlock(algo_handle);
        }
    }
    return ret;
}

XCamReturn
rk_aiq_user_api2_awb_SetWpModeAttrib(const rk_aiq_sys_ctx_t* sys_ctx, rk_aiq_uapiV2_wb_opMode_t attr)
{
    CHECK_USER_API_ENABLE2(sys_ctx);
    CHECK_USER_API_ENABLE(RK_AIQ_ALGO_TYPE_AWB);
    RKAIQ_API_SMART_LOCK(sys_ctx);
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    if (sys_ctx->cam_type == RK_AIQ_CAM_TYPE_GROUP) {
#ifdef RKAIQ_ENABLE_CAMGROUP
        const rk_aiq_camgroup_ctx_t* camgroup_ctx = (rk_aiq_camgroup_ctx_t *)sys_ctx;
        AiqAlgoCamGroupAwbHandler_t* algo_handle =
            (AiqAlgoCamGroupAwbHandler_t*)camgroup_ctx->cam_group_manager->mDefAlgoHandleMap[RK_AIQ_ALGO_TYPE_AWB];


        if (algo_handle) {
            return AiqAlgoCamGroupAwbHandler_setWbOpModeAttrib(algo_handle, attr);
        } else {
			rk_aiq_sys_ctx_t* camCtx = NULL;
			for (int i = 0; i < RK_AIQ_CAM_GROUP_MAX_CAMS; i++) {
				camCtx = camgroup_ctx->cam_ctxs_array[i];
                if (!camCtx)
                    continue;

				AiqAlgoHandlerAwb_t* algo_handle =
					(AiqAlgoHandlerAwb_t*)camCtx->_analyzer->mAlgoHandleMaps[RK_AIQ_ALGO_TYPE_AWB];
                if (algo_handle)
					ret = AiqAlgoHandlerAwb_setWbOpModeAttrib(algo_handle, attr);
            }
        }
#else
        return XCAM_RETURN_ERROR_FAILED;
#endif
    } else {
        AiqAlgoHandlerAwb_t* algo_handle =
            (AiqAlgoHandlerAwb_t*)sys_ctx->_analyzer->mAlgoHandleMaps[RK_AIQ_ALGO_TYPE_AWB];

        if (algo_handle) {
            return AiqAlgoHandlerAwb_setWbOpModeAttrib(algo_handle, attr);
        }
    }
    return ret;
}

XCamReturn
rk_aiq_user_api2_awb_GetWpModeAttrib(const rk_aiq_sys_ctx_t* sys_ctx, rk_aiq_uapiV2_wb_opMode_t *attr)
{
    RKAIQ_API_SMART_LOCK(sys_ctx);

    if (sys_ctx->cam_type == RK_AIQ_CAM_TYPE_GROUP) {
#ifdef RKAIQ_ENABLE_CAMGROUP
        const rk_aiq_camgroup_ctx_t* camgroup_ctx = (rk_aiq_camgroup_ctx_t *)sys_ctx;
        AiqAlgoCamGroupAwbHandler_t* algo_handle =
            (AiqAlgoCamGroupAwbHandler_t*)camgroup_ctx->cam_group_manager->mDefAlgoHandleMap[RK_AIQ_ALGO_TYPE_AWB];


        if (algo_handle) {
            return AiqAlgoCamGroupAwbHandler_getWbOpModeAttrib(algo_handle, attr);
        } else {
			rk_aiq_sys_ctx_t* camCtx = NULL;
			for (int i = 0; i < RK_AIQ_CAM_GROUP_MAX_CAMS; i++) {
				camCtx = camgroup_ctx->cam_ctxs_array[i];
                if (!camCtx)
                    continue;

				AiqAlgoHandlerAwb_t* algo_handle =
					(AiqAlgoHandlerAwb_t*)camCtx->_analyzer->mAlgoHandleMaps[RK_AIQ_ALGO_TYPE_AWB];
                if (algo_handle)
					return AiqAlgoHandlerAwb_getWbOpModeAttrib(algo_handle, attr);
            }
        }
#else
        return XCAM_RETURN_ERROR_FAILED;
#endif
    } else {
        AiqAlgoHandlerAwb_t* algo_handle =
            (AiqAlgoHandlerAwb_t*)sys_ctx->_analyzer->mAlgoHandleMaps[RK_AIQ_ALGO_TYPE_AWB];

        if (algo_handle) {
            return AiqAlgoHandlerAwb_getWbOpModeAttrib(algo_handle, attr);
        }
    }
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
rk_aiq_user_api2_awb_SetMwbAttrib(const rk_aiq_sys_ctx_t* sys_ctx, rk_aiq_wb_mwb_attrib_t attr)
{
    CHECK_USER_API_ENABLE2(sys_ctx);
    CHECK_USER_API_ENABLE(RK_AIQ_ALGO_TYPE_AWB);
    RKAIQ_API_SMART_LOCK(sys_ctx);
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    if (sys_ctx->cam_type == RK_AIQ_CAM_TYPE_GROUP) {
#ifdef RKAIQ_ENABLE_CAMGROUP
        const rk_aiq_camgroup_ctx_t* camgroup_ctx = (rk_aiq_camgroup_ctx_t *)sys_ctx;
        AiqAlgoCamGroupAwbHandler_t* algo_handle =
            (AiqAlgoCamGroupAwbHandler_t*)camgroup_ctx->cam_group_manager->mDefAlgoHandleMap[RK_AIQ_ALGO_TYPE_AWB];

        if (algo_handle) {
            return AiqAlgoCamGroupAwbHandler_setMwbAttrib(algo_handle, attr);
        } else {
			rk_aiq_sys_ctx_t* camCtx = NULL;
			for (int i = 0; i < RK_AIQ_CAM_GROUP_MAX_CAMS; i++) {
				camCtx = camgroup_ctx->cam_ctxs_array[i];
                if (!camCtx)
                    continue;

				AiqAlgoHandlerAwb_t* algo_handle =
					(AiqAlgoHandlerAwb_t*)camCtx->_analyzer->mAlgoHandleMaps[RK_AIQ_ALGO_TYPE_AWB];
                if (algo_handle)
					ret = AiqAlgoHandlerAwb_setMwbAttrib(algo_handle, attr);
            }
        }
#else
        return XCAM_RETURN_ERROR_FAILED;
#endif
    } else {
        AiqAlgoHandlerAwb_t* algo_handle =
            (AiqAlgoHandlerAwb_t*)sys_ctx->_analyzer->mAlgoHandleMaps[RK_AIQ_ALGO_TYPE_AWB];

        if (algo_handle) {
            return AiqAlgoHandlerAwb_setMwbAttrib(algo_handle, attr);
        }
    }
    return ret;
}

XCamReturn
rk_aiq_user_api2_awb_GetMwbAttrib(const rk_aiq_sys_ctx_t* sys_ctx, rk_aiq_wb_mwb_attrib_t *attr)
{
    RKAIQ_API_SMART_LOCK(sys_ctx);
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    if (sys_ctx->cam_type == RK_AIQ_CAM_TYPE_GROUP) {
#ifdef RKAIQ_ENABLE_CAMGROUP
        const rk_aiq_camgroup_ctx_t* camgroup_ctx = (rk_aiq_camgroup_ctx_t *)sys_ctx;
        AiqAlgoCamGroupAwbHandler_t* algo_handle =
            (AiqAlgoCamGroupAwbHandler_t*)camgroup_ctx->cam_group_manager->mDefAlgoHandleMap[RK_AIQ_ALGO_TYPE_AWB];

        if (algo_handle) {
            return AiqAlgoCamGroupAwbHandler_getMwbAttrib(algo_handle, attr);
        } else {
			rk_aiq_sys_ctx_t* camCtx = NULL;
			for (int i = 0; i < RK_AIQ_CAM_GROUP_MAX_CAMS; i++) {
				camCtx = camgroup_ctx->cam_ctxs_array[i];
                if (!camCtx)
                    continue;

				AiqAlgoHandlerAwb_t* algo_handle =
					(AiqAlgoHandlerAwb_t*)camCtx->_analyzer->mAlgoHandleMaps[RK_AIQ_ALGO_TYPE_AWB];
                if (algo_handle)
					ret = AiqAlgoHandlerAwb_getMwbAttrib(algo_handle, attr);
            }
        }
#else
        return XCAM_RETURN_ERROR_FAILED;
#endif
    } else {
        AiqAlgoHandlerAwb_t* algo_handle =
            (AiqAlgoHandlerAwb_t*)sys_ctx->_analyzer->mAlgoHandleMaps[RK_AIQ_ALGO_TYPE_AWB];

        if (algo_handle) {
            return AiqAlgoHandlerAwb_getMwbAttrib(algo_handle, attr);
        }
    }
    return ret;
}



XCamReturn
rk_aiq_user_api2_awb_SetWbGainAdjustAttrib(const rk_aiq_sys_ctx_t* sys_ctx, rk_aiq_uapiV2_wb_awb_wbGainAdjust_t attr)
{
    CHECK_USER_API_ENABLE2(sys_ctx);
    CHECK_USER_API_ENABLE(RK_AIQ_ALGO_TYPE_AWB);
    RKAIQ_API_SMART_LOCK(sys_ctx);
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    if (sys_ctx->cam_type == RK_AIQ_CAM_TYPE_GROUP) {
#ifdef RKAIQ_ENABLE_CAMGROUP
        const rk_aiq_camgroup_ctx_t* camgroup_ctx = (rk_aiq_camgroup_ctx_t *)sys_ctx;
        AiqAlgoCamGroupAwbHandler_t* algo_handle =
            (AiqAlgoCamGroupAwbHandler_t*)camgroup_ctx->cam_group_manager->mDefAlgoHandleMap[RK_AIQ_ALGO_TYPE_AWB];

        if (algo_handle) {
            return AiqAlgoCamGroupAwbHandler_setWbAwbWbGainAdjustAttrib(algo_handle, attr);
        } else {
			rk_aiq_sys_ctx_t* camCtx = NULL;
			for (int i = 0; i < RK_AIQ_CAM_GROUP_MAX_CAMS; i++) {
				camCtx = camgroup_ctx->cam_ctxs_array[i];
                if (!camCtx)
                    continue;

				AiqAlgoHandlerAwb_t* algo_handle =
					(AiqAlgoHandlerAwb_t*)camCtx->_analyzer->mAlgoHandleMaps[RK_AIQ_ALGO_TYPE_AWB];
                if (algo_handle)
					ret = AiqAlgoHandlerAwb_setWbAwbWbGainAdjustAttrib(algo_handle, attr);
            }
        }
#else
        return XCAM_RETURN_ERROR_FAILED;
#endif
    } else {
        AiqAlgoHandlerAwb_t* algo_handle =
            (AiqAlgoHandlerAwb_t*)sys_ctx->_analyzer->mAlgoHandleMaps[RK_AIQ_ALGO_TYPE_AWB];

        if (algo_handle) {
            return AiqAlgoHandlerAwb_setWbAwbWbGainAdjustAttrib(algo_handle, attr);
        }
    }
    return ret;
}

XCamReturn
rk_aiq_user_api2_awb_GetWbGainAdjustAttrib(const rk_aiq_sys_ctx_t* sys_ctx, rk_aiq_uapiV2_wb_awb_wbGainAdjust_t *attr)
{
    RKAIQ_API_SMART_LOCK(sys_ctx);

    if (sys_ctx->cam_type == RK_AIQ_CAM_TYPE_GROUP) {
#ifdef RKAIQ_ENABLE_CAMGROUP
        const rk_aiq_camgroup_ctx_t* camgroup_ctx = (rk_aiq_camgroup_ctx_t *)sys_ctx;
        AiqAlgoCamGroupAwbHandler_t* algo_handle =
            (AiqAlgoCamGroupAwbHandler_t*)camgroup_ctx->cam_group_manager->mDefAlgoHandleMap[RK_AIQ_ALGO_TYPE_AWB];

        if (algo_handle) {
            return AiqAlgoCamGroupAwbHandler_getWbAwbWbGainAdjustAttrib(algo_handle, attr);
        } else {
			rk_aiq_sys_ctx_t* camCtx = NULL;
			for (int i = 0; i < RK_AIQ_CAM_GROUP_MAX_CAMS; i++) {
				camCtx = camgroup_ctx->cam_ctxs_array[i];
                if (!camCtx)
                    continue;

				AiqAlgoHandlerAwb_t* algo_handle =
					(AiqAlgoHandlerAwb_t*)camCtx->_analyzer->mAlgoHandleMaps[RK_AIQ_ALGO_TYPE_AWB];
                if (algo_handle)
					return AiqAlgoHandlerAwb_getWbAwbWbGainAdjustAttrib(algo_handle, attr);
            }
        }
#else
        return XCAM_RETURN_ERROR_FAILED;
#endif
    } else {
        AiqAlgoHandlerAwb_t* algo_handle =
            (AiqAlgoHandlerAwb_t*)sys_ctx->_analyzer->mAlgoHandleMaps[RK_AIQ_ALGO_TYPE_AWB];

        if (algo_handle) {
            return AiqAlgoHandlerAwb_getWbAwbWbGainAdjustAttrib(algo_handle, attr);
        }
    }
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
rk_aiq_user_api2_awb_SetWbGainOffsetAttrib(const rk_aiq_sys_ctx_t* sys_ctx, rk_aiq_uapiV2_wb_awb_wbGainOffset_t attr)
{
    CHECK_USER_API_ENABLE2(sys_ctx);
    CHECK_USER_API_ENABLE(RK_AIQ_ALGO_TYPE_AWB);
    RKAIQ_API_SMART_LOCK(sys_ctx);
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    if (sys_ctx->cam_type == RK_AIQ_CAM_TYPE_GROUP) {
#ifdef RKAIQ_ENABLE_CAMGROUP
        const rk_aiq_camgroup_ctx_t* camgroup_ctx = (rk_aiq_camgroup_ctx_t *)sys_ctx;
        AiqAlgoCamGroupAwbHandler_t* algo_handle =
            (AiqAlgoCamGroupAwbHandler_t*)camgroup_ctx->cam_group_manager->mDefAlgoHandleMap[RK_AIQ_ALGO_TYPE_AWB];

        if (algo_handle) {
            return AiqAlgoCamGroupAwbHandler_setWbAwbWbGainOffsetAttrib(algo_handle, attr);
        } else {
			rk_aiq_sys_ctx_t* camCtx = NULL;
			for (int i = 0; i < RK_AIQ_CAM_GROUP_MAX_CAMS; i++) {
				camCtx = camgroup_ctx->cam_ctxs_array[i];
                if (!camCtx)
                    continue;

				AiqAlgoHandlerAwb_t* algo_handle =
					(AiqAlgoHandlerAwb_t*)camCtx->_analyzer->mAlgoHandleMaps[RK_AIQ_ALGO_TYPE_AWB];
                if (algo_handle)
					ret = AiqAlgoHandlerAwb_setWbAwbWbGainOffsetAttrib(algo_handle, attr);
            }
        }
#else
        return XCAM_RETURN_ERROR_FAILED;
#endif
    } else {
        AiqAlgoHandlerAwb_t* algo_handle =
            (AiqAlgoHandlerAwb_t*)sys_ctx->_analyzer->mAlgoHandleMaps[RK_AIQ_ALGO_TYPE_AWB];

        if (algo_handle) {
            return AiqAlgoHandlerAwb_setWbAwbWbGainOffsetAttrib(algo_handle, attr);
        }
    }
    return ret;
}

XCamReturn
rk_aiq_user_api2_awb_GetWbGainOffsetAttrib(const rk_aiq_sys_ctx_t* sys_ctx, rk_aiq_uapiV2_wb_awb_wbGainOffset_t *attr)
{
    RKAIQ_API_SMART_LOCK(sys_ctx);

    if (sys_ctx->cam_type == RK_AIQ_CAM_TYPE_GROUP) {
#ifdef RKAIQ_ENABLE_CAMGROUP
        const rk_aiq_camgroup_ctx_t* camgroup_ctx = (rk_aiq_camgroup_ctx_t *)sys_ctx;
        AiqAlgoCamGroupAwbHandler_t* algo_handle =
            (AiqAlgoCamGroupAwbHandler_t*)camgroup_ctx->cam_group_manager->mDefAlgoHandleMap[RK_AIQ_ALGO_TYPE_AWB];

        if (algo_handle) {
            return AiqAlgoCamGroupAwbHandler_getWbAwbWbGainOffsetAttrib(algo_handle, attr);
        } else {
			rk_aiq_sys_ctx_t* camCtx = NULL;
			for (int i = 0; i < RK_AIQ_CAM_GROUP_MAX_CAMS; i++) {
				camCtx = camgroup_ctx->cam_ctxs_array[i];
                if (!camCtx)
                    continue;

				AiqAlgoHandlerAwb_t* algo_handle =
					(AiqAlgoHandlerAwb_t*)camCtx->_analyzer->mAlgoHandleMaps[RK_AIQ_ALGO_TYPE_AWB];
                if (algo_handle)
					return AiqAlgoHandlerAwb_getWbAwbWbGainOffsetAttrib(algo_handle, attr);
            }
        }
#else
        return XCAM_RETURN_ERROR_FAILED;
#endif
    } else {
        AiqAlgoHandlerAwb_t* algo_handle =
            (AiqAlgoHandlerAwb_t*)sys_ctx->_analyzer->mAlgoHandleMaps[RK_AIQ_ALGO_TYPE_AWB];

        if (algo_handle) {
            return AiqAlgoHandlerAwb_getWbAwbWbGainOffsetAttrib(algo_handle, attr);
        }
    }
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
rk_aiq_user_api2_awb_SetMultiWindowAttrib(const rk_aiq_sys_ctx_t* sys_ctx, rk_aiq_uapiV2_wb_awb_mulWindow_t attr)
{
    return XCAM_RETURN_ERROR_UNKNOWN;
}

XCamReturn
rk_aiq_user_api2_awb_GetMultiWindowAttrib(const rk_aiq_sys_ctx_t* sys_ctx, rk_aiq_uapiV2_wb_awb_mulWindow_t *attr)
{
    return XCAM_RETURN_ERROR_UNKNOWN;
}

XCamReturn
rk_aiq_user_api2_awb_getStrategyResult(const rk_aiq_sys_ctx_t* sys_ctx, rk_tool_awb_strategy_result_t *attr)
{

    RKAIQ_API_SMART_LOCK(sys_ctx);

    if (sys_ctx->cam_type == RK_AIQ_CAM_TYPE_GROUP) {
#ifdef RKAIQ_ENABLE_CAMGROUP
        const rk_aiq_camgroup_ctx_t* camgroup_ctx = (rk_aiq_camgroup_ctx_t *)sys_ctx;
        AiqAlgoCamGroupAwbHandler_t* algo_handle =
            (AiqAlgoCamGroupAwbHandler_t*)camgroup_ctx->cam_group_manager->mDefAlgoHandleMap[RK_AIQ_ALGO_TYPE_AWB];

        if (algo_handle) {
            return AiqAlgoCamGroupAwbHandler_getStrategyResult(algo_handle, (rk_tool_awb_strategy_result_t*)attr);
        } else {
			rk_aiq_sys_ctx_t* camCtx = NULL;
			for (int i = 0; i < RK_AIQ_CAM_GROUP_MAX_CAMS; i++) {
				camCtx = camgroup_ctx->cam_ctxs_array[i];
                if (!camCtx)
                    continue;

				AiqAlgoHandlerAwb_t* algo_handle =
					(AiqAlgoHandlerAwb_t*)camCtx->_analyzer->mAlgoHandleMaps[RK_AIQ_ALGO_TYPE_AWB];
                if (algo_handle)
					return AiqAlgoHandlerAwb_getStrategyResult(algo_handle, (rk_tool_awb_strategy_result_t*)attr);
            }
        }
#else
        return XCAM_RETURN_ERROR_FAILED;
#endif
    } else {
        AiqAlgoHandlerAwb_t* algo_handle =
            (AiqAlgoHandlerAwb_t*)sys_ctx->_analyzer->mAlgoHandleMaps[RK_AIQ_ALGO_TYPE_AWB];

        if (algo_handle) {
            return AiqAlgoHandlerAwb_getStrategyResult(algo_handle, (rk_tool_awb_strategy_result_t*)attr);
        }
    }
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
rk_aiq_user_api2_awb_getAlgoSta(const rk_aiq_sys_ctx_t* sys_ctx, rk_tool_awb_stat_res_full_t *attr)
{

    RKAIQ_API_SMART_LOCK(sys_ctx);

    if (sys_ctx->cam_type == RK_AIQ_CAM_TYPE_GROUP) {
#ifdef RKAIQ_ENABLE_CAMGROUP
        const rk_aiq_camgroup_ctx_t* camgroup_ctx = (rk_aiq_camgroup_ctx_t *)sys_ctx;
        AiqAlgoCamGroupAwbHandler_t* algo_handle =
            (AiqAlgoCamGroupAwbHandler_t*)camgroup_ctx->cam_group_manager->mDefAlgoHandleMap[RK_AIQ_ALGO_TYPE_AWB];

        if (algo_handle) {
            return AiqAlgoCamGroupAwbHandler_getAlgoStat(algo_handle, (rk_tool_awb_stat_res_full_t*)attr);
        } else {
			rk_aiq_sys_ctx_t* camCtx = NULL;
			for (int i = 0; i < RK_AIQ_CAM_GROUP_MAX_CAMS; i++) {
				camCtx = camgroup_ctx->cam_ctxs_array[i];
                if (!camCtx)
                    continue;

				AiqAlgoHandlerAwb_t* algo_handle =
					(AiqAlgoHandlerAwb_t*)camCtx->_analyzer->mAlgoHandleMaps[RK_AIQ_ALGO_TYPE_AWB];
                if (algo_handle)
					return AiqAlgoHandlerAwb_getAlgoStat(algo_handle, (rk_tool_awb_stat_res_full_t*)attr);
            }
        }
#else
        return XCAM_RETURN_ERROR_FAILED;
#endif
    } else {
        AiqAlgoHandlerAwb_t* algo_handle =
            (AiqAlgoHandlerAwb_t*)sys_ctx->_analyzer->mAlgoHandleMaps[RK_AIQ_ALGO_TYPE_AWB];

        if (algo_handle) {
            return AiqAlgoHandlerAwb_getAlgoStat(algo_handle, (rk_tool_awb_stat_res_full_t*)attr);
        }
    }
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
rk_aiq_user_api2_awbV32_SetMultiWindowAttrib(const rk_aiq_sys_ctx_t* sys_ctx, rk_aiq_uapiV2_wbV32_awb_mulWindow_t attr)
{
    return XCAM_RETURN_ERROR_UNKNOWN;
}

XCamReturn
rk_aiq_user_api2_awbV32_GetMultiWindowAttrib(const rk_aiq_sys_ctx_t* sys_ctx, rk_aiq_uapiV2_wbV32_awb_mulWindow_t *attr)
{
    return XCAM_RETURN_ERROR_UNKNOWN;
}

XCamReturn
rk_aiq_user_api2_awbV39_SetMultiWindowAttrib(const rk_aiq_sys_ctx_t* sys_ctx, rk_aiq_uapiV2_wbV39_awb_nonROI_t attr)
{
#if RKAIQ_HAVE_AWB_V39

    CHECK_USER_API_ENABLE2(sys_ctx);
    CHECK_USER_API_ENABLE(RK_AIQ_ALGO_TYPE_AWB);
    RKAIQ_API_SMART_LOCK(sys_ctx);
    if (sys_ctx->cam_type == RK_AIQ_CAM_TYPE_GROUP) {
        LOGW_AWB("%s:bypass",__FUNCTION__);
        return XCAM_RETURN_NO_ERROR;//TODO
    } else {
        AiqAlgoHandlerAwb_t* algo_handle =
            (AiqAlgoHandlerAwb_t*)sys_ctx->_analyzer->mAlgoHandleMaps[RK_AIQ_ALGO_TYPE_AWB];

        if (algo_handle) {
            return AiqAlgoHandlerAwb_setWbV39AwbMultiWindowAttrib(algo_handle, attr);
        }
        return XCAM_RETURN_NO_ERROR;
    }
#else
    return XCAM_RETURN_ERROR_UNKNOWN;
#endif
}

XCamReturn
rk_aiq_user_api2_awbV39_GetMultiWindowAttrib(const rk_aiq_sys_ctx_t* sys_ctx, rk_aiq_uapiV2_wbV39_awb_nonROI_t *attr)
{

    RKAIQ_API_SMART_LOCK(sys_ctx);

    if (sys_ctx->cam_type == RK_AIQ_CAM_TYPE_GROUP) {
        LOGW_AWB("%s:bypass",__FUNCTION__);
        return XCAM_RETURN_NO_ERROR;//TODO
    } else {
        AiqAlgoHandlerAwb_t* algo_handle =
            (AiqAlgoHandlerAwb_t*)sys_ctx->_analyzer->mAlgoHandleMaps[RK_AIQ_ALGO_TYPE_AWB];

        if (algo_handle) {
            return AiqAlgoHandlerAwb_getWbV39AwbMultiWindowAttrib(algo_handle, attr);
        }
    }
    return XCAM_RETURN_NO_ERROR;
}



XCamReturn
rk_aiq_user_api2_awb_WriteAwbIn(const rk_aiq_sys_ctx_t* sys_ctx, rk_aiq_uapiV2_awb_wrtIn_attr_t attr)
{
#if RKAIQ_HAVE_AWB_V39

    CHECK_USER_API_ENABLE2(sys_ctx);
    CHECK_USER_API_ENABLE(RK_AIQ_ALGO_TYPE_AWB);
    RKAIQ_API_SMART_LOCK(sys_ctx);
    if (sys_ctx->cam_type == RK_AIQ_CAM_TYPE_GROUP) {
        LOGW_AWB("%s:bypass",__FUNCTION__);
        return XCAM_RETURN_NO_ERROR;//TODO
    } else {
        AiqAlgoHandlerAwb_t* algo_handle =
            (AiqAlgoHandlerAwb_t*)sys_ctx->_analyzer->mAlgoHandleMaps[RK_AIQ_ALGO_TYPE_AWB];

        if (algo_handle) {
            return AiqAlgoHandlerAwb_writeAwbIn(algo_handle, attr);
        }
        return XCAM_RETURN_NO_ERROR;
    }
#else
    return XCAM_RETURN_ERROR_UNKNOWN;
#endif
}

XCamReturn
rk_aiq_user_api2_awb_SetFFWbgainAttrib(const rk_aiq_sys_ctx_t* sys_ctx, rk_aiq_uapiV2_awb_ffwbgain_attr_t attr)
{
    CHECK_USER_API_ENABLE2(sys_ctx);
    CHECK_USER_API_ENABLE(RK_AIQ_ALGO_TYPE_AWB);
    RKAIQ_API_SMART_LOCK(sys_ctx);

    if (sys_ctx->cam_type == RK_AIQ_CAM_TYPE_GROUP) {
        return XCAM_RETURN_ERROR_FAILED;
    } else {
        AiqAlgoHandlerAwb_t* algo_handle =
            (AiqAlgoHandlerAwb_t*)sys_ctx->_analyzer->mAlgoHandleMaps[RK_AIQ_ALGO_TYPE_AWB];

        if (algo_handle) {
            return AiqAlgoHandlerAwb_setFFWbgainAttrib(algo_handle, attr);
        }
    }
    return XCAM_RETURN_NO_ERROR;
}


XCamReturn
rk_aiq_user_api2_awbV32_SetIQAutoExtPara(const rk_aiq_sys_ctx_t* sys_ctx, const rk_aiq_uapiV2_Wb_Awb_IqAtExtPa_V32_t* attr)
{
#if RKAIQ_HAVE_AWB_V39

    CHECK_USER_API_ENABLE2(sys_ctx);
    CHECK_USER_API_ENABLE(RK_AIQ_ALGO_TYPE_AWB);
    RKAIQ_API_SMART_LOCK(sys_ctx);
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    if (sys_ctx->cam_type == RK_AIQ_CAM_TYPE_GROUP) {
#ifdef RKAIQ_ENABLE_CAMGROUP
        const rk_aiq_camgroup_ctx_t* camgroup_ctx = (rk_aiq_camgroup_ctx_t *)sys_ctx;
        AiqAlgoCamGroupAwbHandler_t* algo_handle =
            (AiqAlgoCamGroupAwbHandler_t*)camgroup_ctx->cam_group_manager->mDefAlgoHandleMap[RK_AIQ_ALGO_TYPE_AWB];

        if (algo_handle) {
            return AiqAlgoCamGroupAwbHandler_setWbV32IQAutoExtPara(algo_handle, attr);
        } else {
			rk_aiq_sys_ctx_t* camCtx = NULL;
			for (int i = 0; i < RK_AIQ_CAM_GROUP_MAX_CAMS; i++) {
				camCtx = camgroup_ctx->cam_ctxs_array[i];
                if (!camCtx)
                    continue;

				AiqAlgoHandlerAwb_t* algo_handle =
					(AiqAlgoHandlerAwb_t*)camCtx->_analyzer->mAlgoHandleMaps[RK_AIQ_ALGO_TYPE_AWB];
                if (algo_handle)
					ret = AiqAlgoHandlerAwb_setWbV32IQAutoExtPara(algo_handle, attr);
            }
        }
#else
        return XCAM_RETURN_ERROR_FAILED;
#endif
    } else {
        AiqAlgoHandlerAwb_t* algo_handle =
            (AiqAlgoHandlerAwb_t*)sys_ctx->_analyzer->mAlgoHandleMaps[RK_AIQ_ALGO_TYPE_AWB];

        if (algo_handle) {
            return AiqAlgoHandlerAwb_setWbV32IQAutoExtPara(algo_handle, attr);
        }
    }
    return ret;
#else
    return XCAM_RETURN_ERROR_UNKNOWN;
#endif
}
XCamReturn
rk_aiq_user_api2_awbV32_GetIQAutoExtPara(const rk_aiq_sys_ctx_t* sys_ctx, rk_aiq_uapiV2_Wb_Awb_IqAtExtPa_V32_t *attr)
{
#if RKAIQ_HAVE_AWB_V39

    RKAIQ_API_SMART_LOCK(sys_ctx);

    if (sys_ctx->cam_type == RK_AIQ_CAM_TYPE_GROUP) {
#ifdef RKAIQ_ENABLE_CAMGROUP
        const rk_aiq_camgroup_ctx_t* camgroup_ctx = (rk_aiq_camgroup_ctx_t *)sys_ctx;
        AiqAlgoCamGroupAwbHandler_t* algo_handle =
            (AiqAlgoCamGroupAwbHandler_t*)camgroup_ctx->cam_group_manager->mDefAlgoHandleMap[RK_AIQ_ALGO_TYPE_AWB];

        if (algo_handle) {
            return AiqAlgoCamGroupAwbHandler_getWbV32IQAutoExtPara(algo_handle, attr);
        } else {
			rk_aiq_sys_ctx_t* camCtx = NULL;
			for (int i = 0; i < RK_AIQ_CAM_GROUP_MAX_CAMS; i++) {
				camCtx = camgroup_ctx->cam_ctxs_array[i];
                if (!camCtx)
                    continue;

				AiqAlgoHandlerAwb_t* algo_handle =
					(AiqAlgoHandlerAwb_t*)camCtx->_analyzer->mAlgoHandleMaps[RK_AIQ_ALGO_TYPE_AWB];
                if (algo_handle)
					return AiqAlgoHandlerAwb_getWbV32IQAutoExtPara(algo_handle, attr);
            }
        }
#else
        return XCAM_RETURN_ERROR_FAILED;
#endif
    } else {
        AiqAlgoHandlerAwb_t* algo_handle =
            (AiqAlgoHandlerAwb_t*)sys_ctx->_analyzer->mAlgoHandleMaps[RK_AIQ_ALGO_TYPE_AWB];

        if (algo_handle) {
            return AiqAlgoHandlerAwb_getWbV32IQAutoExtPara(algo_handle, attr);
        }
    }
    return XCAM_RETURN_NO_ERROR;
#else
    return XCAM_RETURN_ERROR_UNKNOWN;
#endif
}

XCamReturn
rk_aiq_user_api2_awbV32_SetIQAutoPara(const rk_aiq_sys_ctx_t* sys_ctx, const rk_aiq_uapiV2_Wb_Awb_IqAtPa_V32_t* attr)
{
#if RKAIQ_HAVE_AWB_V39

    CHECK_USER_API_ENABLE2(sys_ctx);
    CHECK_USER_API_ENABLE(RK_AIQ_ALGO_TYPE_AWB);
    RKAIQ_API_SMART_LOCK(sys_ctx);
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    if (sys_ctx->cam_type == RK_AIQ_CAM_TYPE_GROUP) {
#ifdef RKAIQ_ENABLE_CAMGROUP
        const rk_aiq_camgroup_ctx_t* camgroup_ctx = (rk_aiq_camgroup_ctx_t *)sys_ctx;
        AiqAlgoCamGroupAwbHandler_t* algo_handle =
            (AiqAlgoCamGroupAwbHandler_t*)camgroup_ctx->cam_group_manager->mDefAlgoHandleMap[RK_AIQ_ALGO_TYPE_AWB];

        if (algo_handle) {
            return AiqAlgoCamGroupAwbHandler_setWbV32IQAutoPara(algo_handle, attr);
        } else {
			rk_aiq_sys_ctx_t* camCtx = NULL;
			for (int i = 0; i < RK_AIQ_CAM_GROUP_MAX_CAMS; i++) {
				camCtx = camgroup_ctx->cam_ctxs_array[i];
                if (!camCtx)
                    continue;

				AiqAlgoHandlerAwb_t* algo_handle =
					(AiqAlgoHandlerAwb_t*)camCtx->_analyzer->mAlgoHandleMaps[RK_AIQ_ALGO_TYPE_AWB];
                if (algo_handle)
					ret = AiqAlgoHandlerAwb_setWbV32IQAutoPara(algo_handle, attr);
            }
        }
#else
        return XCAM_RETURN_ERROR_FAILED;
#endif
    } else {
        AiqAlgoHandlerAwb_t* algo_handle =
            (AiqAlgoHandlerAwb_t*)sys_ctx->_analyzer->mAlgoHandleMaps[RK_AIQ_ALGO_TYPE_AWB];

        if (algo_handle) {
            return AiqAlgoHandlerAwb_setWbV32IQAutoPara(algo_handle, attr);
        }
    }
    return ret;
#else
    return XCAM_RETURN_ERROR_UNKNOWN;
#endif
}

XCamReturn
rk_aiq_user_api2_awbV32_GetIQAutoPara(const rk_aiq_sys_ctx_t* sys_ctx, rk_aiq_uapiV2_Wb_Awb_IqAtPa_V32_t *attr)
{
#if RKAIQ_HAVE_AWB_V32||RKAIQ_HAVE_AWB_V39

    RKAIQ_API_SMART_LOCK(sys_ctx);

    if (sys_ctx->cam_type == RK_AIQ_CAM_TYPE_GROUP) {
#ifdef RKAIQ_ENABLE_CAMGROUP
        const rk_aiq_camgroup_ctx_t* camgroup_ctx = (rk_aiq_camgroup_ctx_t *)sys_ctx;
        AiqAlgoCamGroupAwbHandler_t* algo_handle =
            (AiqAlgoCamGroupAwbHandler_t*)camgroup_ctx->cam_group_manager->mDefAlgoHandleMap[RK_AIQ_ALGO_TYPE_AWB];

        if (algo_handle) {
            return AiqAlgoCamGroupAwbHandler_getWbV32IQAutoPara(algo_handle, attr);
        } else {
			rk_aiq_sys_ctx_t* camCtx = NULL;
			for (int i = 0; i < RK_AIQ_CAM_GROUP_MAX_CAMS; i++) {
				camCtx = camgroup_ctx->cam_ctxs_array[i];
                if (!camCtx)
                    continue;

				AiqAlgoHandlerAwb_t* algo_handle =
					(AiqAlgoHandlerAwb_t*)camCtx->_analyzer->mAlgoHandleMaps[RK_AIQ_ALGO_TYPE_AWB];
                if (algo_handle)
					return AiqAlgoHandlerAwb_getWbV32IQAutoPara(algo_handle, attr);
            }
        }
#else
        return XCAM_RETURN_ERROR_FAILED;
#endif
    } else {
        AiqAlgoHandlerAwb_t* algo_handle =
            (AiqAlgoHandlerAwb_t*)sys_ctx->_analyzer->mAlgoHandleMaps[RK_AIQ_ALGO_TYPE_AWB];

        if (algo_handle) {
            return AiqAlgoHandlerAwb_getWbV32IQAutoPara(algo_handle, attr);
        }
    }
    return XCAM_RETURN_NO_ERROR;
#else
    return XCAM_RETURN_ERROR_UNKNOWN;
#endif
}

XCamReturn
rk_aiq_user_api2_awb_IqMap2Main(const rk_aiq_sys_ctx_t* sys_ctx, rk_aiq_uapiV2_awb_Slave2Main_Cfg_t att)
{
#if RKAIQ_HAVE_AWB_V39
    //rk_aiq_uapiV2_awb_freeConvertLut(cct_lut_cfg);

    CHECK_USER_API_ENABLE2(sys_ctx);
    CHECK_USER_API_ENABLE(RK_AIQ_ALGO_TYPE_AWB);
    RKAIQ_API_SMART_LOCK(sys_ctx);
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    if (sys_ctx->cam_type == RK_AIQ_CAM_TYPE_GROUP) {
#ifdef RKAIQ_ENABLE_CAMGROUP
        //TODO
#else
        return XCAM_RETURN_ERROR_FAILED;
#endif
    } else {
        AiqAlgoHandlerAwb_t* algo_handle =
            (AiqAlgoHandlerAwb_t*)sys_ctx->_analyzer->mAlgoHandleMaps[RK_AIQ_ALGO_TYPE_AWB];

        if (algo_handle) {
            return AiqAlgoHandlerAwb_awbIqMap2Main(algo_handle, att);
        }
    }
    return ret;
#else
    return XCAM_RETURN_ERROR_UNKNOWN;
#endif
}

XCamReturn
rk_aiq_user_api2_awb_setAwbPreWbgain(const rk_aiq_sys_ctx_t* sys_ctx,  const float att[4])
{
#if RKAIQ_HAVE_AWB_V39

    CHECK_USER_API_ENABLE2(sys_ctx);
    CHECK_USER_API_ENABLE(RK_AIQ_ALGO_TYPE_AWB);
    RKAIQ_API_SMART_LOCK(sys_ctx);
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    if (sys_ctx->cam_type == RK_AIQ_CAM_TYPE_GROUP) {
#ifdef RKAIQ_ENABLE_CAMGROUP
        //TODO
#else
        return XCAM_RETURN_ERROR_FAILED;
#endif
    } else {
        AiqAlgoHandlerAwb_t* algo_handle =
            (AiqAlgoHandlerAwb_t*)sys_ctx->_analyzer->mAlgoHandleMaps[RK_AIQ_ALGO_TYPE_AWB];

        if (algo_handle) {
            return AiqAlgoHandlerAwb_setAwbPreWbgain(algo_handle, att);
        }
    }
    return ret;
#else
    return XCAM_RETURN_ERROR_UNKNOWN;
#endif
}

XCamReturn
rk_aiq_user_api2_awb_freeConvertLut(rk_aiq_uapiV2_awb_Gain_Lut_Cfg_t *cct_lut_cfg)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    ret = rk_aiq_uapiV2_awb_freeConvertLut(cct_lut_cfg);
    return ret;

}
XCamReturn
rk_aiq_user_api2_awb_loadConvertLut(rk_aiq_uapiV2_awb_Gain_Lut_Cfg_t *cct_lut_cfg, char *fullfile)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    ret = rk_aiq_uapiV2_awb_loadConvertLut(cct_lut_cfg, fullfile);
    return ret;

}
XCamReturn
rk_aiq_user_api2_awb_wbgainConvert2(rk_aiq_wb_gain_t  wbGainIn, rk_aiq_uapiV2_awb_Gain_Lut_Cfg_t *cct_lut_cfg, rk_aiq_wb_gain_t  *wbGainOut)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    ret = rk_aiq_uapiV2_awb_wbgainConvert2(wbGainIn, cct_lut_cfg, wbGainOut);
    return ret;

}

RKAIQ_END_DECLARE
