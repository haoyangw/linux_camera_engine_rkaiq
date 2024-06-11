#ifndef __SAMPLE_ALSC_MODULE_H__
#define __SAMPLE_ALSC_MODULE_H__

#include "xcore/base/xcam_common.h"

XCamReturn sample_alsc_module(const void* arg);
#ifdef USE_NEWSTRUCT
int sample_lsc_test(const rk_aiq_sys_ctx_t* ctx);
int sample_query_lsc_status(const rk_aiq_sys_ctx_t* ctx);
int sample_lsc_setCalib_test(const rk_aiq_sys_ctx_t* ctx);
#endif
#endif  /*__SAMPLE_ALSC_MODULE_H__*/
