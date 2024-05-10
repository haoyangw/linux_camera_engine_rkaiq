#include "rk_aiq_isp39_modules.h"

#define FIXTNRWGT       10
#define FIXTNRKAL       8
#define FIXTNRWWW       12

int bayertnr_find_top_one_pos(int data) {
    int i, j = 1;
    int pos = 0;

    for(i = 0; i < 32; i++)
    {
        if(data & j)
        {
            pos = i;
        }
        j = j << 1;
    }

    return pos;
}

void bayertnr_logtrans_init(int bayertnr_trans_mode, int bayertnr_trans_mode_scale, btnr_trans_params_t *pTransPrarms)
{
    int i, j;
    int bayertnr_logprecision;
    int bayertnr_logfixbit;
    int bayertnr_logtblbit;
    int bayertnr_logscalebit;
    int bayertnr_logfixmul;
    int bayertnr_logtblmul;

    if(bayertnr_trans_mode)
    {
        bayertnr_logprecision = 6;
        bayertnr_logfixbit = 12;
        bayertnr_logtblbit = 12;
        bayertnr_logscalebit = 6;
        bayertnr_logfixmul = (1 << bayertnr_logfixbit);
        bayertnr_logtblmul = (1 << bayertnr_logtblbit);
        {
            double tmp, tmp1;
            for (i = 0; i < (1 << bayertnr_logprecision); i++)
            {
                tmp1 = ( 1 << bayertnr_logprecision);
                tmp = i;
                tmp = 1 + tmp * 3 / tmp1;
                tmp = sqrt(tmp);
                pTransPrarms->bayertnr_logtablef[i] = (int)(tmp * bayertnr_logtblmul);

                tmp = i;
                tmp = 1 + tmp / tmp1;
                tmp = pow(tmp, 2);
                pTransPrarms->bayertnr_logtablei[i] = (int)(tmp * bayertnr_logtblmul);
            }
            pTransPrarms->bayertnr_logtablef[i] = 2 * bayertnr_logtblmul;
            pTransPrarms->bayertnr_logtablei[i] = 4 * bayertnr_logtblmul;
        }
    }
    else
    {
        bayertnr_logprecision = 6;
        bayertnr_logfixbit = 12;
        bayertnr_logtblbit = 12;
        bayertnr_logscalebit = bayertnr_trans_mode_scale + 8;
        bayertnr_logfixmul = (1 << bayertnr_logfixbit);
        bayertnr_logtblmul = (1 << bayertnr_logtblbit);
        {
            double tmp, tmp1;
            for (i = 0; i < (1 << bayertnr_logprecision); i++)
            {
                tmp1 = (1 << bayertnr_logprecision);
                tmp = i;
                tmp = 1 + tmp / tmp1;
                tmp = log(tmp)  /  log(2.0);
                pTransPrarms->bayertnr_logtablef[i] = (int)(tmp * bayertnr_logtblmul);
                tmp = i;
                tmp = tmp / tmp1;
                tmp = pow(2, tmp);
                pTransPrarms->bayertnr_logtablei[i] = (int)(tmp * bayertnr_logtblmul);
            }
            pTransPrarms->bayertnr_logtablef[i] = 1 * bayertnr_logtblmul;
            pTransPrarms->bayertnr_logtablei[i] = 2 * bayertnr_logtblmul;
        }
    }

    pTransPrarms->bayertnr_logprecision = bayertnr_logprecision;
    pTransPrarms->bayertnr_logfixbit = bayertnr_logfixbit;
    pTransPrarms->bayertnr_logtblbit = bayertnr_logtblbit;
    pTransPrarms->bayertnr_logscalebit = bayertnr_logscalebit;
    pTransPrarms->bayertnr_logfixmul = bayertnr_logfixmul;
    pTransPrarms->bayertnr_logtblmul = bayertnr_logtblmul;
}

int bayertnr_logtrans(uint32_t tmpfix, btnr_trans_params_t *pTransPrarms)
{
    long long x8, one = 1;
    long long gx, n, ix1, ix2, dp;
    long long lt1, lt2, dx, fx;
    int bayertnr_logprecision = pTransPrarms->bayertnr_logprecision;
    int bayertnr_logfixbit = pTransPrarms->bayertnr_logfixbit;
    int bayertnr_logtblbit = pTransPrarms->bayertnr_logtblbit;
    int bayertnr_logscalebit = pTransPrarms->bayertnr_logscalebit;
    int bayertnr_logfixmul = pTransPrarms->bayertnr_logfixmul;
    int bayertnr_logtblmul = pTransPrarms->bayertnr_logtblmul;


    if(pTransPrarms->transf_mode)
    {
        long long dn;

        x8 = MIN((tmpfix + pTransPrarms->transf_mode_offset), pTransPrarms->transf_data_max_limit);

        // find highest bit
        n  = (long long)bayertnr_find_top_one_pos((int)x8);
        n  = n >> 1;
        dn = n * 2;

        gx = x8 - (one << dn);
        gx = gx * (one << bayertnr_logprecision) * bayertnr_logfixmul;
        gx = gx / (one << dn);
        gx = gx / 3;

        ix1 = gx >> bayertnr_logfixbit;
        dp = gx - ix1 * bayertnr_logfixmul;

        dp = dp / 64;       // opt
        ix2 = ix1 + 1;

        lt1 = pTransPrarms->bayertnr_logtablef[ix1];
        lt2 = pTransPrarms->bayertnr_logtablef[ix2];

        dx = lt1 * (bayertnr_logfixmul / 64 - dp) + lt2 * dp;   // opt
        dp = dp * 64;       // opt

        fx = dx + (one << (bayertnr_logfixbit + bayertnr_logtblbit - bayertnr_logscalebit - n - 1));
        fx = fx >> (bayertnr_logfixbit + bayertnr_logtblbit - bayertnr_logscalebit - n);

        fx = fx - pTransPrarms->itransf_mode_offset;
    }
    else
    {
        x8 = MIN((tmpfix + pTransPrarms->transf_mode_offset), pTransPrarms->transf_data_max_limit);

        // find highest bit
        n = (long long)bayertnr_find_top_one_pos((int)x8);

        gx = x8 - (one << n);
        gx = gx * (one << bayertnr_logprecision) * bayertnr_logfixmul;
        gx = gx / (one << n);

        ix1 = gx >> bayertnr_logfixbit;
        dp = gx - ix1 * bayertnr_logfixmul;

        dp = (dp + 32) / 64;        // opt
        ix2 = ix1 + 1;

        lt1 = pTransPrarms->bayertnr_logtablef[ix1];
        lt2 = pTransPrarms->bayertnr_logtablef[ix2];

        dx = lt1 * (bayertnr_logfixmul / 64 - dp) + lt2 * dp; // opt
        dx = dx * 64;       // opt

        fx = dx + n * (one << (bayertnr_logfixbit + bayertnr_logtblbit));
        fx = fx + (one << (bayertnr_logfixbit + bayertnr_logtblbit - bayertnr_logscalebit - 1));
        fx = fx >> (bayertnr_logfixbit + bayertnr_logtblbit - bayertnr_logscalebit);

        fx = fx - pTransPrarms->itransf_mode_offset;
    }

    return (int)fx;
}

void bayertnr_save_stats(void *stats_buffer, btnr_cvt_info_t *pBtnrInfo)
{
    if (stats_buffer == NULL)
        return;

#if RKAIQ_HAVE_BAYERTNR_V30
    struct rkisp39_stat_buffer* stats = stats_buffer;
    if (stats->meas_type & ISP39_STAT_BAY3D) {
        uint8_t min_idx = 0;
        for (uint8_t i = 0; i < RK_AIQ_BTNR_STATS_CNT; i++) {
            if (pBtnrInfo->mBtnrStats[min_idx].id > pBtnrInfo->mBtnrStats[i].id)
                min_idx = i;
        }
        btnr_stats_t *btnr_stats = &pBtnrInfo->mBtnrStats[min_idx];
        btnr_stats->id = stats->frame_id;
        btnr_stats->sigma_num = stats->stat.bay3d.tnr_auto_sigma_count;
        for (uint8_t i=0; i<20; i++) {
            btnr_stats->sigma_y[i] = stats->stat.bay3d.tnr_auto_sigma_calc[i];
        }
    }
#elif RKAIQ_HAVE_BAYERTNR_V41
    struct rkisp33_stat_buffer* stats = stats_buffer;
    if (stats->meas_type & ISP33_STAT_BAY3D) {
        uint8_t min_idx = 0;
        for (uint8_t i = 0; i < RK_AIQ_BTNR_STATS_CNT; i++) {
            if (pBtnrInfo->mBtnrStats[min_idx].id > pBtnrInfo->mBtnrStats[i].id)
                min_idx = i;
        }
        btnr_stats_t *btnr_stats = &pBtnrInfo->mBtnrStats[min_idx];
        btnr_stats->id = stats->frame_id;
        btnr_stats->sigma_num = stats->stat.bay3d.sigma_num;
        for (uint8_t i=0; i<20; i++) {
            btnr_stats->sigma_y[i] = stats->stat.bay3d.sigma_y[i];
        }
    }
#endif

    //printf("mBtnrStatsId [%d %d %d]\n", pBtnrInfo->mBtnrStats[0].id, pBtnrInfo->mBtnrStats[1].id, pBtnrInfo->mBtnrStats[2].id);
}

btnr_stats_t *bayertnr_get_stats(btnr_cvt_info_t *pBtnrInfo, uint32_t frameId)
{
    uint8_t idx = 0;

    for (uint8_t i = 0; i < RK_AIQ_BTNR_STATS_CNT; i++) {
        if (pBtnrInfo->mBtnrStats[i].id == frameId - BAYERTNR_STATS_DELAY)
            idx = i;
    }
    return &pBtnrInfo->mBtnrStats[idx];
}

int bayertnr_kalm_bitcut(int datain, int bitsrc, int bitdst)
{
    int out;
    out = bitsrc == bitdst ? datain : ((datain + (1 << (bitsrc - bitdst - 1))) >> (bitsrc - bitdst));
    return out;
}

int bayertnr_wgt_sqrt_tab(int index)
{
    int i, res, ratio;
    int len = 10;
    int tab_x[10] = {0, 1, 2, 4, 8, 16, 32, 64, 128, 256};
    int tab_y[10] = {0, 16, 23, 32, 45, 64, 91, 128, 181, 256};

    for(i = 0; i < len; i++)
    {
        if(index < tab_x[i])
            break;
    }

    if(i <= 0)
        res = tab_y[0];
    else if(i > len - 1)
        res = tab_y[len - 1];
    else
    {
        ratio = (index - tab_x[i - 1]);
        ratio = ratio * (tab_y[i] - tab_y[i - 1]);
        ratio = ratio / (tab_x[i] - tab_x[i - 1]);
        res = (int)(tab_y[i - 1] + ratio);
    }

    return res;
}


int bayertnr_update_sq(btnr_trans_params_t *pTransPrarms)
{
    int tmp, tmp0, tmp1;

    /* BAY3D_TNRSIGORG */
    // noise balance update info
    int pre_wk_stat, pre_pk_stat, wk_stat;
    int sigorg, sigsta, wsta;
    long long pkp1_sq, tmpL0, tmpL1;
    sigorg = 256;
    if(pTransPrarms->isFirstFrame)
    {
        pTransPrarms->bayertnr_pk_stat  = sigorg;
        pTransPrarms->bayertnr_wgt_stat = 0;
        pre_wk_stat = pTransPrarms->bayertnr_wgt_stat;
        pre_pk_stat = pTransPrarms->bayertnr_pk_stat;
    }
    else
    {
        pre_wk_stat = pTransPrarms->bayertnr_wgt_stat;
        pre_pk_stat = pTransPrarms->bayertnr_pk_stat;

        wk_stat = (1 << FIXTNRWWW) * (1 << FIXTNRWWW) / ((2 << FIXTNRWWW) - pre_wk_stat);
        wk_stat = CLIP(wk_stat, pTransPrarms->bayertnr_lo_wgt_clip_min_limit, pTransPrarms->bayertnr_lo_wgt_clip_max_limit);

        tmp0 = bayertnr_kalm_bitcut(wk_stat, FIXTNRWWW, FIXTNRKAL);
        tmp1  = bayertnr_wgt_sqrt_tab((1 << FIXTNRKAL) - tmp0);
        tmp1  = (tmp1 * sigorg) >> FIXTNRKAL;
        tmp1  = CLIP(tmp1, 1, (1 << FIXTNRKAL));

        pTransPrarms->bayertnr_wgt_stat = wk_stat;
        pTransPrarms->bayertnr_pk_stat  = tmp1;
    }

    sigorg = 256;
    sigsta = pre_pk_stat;
    wsta   = bayertnr_kalm_bitcut(pTransPrarms->bayertnr_wgt_stat, FIXTNRWWW, FIXTNRWGT);
    pkp1_sq  = (long long)(sigsta * sigsta);
    tmpL0 = (long long)((wsta * wsta) / (1 << FIXTNRWGT));
    tmpL0 = (long long)(tmpL0 * pkp1_sq);
    tmpL1 = (long long)((((1 << FIXTNRWGT) - wsta) * ((1 << FIXTNRWGT) - wsta)) / (1 << FIXTNRWGT));
    tmp = tmpL0 + tmpL1 * sigorg * sigorg;
    tmp =  CLIP(tmp, 0, 0x3ffffff);

    //printf("%s:, baytnr_out_sigma_sq 0x%x\n", __func__, tmp);
    return tmp;

}
void bay_gauss5x5_filter_coeff(float sigma, int halftaby, int halftabx, int strdtabx, int *gstab, int coefsum)
{
    int halfx = halftabx;
    int halfy = halftaby;
    int strdx = strdtabx;
    int stridex = strdx / 2;
    int centerx = halfx / 2;
    int centery = halfy / 2;
    int gausstab[5 * 5];
    int i, j, sumc;
    float tmpf0, tmpf1;
    float tmpf2, gausstabf[5 * 5];
    int gstabidx[5 * 5] =
    {   5, 4, 3, 4, 5,
        4, 2, 1, 2, 4,
        3, 1, 0, 1, 3,
        4, 2, 1, 2, 4,
        5, 4, 3, 4, 5,
    };

    tmpf2 = 0;
    sumc = 0;
    for (i = 0; i < halfy; i++)
    {
        for (j = 0; j < halfx; j++)
        {
            tmpf0 = (float)((i - centery) * (i - centery) + (j - centerx) * (j - centerx));
            tmpf0 = tmpf0 / (2 * sigma * sigma);
            tmpf1 = expf(-tmpf0);
            tmpf2 = tmpf2 + tmpf1;
            gausstabf[i * halfx + j] = tmpf1;
        }
    }
    for (i = 0; i < halfy; i++)
    {
        for (j = 0; j < halfx; j++)
        {
            gausstab[i * halfx + j] = (int)(gausstabf[i * halfx + j] / tmpf2 * coefsum);
            sumc = sumc + gausstab[i * halfx + j];
        }
    }
    gausstab[halfy / 2 * halfx + halfx / 2] += (coefsum - sumc);

    for (i = 0; i < halfy; i++)
    {
        for (j = 0; j < halfx; j++)
        {
            gstab[gstabidx[i * halfx + j]] = gausstab[i * halfx + j];
        }
    }
}

int bayertnr_autosigma_config(btnr_stats_t *pStats, btnr_trans_params_t *pTransPrarms)
{
    // update auto sigma curve
    int sigma_bins = 20;
    uint16_t tmp = 0;
    uint16_t *sigmay_data = pStats->sigma_y;
    uint16_t *sigmay_curve = pTransPrarms->tnr_luma_sigma_y;
    int iir_wgt  = pTransPrarms->bayertnr_auto_sig_count_filt_wgt;
    int filt_coef[5] = {0, 0, 1, 0, 0};
    int sigmay_calc[24 + 2 + 2], sigmay_tmp[24];
    int i = 0, j = 0, tmp0 = 0, tmp1 = 0, coefacc = 0;

    if (pTransPrarms->bayertnr_auto_sig_count_en == 0)
        return 0;

    sigmay_calc[0] = sigmay_data[0];
    sigmay_calc[1] = sigmay_data[0];
    for (j = 0; j < sigma_bins; j++)
    {
        sigmay_calc[j + 2] = sigmay_data[j];
    }
    sigmay_calc[2 + sigma_bins + 0] = sigmay_data[sigma_bins - 1];
    sigmay_calc[2 + sigma_bins + 1] = sigmay_data[sigma_bins - 1];

    for (j = 0; j < 5; j++)
        coefacc += filt_coef[j];
    for (j = 2; j < sigma_bins + 2; j++)
    {
        tmp1 = sigmay_calc[j - 2] * filt_coef[0] + sigmay_calc[j - 1] * filt_coef[1];
        tmp1 = tmp1 + sigmay_calc[j] * filt_coef[2] + sigmay_calc[j + 1] * filt_coef[3];
        tmp1 = tmp1 + sigmay_calc[j + 2] * filt_coef[4];
        sigmay_tmp[j - 2] = tmp1 / coefacc;
    }

    // calc special point
    for (j = 0; j < sigma_bins; j++)
    {
        if(sigmay_tmp[j] == 0)
        {
            for (i = 1; i < sigma_bins + 1; i++)
            {
                tmp0 = CLIP((j - i), 0, sigma_bins);
                if(sigmay_tmp[tmp0])
                    break;
                tmp0 = CLIP((j + i), 0, sigma_bins);
                if(sigmay_tmp[tmp0])
                    break;
            }
            sigmay_tmp[j] = sigmay_tmp[tmp0];
            if(i >= sigma_bins) {
                LOGW_ANR("sigmay_curve no point use!");
                pTransPrarms->bayertnr_auto_sig_count_valid = 0;
                break;
            }
        }
    }

    // sigma iir
    for (j = 1; j < sigma_bins; j++)
    {
        sigmay_tmp[j] = MAX(sigmay_tmp[j], sigmay_tmp[j - 1]);
    }

    if(pStats->sigma_num < pTransPrarms->bayertnr_auto_sig_count_max)
    {
        LOGW_ANR("sigma_curve capture 0x%x point not enough, sigma_cout_max:0x%x !\n",
                 pStats->sigma_num, pTransPrarms->bayertnr_auto_sig_count_max);
        pTransPrarms->bayertnr_auto_sig_count_valid = 0;
    }
    else
        pTransPrarms->bayertnr_auto_sig_count_valid = 1;

    if(pTransPrarms->bayertnr_auto_sig_count_valid == 0)
        iir_wgt = 1024;
    for (j = 0; j < sigma_bins; j++)
    {
        tmp = (iir_wgt * sigmay_curve[j] + (1024 - iir_wgt) * sigmay_tmp[j]) >> 10;
        sigmay_curve[j] = tmp;
    }

    /*
    printf("sigmay_curve [%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d]\n",
            sigmay_curve[0],
            sigmay_curve[1],
            sigmay_curve[2],
            sigmay_curve[3],
            sigmay_curve[4],
            sigmay_curve[5],
            sigmay_curve[6],
            sigmay_curve[7],
            sigmay_curve[8],
            sigmay_curve[9],
            sigmay_curve[10],
            sigmay_curve[11],
            sigmay_curve[12],
            sigmay_curve[13],
            sigmay_curve[14],
            sigmay_curve[15],
            sigmay_curve[16],
            sigmay_curve[17],
            sigmay_curve[18],
            sigmay_curve[19]);
    */
    return 0;
}

int bayertnr_tnr_noise_curve(int data, int isHdrShort, btnr_trans_params_t *pTransPrarms)
{
    int sigbins = 20;
    int dbl_en  = pTransPrarms->bayertnr_tnr_sigma_curve_double_en;
    int dbl_pos = pTransPrarms->bayertnr_tnr_sigma_curve_double_pos;
    int i, sigma;
    int ratio;

    for(i = 0; i < sigbins; i++)
    {
        if(data < pTransPrarms->tnr_luma_sigma_x[i])
            break;
    }

    if(isHdrShort && dbl_en && i <= dbl_pos)
        sigma = pTransPrarms->tnr_luma_sigma_y[dbl_pos];
    else if(dbl_en && i == dbl_pos)
        sigma = pTransPrarms->tnr_luma_sigma_y[dbl_pos];
    else if(i <= 0)
        sigma = pTransPrarms->tnr_luma_sigma_y[0];
    else if(i > (sigbins - 1))
        sigma = pTransPrarms->tnr_luma_sigma_y[sigbins - 1];
    else
    {
        ratio = (data - pTransPrarms->tnr_luma_sigma_x[i - 1]) * (pTransPrarms->tnr_luma_sigma_y[i] - pTransPrarms->tnr_luma_sigma_y[i - 1]);
        ratio = ratio / (pTransPrarms->tnr_luma_sigma_x[i] - pTransPrarms->tnr_luma_sigma_x[i - 1]);

        sigma = pTransPrarms->tnr_luma_sigma_y[i - 1] + ratio;
    }

    return sigma;
}
