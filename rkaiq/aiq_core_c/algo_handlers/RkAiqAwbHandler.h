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
#ifndef _AIQ_ALGO_HANDLE_AWB_C_H_
#define _AIQ_ALGO_HANDLE_AWB_C_H_

#include "aiq_algo_handler.h"

RKAIQ_BEGIN_DECLARE

typedef struct AiqAlgoHandlerAwb_s {
    AiqAlgoHandler_t _base;
    uint32_t mWbGainSyncFlag;
    uint32_t mWbParamSyncFlag;
    AlgoRstShared_t* mProcResShared;
} AiqAlgoHandlerAwb_t;

AiqAlgoHandler_t* AiqAlgoHandlerAwb_constructor(RkAiqAlgoDesComm* des, AiqCore_t* aiqCore);

XCamReturn AiqAlgoHandlerAwb_setWbV39Attrib(AiqAlgoHandlerAwb_t* pAwbHdl, rk_aiq_uapiV2_wbV39_attrib_t att);
XCamReturn AiqAlgoHandlerAwb_getWbV39Attrib(AiqAlgoHandlerAwb_t* pAwbHdl, rk_aiq_uapiV2_wbV39_attrib_t* att);
XCamReturn AiqAlgoHandlerAwb_setWbV39AwbMultiWindowAttrib(AiqAlgoHandlerAwb_t* pAwbHdl, rk_aiq_uapiV2_wbV39_awb_nonROI_t att) ;
XCamReturn AiqAlgoHandlerAwb_getWbV39AwbMultiWindowAttrib(AiqAlgoHandlerAwb_t* pAwbHdl, rk_aiq_uapiV2_wbV39_awb_nonROI_t* att);
XCamReturn AiqAlgoHandlerAwb_writeAwbIn(AiqAlgoHandlerAwb_t* pAwbHdl, rk_aiq_uapiV2_awb_wrtIn_attr_t att);
XCamReturn AiqAlgoHandlerAwb_setWbV32IQAutoExtPara(AiqAlgoHandlerAwb_t* pAwbHdl, const rk_aiq_uapiV2_Wb_Awb_IqAtExtPa_V32_t* att);
XCamReturn AiqAlgoHandlerAwb_getWbV32IQAutoExtPara(AiqAlgoHandlerAwb_t* pAwbHdl, rk_aiq_uapiV2_Wb_Awb_IqAtExtPa_V32_t* att);
XCamReturn AiqAlgoHandlerAwb_setWbV32IQAutoPara(AiqAlgoHandlerAwb_t* pAwbHdl, const rk_aiq_uapiV2_Wb_Awb_IqAtPa_V32_t* att);
XCamReturn AiqAlgoHandlerAwb_getWbV32IQAutoPara(AiqAlgoHandlerAwb_t* pAwbHdl, rk_aiq_uapiV2_Wb_Awb_IqAtPa_V32_t* att);
XCamReturn AiqAlgoHandlerAwb_awbIqMap2Main(AiqAlgoHandlerAwb_t* pAwbHdl,  rk_aiq_uapiV2_awb_Slave2Main_Cfg_t att);
XCamReturn AiqAlgoHandlerAwb_setAwbPreWbgain(AiqAlgoHandlerAwb_t* pAwbHdl, const float att[4]);

XCamReturn AiqAlgoHandlerAwb_getCct(AiqAlgoHandlerAwb_t* pAwbHdl, rk_aiq_wb_cct_t* cct);
XCamReturn AiqAlgoHandlerAwb_queryWBInfo(AiqAlgoHandlerAwb_t* pAwbHdl, rk_aiq_wb_querry_info_t* wb_querry_info);
XCamReturn AiqAlgoHandlerAwb_lock(AiqAlgoHandlerAwb_t* pAwbHdl);
XCamReturn AiqAlgoHandlerAwb_unlock(AiqAlgoHandlerAwb_t* pAwbHdl);
XCamReturn AiqAlgoHandlerAwb_setWbOpModeAttrib(AiqAlgoHandlerAwb_t* pAwbHdl, rk_aiq_uapiV2_wb_opMode_t att);
XCamReturn AiqAlgoHandlerAwb_getWbOpModeAttrib(AiqAlgoHandlerAwb_t* pAwbHdl, rk_aiq_uapiV2_wb_opMode_t* att);
XCamReturn AiqAlgoHandlerAwb_setMwbAttrib(AiqAlgoHandlerAwb_t* pAwbHdl, rk_aiq_wb_mwb_attrib_t att);
XCamReturn AiqAlgoHandlerAwb_getMwbAttrib(AiqAlgoHandlerAwb_t* pAwbHdl, rk_aiq_wb_mwb_attrib_t* att);
XCamReturn AiqAlgoHandlerAwb_setWbAwbWbGainAdjustAttrib(AiqAlgoHandlerAwb_t* pAwbHdl, rk_aiq_uapiV2_wb_awb_wbGainAdjust_t att);
XCamReturn AiqAlgoHandlerAwb_getWbAwbWbGainAdjustAttrib(AiqAlgoHandlerAwb_t* pAwbHdl, rk_aiq_uapiV2_wb_awb_wbGainAdjust_t* att);
XCamReturn AiqAlgoHandlerAwb_setWbAwbWbGainOffsetAttrib(AiqAlgoHandlerAwb_t* pAwbHdl, rk_aiq_uapiV2_wb_awb_wbGainOffset_t att);
XCamReturn AiqAlgoHandlerAwb_getWbAwbWbGainOffsetAttrib(AiqAlgoHandlerAwb_t* pAwbHdl, rk_aiq_uapiV2_wb_awb_wbGainOffset_t* att);
XCamReturn AiqAlgoHandlerAwb_setWbAwbMultiWindowAttrib(AiqAlgoHandlerAwb_t* pAwbHdl, rk_aiq_uapiV2_wb_awb_mulWindow_t att);
XCamReturn AiqAlgoHandlerAwb_getWbAwbMultiWindowAttrib(AiqAlgoHandlerAwb_t* pAwbHdl, rk_aiq_uapiV2_wb_awb_mulWindow_t* att);
XCamReturn AiqAlgoHandlerAwb_setFFWbgainAttrib(AiqAlgoHandlerAwb_t* pAwbHdl, rk_aiq_uapiV2_awb_ffwbgain_attr_t att);
XCamReturn AiqAlgoHandlerAwb_getAlgoStat(AiqAlgoHandlerAwb_t* pAwbHdl, rk_tool_awb_stat_res_full_t *awb_stat_algo);
XCamReturn AiqAlgoHandlerAwb_getStrategyResult(AiqAlgoHandlerAwb_t* pAwbHdl, rk_tool_awb_strategy_result_t *awb_strategy_result);

RKAIQ_END_DECLARE

#endif
