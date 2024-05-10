/*
* demosaic_types_prvt.h

* for rockchip v2.0.0
*
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
/* for rockchip v2.0.0*/

#ifndef __RKAIQ_TYPES_Drc_ALGO_PRVT_H__
#define __RKAIQ_TYPES_Drc_ALGO_PRVT_H__

/**
 * @file drc_types_prvt.h
 *
 * @brief
 *
 *****************************************************************************/
/**
 * @page module_name_page Module Name
 * Describe here what this module does.
 *
 * For a detailed list of functions and implementation detail refer to:
 * - @ref module_name
 *
 * @defgroup Drc Auto drc
 * @{
 *
 */
#include "include/drc_algo_api.h"
#include "RkAiqCalibDbTypes.h"
#include "RkAiqCalibDbTypesV2.h"
#include "RkAiqCalibDbV2Helper.h"
#include "xcam_log.h"

#define DEFAULT_RECALCULATE_DELTA_ISO (0.01)
#define LONG_FRAME_MODE_RATIO (1.0f)
#define RATIO_DEFAULT         (1.0f)
#define RATIO_MAX             (256.0f)
#define INIT_CALC_PARAMS_NUM  (2)
#define ISP_HDR_BIT_NUM_MAX   (20)
#define ISP_HDR_BIT_NUM_MIN   (12)
#define ISP_PREDGAIN_DEFAULT  (1.0f)
#define SW_DRC_OFFSET_POW2_FIX (8)
#define MFHDR_LOG_Q_BITS (11)
#define DSTBITS (ISP_RAW_BIT << MFHDR_LOG_Q_BITS)
#define DRC_COMPRESS_Y_OFFSET   (0.0156f)
#define OFFSETBITS_INT (SW_DRC_OFFSET_POW2_FIX)
#define OFFSETBITS (OFFSETBITS_INT << MFHDR_LOG_Q_BITS)
#define VALIDBITS (DSTBITS - OFFSETBITS)
#define DELTA_SCALEIN_FIX ((256 << MFHDR_LOG_Q_BITS) / VALIDBITS)
#define GAS_L0_DEFAULT          (24)
#define GAS_L1_DEFAULT          (10)
#define GAS_L2_DEFAULT          (10)
#define GAS_L3_DEFAULT          (5)
#define GAS_T_MAX               (4)
#define GAS_T_MIN               (0)
#define GAS_L_MAX               (64)
#define GAS_L_MIN               (0)
#define AE_RATIO_MAX          (256)
#define AE_RATIO_L2M_MAX      (32.0f)

typedef struct CurrData_s {
    float MotionCoef;
    drc_OpMode_t ApiMode;
    DrcAEData_t AEData;
    drc_params_dyn_t dynParams;
} CurrData_t;

typedef struct DrcContext_s {
    const RkAiqAlgoCom_prepare_t* prepare_params;
    drc_api_attrib_t* drc_attrib;
    int iso;
    bool isReCal_;
    FrameNumber_t FrameNumber;
    unsigned char compr_bit;
    bool isCapture;
    bool isDampStable;
    uint32_t FrameID;
    bool blc_ob_enable;
    float isp_ob_predgain;
    CurrData_t CurrData;
    NextData_t NextData;
} DrcContext_t;


XCAM_BEGIN_DECLARE

XCamReturn DrcSelectParam(DrcContext_t* pDrcCtx, drc_param_t* out, int iso);
#if RKAIQ_HAVE_DRC_V12
void DrcExpoParaProcessing(DrcContext_t* pDrcCtx, drc_param_t* out);
#endif

XCAM_END_DECLARE

#endif//__RKAIQ_TYPES_Drc_ALGO_PRVT_H__
