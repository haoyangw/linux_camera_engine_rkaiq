#include "rk_aiq_isp39_modules.h"
#include <math.h> 

#define     RKGIC_V30_HW_BIT                    10


#define     RKGIC_V30_LUMA_POINT_NUM            8
#define     RKGIC_V30_SIGMA_TALBE_LEN           1024
#define     RKGIC_V30_NOISE_CURVE_STEP_BITS     6
#define     RKGIC_V30_NOISE_CURVE_TABLE_LEN     17
#define     RKGIC_V30_SGM_ADJ_TABLE_LEN         13


//
#define     RKGIC_V30_GAUS_FLT_RADIUS           1
#define     RKGIC_V30_BF_FLT_RADIUS             1
#define     RKGIC_V30_NOISE_LUMA_FLT_RADIUS     2
#define     RKGIC_V30_MED_FLT_RADIUS            1
#define     RKGIC_V30_PRE_FLT_RADIUS            1
// 5x4
#define     RKGIC_V30_FLT_GR_GB_RADIUS          2
// 3x1
#define     RKGIC_V30_FLT_THED_RADIUS_X         1
#define     RKGIC_V30_FLT_THED_RADIUS_Y         0

// fix bit
#define     RKGIC_V30_DIV_SIGMA_FIX_BIT         14//16
#define     RKGIC_V30_BF_COEFF_FIX_BITS         5//7
#define     RKGIC_V30_BF_WGT_FIX_BIT            7//8
#define     RKGIC_V30_BF_WGT_SLOPE_FIX_BIT      RKGIC_V30_BF_WGT_FIX_BIT
#define     RKGIC_V30_MED_FLT_RATIO_FIX_BIT     7
#define     RKGIC_V30_BF_FLT_RATIO_FIX_BIT      7
#define     RKGIC_V30_LOCAL_GAIN_FIX_BITS       4
#define     RKGIC_V30_G_GAIN_ALPHA_FIX_BITS     3
#define     RKGIC_V30_GAIN_ISO_FIX_BITS         7
#define     RKGIC_V30_BF_WGT_OFFSET_FIX_BITS    8
#define     RKGIC_V30_BF_WGT_SCALE_FIX_BITS     5//8
#define     RKGIC_V30_CURVE_SCALE_FIX_BITS      7
#define     RKGIC_V30_NOISE_ALPHA_FIX_BITS      7
#define     RKGIC_V30_sgmRatio_FIX_BITS         7
#define     RKGIC_V30_THED_SCALE_FIX_BITS       7
#define     RKGIC_V30_COEFF_INV_FIX_BITS        12
#define     RKGIC_V30_GAIN_SCALE_FIX_BITS       7

#define     RKGIC_V30_WRITE_DEBUG_DATA          0

static void GicV30CreateKernelCoeffs(int radius, int max_radius, float rsigma, uint8_t* kernel_coeffs, int fix_bits, int dim)
{
    // calcute distance
    int i = 0;
    int j = 0;
    int k = 0;

    double e = 2.71828182845905;
    float gaus_table    [10] = { 0 };
    float distance_table[10] = { 0 };
    float sumTable = 0;
    float tmp;

    float coeffScale_table1D[10] = { 1, 2, 2, 2, 2, 2, 2, 2, 2, 2 };
    float coeffScale_table2D[10] = { 1, 4, 4, 4, 8, 4, 4, 8, 8, 4 };

    int coeffNums     = (radius     + 1) * (radius     + 2) / 2;
    int coeffNums_max = (max_radius + 1) * (max_radius + 2) / 2;

    if (dim == 1)
    {
        coeffNums = radius + 1;
    }

    // calc distance
    if (dim == 2)
    {
        for (i = 0; i <= radius; i++)
        {
            for (j = 0; j <= i; j++)
            {
                distance_table[i * (i + 1) / 2 + j] = pow(i, 2) + pow(j, 2);
            }
        }
    }
    else
    {
        for (i = 0; i <= radius; i++)
        {
            distance_table[i] = pow(i, 2);
        }
    }

    // calc coeff
    for (k = 0; k < coeffNums; k++)
    {
        tmp = pow(e, -distance_table[k] / 2.0 / rsigma / rsigma);
        gaus_table[k] = tmp;

        if (dim == 2)
        {
            sumTable += coeffScale_table2D[k] * gaus_table[k];
        }
        else
        {
            sumTable += coeffScale_table1D[k] * gaus_table[k];
        }
    }

    for (k = 0; k < coeffNums_max; k++)
    {
        gaus_table[k] = gaus_table[k] / sumTable;
        kernel_coeffs[k] = ROUND_F(gaus_table[k] * (1 << fix_bits));
    }

    //check gaus params
    int sum_coeff = 0;
    sum_coeff = kernel_coeffs[0]
                + 4 * kernel_coeffs[1]
                + 4 * kernel_coeffs[2];

    int offset = (1 << fix_bits) - sum_coeff;
    kernel_coeffs[0] = kernel_coeffs[0] + offset;
}

void rk_aiq_gic30_params_cvt(void* attr, struct isp33_gic_cfg* gic_cfg)
{
    int i, tmp;
    struct isp33_gic_cfg *pFix = gic_cfg;
    gic_param_t *gic_param = (gic_param_t *) attr;
    gic_params_dyn_t* pdyn = &gic_param->dyn;

    /* CTRL */
    pFix->pro_mode = pdyn->hw_gicT_pro_mode;
    pFix->manualnoisecurve_en = pdyn->hw_gicT_manualNoiseCurve_en;
    pFix->manualnoisethred_en = pdyn->hw_gicT_manualNoiseThred_en;
    pFix->gain_bypass_en = pdyn->hw_gicT_gain_bypass;

    /* MEDFLT_PARA */
    tmp = pdyn->hw_gicT_medFlt_minThred;
    pFix->medflt_minthred =  CLIP(tmp, 0, 0xf);
    tmp = pdyn->hw_gicT_medFlt_maxThred;
    pFix->medflt_maxthred = CLIP(tmp, 0, 0xf);
    tmp = ROUND_F(pdyn->sw_gicT_medFlt_ratio * (1 << RKGIC_V30_MED_FLT_RATIO_FIX_BIT));
    pFix->medflt_ratio = CLIP(tmp, 0, 0xff);

    /* MEDFLTUV_PARA */
    tmp = pdyn->hw_gicT_medFltUV_minThred;
    pFix->medfltuv_minthred = CLIP(tmp, 0, 0xf);
    tmp = pdyn->hw_gicT_medFltUV_maxThred;
    pFix->medfltuv_maxthred = CLIP(tmp, 0, 0xf);
    tmp = ROUND_F(pdyn->sw_gicT_medFltUV_ratio * (1 << RKGIC_V30_MED_FLT_RATIO_FIX_BIT));
    pFix->medfltuv_ratio = CLIP(tmp, 0, 0xff);

    /* NOISE_SCALE */
    tmp = ROUND_F(pdyn->sw_gicT_noiseCurve_scale * 1.414 * (1 << RKGIC_V30_CURVE_SCALE_FIX_BITS));
    if (0 == pdyn->hw_gicT_manualNoiseCurve_en) {
        tmp = ROUND_F(pdyn->sw_gicT_noiseCurve_scale * 1.414 * (1 << RKGIC_V30_CURVE_SCALE_FIX_BITS));
        tmp     = ROUND_F(tmp * 0.5);
    }
    pFix->noisecurve_scale = CLIP(tmp, 0, 0x3ff);

    /* BILAT_PARA1 */
    tmp = ROUND_F(pdyn->sw_gicT_bfFltWgt_minThred * (1 << RKGIC_V30_BF_WGT_OFFSET_FIX_BITS));
    pFix->gffltwgt_offset = CLIP(tmp, 0, 0x3ff);
    float bfFltWgt_slope = 1.0 / MAX(pdyn->sw_gicT_bfFltWgt_maxThred - pdyn->sw_gicT_bfFltWgt_minThred, 0.01);
    tmp = ROUND_F(bfFltWgt_slope * (1 << RKGIC_V30_BF_WGT_SCALE_FIX_BITS));
    pFix->gffltwgt_scale = CLIP(tmp, 0, 0xff);

    /* BILAT_PARA2 */
    tmp = ROUND_F(pdyn->sw_gicT_bfFlt_ratio * (1 << RKGIC_V30_BF_FLT_RATIO_FIX_BIT));
    pFix->bfflt_ratio = CLIP(tmp, 0, 0xff);

    /* DISWGT_COEFF */
    uint8_t bfflt_coeff[3];
    GicV30CreateKernelCoeffs(1, 1, pdyn->sw_gicT_bfFlt_rsigma, bfflt_coeff, RKGIC_V30_BF_COEFF_FIX_BITS, 2);
    if(pFix->pro_mode == 1)
    {
        bfflt_coeff[0]         = 8;
        bfflt_coeff[1]         = 4;
        bfflt_coeff[2]         = 2;
    }
    for(int i = 0; i < 3; i++) {
        bfflt_coeff[i]  = CLIP(bfflt_coeff[i], 0, 0x3f);
    }
    pFix->bfflt_coeff0 = bfflt_coeff[0];
    pFix->bfflt_coeff1 = bfflt_coeff[1];
    pFix->bfflt_coeff2 = bfflt_coeff[2];

    /* SIGMA_Y */
    for(int i = 0; i < 17; i++) {
        tmp = ROUND_F(pdyn->sw_gicT_bfFlt_vsigma[i]);
        pFix->bfflt_vsigma_y[i] = CLIP(tmp, 0, 0x3ff);
    }
    /* LUMA_DX */
    for(int i = 0; i < 7; i++) {
        tmp = LOG2(pdyn->sw_gicT_curve_idx[i + 1] - pdyn->sw_gicT_curve_idx[i]);
        pFix->luma_dx[i] = CLIP(tmp, 0, 0xf);
    }

    /* THRED_Y */
    /* MIN_THRED_Y */
    for(int i = 0; i < 8; i++) {
        tmp = ROUND_F(pdyn->hw_gicT_noise_thred[i]);
        pFix->thred_y[i] = CLIP(tmp, 0, 0x1ff);
        tmp = ROUND_F(pdyn->hw_gicT_noise_minThred[i]);
        pFix->minthred_y[i] = CLIP(tmp, 0, 0x1ff);
    }

    /* THRED_SCALE */
    tmp = ROUND_F(pdyn->sw_gicT_autoNoiseThred_scale * (1 << RKGIC_V30_THED_SCALE_FIX_BITS));
    pFix->autonoisethred_scale = CLIP(tmp, 0, 0x3ff);

    /* LOFLTGR_COEFF */
    uint8_t loFltGr_coeff[4];
    for (int i = 0; i < 4; i++) {
        tmp = pdyn->hw_gicT_loFltGr_coeff[i];
        loFltGr_coeff[i] = CLIP(tmp, 0, 0x1f);
    }
    pFix->lofltgr_coeff0 = loFltGr_coeff[0];
    pFix->lofltgr_coeff1 = loFltGr_coeff[1];
    pFix->lofltgr_coeff2 = loFltGr_coeff[2];
    pFix->lofltgr_coeff3 = loFltGr_coeff[3];

    /* LOFLTGB_COEFF */
    tmp = pdyn->hw_gicT_loFltGb_coeff[0];
    pFix->lofltgb_coeff0 = CLIP(tmp, 0, 0x1f);
    tmp = pdyn->hw_gicT_loFltGb_coeff[1];
    pFix->lofltgb_coeff1 = CLIP(tmp, 0, 0x1f);

    /* SUM_LOFLT_INV */
    int sumLoFltGrCoeff                     = loFltGr_coeff[0] * 2 + loFltGr_coeff[1] + 2 * loFltGr_coeff[2] + loFltGr_coeff[3];
    int sumLoFltGbCoeff                     = pFix->lofltgb_coeff0 * 2 + pFix->lofltgb_coeff1 * 2;
    if(sumLoFltGrCoeff != sumLoFltGbCoeff)
    {
        printf("-----------------sumLoFltGrCoeff must be the same as sumLoFltGbCoeff\n");
    }
    tmp =  ROUND_F(1.0f / MAX(sumLoFltGrCoeff, sumLoFltGbCoeff) * (1 << RKGIC_V30_COEFF_INV_FIX_BITS));
    pFix->sumlofltcoeff_inv = CLIP(tmp, 0, 0x1fff);

    /* LOFLTTHRED_COEFF */
    tmp = pdyn->hw_gicT_loFltThed_coeff[0];
    pFix->lofltthred_coeff0 = CLIP(tmp, 0, 0x1f);
    tmp = pdyn->hw_gicT_loFltThed_coeff[1];
    pFix->lofltthred_coeff1 = CLIP(tmp, 0, 0x1f);

    /* GAIN */
    tmp = pdyn->sw_gicT_globalGain_alpha * (1 << RKGIC_V30_G_GAIN_ALPHA_FIX_BITS);
    pFix->globalgain_alpha =  CLIP(tmp, 0, 0xf);
    tmp = pdyn->sw_gicT_localGain_scale * (1 << RKGIC_V30_GAIN_ISO_FIX_BITS);
    pFix->globalgain_scale =   CLIP(tmp, 0, 0xff);
    tmp = pdyn->sw_gicT_global_gain * (1 << RKGIC_V30_LOCAL_GAIN_FIX_BITS);
    pFix->global_gain =  CLIP(tmp, 0, 0x3ff);

    /* GAIN_SLOPE */
    tmp = ROUND_F(pdyn->sw_gicT_gain_minThred * (1 << RKGIC_V30_LOCAL_GAIN_FIX_BITS));
    pFix->gain_offset = CLIP(tmp, 0, 0x3ff);
    float gain_adj_strg_slope   = (pdyn->sw_gicT_BfFltStrg_maxThred - pdyn->sw_gicT_BfFltStrg_minThred)
                                  / MAX(pdyn->sw_gicT_gain_maxThred - pdyn->sw_gicT_gain_minThred, 0.01);
    tmp = ROUND_F(gain_adj_strg_slope * (1 << RKGIC_V30_GAIN_SCALE_FIX_BITS));
    pFix->gain_scale = CLIP(tmp, 0, 0x3fff);

    /* GAIN_THRED */
    tmp =  ROUND_F(pdyn->sw_gicT_BfFltStrg_minThred * (1 << RKGIC_V30_sgmRatio_FIX_BITS));
    pFix->gainadjflt_minthred = CLIP(tmp, 0, 0x3ff);
    tmp = ROUND_F(pdyn->sw_gicT_BfFltStrg_maxThred * (1 << RKGIC_V30_sgmRatio_FIX_BITS));
    pFix->gainadjflt_maxthred =  CLIP(tmp, 0, 0x3ff);
    
    return;
}
