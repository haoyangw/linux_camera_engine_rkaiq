#ifndef _RK_AIQ_ISP32_MODULES_H_
#define _RK_AIQ_ISP32_MODULES_H_

#include <stdint.h>

#include "include/common/rk-camera-module.h"
#include "include/common/rk_aiq_comm.h"
#include "include/common/rkisp32-config.h"
#include "rk_aiq_isp_blc30.h"
#if RKAIQ_HAVE_DEHAZE_V14
#include "rk_aiq_isp_histeq23.h"
#elif RKAIQ_HAVE_HISTEQ_V10
#include "rk_aiq_isp_histeq30.h"
#endif
#if RKAIQ_HAVE_SHARP_V40
#include "rk_aiq_isp_texEst40.h"
#endif

typedef struct RKAiqAecExpInfo_s RKAiqAecExpInfo_t;

typedef struct blc_res_cvt_s {
    bool en;
    /* M4_GENERIC_DESC(
        M4_ALIAS(obcPreTnr),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The dynamic params of optical black correction before TNR.))  */
    blc_obcPreTnr_dyn_t obcPreTnr;
    /* M4_GENERIC_DESC(
        M4_ALIAS(obcPostTnr),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(The dynamic params of optical black correction after TNR.))  */
    blc_obcPostTnr_dyn_t obcPostTnr;
} blc_res_cvt_t;

typedef struct {
    uint32_t frameId;
    uint16_t rawWidth;
    uint16_t rawHeight;
    bool isGrayMode;
    bool isFirstFrame;
    uint8_t frameNum;
    float preDGain;
    int frameIso[3];
    float frameEt[3];
    float frameDGain[3];
    struct rkmodule_awb_inf otp_awb;
    struct rkmodule_lsc_inf *otp_lsc;
    struct rkmodule_af_inf *otp_af;
	struct rkmodule_pdaf_inf *otp_pdaf;
    RKAiqAecExpInfo_t *ae_exp;
    blc_res_cvt_t blc_res;
    bool use_aiisp;
    bool cnr_path_valid;
    int ynr_count;
    int sharp_count;
    float L2S_Ratio;
#ifdef USE_NEWSTRUCT
    float ynr_sigma[HIST_SIGMA_LUT_NUM];
#endif
    bool cmps_on;
    bool dehaze_en;
    bool histeq_en;
#if defined(ISP_HW_V33)
    texEst_param_t texEst_param;
#endif
} common_cvt_info_t;

typedef struct {
#if defined(ISP_HW_V39)
    struct isp39_isp_params_cfg* isp_cfg;
#elif defined(ISP_HW_V33)
    struct isp33_isp_params_cfg* isp_cfg;
#endif
} isp_params_t;

RKAIQ_BEGIN_DECLARE

void rk_aiq_dm21_params_cvt(void* attr, struct isp32_isp_params_cfg* isp_cfg);
void rk_aiq_btnr32_params_cvt(void* attr, struct isp32_isp_params_cfg* isp_cfg, bool bypass);
void rk_aiq_ynr32_params_cvt(void* attr, struct isp32_isp_params_cfg* isp_cfg,common_cvt_info_t *cvtinfo);
void rk_aiq_sharp32_params_cvt(void* attr, struct isp32_isp_params_cfg* isp_cfg, common_cvt_info_t *cvtinfo);
void rk_aiq_dehaze22_params_cvt(void* attr, struct isp32_isp_params_cfg* isp_cfg);
// void rk_aiq_gamma21_params_cvt(void* attr, struct isp32_isp_params_cfg* isp_cfg);

RKAIQ_END_DECLARE
#endif

