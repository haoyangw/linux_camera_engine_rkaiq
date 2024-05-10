#ifndef _RK_AIQ_ISP39_MODULES_H_
#define _RK_AIQ_ISP39_MODULES_H_

#include "rk_aiq_isp32_modules.h"
#include "common/rk-isp39-config.h"
#include "common/rk-isp33-config.h"
#include "rk_aiq_module_btnr_common.h"

RKAIQ_BEGIN_DECLARE

void rk_aiq_btnr40_params_cvt(void* attr, isp_params_t* isp_params, common_cvt_info_t *cvtinfo, btnr_cvt_info_t *pBtnrInfo);
void rk_aiq_sharp34_params_cvt(void* attr, isp_params_t* isp_params, common_cvt_info_t *cvtinfo);
void rk_aiq_ynr34_params_cvt(void* attr, isp_params_t* isp_params, common_cvt_info_t *cvtinfo);
void rk_aiq_cnr34_params_cvt(void* attr, isp_params_t* isp_params, common_cvt_info_t *cvtinfo);
void rk_aiq_drc40_params_cvt(void* attr, isp_params_t* isp_params, common_cvt_info_t *cvtinfo);
void rk_aiq_blc30_params_cvt(void* attr, isp_params_t* isp_params, common_cvt_info_t *cvtinfo);
void rk_aiq_dpcc21_params_cvt(void* attr, isp_params_t* isp_params);
void rk_aiq_gamma21_params_cvt(void* attr, isp_params_t* isp_params);
void rk_aiq_gic21_params_cvt(void* attr, struct isp39_gic_cfg* gic_cfg);
void rk_aiq_dehaze23_params_cvt(void* attr, isp_params_t* isp_params, common_cvt_info_t *cvtinfo);
void rk_aiq_histeq23_params_cvt(void* attr, isp_params_t* isp_params, common_cvt_info_t *cvtinfo);
void rk_aiq_dehazeHisteq23_sigma_params_cvt(isp_params_t* isp_params,
                                            common_cvt_info_t* cvtinfo);
void rk_aiq_yme10_params_cvt(void* attr, isp_params_t* isp_params, common_cvt_info_t *cvtinfo);
void rk_aiq_cac21_params_cvt(void* attr, isp_params_t* isp_params,
                             isp_params_t* isp_cfg_right, bool is_multi_isp);
void rk_aiq_ldch22_params_cvt(void* attr, isp_params_t* isp_params,
                              isp_params_t* isp_cfg_right, bool is_multi_isp);
void rk_aiq_csm21_params_cvt(void* attr, isp_params_t* isp_params);
void rk_aiq_dm23_params_cvt(void* attr, isp_params_t* isp_params, common_cvt_info_t *cvtinfo);
void rk_aiq_merge22_params_cvt(void* attr, isp_params_t* isp_params, common_cvt_info_t* cvtinfo);
void rk_aiq_lsc21_params_cvt(void* attr, isp_params_t* isp_params, common_cvt_info_t *cvtinfo);
void rk_aiq_ae25_stats_cfg_cvt(void* meas, isp_params_t* isp_params);

void rk_aiq_rgbir10_params_cvt(void* attr, isp_params_t* isp_params);
void rk_aiq_cgc10_params_cvt(void* attr, isp_params_t* isp_params);
void rk_aiq_cp10_params_cvt(void* attr, isp_params_t* isp_params);
void rk_aiq_gain20_params_cvt(void* attr, isp_params_t* isp_params, common_cvt_info_t* cvtinfo);
void rk_aiq_lut3d20_params_cvt(void* attr, isp_params_t* isp_params);
void rk_aiq_ccm22_params_cvt(void* attr, isp_params_t* isp_params, common_cvt_info_t *cvtinfo);

RKAIQ_END_DECLARE

#endif

