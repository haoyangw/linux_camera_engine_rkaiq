#ifndef _LDCH_ALOG_API_H_
#define _LDCH_ALOG_API_H_

#include "rk_aiq_algo_des.h"

#include "isp/rk_aiq_isp_ldch22.h"
#include "algos/rk_aiq_api_types_ldch.h"

XCamReturn
algo_ldch_SetAttrib
(
    RkAiqAlgoContext* ctx,
    ldch_api_attrib_t *attr
);

XCamReturn
algo_ldch_GetAttrib
(
    RkAiqAlgoContext*  ctx,
    ldch_api_attrib_t *attr
);

XCAM_BEGIN_DECLARE
extern RkAiqAlgoDescription g_RkIspAlgoDescLdch;
XCAM_END_DECLARE

#endif
