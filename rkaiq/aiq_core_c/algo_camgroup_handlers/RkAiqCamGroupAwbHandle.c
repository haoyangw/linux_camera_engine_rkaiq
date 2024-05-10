
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

#include "RkAiqCamGroupAwbHandle.h"

XCamReturn AiqAlgoCamGroupAwbHandler_setWbV21Attrib(AiqAlgoCamGroupAwbHandler_t* pAwbHdl, rk_aiq_uapiV2_wbV21_attrib_t att)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
	AiqAlgoCamGroupHandler_t* pHdl = (AiqAlgoCamGroupHandler_t*)pAwbHdl;
	aiqMutex_lock(&pHdl->mCfgMutex);
	ret = rk_aiq_uapiV2_camgroup_awbV21_SetAttrib(pHdl->mAlgoCtx, &att, false);
	aiqMutex_unlock(&pHdl->mCfgMutex);
    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn AiqAlgoCamGroupAwbHandler_getWbV21Attrib(AiqAlgoCamGroupAwbHandler_t* pAwbHdl, rk_aiq_uapiV2_wbV21_attrib_t *att)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
	AiqAlgoCamGroupHandler_t* pHdl = (AiqAlgoCamGroupHandler_t*)pAwbHdl;
	aiqMutex_lock(&pHdl->mCfgMutex);
	ret = rk_aiq_uapiV2_camgroup_awbV21_GetAttrib(pHdl->mAlgoCtx, att);
	aiqMutex_unlock(&pHdl->mCfgMutex);
    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn AiqAlgoCamGroupAwbHandler_getCct(AiqAlgoCamGroupAwbHandler_t* pAwbHdl, rk_aiq_wb_cct_t *cct)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
	AiqAlgoCamGroupHandler_t* pHdl = (AiqAlgoCamGroupHandler_t*)pAwbHdl;
	aiqMutex_lock(&pHdl->mCfgMutex);
	ret = rk_aiq_uapiV2_camgroup_awb_GetCCT(pHdl->mAlgoCtx, cct);
	aiqMutex_unlock(&pHdl->mCfgMutex);
    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn AiqAlgoCamGroupAwbHandler_queryWBInfo(AiqAlgoCamGroupAwbHandler_t* pAwbHdl, rk_aiq_wb_querry_info_t *wb_querry_info )
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
	AiqAlgoCamGroupHandler_t* pHdl = (AiqAlgoCamGroupHandler_t*)pAwbHdl;
	aiqMutex_lock(&pHdl->mCfgMutex);
	ret = rk_aiq_uapiV2_camgroup_awb_QueryWBInfo(pHdl->mAlgoCtx, wb_querry_info);
	aiqMutex_unlock(&pHdl->mCfgMutex);
    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn AiqAlgoCamGroupAwbHandler_lock(AiqAlgoCamGroupAwbHandler_t* pAwbHdl)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
	AiqAlgoCamGroupHandler_t* pHdl = (AiqAlgoCamGroupHandler_t*)pAwbHdl;
	aiqMutex_lock(&pHdl->mCfgMutex);
	ret = rk_aiq_uapiV2_camgroup_awb_Lock(pHdl->mAlgoCtx);
	aiqMutex_unlock(&pHdl->mCfgMutex);
    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn AiqAlgoCamGroupAwbHandler_unlock(AiqAlgoCamGroupAwbHandler_t* pAwbHdl)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
	AiqAlgoCamGroupHandler_t* pHdl = (AiqAlgoCamGroupHandler_t*)pAwbHdl;
	aiqMutex_lock(&pHdl->mCfgMutex);
	ret = rk_aiq_uapiV2_camgroup_awb_Unlock(pHdl->mAlgoCtx);
	aiqMutex_unlock(&pHdl->mCfgMutex);
    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn AiqAlgoCamGroupAwbHandler_setWbOpModeAttrib(AiqAlgoCamGroupAwbHandler_t* pAwbHdl, rk_aiq_uapiV2_wb_opMode_t att)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
	AiqAlgoCamGroupHandler_t* pHdl = (AiqAlgoCamGroupHandler_t*)pAwbHdl;
	aiqMutex_lock(&pHdl->mCfgMutex);
	ret = rk_aiq_uapiV2_camgroup_awb_SetMwbMode(pHdl->mAlgoCtx, &att.mode, false);
	aiqMutex_unlock(&pHdl->mCfgMutex);
    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn AiqAlgoCamGroupAwbHandler_getWbOpModeAttrib(AiqAlgoCamGroupAwbHandler_t* pAwbHdl, rk_aiq_uapiV2_wb_opMode_t *att)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
	AiqAlgoCamGroupHandler_t* pHdl = (AiqAlgoCamGroupHandler_t*)pAwbHdl;
	aiqMutex_lock(&pHdl->mCfgMutex);
	ret = rk_aiq_uapiV2_camgroup_awb_GetMwbMode(pHdl->mAlgoCtx, &att->mode);
	aiqMutex_unlock(&pHdl->mCfgMutex);
    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn AiqAlgoCamGroupAwbHandler_setMwbAttrib(AiqAlgoCamGroupAwbHandler_t* pAwbHdl, rk_aiq_wb_mwb_attrib_t att)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
	AiqAlgoCamGroupHandler_t* pHdl = (AiqAlgoCamGroupHandler_t*)pAwbHdl;
	aiqMutex_lock(&pHdl->mCfgMutex);
    ret = rk_aiq_uapiV2_camgroup_awb_SetMwbAttrib(pHdl->mAlgoCtx, &att, false);
	aiqMutex_unlock(&pHdl->mCfgMutex);
    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn AiqAlgoCamGroupAwbHandler_getMwbAttrib(AiqAlgoCamGroupAwbHandler_t* pAwbHdl, rk_aiq_wb_mwb_attrib_t *att)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
	AiqAlgoCamGroupHandler_t* pHdl = (AiqAlgoCamGroupHandler_t*)pAwbHdl;
	aiqMutex_lock(&pHdl->mCfgMutex);
    ret = rk_aiq_uapiV2_camgroup_awb_GetMwbAttrib(pHdl->mAlgoCtx, att);
	aiqMutex_unlock(&pHdl->mCfgMutex);
    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn AiqAlgoCamGroupAwbHandler_setWbAwbWbGainAdjustAttrib(AiqAlgoCamGroupAwbHandler_t* pAwbHdl, rk_aiq_uapiV2_wb_awb_wbGainAdjust_t att)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
	AiqAlgoCamGroupHandler_t* pHdl = (AiqAlgoCamGroupHandler_t*)pAwbHdl;
	aiqMutex_lock(&pHdl->mCfgMutex);
	ret = rk_aiq_uapiV2_camgroup_awb_SetAwbGainAdjust(pHdl->mAlgoCtx, &att, false);
	aiqMutex_unlock(&pHdl->mCfgMutex);
    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn AiqAlgoCamGroupAwbHandler_getWbAwbWbGainAdjustAttrib(AiqAlgoCamGroupAwbHandler_t* pAwbHdl, rk_aiq_uapiV2_wb_awb_wbGainAdjust_t *att)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
	AiqAlgoCamGroupHandler_t* pHdl = (AiqAlgoCamGroupHandler_t*)pAwbHdl;
	aiqMutex_lock(&pHdl->mCfgMutex);
	ret = rk_aiq_uapiV2_camgroup_awb_GetAwbGainAdjust(pHdl->mAlgoCtx, att);
	aiqMutex_unlock(&pHdl->mCfgMutex);
    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn AiqAlgoCamGroupAwbHandler_setWbAwbWbGainOffsetAttrib(AiqAlgoCamGroupAwbHandler_t* pAwbHdl, rk_aiq_uapiV2_wb_awb_wbGainOffset_t att)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
	AiqAlgoCamGroupHandler_t* pHdl = (AiqAlgoCamGroupHandler_t*)pAwbHdl;
	aiqMutex_lock(&pHdl->mCfgMutex);
	ret = rk_aiq_uapiV2_camgroup_awb_SetAwbGainOffset(pHdl->mAlgoCtx, att.gainOffset, false);
	aiqMutex_unlock(&pHdl->mCfgMutex);
    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn AiqAlgoCamGroupAwbHandler_getWbAwbWbGainOffsetAttrib(AiqAlgoCamGroupAwbHandler_t* pAwbHdl, rk_aiq_uapiV2_wb_awb_wbGainOffset_t *att)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
	AiqAlgoCamGroupHandler_t* pHdl = (AiqAlgoCamGroupHandler_t*)pAwbHdl;
	aiqMutex_lock(&pHdl->mCfgMutex);
	ret = rk_aiq_uapiV2_camgroup_awb_GetAwbGainOffset(pHdl->mAlgoCtx, &att->gainOffset);
	aiqMutex_unlock(&pHdl->mCfgMutex);
    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn AiqAlgoCamGroupAwbHandler_setWbAwbMultiWindowAttrib(AiqAlgoCamGroupAwbHandler_t* pAwbHdl, rk_aiq_uapiV2_wb_awb_mulWindow_t att)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
	AiqAlgoCamGroupHandler_t* pHdl = (AiqAlgoCamGroupHandler_t*)pAwbHdl;
	aiqMutex_lock(&pHdl->mCfgMutex);
	ret = rk_aiq_uapiV2_camgroup_awb_SetAwbMultiwindow(pHdl->mAlgoCtx, &att.multiWindw, false);
	aiqMutex_unlock(&pHdl->mCfgMutex);
    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn AiqAlgoCamGroupAwbHandler_getWbAwbMultiWindowAttrib(AiqAlgoCamGroupAwbHandler_t* pAwbHdl, rk_aiq_uapiV2_wb_awb_mulWindow_t *att)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
	AiqAlgoCamGroupHandler_t* pHdl = (AiqAlgoCamGroupHandler_t*)pAwbHdl;
	aiqMutex_lock(&pHdl->mCfgMutex);
	ret = rk_aiq_uapiV2_camgroup_awb_GetAwbMultiwindow(pHdl->mAlgoCtx, &att->multiWindw);
	aiqMutex_unlock(&pHdl->mCfgMutex);
    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn AiqAlgoCamGroupAwbHandler_getAlgoStat(AiqAlgoCamGroupAwbHandler_t* pAwbHdl, rk_tool_awb_stat_res_full_t *awb_stat_algo)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
	AiqAlgoCamGroupHandler_t* pHdl = (AiqAlgoCamGroupHandler_t*)pAwbHdl;
	aiqMutex_lock(&pHdl->mCfgMutex);
	ret = rk_aiq_uapiV2_camgroup_awb_GetAlgoStat(pHdl->mAlgoCtx, awb_stat_algo);
	aiqMutex_unlock(&pHdl->mCfgMutex);
    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn AiqAlgoCamGroupAwbHandler_getStrategyResult(AiqAlgoCamGroupAwbHandler_t* pAwbHdl, rk_tool_awb_strategy_result_t *awb_strategy_result)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
	AiqAlgoCamGroupHandler_t* pHdl = (AiqAlgoCamGroupHandler_t*)pAwbHdl;
	aiqMutex_lock(&pHdl->mCfgMutex);
	ret = rk_aiq_uapiV2_camgroup_awb_GetStrategyResult(pHdl->mAlgoCtx, awb_strategy_result);
	aiqMutex_unlock(&pHdl->mCfgMutex);
    EXIT_ANALYZER_FUNCTION();
    return ret;
}

//isp32
XCamReturn AiqAlgoCamGroupAwbHandler_setWbV32Attrib(AiqAlgoCamGroupAwbHandler_t* pAwbHdl, rk_aiq_uapiV2_wbV32_attrib_t att)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
	AiqAlgoCamGroupHandler_t* pHdl = (AiqAlgoCamGroupHandler_t*)pAwbHdl;
	aiqMutex_lock(&pHdl->mCfgMutex);
	ret = rk_aiq_uapiV2_camgroup_awbV32_SetAttrib(pHdl->mAlgoCtx, &att, false);
	aiqMutex_unlock(&pHdl->mCfgMutex);
    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn AiqAlgoCamGroupAwbHandler_getWbV32Attrib(AiqAlgoCamGroupAwbHandler_t* pAwbHdl, rk_aiq_uapiV2_wbV32_attrib_t* att)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
	AiqAlgoCamGroupHandler_t* pHdl = (AiqAlgoCamGroupHandler_t*)pAwbHdl;
	aiqMutex_lock(&pHdl->mCfgMutex);
	ret = rk_aiq_uapiV2_camgroup_awbV32_GetAttrib(pHdl->mAlgoCtx, att);
	aiqMutex_unlock(&pHdl->mCfgMutex);
    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn AiqAlgoCamGroupAwbHandler_setWbV32AwbMultiWindowAttrib(AiqAlgoCamGroupAwbHandler_t* pAwbHdl, rk_aiq_uapiV2_wbV32_awb_mulWindow_t att)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
	AiqAlgoCamGroupHandler_t* pHdl = (AiqAlgoCamGroupHandler_t*)pAwbHdl;
	aiqMutex_lock(&pHdl->mCfgMutex);
	ret = rk_aiq_uapiV2_camgroup_awbV32_SetAwbMultiwindow(pHdl->mAlgoCtx, &att, false);
	aiqMutex_unlock(&pHdl->mCfgMutex);
    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn AiqAlgoCamGroupAwbHandler_getWbV32AwbMultiWindowAttrib(AiqAlgoCamGroupAwbHandler_t* pAwbHdl, rk_aiq_uapiV2_wbV32_awb_mulWindow_t* att)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
	AiqAlgoCamGroupHandler_t* pHdl = (AiqAlgoCamGroupHandler_t*)pAwbHdl;
	aiqMutex_lock(&pHdl->mCfgMutex);
	ret = rk_aiq_uapiV2_camgroup_awbV32_GetAwbMultiwindow(pHdl->mAlgoCtx, att);
	aiqMutex_unlock(&pHdl->mCfgMutex);
    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn AiqAlgoCamGroupAwbHandler_setWbV32IQAutoExtPara(AiqAlgoCamGroupAwbHandler_t* pAwbHdl, const rk_aiq_uapiV2_Wb_Awb_IqAtExtPa_V32_t* att)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
	AiqAlgoCamGroupHandler_t* pHdl = (AiqAlgoCamGroupHandler_t*)pAwbHdl;
	aiqMutex_lock(&pHdl->mCfgMutex);
	ret = rk_aiq_uapiV2_camgroup_awb_SetIQAutoExtPara(pHdl->mAlgoCtx, att, false);
	aiqMutex_unlock(&pHdl->mCfgMutex);
    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn AiqAlgoCamGroupAwbHandler_getWbV32IQAutoExtPara(AiqAlgoCamGroupAwbHandler_t* pAwbHdl, rk_aiq_uapiV2_Wb_Awb_IqAtExtPa_V32_t* att)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
	AiqAlgoCamGroupHandler_t* pHdl = (AiqAlgoCamGroupHandler_t*)pAwbHdl;
	aiqMutex_lock(&pHdl->mCfgMutex);
	ret = rk_aiq_uapiV2_camgroup_awb_GetIQAutoExtPara(pHdl->mAlgoCtx, att);
	aiqMutex_unlock(&pHdl->mCfgMutex);
    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn AiqAlgoCamGroupAwbHandler_setWbV32IQAutoPara(AiqAlgoCamGroupAwbHandler_t* pAwbHdl, const rk_aiq_uapiV2_Wb_Awb_IqAtPa_V32_t* att)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
	AiqAlgoCamGroupHandler_t* pHdl = (AiqAlgoCamGroupHandler_t*)pAwbHdl;
	aiqMutex_lock(&pHdl->mCfgMutex);
	ret = rk_aiq_uapiV2_camgroup_awb_SetIQAutoPara(pHdl->mAlgoCtx, att, false);
	aiqMutex_unlock(&pHdl->mCfgMutex);
    EXIT_ANALYZER_FUNCTION();
    return ret;
}

XCamReturn AiqAlgoCamGroupAwbHandler_getWbV32IQAutoPara(AiqAlgoCamGroupAwbHandler_t* pAwbHdl, rk_aiq_uapiV2_Wb_Awb_IqAtPa_V32_t* att)
{
    ENTER_ANALYZER_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
	AiqAlgoCamGroupHandler_t* pHdl = (AiqAlgoCamGroupHandler_t*)pAwbHdl;
	aiqMutex_lock(&pHdl->mCfgMutex);
	ret = rk_aiq_uapiV2_camgroup_awb_GetIQAutoPara(pHdl->mAlgoCtx, att);
	aiqMutex_unlock(&pHdl->mCfgMutex);
    EXIT_ANALYZER_FUNCTION();
    return ret;
}
