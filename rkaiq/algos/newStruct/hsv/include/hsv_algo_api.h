#ifndef _HSV_ALOG_API_H_
#define _HSV_ALOG_API_H_

#include "rk_aiq_algo_des.h"

#if RKAIQ_HAVE_HSV_V10
#include "isp/rk_aiq_isp_hsv10.h"
#else
#error "wrong hsv hw version !"
#endif
#include "algos/rk_aiq_api_types_hsv.h"

XCAM_BEGIN_DECLARE
XCamReturn algo_hsv_queryahsvStatus(RkAiqAlgoContext* ctx, ahsv_status_t* status);
XCamReturn
algo_hsv_SetCalib
(
    RkAiqAlgoContext* ctx,
    ahsv_hsvCalib_t* calib
);
XCamReturn
algo_hsv_GetCalib
(
    RkAiqAlgoContext* ctx,
    ahsv_hsvCalib_t* calib
);

extern RkAiqAlgoDescription g_RkIspAlgoDescHsv;
XCAM_END_DECLARE

#endif
