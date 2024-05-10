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

#ifndef _AIQ_ALGO_CAMGROUP_AWB_HANDLE_H_
#define _AIQ_ALGO_CAMGROUP_AWB_HANDLE_H_

#include "aiq_algo_camgroup_handler.h"
#include "awb/rk_aiq_uapiv2_camgroup_awb_int.h"

RKAIQ_BEGIN_DECLARE

typedef AiqAlgoCamGroupHandler_t AiqAlgoCamGroupAwbHandler_t;

XCamReturn AiqAlgoCamGroupAwbHandler_setWbV21Attrib(AiqAlgoCamGroupAwbHandler_t* pAwbHdl, rk_aiq_uapiV2_wbV21_attrib_t att);
XCamReturn AiqAlgoCamGroupAwbHandler_getWbV21Attrib(AiqAlgoCamGroupAwbHandler_t* pAwbHdl, rk_aiq_uapiV2_wbV21_attrib_t *att);
XCamReturn AiqAlgoCamGroupAwbHandler_getCct(AiqAlgoCamGroupAwbHandler_t* pAwbHdl, rk_aiq_wb_cct_t *cct);
XCamReturn AiqAlgoCamGroupAwbHandler_queryWBInfo(AiqAlgoCamGroupAwbHandler_t* pAwbHdl, rk_aiq_wb_querry_info_t *wb_querry_info );
XCamReturn AiqAlgoCamGroupAwbHandler_lock(AiqAlgoCamGroupAwbHandler_t* pAwbHdl);
XCamReturn AiqAlgoCamGroupAwbHandler_unlock(AiqAlgoCamGroupAwbHandler_t* pAwbHdl);
XCamReturn AiqAlgoCamGroupAwbHandler_setWbOpModeAttrib(AiqAlgoCamGroupAwbHandler_t* pAwbHdl, rk_aiq_uapiV2_wb_opMode_t att);
XCamReturn AiqAlgoCamGroupAwbHandler_getWbOpModeAttrib(AiqAlgoCamGroupAwbHandler_t* pAwbHdl, rk_aiq_uapiV2_wb_opMode_t *att);
XCamReturn AiqAlgoCamGroupAwbHandler_setMwbAttrib(AiqAlgoCamGroupAwbHandler_t* pAwbHdl, rk_aiq_wb_mwb_attrib_t att);
XCamReturn AiqAlgoCamGroupAwbHandler_getMwbAttrib(AiqAlgoCamGroupAwbHandler_t* pAwbHdl, rk_aiq_wb_mwb_attrib_t *att);
XCamReturn AiqAlgoCamGroupAwbHandler_setWbAwbWbGainAdjustAttrib(AiqAlgoCamGroupAwbHandler_t* pAwbHdl, rk_aiq_uapiV2_wb_awb_wbGainAdjust_t att);
XCamReturn AiqAlgoCamGroupAwbHandler_getWbAwbWbGainAdjustAttrib(AiqAlgoCamGroupAwbHandler_t* pAwbHdl, rk_aiq_uapiV2_wb_awb_wbGainAdjust_t *att);
XCamReturn AiqAlgoCamGroupAwbHandler_setWbAwbWbGainOffsetAttrib(AiqAlgoCamGroupAwbHandler_t* pAwbHdl, rk_aiq_uapiV2_wb_awb_wbGainOffset_t att);
XCamReturn AiqAlgoCamGroupAwbHandler_getWbAwbWbGainOffsetAttrib(AiqAlgoCamGroupAwbHandler_t* pAwbHdl, rk_aiq_uapiV2_wb_awb_wbGainOffset_t *att);
XCamReturn AiqAlgoCamGroupAwbHandler_setWbAwbMultiWindowAttrib(AiqAlgoCamGroupAwbHandler_t* pAwbHdl, rk_aiq_uapiV2_wb_awb_mulWindow_t att);
XCamReturn AiqAlgoCamGroupAwbHandler_getWbAwbMultiWindowAttrib(AiqAlgoCamGroupAwbHandler_t* pAwbHdl, rk_aiq_uapiV2_wb_awb_mulWindow_t *att);
XCamReturn AiqAlgoCamGroupAwbHandler_getAlgoStat(AiqAlgoCamGroupAwbHandler_t* pAwbHdl, rk_tool_awb_stat_res_full_t *awb_stat_algo);
XCamReturn AiqAlgoCamGroupAwbHandler_getStrategyResult(AiqAlgoCamGroupAwbHandler_t* pAwbHdl, rk_tool_awb_strategy_result_t *awb_strategy_result);
//isp32
XCamReturn AiqAlgoCamGroupAwbHandler_setWbV32Attrib(AiqAlgoCamGroupAwbHandler_t* pAwbHdl, rk_aiq_uapiV2_wbV32_attrib_t att);
XCamReturn AiqAlgoCamGroupAwbHandler_getWbV32Attrib(AiqAlgoCamGroupAwbHandler_t* pAwbHdl, rk_aiq_uapiV2_wbV32_attrib_t* att);
XCamReturn AiqAlgoCamGroupAwbHandler_setWbV32AwbMultiWindowAttrib(AiqAlgoCamGroupAwbHandler_t* pAwbHdl, rk_aiq_uapiV2_wbV32_awb_mulWindow_t att);
XCamReturn AiqAlgoCamGroupAwbHandler_getWbV32AwbMultiWindowAttrib(AiqAlgoCamGroupAwbHandler_t* pAwbHdl, rk_aiq_uapiV2_wbV32_awb_mulWindow_t* att);
XCamReturn AiqAlgoCamGroupAwbHandler_setWbV32IQAutoExtPara(AiqAlgoCamGroupAwbHandler_t* pAwbHdl, const rk_aiq_uapiV2_Wb_Awb_IqAtExtPa_V32_t* att);
XCamReturn AiqAlgoCamGroupAwbHandler_getWbV32IQAutoExtPara(AiqAlgoCamGroupAwbHandler_t* pAwbHdl, rk_aiq_uapiV2_Wb_Awb_IqAtExtPa_V32_t* att);
XCamReturn AiqAlgoCamGroupAwbHandler_setWbV32IQAutoPara(AiqAlgoCamGroupAwbHandler_t* pAwbHdl, const rk_aiq_uapiV2_Wb_Awb_IqAtPa_V32_t* att);
XCamReturn AiqAlgoCamGroupAwbHandler_getWbV32IQAutoPara(AiqAlgoCamGroupAwbHandler_t* pAwbHdl, rk_aiq_uapiV2_Wb_Awb_IqAtPa_V32_t* att);

RKAIQ_END_DECLARE

#endif
