#include "rk_aiq_isp33_modules.h"

#define BIT_MIN               (0)
#define BIT_3_MAX             (7)
#define BIT_4_MAX             (15)
#define BIT_8_MAX             (255)
#define BIT_10_MAX            (1023)
#define BIT_14_MAX            (16383)
#define BIT_17_MAX            (131071)
#define LIMIT_VALUE(value,max_value,min_value)      (value > max_value? max_value : value < min_value ? min_value : value)

// for noise sigma
#define NOISE_SIGMA_FIX_BIT 3
#define GAIN_YNR_FIX_BITS_DECI 4
#define NOISE_CURVE_POINT_NUM 17

int find_top_one_pos_v3(int data)
{
    int i, j = 1;
    int pos = 0;
    for(i = 0; i < 32; i++)
    {
        if(data & j)
        {
            pos = i + 1;
        }
        j = j << 1;
    }
    return pos;
}
void rk_aiq_ynr33_params_cvt(void* attr, isp_params_t* isp_params,common_cvt_info_t *cvtinfo)
{
    struct isp33_ynr_cfg *pFix = &isp_params->isp_cfg->others.ynr_cfg;
    ynr_param_t *ynr_param = (ynr_param_t *) attr;
    ynr_params_static_t* psta = &ynr_param->sta;
    ynr_params_dyn_t* pdyn = &ynr_param->dyn;
    int rows = cvtinfo->rawHeight;
    int cols = cvtinfo->rawWidth;
    float gain_base_inv_f = 1.0f;

    // module bypass control
    pFix->hi_spnr_bypass = !pdyn->hiNr_filt.hw_ynrT_filt_en;
    pFix->mi_spnr_bypass = !pdyn->miNr_filt.hw_ynrT_filt_en;
    pFix->lo_spnr_bypass = !pdyn->loNr_filt.hw_ynrT_filt_en;
    pFix->rnr_en = 1;

    pFix->global_set_gain  = (int)(pdyn->ynrScl_locSgmStrg.hw_ynrT_glbSgmStrg_val * (1 << 4));
    pFix->gain_merge_alpha = (int)(pdyn->ynrScl_locSgmStrg.hw_ynrT_glbSgmStrg_alpha * (1 << 3));

    float r_sq_inv = 16.0f / (cols * cols + rows * rows); // divide 2
    int* number_ptr = (int*)(&r_sq_inv);
    int EE = ((*number_ptr) >> 23) & (0x0ff);
    EE = -(EE - 127 + 1);
    int MM = (*number_ptr) & 0x7fffff;
    float tmp = ((MM / (float)(1 << 23)) + 1) / 2;
    MM = (int)(256 * tmp + 0.5);
    pFix->rnr_max_radius = (MM << 5) + EE;

    pFix->rnr_center_h = cols / 2;
    pFix->rnr_center_v = rows / 2;

    // get rnr parameters
    for (int i = 0; i < 17; i++) {
        pFix->radius2strg[i] = (int)(pdyn->ynrScl_radi.hw_ynrT_radiDist2YnrScl_val[i] * 16);
    }

    // get noise curve
    for (int i = 0; i < ISO_CURVE_POINT_NUM; i++) {
        pFix->luma2sima_x[i] = pdyn->hw_ynrC_luma2Sigma_curve.idx[i];
        pFix->luma2sima_y[i] = (int)(pdyn->hw_ynrC_luma2Sigma_curve.val[i] * (1 << NOISE_SIGMA_FIX_BIT));
    }

    float loFreqLumaNrCurvePoint[6];
    float loFreqLumaNrCurveRatio[6];
    //float miFreqLumaNrCurvePoint[6];
    //float miFreqLumaNrCurveRatio[6];
    for (int i = 0; i < 6; i++)
    {
        loFreqLumaNrCurvePoint[i] = pdyn->sigmaRatio.hw_ynrT_sigmaRatio_idx[i];
        loFreqLumaNrCurveRatio[i] = pdyn->sigmaRatio.hw_ynrT_sigmaRatio_val[i];
        //  miFreqLumaNrCurvePoint[i] = pSelect->miFreqLumaNrCurvePoint[i];
        //  miFreqLumaNrCurveRatio[i] = pSelect->miFreqLumaNrCurveRatio[i];
    }
    //update lo noise curve;
    for (int i = 0; i < ISO_CURVE_POINT_NUM; i++)
    {
        float rate;

        int j = 0;
        for (j = 0; j < 6; j++)
        {
            if (pFix->luma2sima_x[i] <= loFreqLumaNrCurvePoint[j])
                break;
        }

        if (j <= 0)
            rate = loFreqLumaNrCurveRatio[0];
        else if (j >= 6)
            rate = loFreqLumaNrCurveRatio[5];
        else
        {
            rate = ((float)pFix->luma2sima_x[i] - loFreqLumaNrCurvePoint[j - 1]) / (loFreqLumaNrCurvePoint[j] - loFreqLumaNrCurvePoint[j - 1]);
            rate = loFreqLumaNrCurveRatio[j - 1] + rate * (loFreqLumaNrCurveRatio[j] - loFreqLumaNrCurveRatio[j - 1]);
        }
        LOGD_ANR("ynr sigma[%d]:rate:%f 0x%x 0x%x\n", i, rate, pFix->luma2sima_y[i], (int)(rate * pFix->luma2sima_y[i]));
        pFix->luma2sima_y[i] = MIN((int)(rate * pFix->luma2sima_y[i]), 4095);
    }

    //  ========================================== Set Hi-NR parameters ===================================
    pFix->hi_spnr_filt_wgt_offset  = MIN((int)(pdyn->hiNr_filt.hw_ynrT_nlmRgeWgt_negOff * (1 << 10)), 1023);             // [0, 1023], 10 bits (exp(-3) * 1024);
    pFix->hi_spnr_filt_center_wgt  = MIN((int)(pdyn->hiNr_filt.hw_ynrT_centerPix_wgt * 256 * (1 << (10 - 6))), 4 * 1024); // [0, 4*1024],
    pFix->hi_spnr_sigma_min_limit  = MIN((int)(pdyn->hiNr_filt.hw_ynrT_nlmSgm_minLimit * (1 << 11)), 2047);             // [0, 2047], 11bit, 2x
    pFix->hi_spnr_strg             = MIN((int)(pdyn->hiNr_filt.hw_ynrT_filt_strg * (1 << 6)), 1023);                       // [1, 1023], 64x
    pFix->hi_spnr_local_gain_alpha = MIN((int)(pdyn->hiNr_filt.hw_ynrT_localYnrScl_alpha * (1 << 4)), 16);                            // [0, 16], 16x,
    pFix->hi_lp_en                 = pdyn->hiNr_filt.hw_ynrT_lp_en;

    // calculate gaussian weight (driver code)
    float ynr_hi_gauss_sigma = pdyn->hiNr_filt.hw_ynrT_spatial_strg;
    float w01_f = exp(-(0 + 1) / (2.0 * ynr_hi_gauss_sigma * ynr_hi_gauss_sigma));
    float w02_f = exp(-(0 + 4) / (2.0 * ynr_hi_gauss_sigma * ynr_hi_gauss_sigma));
    float w11_f = exp(-(1 + 1) / (2.0 * ynr_hi_gauss_sigma * ynr_hi_gauss_sigma));
    float w12_f = exp(-(1 + 4) / (2.0 * ynr_hi_gauss_sigma * ynr_hi_gauss_sigma));
    float w22_f = exp(-(4 + 4) / (2.0 * ynr_hi_gauss_sigma * ynr_hi_gauss_sigma));
    float w_sum_f = 1.0f + w01_f * 4 + w02_f * 4 + w11_f * 4 + w12_f * 8 + w22_f * 4;
    pFix->hi_spnr_filt_coeff[0] = (int)(w01_f / w_sum_f * 256 + 0.5f); // [0, 63]
    pFix->hi_spnr_filt_coeff[1] = (int)(w02_f / w_sum_f * 256 + 0.5f); // [0, 31]
    pFix->hi_spnr_filt_coeff[2] = (int)(w11_f / w_sum_f * 256 + 0.5f); // [0, 31]
    pFix->hi_spnr_filt_coeff[3] = (pFix->hi_lp_en == 1) ? 0 : (int)(w12_f / w_sum_f * 256 + 0.5f); // [0, 15]
    pFix->hi_spnr_filt_coeff[4] = (pFix->hi_lp_en == 1) ? 0 : (int)(w22_f / w_sum_f * 256 + 0.5f); // [0, 15]
    
    // For Test
    pFix->hi_spnr_filt1_wgt_alpha = MIN((int)(pdyn->hiNr_smooth.hw_ynrT_filtOut_alpha * 256), 256);
    pFix->hi_spnr_filt1_tex_thred = MIN((int)(pdyn->hiNr_smooth.sw_ynr_texArea_thred * 256), 2047);
    pFix->hi_spnr_filt1_tex_scale = MIN(MAX((int)(pdyn->hiNr_smooth.sw_ynr_texArea_scale * 8), 1), 1023);
    float ynr_hiFilt1_gauss_sigma = pdyn->hiNr_smooth.hw_ynrT_spatial_strg;
    float filt1_w01_f = exp(-(0 + 1) / (2.0 * ynr_hiFilt1_gauss_sigma * ynr_hiFilt1_gauss_sigma));
    float filt1_w02_f = exp(-(0 + 4) / (2.0 * ynr_hiFilt1_gauss_sigma * ynr_hiFilt1_gauss_sigma));
    float filt1_w11_f = exp(-(1 + 1) / (2.0 * ynr_hiFilt1_gauss_sigma * ynr_hiFilt1_gauss_sigma));
    float filt1_w12_f = exp(-(1 + 4) / (2.0 * ynr_hiFilt1_gauss_sigma * ynr_hiFilt1_gauss_sigma));
    float filt1_w22_f = exp(-(4 + 4) / (2.0 * ynr_hiFilt1_gauss_sigma * ynr_hiFilt1_gauss_sigma));
    float filt1_w_sum_f = 1.0f + filt1_w01_f * 4 + filt1_w02_f * 4 + filt1_w11_f * 4 + filt1_w12_f * 8 + filt1_w22_f * 4;
    int g_w01 = (int)(filt1_w01_f / filt1_w_sum_f * 256); // [0, 63]
    int g_w02 = (int)(filt1_w02_f / filt1_w_sum_f * 256); // [0, 31]
    int g_w11 = (int)(filt1_w11_f / filt1_w_sum_f * 256); // [0, 31]
    int g_w12 = (int)(filt1_w12_f / filt1_w_sum_f * 256); // [0, 15]
    int g_w22 = (int)(filt1_w22_f / filt1_w_sum_f * 256); // [0, 15]
    int g_w00 = 256 - 4 * g_w01 - 4 * g_w02 - 4 * g_w11 - 8 * g_w12 - 4 * g_w22;
    pFix->hi_spnr_filt1_coeff[0] = g_w00;
    pFix->hi_spnr_filt1_coeff[1] = g_w01;
    pFix->hi_spnr_filt1_coeff[2] = g_w02;
    pFix->hi_spnr_filt1_coeff[3] = g_w11;
    pFix->hi_spnr_filt1_coeff[4] = g_w12;
    pFix->hi_spnr_filt1_coeff[5] = g_w22;

    //  ========================================== Set Mi-NR parameters ===================================
    pFix->mi_spnr_strg             = MIN((int)(pdyn->miNr_filt.hw_ynrT_filt_strg * (1 << 6)), 1023);            // [1, 1023], 64x
    pFix->mi_spnr_filt_center_wgt  = MIN((int)(pdyn->miNr_filt.hw_ynrT_centerPix_wgt * (1 << 10)), 1024);    // [0, 1024], 1024x
    pFix->mi_spnr_soft_thred_scale = MIN((int)(pdyn->miNr_filt.hw_ynrT_softThd_scale * (1 << 10)), 4095);    // [0, 4095], 1024x
    pFix->mi_spnr_wgt              = MIN((int)(pdyn->miNr_filt.hw_ynrT_filtOut_alpha * (1 << 7)), 128);      // [0, 128], 7 bits, 128x
    float ynr_mi_gauss_sigma             = pdyn->miNr_filt.hw_ynrT_spatial_strg;
    float m_w00_f = 1.0f;
    float m_w01_f = exp(-(0 + 4) / (1.0f * ynr_mi_gauss_sigma * ynr_mi_gauss_sigma));
    float m_w11_f = exp(-(4 + 4) / (1.0f * ynr_mi_gauss_sigma * ynr_mi_gauss_sigma));
    float m_w_sum_f = m_w00_f + m_w01_f * 4 +  m_w11_f * 4;
    int m_w01 = (int)(m_w01_f / m_w_sum_f * 256);
    int m_w11 = (int)(m_w11_f / m_w_sum_f * 256);
    pFix->mi_spnr_filt_coeff0 = 256 - m_w01 * 4 - m_w11 * 4;
    pFix->mi_spnr_filt_coeff1 = m_w01;
    pFix->mi_spnr_filt_coeff2 = m_w11;
    pFix->mi_ehance_scale_en    = pdyn->miNr_filt.hw_ynrT_enhance_en;
    pFix->mi_ehance_scale       = MIN(pdyn->miNr_filt.hw_ynrT_fenhance_scale * 8, 255);

    // ========================================== Set Lo-NR parameters ===================================
    pFix->lo_spnr_strg              = MIN((int)(pdyn->loNr_filt.hw_ynrT_filt_strg * (1 << 6)), 1023);            // [1, 1023], 64x
    pFix->lo_spnr_filt_center_wgt   = MIN((int)(pdyn->loNr_filt.hw_ynrT_centerPix_wgt * (1 << 8) * 6), 8191);  // [0, 1023*6], (256*6)x
    pFix->lo_spnr_thumb_thred_scale = MIN((int)(pdyn->loNr_filt.hw_ynrT_texThred_scale * (1 << 6)), 1023);    // [0, 1023], 64x
    pFix->lo_spnr_soft_thred_scale  = MIN((int)(pdyn->loNr_filt.hw_ynrT_softThd_scale * (1 << 10)), 4095);    // [0, 4095], 1024x
    pFix->lo_spnr_wgt               = MIN((int)(pdyn->loNr_filt.hw_ynrT_filtOut_alpha * (1 << 7)), 128);      // [0, 128], 7 bits, 128x

    // ========================================== Set DS-filter parametrers ===============================
    /*
    pFix->hw_ynr_dsFilt_strg            = MIN((int)(pdyn->loNr_iirGuide.hw_ynrT_filt_strg * (1 << 6)), 1023);
    pFix->hw_ynr_dsFiltVsoftThred_scale = MIN((int)(pdyn->loNr_iirGuide.hw_ynrT_softThdV_scale * (1 << 8)), 4095);
    pFix->hw_ynr_dsFilt_vCenterWgt      = MIN((int)(pdyn->loNr_iirGuide.hw_ynrT_centerPixV_wgt * (1 << 7)), 128);
    pFix->hw_ynr_dsFiltHsoftThred_scale = MIN((int)(pdyn->loNr_iirGuide.hw_ynrT_softThdH_scale * (1 << 8)), 4095);
    pFix->hw_ynr_dsFilt_hCenterWgt      = MIN((int)(pdyn->loNr_iirGuide.hw_ynrT_centerPixV_wgt * (1 << 7)), 128);
    pFix->hw_ynr_dsFiltSoftThred_scale  = MIN((int)(pdyn->loNr_iirGuide.hw_ynrT_softThd_scale * (1 << 8)), 4095);
    pFix->hw_ynr_dsFilt_centerWgt       = MIN((int)(pdyn->loNr_iirGuide.hw_ynrT_centerPix_wgt * (1 << 7)), 128);
    */

    // ========================================== Gain adjustment parameters ===================================
    for (int i = 0; i < 9; i++) {
        pFix->lo_spnr_gain2strg[i] = MIN((int)(pdyn->ynrScl_locSgmStrg.hw_ynrT_locSgmStrg2YnrScl_val[i] * (1 << 4)), 511);
        pFix->lo_gain2wgt[i]  = MIN((int)(pdyn->ynrScl_locSgmStrg.hw_ynrT_locSgmStrg2YnrWgt_val[i] * 128), 128);
    }

    //---------------------------------------- RK1106 Hardware Register Configuration ---------------------
    pFix->local_gain_scale = gain_base_inv_f * (1 << 7);  // {1i, 7f}, sqrt(1/4095/4)*128 = 1;
    //-----------------------------------------------------------------------------------------------------

    // texture form sharpen
    pFix->tex2lo_strg_en = pdyn->loNr_texDct.hw_ynrT_texDct_en;
    pFix->tex2lo_strg_lower_thred = (int)(pdyn->loNr_texDct.sw_ynr_texDct_minThred * 1023);              // range : [0, 1023]; 10 bits
    pFix->tex2lo_strg_upper_thred = (int)(pdyn->loNr_texDct.sw_ynr_texDct_maxThred * 1023);    // range : [0, 1023]; 10 bits
    int tex2strg_step = (1 << 20) / MAX(pFix->tex2lo_strg_upper_thred - pFix->tex2lo_strg_lower_thred, 1);
    int ynr_tex2loStrg_minLimit   = (int)(pdyn->loNr_texDct.sw_ynr_texArea_strg * 1024);     // range : [0, 1024]; 11 bits
    int tex_lo_strg_slop = 1024 - ynr_tex2loStrg_minLimit; // [0, 1024]
    int pos_idx = find_top_one_pos_v3(tex2strg_step);
    int tex2strg_step_exponent = MAX(pos_idx - 11, 0);                            // [0, 10]
    int tex2strg_step_mantissa = tex2strg_step >> tex2strg_step_exponent;         // [1,1025]
    tex2strg_step_exponent = 20 - tex2strg_step_exponent;  // [10, 20]

    pFix->tex2lo_strg_mantissa = (tex_lo_strg_slop * tex2strg_step_mantissa) >> 10;  //  [0, 1024]x[1024,2047]->[0, 2047]
    pFix->tex2lo_strg_exponent = tex2strg_step_exponent - 10;  // [0, 10]

    return;
}
