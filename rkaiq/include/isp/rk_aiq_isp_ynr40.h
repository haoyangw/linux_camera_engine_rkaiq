/*
 *  Copyright (c) 2023 Rockchip Corporation
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

#ifndef _RK_AIQ_PARAM_YNR40_H_
#define _RK_AIQ_PARAM_YNR40_H_

#include "rk_aiq_isp_common_ynr.h"

typedef struct {
    // reg: sw_ynr_hiSpnr_bypass
    bool hw_ynrT_filt_en;
    // reg: sw_ynr_hiSpnr_lp_en
    bool hw_ynrT_lp_en;
    // reg: sw_ynr_hiSpnr_strg
    float hw_ynrT_filt_strg;
    // reg: sw_ynr_hiSpnrFilt_distSigma
    float hw_ynrT_spatial_strg;
    // reg: sw_ynr_hiSpnrFilt_centerWgt
    float hw_ynrT_centerPix_wgt;
    // reg: sw_ynr_hiSpnrLocalGain_alpha
    float hw_ynrT_localYnrScl_alpha;
    // reg: sw_ynr_hiSpnrSigma_minLimit
    float hw_ynrT_nlmSgm_minLimit;
    // reg: sw_ynr_hiSpnrFilt_wgtOffset
    float hw_ynrT_nlmRgeWgt_negOff;
} ynr_hiNr_filt_dyn_t;

typedef struct {
    //reg: sw_ynr_hiSpnrFilt1_wgtAlpha
    float hw_ynrT_filtOut_alpha;
    //reg: sw_ynr_hiSpnrFilt1_distSigma
    float hw_ynrT_spatial_strg;
    //reg: hw_ynr_hiSpnr_gainThred
    float hw_ynrT_locSgmStrgAlphaUp_thred;
	//reg: sw_ynr_hiSpnrFilt1_texThred
    float sw_ynr_texArea_thred;
	//reg: sw_ynr_hiSpnrFilt1_texScale
    float sw_ynr_texArea_scale;
} ynr_hiNr_smooth_dyn_t;

typedef struct {
    // reg: sw_ynr_miSpnr_bypass
    bool hw_ynrT_filt_en;
    // reg: sw_ynr_miSpnr_strg
    float hw_ynrT_filt_strg;
    // reg: sw_ynr_miSpnrFilt_distSigma
	float hw_ynrT_spatial_strg;
    // reg: sw_ynr_miSpnrFilt_centerWgt
    float hw_ynrT_centerPix_wgt;
    // reg: sw_ynr_miSpnrSoftThred_scale
    float hw_ynrT_softThd_scale;
    // reg: sw_ynr_miSpnr_wgt
    float hw_ynrT_filtOut_alpha;
    // reg: sw_ynr_miEnhance_scale_en
    bool hw_ynrT_enhance_en;
    // reg: sw_ynr_miEnhance_scale
    float hw_ynrT_fenhance_scale;
} ynr_miNr_filt_dyn_t;

typedef struct {
    // reg: sw_ynr_dsFilt_bypass
    bool hw_ynrT_filt_en;
    // reg: sw_ynr_dsFilt_strg
    float hw_ynrT_filt_strg;
    // reg: sw_ynr_dsFiltSoftThred_scale
    float hw_ynrT_softThd_scale;
    // reg: sw_ynr_dsFilt_centerWgt
    float hw_ynrT_centerPix_wgt;
    // reg: sw_ynr_dsFiltVsoftThred_scale
    float hw_ynrT_softThdV_scale;
    // reg: sw_ynr_dsFilt_vCenterWgt
    float hw_ynrT_centerPixV_wgt;
    // reg: sw_ynr_dsFiltHsoftThred_scale
    float hw_ynrT_softThdH_scale;
    // reg: sw_ynr_dsFilt_hCenterWgt
    float hw_ynrT_centerPixH_wgt;
} ynr_loNr_iirGuide_t;

typedef struct {
    // reg: sw_ynr_loSpnr_bypass
    bool hw_ynrT_filt_en;
    // reg: sw_ynr_loSpnr_strg
    float hw_ynrT_filt_strg;
    // reg: sw_ynr_loSpnrFilt_centerWgt
    float hw_ynrT_centerPix_wgt;
    // reg: sw_ynr_loSpnrThumbThred_scale
    float hw_ynrT_texThred_scale;
    // reg: sw_ynr_loSpnrSoftThred_scale
    float hw_ynrT_softThd_scale;
    // reg: sw_ynr_loSpnr_wgt
    float hw_ynrT_filtOut_alpha;
} ynr_loNr_filt_t;

typedef struct {
    // reg: sw_ynr_tex2loStrg_en
    bool hw_ynrT_texDct_en;
    // reg: sw_ynr_tex2loStrg_minLimit
    float sw_ynr_texArea_strg;
    // reg: sw_ynr_tex2loStrg_lowerThred
    float sw_ynr_texDct_minThred;
    // reg: sw_ynr_tex2loStrg_upperThred
    float sw_ynr_texDct_maxThred;
} ynr_loNr_texDct_t;

typedef struct {
    // reg: sw_ynr_globalSet_gain
    float hw_ynrT_glbSgmStrg_val;
    // reg: sw_ynr_gainMerge_alpha
    float hw_ynrT_glbSgmStrg_alpha;
	// reg: sw_ynr_loGain2strg_val
    float hw_ynrT_locSgmStrg2YnrScl_val[YNR_PIXSGMSCL_SEGMENT_MAX];
	// reg: sw_ynr_loGain2wgt_val
    float hw_ynrT_locSgmStrg2YnrWgt_val[YNR_PIXSGMSCL_SEGMENT_MAX];
} ynr_ynrScl_locSgmStrg_dyn_t;

typedef struct {
    float hw_ynrT_sigmaRatio_idx[YNR_LUMA2SIGMASTRENGTH_SEGMENT_MAX];
    float hw_ynrT_sigmaRatio_val[YNR_LUMA2SIGMASTRENGTH_SEGMENT_MAX];
} ynr_sigmaRatio_dyn_t;

typedef struct ynr_params_static_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(ynrScl_radi),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(TODO))  */
    ynr_ynrScl_radi_static_t ynrScl_radi;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_ynrT_sigmaCfg_mode),
        M4_TYPE(enum),
        M4_ENUM_DEF(ynr_sigmaCurveCfg_mode_t),
        M4_DEFAULT(ynr_cfgByCoeff2Curve_mode),
        M4_HIDE_EX(1),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The way to configure the sigma curve point. Reference enum types.\n
		Freq of use: low))  */
    ynr_sigmaCurveCfg_mode_t sw_ynrCfg_sgmCurve_mode;
} ynr_params_static_t;

typedef struct ynr_params_dyn_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(ynrScl_radi),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(TODO))  */
    ynr_ynrScl_radi_dyn_t ynrScl_radi;
    /* M4_GENERIC_DESC(
        M4_ALIAS(localSgmScl_inPixSgm),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO))  */
    ynr_ynrScl_locSgmStrg_dyn_t ynrScl_locSgmStrg;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_ynrT_sigamaCurve_Coeff),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(The sigma calibration curve of the ynr module. \n
        The ynr only use the sigma coeffs of the curve when sw_ynrCfg_sgmCurve_mode == ynr_cfgByCoeff2Curve_mode.\n
        ))  */
	ynr_coeff2SgmCurve_t coeff2SgmCurve;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_ynrC_luma2Sigma_curve),
        M4_TYPE(struct),
        M4_UI_MODULE(curve_ui),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(...\n
        The ynr will use the sigma point data of the curve when sw_ynrCfg_sigmaCfg_mode == ynr_cfgByCurveDirectly_mode.))  */
   // reg: hw_ynr_luma2sigma_idx0 ~ 16, hw_ynr_luma2sigma_val0 ~ 16
    ynr_sigmaCurve_dyn_t hw_ynrC_luma2Sigma_curve;

    ynr_sigmaRatio_dyn_t sigmaRatio;
    ynr_hiNr_filt_dyn_t hiNr_filt;
    ynr_hiNr_smooth_dyn_t hiNr_smooth;
    ynr_miNr_filt_dyn_t miNr_filt;
    ynr_loNr_iirGuide_t loNr_iirGuide;
    ynr_loNr_filt_t loNr_filt;
    ynr_loNr_texDct_t loNr_texDct;
} ynr_params_dyn_t;

typedef struct ynr_param_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(sta),
        M4_TYPE(struct),
        M4_UI_MODULE(static_ui),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(TODO))  */
    ynr_params_static_t sta;
    /* M4_GENERIC_DESC(
        M4_ALIAS(dyn),
        M4_TYPE(struct),
        M4_UI_MODULE(dynamic_ui),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(TODO))  */
    ynr_params_dyn_t dyn;
} ynr_param_t;

#endif
