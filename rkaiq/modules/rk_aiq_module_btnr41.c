#include "rk_aiq_isp33_modules.h"

// fix define
#define FIXTNRSQRT      40
#define FIXTNRWGT       10
#define FIXTNRKAL       8
#define FIXTNRWWW       12
#define FIXGAINOUT      8
#define FIXTNRSFT       10
#define FIXMERGEWG      10      // HDR merge weight
#define MERGEWGBIT      6       // HDR merge weight cut bits
#define FIXGAINBIT      8
#define FIXTNRCALC      10
#define FIXBILCALC      10
#define FIXDIFMAX       ((long long)1<<(14))     // DIFF USE 12bits
#define FIXBITIIRSPR    8
#define FIXBITWFWGT     8
#define FIXBITDGAIN     8

void bay_gauss5x5_spnr_coeff(float sigma, int halftaby, int halftabx, int strdtabx, int *gstab)
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
            tmpf0 = tmpf0 / (sigma * sigma);
            tmpf1 = expf(-tmpf0);
            tmpf2 = tmpf2 + tmpf1;
            gausstabf[i * halfx + j] = tmpf1;
        }
    }
    for (i = 0; i < halfy; i++)
    {
        for (j = 0; j < halfx; j++)
        {
            gausstab[i * halfx + j] = (int)(gausstabf[i * halfx + j] * (1 << 6));
            sumc = sumc + gausstab[i * halfx + j];
        }
    }

    for (i = 0; i < halfy; i++)
    {
        for (j = 0; j < halfx; j++)
        {
            gstab[gstabidx[i * halfx + j]] = gausstab[i * halfx + j];
        }
    }
}

void bay_gauss7x5_filter_coeff(float sigma, int halftaby, int halftabx, int strdtabx, int *gstab)
{
    int halfx = halftabx;
    int halfy = halftaby;
    int strdx = strdtabx;
    int stridex = strdx / 2;
    int centerx = halfx / 2;
    int centery = halfy / 2;
    int gausstab[7 * 5];
    int i, j, sumc;
    float tmpf0, tmpf1;
    float tmpf2, gausstabf[7 * 5];
    int gstabidx[7 * 5] =
    {   6, 5, 4, 3, 4, 5, 6,
        7, 4, 2, 1, 2, 4, 7,
        8, 3, 1, 0, 1, 3, 8,
        7, 4, 2, 1, 2, 4, 7,
        6, 5, 4, 3, 4, 5, 6
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
            gausstab[i * halfx + j] = (int)(gausstabf[i * halfx + j] / tmpf2 * (1 << 10));
            sumc = sumc + gausstab[i * halfx + j];
        }
    }
    gausstab[halfy / 2 * halfx + halfx / 2] += ((1 << 10) - sumc);

    for (i = 0; i < halfy; i++)
    {
        for (j = 0; j < halfx; j++)
        {
            gstab[gstabidx[i * halfx + j]] = gausstab[i * halfx + j];
        }
    }
}

int bayertnr_kalm_bitcut_V40(int datain, int bitsrc, int bitdst)
{
    int out;
    out = bitsrc == bitdst ? datain : ((datain + (1 << (bitsrc - bitdst - 1))) >> (bitsrc - bitdst));
    return out;
}

int bayertnr_wgt_sqrt_tab_V40(int index)
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

void bayertnr_luma2sigmax_config_v41(btnr_trans_params_t *pTransParams)
{
    // tnr sigma curve
    float kcoef0, kcoef1;
    int i, tmp, tmp0, tmp1;
    int max_sig, pix_max;
    int sigbins = 20;
    bool transf_bypass_en = pTransParams->isTransfBypass;

    pix_max = !transf_bypass_en  ? ((1 << 12) - 1) : bayertnr_logtrans((1 << 12) - 1, pTransParams);
    if(pTransParams->isHdrMode)
    {
        pTransParams->bayertnr_tnr_sigma_curve_double_en = 1;
        pTransParams->bayertnr_tnr_sigma_curve_double_pos = 10;
        // hdr long bins
        int lgbins = pTransParams->bayertnr_tnr_sigma_curve_double_pos;

        float blc_remain_log = 0;
        /*
        if(pExpInfo->blc1_enable) {
            blc_remain_log = MAX(blc_remain_log, pExpInfo->blc1_r);
            blc_remain_log = MAX(blc_remain_log, pExpInfo->blc1_gr);
            blc_remain_log = MAX(blc_remain_log, pExpInfo->blc1_gb);
            blc_remain_log = MAX(blc_remain_log, pExpInfo->blc1_b);
        }

        if(pExpInfo->blc_ob_offset != 0 && pExpInfo->blc_ob_enable) {
            blc_remain_log += pExpInfo->blc_ob_offset;
        }
        */

        for(i = 0; i < lgbins; i++)
        {
            pTransParams->tnr_luma_sigma_x[i] = pix_max * (i + 1) / lgbins; //pSelect->bayertnr_tnr_lum[i];
        }
        pTransParams->tnr_luma_sigma_x[lgbins - 1] = pix_max;

        // hdr short bins
        int shbins = sigbins - lgbins;
        i = 8;

        for(i = lgbins; i < lgbins + 6; i++) {
            tmp0 = 128 * (i - lgbins + 1)  + pTransParams->tnr_luma_sigma_x[lgbins - 1]; //pParser->bayertnr_tnr_lum[i];
            pTransParams->tnr_luma_sigma_x[i] = tmp0;
        }
        for(i = lgbins + 6; i < sigbins; i++) {
            tmp0 = 256 * (i - lgbins - 6 + 1)  + pTransParams->tnr_luma_sigma_x[lgbins + 6 - 1]; //pParser->bayertnr_tnr_lum[i];
            pTransParams->tnr_luma_sigma_x[i] = tmp0;
        }
        pTransParams->tnr_luma_sigma_x[sigbins - 1] = pix_max;

    } else if(!transf_bypass_en)  {
        int segs = 6;
        pTransParams->bayertnr_tnr_sigma_curve_double_en = 0;
        pTransParams->bayertnr_tnr_sigma_curve_double_pos = 0;

        float blc_remain_log = 0;
        /*
        if(pExpInfo->blc1_enable) {
            blc_remain_log = MAX(blc_remain_log, pExpInfo->blc1_r);
            blc_remain_log = MAX(blc_remain_log, pExpInfo->blc1_gr);
            blc_remain_log = MAX(blc_remain_log, pExpInfo->blc1_gb);
            blc_remain_log = MAX(blc_remain_log, pExpInfo->blc1_b);
        }

        if(pExpInfo->blc_ob_offset != 0 && pExpInfo->blc_ob_enable) {
            blc_remain_log += pExpInfo->blc_ob_offset;
        }
        */

        int small_step = 32;

        /*
        if(pExpInfo->blc_ob_predgain > 1.0) {
            blc_remain_log *= pExpInfo->blc_ob_predgain;
            pix_max = bayertnr_logtrans((1 << 12) * pExpInfo->blc_ob_predgain - 1);

        } else {
            pix_max = bayertnr_logtrans((1 << 12) * 1 - 1, pFix);
        }
        */

        if(blc_remain_log != 0) {
            pTransParams->tnr_luma_sigma_x[0] = bayertnr_logtrans(blc_remain_log, pTransParams);
            small_step = (pix_max - pTransParams->tnr_luma_sigma_x[0]) / 33;
        } else {
            small_step = (pix_max) / 34;
            pTransParams->tnr_luma_sigma_x[0] = small_step;
        }


        for(i = 1; i < segs; i++) {
            pTransParams->tnr_luma_sigma_x[i] = small_step * (i + 0) + pTransParams->tnr_luma_sigma_x[0];
        }
        for(i = segs; i < sigbins; i++) {
            pTransParams->tnr_luma_sigma_x[i] = small_step * 2 * (i - segs + 1) + pTransParams->tnr_luma_sigma_x[segs - 1];
        }
        pTransParams->tnr_luma_sigma_x[sigbins - 1] = pix_max;
    }
    else
    {
        pTransParams->bayertnr_tnr_sigma_curve_double_en = 0;
        pTransParams->bayertnr_tnr_sigma_curve_double_pos = 0;
        float blc_remain = 0;
        /*
        if(pExpInfo->blc1_enable) {
            blc_remain = MAX(blc_remain, pExpInfo->blc1_r);
            blc_remain = MAX(blc_remain, pExpInfo->blc1_gr);
            blc_remain = MAX(blc_remain, pExpInfo->blc1_gb);
            blc_remain = MAX(blc_remain, pExpInfo->blc1_b);
        }
        if(pExpInfo->blc_ob_offset != 0 && pExpInfo->blc_ob_enable) {
            blc_remain += pExpInfo->blc_ob_offset;
        }
        */

        if(blc_remain != 0) {
            pTransParams->tnr_luma_sigma_x[0] = blc_remain;
        } else {
            pTransParams->tnr_luma_sigma_x[0] = 128;
        }

        for(i = 1; i < 8; i++) {
            pTransParams->tnr_luma_sigma_x[i] = 128 * (i + 0) + pTransParams->tnr_luma_sigma_x[0];
        }
        for(i = 8; i < sigbins; i++) {
            tmp0 = 256 * (i - 8 + 1)  + pTransParams->tnr_luma_sigma_x[ 8 - 1];
            pTransParams->tnr_luma_sigma_x[i] = tmp0;
        }
        pTransParams->tnr_luma_sigma_x[sigbins - 1] = pix_max;
    }

}

void rk_aiq_btnr41_params_cvt(void* attr, isp_params_t* isp_params, common_cvt_info_t* cvtinfo, btnr_cvt_info_t* pBtnrInfo)
{
    btnr_trans_params_t *pTransParams = &pBtnrInfo->mBtnrTransParams;
    btnr_stats_t *btnr_stats = &pBtnrInfo->mBtnrStats[0];
    bool bypass = pBtnrInfo->bypass;

    if (cvtinfo->isFirstFrame) {
        memset(pBtnrInfo, 0, sizeof(btnr_cvt_info_t));
    } else {
        btnr_stats = bayertnr_get_stats(pBtnrInfo, cvtinfo->frameId);
        if (cvtinfo->frameId-BAYERTNR_STATS_DELAY != btnr_stats->id) {
            LOGE_ANR("Btnr stats miss match! (%d %d)", cvtinfo->frameId, btnr_stats->id);
        }
    }

    float tmpf;
    int i, j, tmp, tmp0, tmp1, sigbins, halfx;
    struct isp33_bay3d_cfg *pCfg = &isp_params->isp_cfg->others.bay3d_cfg;

    btnr_param_t *btnr_param = (btnr_param_t *) attr;
    btnr_params_static_t* psta = &btnr_param->sta;
    btnr_other_dyn_t* pdyn = &btnr_param->spNrDyn;
    btnr_mdMe_dyn_t* pmdDyn = &btnr_param->mdMeDyn;
	
    bool bayertnr_default_noise_curve_use = false;
    int bayertnr_iso_cur = cvtinfo->frameIso[cvtinfo->frameNum - 1];
    bayertnr_default_noise_curve_use = bayertnr_find_top_one_pos(bayertnr_iso_cur) != bayertnr_find_top_one_pos(pTransParams->bayertnr_iso_pre);
    pTransParams->bayertnr_iso_pre = bayertnr_iso_cur;
    pTransParams->isFirstFrame = cvtinfo->isFirstFrame;
    pTransParams->isHdrMode = cvtinfo->frameNum == 2;

/*	
    float frameiso[3];
    float frameEt[3];
    float fdGain[3];
	
    int framenum = cvtinfo->frameNum;
    frameiso[0] = cvtinfo->frameIso[0];
    frameiso[1] = cvtinfo->frameIso[1];
    frameiso[2] = cvtinfo->frameIso[2];

    frameEt[0] = cvtinfo->frameEt[0];
    frameEt[1] = cvtinfo->frameEt[1];
    frameEt[2] = cvtinfo->frameEt[2];
    for (i = 0; i < framenum; i++) {
        fdGain[i] = frameiso[i] * frameEt[i];
    }
    for (i = 0; i < framenum; i++) {
        fdGain[i] = fdGain[framenum - 1] / fdGain[i];
    }
    pFix->rawWidth          = cvtinfo->rawWidth;
    pFix->rawHeight         = cvtinfo->rawHeight;
    pFix->bayertnr_framenum_hdr = cvtinfo->frameNum;
    pFix->bayertnr_framecnt  = cvtinfo->frameId;
*/
	
    // REG: BAY3D_CTRL0
    pCfg->bypass_en = 0;
    if (pdyn->sw_btnrT_outFrmBase_mode == btnr_curBaseOut_mode) {
        pCfg->out_use_pre_mode = pdyn->noiseBal_curBaseOut.sw_btnrT_noiseBal_mode;
    } else {
        pCfg->out_use_pre_mode = pdyn->noiseBal_preBaseOut.sw_btnrT_noiseBal_mode;
    }
    //pCfg->out_use_pre_mode = CLIP(pdyn->hw_btnrT_noiseBal_mode, 0, 0x7);
    pCfg->motion_est_en = pmdDyn->motionEst.hw_btnrT_me_en;

    // REG: BAY3D_CTRL1
    switch (psta->hw_btnrCfg_pixDomain_mode) {
    case btnr_pixLog2Domain_mode :
        pCfg->transf_bypass_en = 0;
        pCfg->transf_mode  = 0;
        break;
    case btnr_pixLinearDomain_mode:
        pCfg->transf_bypass_en = 1;
        break;
    }
    switch (pmdDyn->loMd.hw_btnrT_loMd_mode) {
    case btnr_allSubLoMdMix_mode:
        pCfg->lo_mge_wgt_mode = 0;
        break;
    case btnr_subLoMd0Only_mode:
        pCfg->lo_mge_wgt_mode = 1;
        break;
    case btnr_subLoMd1Only_mode:
        pCfg->lo_mge_wgt_mode = 2;
        break;
    case btnr_subExtLoMdOnly_mode:
        pCfg->lo_mge_wgt_mode = 3;
        break;
    }
    if (psta->debug.sw_btnrT_dbgOut_en) {
        switch (psta->debug.hw_btnrT_dbgOut_mode) {
        case btnr_dbgOut_preSpNr_mode:
            pCfg->curdbg_en = 0;
            pCfg->iirdbg_en = 1;
            pCfg->wgtdbg_en = 0;
            break;
        case btnr_dbgOut_curSpNr_mode :
            pCfg->curdbg_en = 1;
            pCfg->iirdbg_en = 0;
            pCfg->wgtdbg_en = 0;
            break;
        case btnr_dbgOut_mdWgt_mode:
            pCfg->curdbg_en = 0;
            pCfg->iirdbg_en = 0;
            pCfg->wgtdbg_en = 1;
            break;
        }
    } else {
        pCfg->wgtdbg_en = 0;
        pCfg->iirdbg_en = 0;
        pCfg->curdbg_en = 0;
    }

    if (pmdDyn->subExtLoMd.sw_btnrT_extLoSrc_mode == btnr_subLoMd1_mode)
        pCfg->md_large_lo_use_mode = 0;
    else
        pCfg->md_large_lo_use_mode = 1;

    pCfg->tnrsigma_curve_double_en =  (cvtinfo->frameNum > 1) ? 1 : 0;
    pCfg->md_large_lo_min_filter_en = !pmdDyn->subExtLoMd.hw_btnrT_minFilt_en;
    pCfg->md_large_lo_gauss_filter_en = !pmdDyn->subExtLoMd.hw_btnrT_lpf_en;
    pCfg->md_large_lo_md_wgt_en = !pmdDyn->subExtLoMd.hw_btnrT_extLoMd_en;
    //pCfg->pre_ddr_out_mode = CLIP(pFix->bayertnr_iir_pix_out_mode, 0, 0x1);
    pCfg->lpf_hi_en = !pmdDyn->loAsRatio_hiMd0.hw_btnrT_hfLpf_en;
    pCfg->lo_diff_vfilt_en = !pmdDyn->subLoMd0.diffCh.hw_btnrT_vFilt_en;
    pCfg->lpf_lo_en = !pmdDyn->subLoMd1.hw_btnrT_lpf_en;
    pCfg->lo_wgt_hfilt_en = pmdDyn->subLoMd1.hw_btnrT_hFilt_en;
    pCfg->lo_diff_hfilt_en = pmdDyn->subLoMd0.diffCh.hw_btnrT_hFilt_en;
    pCfg->sig_hfilt_en = pmdDyn->subLoMd0.sgmCh.hw_btnrT_hFilt_en;
    pCfg->md_bypass_en = !pmdDyn->hw_btnrT_md_en;

    // REG: BAY3D_CTRL2
    if (pdyn->preSpNr.sigma.hw_btnrT_sigma_mode == btnr_lutSgmOnly_mode)
        pCfg->spnr_presigma_use_en = 0;
    else
        pCfg->spnr_presigma_use_en = 1;

    if (pdyn->preSpNr.sigma.hw_btnrT_sigmaCurve_mode == btnr_midSegmInterpOn_mode)
        pCfg->baynr1_sigma_dbl_en = 0;
    else
        pCfg->baynr1_sigma_dbl_en = 1;

    if (pdyn->preSpNr.sigma.hw_btnrT_idxLpfStrg_mode == btnr_lpfStrgH_mode)
        pCfg->pre_spnr_sigma_idx_filt_mode = 0;
    else
        pCfg->pre_spnr_sigma_idx_filt_mode = 1;

    switch (pdyn->preSpNr.hiNr.sw_btnrT_rgeWgt_mode) {
    case btnr_bayerAvg_mode:
        pCfg->pre_spnr_hi_filter_wgt_mode = 0;
        break;
    case btnr_gIndividual_rbAvg_mode :
        pCfg->pre_spnr_hi_filter_wgt_mode = 1;
        pCfg->pre_spnr_hi_filter_rb_wgt_mode = 0;
        break;
    case btnr_bayerIndividual_mode :
        pCfg->pre_spnr_hi_filter_wgt_mode = 1;
        pCfg->pre_spnr_hi_filter_rb_wgt_mode = 1;
        break;
    }
    switch (pdyn->preSpNr.loNr.sw_btnrT_rgeWgt_mode) {
    case btnr_bayerAvg_mode:
        pCfg->pre_spnr_lo_filter_wgt_mode = 0;
        break;
    case btnr_gIndividual_rbAvg_mode:
        pCfg->pre_spnr_lo_filter_wgt_mode = 1;
        pCfg->pre_spnr_lo_filter_rb_wgt_mode = 0;
        break;
    case btnr_bayerIndividual_mode:
        pCfg->pre_spnr_lo_filter_wgt_mode = 1;
        pCfg->pre_spnr_lo_filter_rb_wgt_mode = 1;
        break;
    }
    pCfg->cur_spnr_filter_en = !pdyn->curSpNr.hw_btnrT_spNr_en;
    pCfg->pre_spnr_hi_filter_gic_en = pdyn->preSpNr.hiNr.hw_btnrT_gic_en;
    pCfg->pre_spnr_hi_filter_gic_enhance_en = pdyn->preSpNr.hiNr.hw_btnrT_gicEnhance_en;
    pCfg->pre_spnr_lo_filter_en = !pdyn->preSpNr.loNr.hw_btnrT_loNr_en;
    pCfg->pre_spnr_hi_filter_en = !pdyn->preSpNr.hiNr.hw_btnrT_hiNr_en;
    pCfg->pre_spnr_hi_guide_filter_en = !pdyn->preSpNr.hiNr.hw_btnrT_guideLpf_en;
    pCfg->pre_spnr_sigma_idx_filt_en = !pdyn->preSpNr.sigma.hw_btnrT_sigmaIdxLpf_en;
    pCfg->pre_spnr_hi_noise_ctrl_en = pdyn->preSpNr.hiNr.hw_btnrT_guideNsCtrl_en;
    pCfg->pre_hi_gic_lp_en =  psta->lowPowerCfg.preSpnrLP.hiNrLP.hw_btnrCfg_gicLP_en;
    pCfg->pre_hi_bf_lp_en =  psta->lowPowerCfg.preSpnrLP.hiNrLP.hw_btnrCfg_bfLP_en;
    pCfg->pre_lo_avg_lp_en =  psta->lowPowerCfg.preSpnrLP.loNrLP.hw_btnrCfg_avgRgeWgtLP_en;

    // REG: BAY3D_CTRL3
    pCfg->lomdwgt_dbg_en = 0;
    pCfg->lo_detection_en = 1;
    if (pmdDyn->hw_btnrT_md_mode == btnr_loAsRatio_hi0_mode) {
        pCfg->wgt_cal_mode = 0;
    } else {
        pCfg->wgt_cal_mode = 1;
    }
    pCfg->lo_detection_en = pmdDyn->loMd.hw_btnrT_loMd_en;
    //pCfg->lomdwgt_dbg_en = 1;

    if (psta->hw_btnrCfg_megWgtDs_mode == btnr_megWgtDs_avg_mode)
        pCfg->ww_mode = 0;
    else
        pCfg->ww_mode = 1;

    pCfg->transf_mode = 0;
    pCfg->wgt_last_mode = CLIP(psta->hw_btnrCfg_kalWgtDs_mode, 0, 0x3);
    pCfg->mge_wgt_hdr_sht_thred = (1 << MERGEWGBIT) - 1;
    pCfg->sigma_calc_mge_wgt_hdr_sht_thred = (1 << MERGEWGBIT) - 1;

    // REG: BAY3D_TRANS0
    if (psta->transCfg.hw_btnrCfg_trans_mode == btnr_pixInBw15b_mode)
        pCfg->transf_mode_scale = 1;
    else
        pCfg->transf_mode_scale = 0;

    tmp = psta->transCfg.hw_btnrCfg_trans_offset;
    int n  = bayertnr_find_top_one_pos(tmp);
    if(pCfg->transf_mode_scale == 1) {
        n = 8;
    } else {
        if(n < 8)
            n = 8;
        if(n > 12)
            n = 12;
    }
    tmp = (1 << n);
    pCfg->transf_mode_offset = CLIP(tmp, 0, 0x1fff);

    if (pCfg->transf_mode_scale == 1) {
        pCfg->itransf_mode_offset = (1 << 9) * n;
    } else {
        pCfg->itransf_mode_offset = (1 << 8) * n;
    }

    // REG: BAY3D_TRANS1
    // mode scale只有0和1两种选择，对应log域的小数精度，
    // 0对应8bit小数，4bit整数， 4bit整数导致最大线性输入数据位宽为20bit->transData_maxlimit为2的20次方-1
    // 1对应9bit小数，3bit整数， 3bit整数限制最大线性输入数据位宽为15bit->transData_maxlimit为2的15次方-1
    if(pCfg->transf_bypass_en == 1) {
        pCfg->transf_data_max_limit = ((1 << 12) - 1);
    } else {
        if (pCfg->transf_mode_scale == 1) {
            pCfg->transf_data_max_limit = ((1 << 15) - 1);
        }
        else {
            pCfg->transf_data_max_limit = ((1 << 20) - 1);
        }
    }

    // REG: BAY3D_CURHI_SIGSCL
    tmp = (pdyn->preSpNr.sigma.hw_btnrT_preSigma_scale) * (1 << 0);
    pCfg->pre_sig_ctrl_scl = CLIP(tmp, 0, 0xfff);
    // REG: BAY3D_CURHI_SIGOF
    tmp = (pdyn->preSpNr.hiNr.hw_btnrT_guideLpf3_alpha) * (1 << 6);
    pCfg->pre_hi_guide_out_wgt = CLIP(tmp, 0, 0x7f);

    {
        int coeff[6];
        if (pdyn->curSpNr.sw_btnrT_filtCfg_mode == btnr_cfgByFiltStrg_mode) {
            bay_gauss5x5_filter_coeff(pdyn->curSpNr.sw_btnrT_filtSpatial_strg, 5, 5, 5, coeff, (1 << 8));
        } else {
            for (i=0; i<6; i++)
                coeff[i] = pdyn->curSpNr.sw_btnrT_filtSpatial_wgt[i];
        }
        // REG: BAY3D_CURHISPW0
        pCfg->cur_spnr_filter_coeff[0] = CLIP(coeff[0], 0, 0xff);
        pCfg->cur_spnr_filter_coeff[1] = CLIP(coeff[1], 0, 0x3f);
        pCfg->cur_spnr_filter_coeff[2] = CLIP(coeff[2], 0, 0x3f);
        // REG: BAY3D_CURHISPW1
        pCfg->cur_spnr_filter_coeff[3] = CLIP(coeff[3], 0, 0x3f);
        pCfg->cur_spnr_filter_coeff[4] = CLIP(coeff[4], 0, 0x3f);
        pCfg->cur_spnr_filter_coeff[5] = CLIP(coeff[5], 0, 0x3f);
    }


    // REG: BAY3D_PREHI_SIGSCL
    tmp = (pdyn->preSpNr.hiNr.hw_btnrT_rgeSgm_scale) * (1 << FIXBITDGAIN);
    pCfg->pre_spnr_hi_sigma_scale = CLIP(tmp, 0, 0xfff);
    // REG: BAY3D_PREHI_WSCL
    tmp = (pdyn->preSpNr.hiNr.hw_btnrT_diffSgmRatio_scale) * (1 << 6);
    pCfg->pre_spnr_hi_wgt_calc_scale = CLIP(tmp, 0, 0xff);
    // REG: BAY3D_PREHIWMM
    tmp = (pdyn->preSpNr.hiNr.hw_btnrT_centerWgt_minLimit) * (1 << 8);
    pCfg->pre_spnr_hi_filter_wgt_min_limit = CLIP(tmp, 0, 0xff);
    // REG: BAY3D_PREHISIGOF
    tmp = (pdyn->preSpNr.hiNr.hw_btnrT_hiNrOut_alpha) * (1 << 7);
    pCfg->pre_spnr_hi_filter_out_wgt = CLIP(tmp, 0, 0xff);
    tmp = (pdyn->preSpNr.sigma.hw_btnrT_sigma_offset) * (1 << FIXBITDGAIN);
    pCfg->pre_spnr_sigma_offset = CLIP(tmp, 0, 0xff);
    tmp = (pdyn->preSpNr.sigma.hw_btnrT_sigmaHdrS_offset) * (1 << FIXBITDGAIN);
    pCfg->pre_spnr_sigma_hdr_sht_offset = CLIP(tmp, 0, 0xff);
    // REG: BAY3D_PREHISIGSCL
    tmp = (pdyn->preSpNr.sigma.hw_btnrT_sigma_scale) * (1 << FIXBITDGAIN);
    pCfg->pre_spnr_sigma_scale = CLIP(tmp, 0, 0x3fff);
    tmp = (pdyn->preSpNr.sigma.hw_btnrT_sigmaHdrS_scale) * (1 << FIXBITDGAIN);
    pCfg->pre_spnr_sigma_hdr_sht_scale = CLIP(tmp, 0, 0x3fff);

    {
        int coeff[6];
        if (pdyn->preSpNr.hiNr.sw_btnrT_filtCfg_mode == btnr_cfgByFiltStrg_mode) {
            bay_gauss5x5_spnr_coeff(pdyn->preSpNr.hiNr.sw_btnrT_filtSpatial_strg, 5, 5, 5, coeff);
        } else {
            for (i=0; i<6; i++)
                coeff[i] = pdyn->preSpNr.hiNr.sw_btnrT_filtSpatial_wgt[i];
        }
        // REG: BAY3D_PREHISPW0
        pCfg->pre_spnr_hi_filter_coeff[0] = CLIP(coeff[0], 0, 0x3f);
        pCfg->pre_spnr_hi_filter_coeff[1] = CLIP(coeff[1], 0, 0x3f);
        pCfg->pre_spnr_hi_filter_coeff[2] = CLIP(coeff[2], 0, 0x3f);
        // REG: BAY3D_PREHISPW1
        pCfg->pre_spnr_hi_filter_coeff[3] = CLIP(coeff[3], 0, 0x3f);
        pCfg->pre_spnr_hi_filter_coeff[4] = CLIP(coeff[4], 0, 0x3f);
        pCfg->pre_spnr_hi_filter_coeff[5] = CLIP(coeff[5], 0, 0x3f);
    }

    // REG: BAY3D_PRELOSIGCSL
    tmp = (pdyn->preSpNr.hiNr.hw_btnrT_rgeSgm_scale) * (1 << FIXBITDGAIN);
    pCfg->pre_spnr_lo_sigma_scale = CLIP(tmp, 0, 0xfff);
    // REG: BAY3D_PRELOSIGOF
    tmp = (pdyn->preSpNr.loNr.hw_btnrT_rgeWgt_scale) * (1 << 6);
    pCfg->pre_spnr_lo_wgt_calc_scale = CLIP(tmp, 0, 0xff);
    // REG: BAY3D_PREHI_NRCT
    tmp = (pdyn->preSpNr.hiNr.hw_btnrT_guideNsCtrl_scale) * (1 << 7);
    pCfg->pre_spnr_hi_noise_ctrl_scale = CLIP(tmp, 0, 0x3ff);
    tmp = (pdyn->preSpNr.hiNr.hw_btnrT_guideNsCtrl_offset) * (1 << 7);
    pCfg->pre_spnr_hi_noise_ctrl_offset = CLIP(tmp, 0, 0x7f);


    {
        int coeff[9];
        if (pmdDyn->loAsRatio_hiMd0.sw_btnrT_hfLpfCfg_mode == btnr_cfgByFiltStrg_mode) {
            bay_gauss7x5_filter_coeff(pmdDyn->loAsRatio_hiMd0.sw_btnrT_hfLpf_strg, 5, 7, 7, coeff);
        } else {
            for (i=0; i<9; i++)
                coeff[i] = pmdDyn->loAsRatio_hiMd0.hw_btnrT_hfLpfSpatial_wgt[i];
        }
        // REG: BAY3D_HIWD0
        pCfg->lpf_hi_coeff[0] = CLIP(coeff[0], 0, 0x3ff);
        pCfg->lpf_hi_coeff[1] = CLIP(coeff[1], 0, 0x3ff);
        pCfg->lpf_hi_coeff[2] = CLIP(coeff[2], 0, 0x3ff);
        // REG: BAY3D_HIWD3
        pCfg->lpf_hi_coeff[3] = CLIP(coeff[3], 0, 0x3ff);
        pCfg->lpf_hi_coeff[4] = CLIP(coeff[4], 0, 0x3ff);
        pCfg->lpf_hi_coeff[5] = CLIP(coeff[5], 0, 0x3ff);
        // REG: BAY3D_HIWD6
        pCfg->lpf_hi_coeff[6] = CLIP(coeff[6], 0, 0x3ff);
        pCfg->lpf_hi_coeff[7] = CLIP(coeff[7], 0, 0x3ff);
        pCfg->lpf_hi_coeff[8] = CLIP(coeff[8], 0, 0x3ff);
    }

    {
        int coeff[9];
        if (pmdDyn->subLoMd1.sw_btnrT_lpfCfg_mode == btnr_cfgByFiltStrg_mode) {
            bay_gauss7x5_filter_coeff(pmdDyn->subLoMd1.sw_btnrT_lpf_strg, 5, 7, 7, coeff);
        } else {
            for (i=0; i<9; i++)
                coeff[i] = pmdDyn->subLoMd1.hw_btnrT_lpfSpatial_wgt[i];
        }
        // REG: BAY3D_HIWD0
        pCfg->lpf_hi_coeff[0] = CLIP(coeff[0], 0, 0x3ff);
        pCfg->lpf_hi_coeff[1] = CLIP(coeff[1], 0, 0x3ff);
        pCfg->lpf_hi_coeff[2] = CLIP(coeff[2], 0, 0x3ff);
        // REG: BAY3D_HIWD3
        pCfg->lpf_hi_coeff[3] = CLIP(coeff[3], 0, 0x3ff);
        pCfg->lpf_hi_coeff[4] = CLIP(coeff[4], 0, 0x3ff);
        pCfg->lpf_hi_coeff[5] = CLIP(coeff[5], 0, 0x3ff);
        // REG: BAY3D_HIWD6
        pCfg->lpf_hi_coeff[6] = CLIP(coeff[6], 0, 0x3ff);
        pCfg->lpf_hi_coeff[7] = CLIP(coeff[7], 0, 0x3ff);
        pCfg->lpf_hi_coeff[8] = CLIP(coeff[8], 0, 0x3ff);
    }

    {
        int coeff[6];
        if (pmdDyn->mdSigma.sw_btnrT_sigmaIdxFltCft_mode == btnr_cfgByFiltStrg_mode) {
            bay_gauss5x5_filter_coeff(pmdDyn->mdSigma.hw_btnrT_sigmaIdxFilt_strg, 5, 5, 5, coeff, (1 << 8));
        } else {
            for (i=0; i<6; i++)
                coeff[i] = pmdDyn->mdSigma.hw_btnrT_sigmaIdxFilt_coeff[i];
        }
        // REG: BAY3D_GF3
        pCfg->sigma_idx_filt_coeff[0] = CLIP(coeff[0], 0, 0xff);
        pCfg->sigma_idx_filt_coeff[1] = CLIP(coeff[1], 0, 0xff);
        pCfg->sigma_idx_filt_coeff[2] = CLIP(coeff[2], 0, 0xff);
        pCfg->sigma_idx_filt_coeff[3] = CLIP(coeff[3], 0, 0xff);
        // REG: BAY3D_GF4
        pCfg->sigma_idx_filt_coeff[4] = CLIP(coeff[4], 0, 0xff);
        pCfg->sigma_idx_filt_coeff[5] = CLIP(coeff[5], 0, 0xff);
    }
    tmp = (pmdDyn->subLoMd1.hw_btnrT_mdWgtFstLn_negOff) * (1 << 8);
    pCfg->lo_wgt_cal_first_line_sigma_scale = CLIP(tmp, 0, 0xfff);

    // REG: BAY3D_VIIR
    tmp = (pmdDyn->subLoMd0.diffCh.hw_btnrT_vIIRFilt_strg) * (1 << 4);
    pCfg->lo_diff_vfilt_wgt = CLIP(tmp, 0, 0x1f);
    tmp = (pmdDyn->subLoMd1.hw_btnrT_vIIRFilt_strg) * (1 << 4);
    pCfg->lo_wgt_vfilt_wgt = CLIP(tmp, 0, 0x1f);
    tmp = (pmdDyn->subLoMd0.sgmCh.hw_btnrT_vIIRFstLn_scale) * (1 << 4);
    pCfg->sig_first_line_scale = CLIP(tmp, 0, 0x3f);
    tmp = (pmdDyn->subLoMd0.diffCh.hw_btnrT_vIIRFstLn_scale) * (1 << 4);
    pCfg->lo_diff_first_line_scale = CLIP(tmp, 0, 0x3f);
    // REG: BAY3D_LFSCL
    tmp = (pmdDyn->subLoMd1.hw_btnrT_mdWgt_negOff) * (1 << 10);
    pCfg->lo_wgt_cal_offset = CLIP(tmp, 0, 0x3fff);
    tmp = (pmdDyn->subLoMd1.hw_btnrT_mdWgt_scale) * (1 << 10);
    pCfg->lo_wgt_cal_scale = CLIP(tmp, 0, 0x3fff);
    // REG: BAY3D_LFSCLTH
    tmp = (pmdDyn->subLoMd1.hw_btnrT_mdWgt_maxLimit) * (1 << 10);
    pCfg->lo_wgt_cal_max_limit = CLIP(tmp, 0, 0x3fff);
    tmp = (pmdDyn->loAsRatio_hiMd0.hw_btnrT_hiNsBase_ratio) * (1 << 10);
    pCfg->mode0_base_ratio = CLIP(tmp, 0, 0x3fff);
    // REG: BAY3D_DSWGTSCL
    tmp = (pmdDyn->subLoMd0.wgtOpt.hw_btnrT_mdWgt_negOff) * (1 << 10);
    pCfg->lo_diff_wgt_cal_offset = CLIP(tmp, 0, 0x3fff);
    tmp = (pmdDyn->subLoMd0.wgtOpt.hw_btnrT_mdWgt_scale) * (1 << 10);
    pCfg->lo_diff_wgt_cal_scale = CLIP(tmp, 0, 0x3fff);
    // REG: BAY3D_WGTlastSCL
    tmp = (pmdDyn->loMd.hw_btnrT_preWgtMge_offset) * (1 << 10);
    pCfg->lo_mge_pre_wgt_offset = CLIP(tmp, 0, 0x3fff);
    tmp = (pmdDyn->loMd.hw_btnrT_preWgtMge_scale) * (1 << 10);
    pCfg->lo_mge_pre_wgt_scale = CLIP(tmp, 0, 0x3fff);
    // REG: BAY3D_WGTSCL0
    tmp = (pmdDyn->loAsRatio_hiMd0.hw_btnrT_loWgtStat_scale) * (1 << 10);
    pCfg->mode0_lo_wgt_scale = CLIP(tmp, 0, 0x3fff);
    tmp = (pmdDyn->loAsRatio_hiMd0.hw_btnrT_loWgtStatHdrS_scale) * (1 << 10);
    pCfg->mode0_lo_wgt_hdr_sht_scale = CLIP(tmp, 0, 0x3fff);
    // REG: BAY3D_WGTSCL1
    tmp = (pmdDyn->loAsBias_hiMd1.hw_btnrT_loWgtStat_scale) * (1 << 10);
    pCfg->mode1_lo_wgt_scale = CLIP(tmp, 0, 0x3fff);
    tmp = (pmdDyn->loAsBias_hiMd1.hw_btnrT_loWgtStatHdrS_scale) * (1 << 10);
    pCfg->mode1_lo_wgt_hdr_sht_scale = CLIP(tmp, 0, 0x3fff);
    // REG: BAY3D_WGTSCL2
    tmp = (pmdDyn->loAsBias_hiMd1.hw_btnrT_mdWgt_scale) * (1 << 10);
    pCfg->mode1_wgt_scale = CLIP(tmp, 0, 0x3fff);
    tmp = (pmdDyn->loAsBias_hiMd1.hw_btnrT_mdWgtHdrS_scale) * (1 << 10);
    pCfg->mode1_wgt_hdr_sht_scale = CLIP(tmp, 0, 0x3fff);
    // REG: BAY3D_WGTOFF
    tmp = (pmdDyn->loAsBias_hiMd1.hw_btnrT_loWgtStat_negOff) * (1 << 10);
    pCfg->mode1_lo_wgt_offset = CLIP(tmp, 0, 0x3fff);
    tmp = (pmdDyn->loAsBias_hiMd1.hw_btnrT_loWgtStatHdrS_negOff) * (1 << 10);
    pCfg->mode1_lo_wgt_hdr_sht_offset = CLIP(tmp, 0, 0x3fff);
    // REG: BAY3D_WGT1OFF
    tmp = (pdyn->sigmaEnv.hw_btnrT_statsPixAlpha_thred) * (1 << 10);
    pCfg->auto_sigma_count_wgt_thred = CLIP(tmp, 0, 0x3ff);
    tmp = (pmdDyn->loAsBias_hiMd1.hw_btnrT_mdWgt_minLimit) * (1 << 10);
    pCfg->mode1_wgt_min_limit = CLIP(tmp, 0, 0x3ff);
    tmp = (pmdDyn->loAsBias_hiMd1.hw_btnrT_mdWgt_offset) * (1 << 10);
    pCfg->mode1_wgt_offset = CLIP(tmp, 0, 0xfff);
    // REG: BAY3D_WGTLO_L
    tmpf = pmdDyn->frmFusion.hw_btnrT_loFusion_minLimit;
    tmp = tmpf > 4095 ? 4095 : (tmpf == 0 ? 0 : (int)((1.0 - 1.0 / tmpf) * (1 << FIXTNRWWW)));
    pCfg->lo_wgt_clip_min_limit = CLIP(tmp, 0, 0x3fff);
    tmpf = pmdDyn->frmFusion.hw_btnrT_loFusionHdrS_minLimit;
    tmp = tmpf > 4095 ? 4095 : (tmpf == 0 ? 0 : (int)((1.0 - 1.0 / tmpf) * (1 << FIXTNRWWW)));
    pCfg->lo_wgt_clip_hdr_sht_min_limit = CLIP(tmp, 0, 0x3fff);
    // REG: BAY3D_WGTLO_H
    tmpf = pmdDyn->frmFusion.hw_btnrT_loFusion_maxLimit;
    tmp = tmpf > 4095 ? 4095 : (tmpf == 0 ? 0 : (int)((1.0 - 1.0 / tmpf) * (1 << FIXTNRWWW)));
    pCfg->lo_wgt_clip_max_limit = CLIP(tmp, 0, 0x3fff);
    tmpf = pmdDyn->frmFusion.hw_btnrT_loFusionHdrS_maxLimit;
    tmp = tmpf > 4095 ? 4095 : (tmpf == 0 ? 0 : (int)((1.0 - 1.0 / tmpf) * (1 << FIXTNRWWW)));
    pCfg->lo_wgt_clip_hdr_sht_max_limit = CLIP(tmp, 0, 0x3fff);
    // REG: BAY3D_STH_SCL
    tmp = (pmdDyn->frmFusion.hw_btnrT_loGgSoftThd_scale) * (1 << 8);
    pCfg->lo_pre_gg_soft_thresh_scale = CLIP(tmp, 0, 0xfff);
    tmp = (pmdDyn->frmFusion.hw_btnrT_loRbSoftThd_scale) * (1 << 8);
    pCfg->lo_pre_rb_soft_thresh_scale = CLIP(tmp, 0, 0xfff);
    // REG: BAY3D_STH_LIMIT
    tmp = (pmdDyn->frmFusion.hw_btnrT_loSoftThd_maxLimit);
    pCfg->lo_pre_soft_thresh_max_limit = CLIP(tmp, 0, 0xfff);
    tmp = (pmdDyn->frmFusion.hw_btnrT_loSoftThd_minLimit);
    pCfg->lo_pre_soft_thresh_min_limit = CLIP(tmp, 0, 0xfff);
    // REG: BAY3D_HIKEEP
    //tmp = (pdyn->noiseBal_curBaseOut.hw_btnrT_curHiOrg_alpha) * (1 << FIXBITWFWGT);
    //pCfg->cur_spnr_hi_wgt_min_limit = CLIP(tmp, 0, 0xff);
    //tmp = (pdyn->noiseBal_curBaseOut.hw_btnrT_iirHiOrg_alpha) * (1 << FIXBITWFWGT);
    //pCfg->pre_spnr_hi_wgt_min_limit = CLIP(tmp, 0, 0xff);
    tmp = (pmdDyn->motionEst.hw_btnrT_meLoWgt_thred) * (1 << 10);
    pCfg->motion_est_lo_wgt_thred = CLIP(tmp, 0, 0x3ff);
    // REG: BAY3D_SIGNUMTH
    tmp = (psta->sigmaEnv.hw_btnrCfg_statsPixCnt_thred);
    pCfg->sigma_num_th = CLIP(tmp, 0, 0xfffff);
    // REG: BAY3D_MONR
    if (pdyn->sw_btnrT_outFrmBase_mode == btnr_curBaseOut_mode) {
        tmp0 = MIN((sqrt((float)1.0 / pdyn->noiseBal_curBaseOut.hw_btnrT_hiNoiseBal_strg) * (1 << 10)), (1 << 10));
        tmp1 = MIN((sqrt((float)1.0 / pdyn->noiseBal_curBaseOut.hw_btnrT_midNoiseBal_strg) * (1 << 10)), (1 << 10));
    } else {
        tmp0 = MIN((sqrt((float)1.0 / pdyn->noiseBal_preBaseOut.hw_btnrT_hiNoiseBal_strg) * (1 << 10)), (1 << 10));
        tmp1 = MIN((sqrt((float)1.0 / pdyn->noiseBal_preBaseOut.hw_btnrT_midNoiseBal_strg) * (1 << 10)), (1 << 10));
    }
    pCfg->out_use_hi_noise_bal_nr_strg = CLIP(((tmp1 << 11) | tmp0), 0, 0x3fffff);
    tmp = (pdyn->locSgmStrg.hw_bnrT_locSgmStrg_maxLimit) * (1 << 8);
    pCfg->gain_out_max_limit = CLIP(tmp, 0, 0xff);
    // REG: BAY3D_SIGSCL
    tmp = (pmdDyn->mdSigma.hw_btnrT_sigma_scale) * (1 << 10);
    pCfg->sigma_scale = CLIP(tmp, 0, 0x3fff);
    tmp = (pmdDyn->mdSigma.hw_btnrT_sigmaHdrS_scale) * (1 << 10);
    pCfg->sigma_hdr_sht_scale = CLIP(tmp, 0, 0x3fff);
    // REG: BAY3D_DSOFF
    tmp = (pmdDyn->subLoMd1.hw_btnrT_vIIRWgt_offset) * (1 << 10);
    pCfg->lo_wgt_vfilt_offset = CLIP(tmp, 0, 0x3ff);
    tmp = (pmdDyn->subLoMd0.diffCh.hw_btnrT_vIIRWgt_offset) * (1 << 12);
    pCfg->lo_diff_vfilt_offset = CLIP(tmp, 0, 0xfff);
    tmp = (0) * (1 << 4);
    pCfg->lo_wgt_cal_first_line_vfilt_wgt = CLIP(tmp, 0, 0x3f);
    // REG: BAY3D_DSSCL
    tmp = (pmdDyn->subLoMd1.hw_btnrT_vIIRWgt_scale) * (1 << 4);
    pCfg->lo_wgt_vfilt_scale = CLIP(tmp, 0, 0x3f);
    {
        // calc vfilt scale
        int bayertnr_lodiff_vfilt_scale_bit;
        int bayertnr_lodiff_vfilt_scale;
        for (i = 8; i >= 2; i--) {
            if (pmdDyn->subLoMd0.diffCh.hw_btnrT_vIIRWgt_scale < (float)1.0 / (1 << i)) {
                bayertnr_lodiff_vfilt_scale_bit = (i + 1) - 2;
                bayertnr_lodiff_vfilt_scale = MIN((int)(pmdDyn->subLoMd0.diffCh.hw_btnrT_vIIRWgt_scale * (1 << (i + 1)) + 0.5), 63); //U2.4
                break;
            }
        }
        if(i == 1) {
            bayertnr_lodiff_vfilt_scale_bit = 2 - 2;
            bayertnr_lodiff_vfilt_scale = MIN((int)(pmdDyn->subLoMd0.diffCh.hw_btnrT_vIIRWgt_scale * (1 << 2) + 0.5), 63);  //U2.4
        }
        pCfg->lo_diff_vfilt_scale_bit = CLIP(bayertnr_lodiff_vfilt_scale_bit, 0, 0x7);
        pCfg->lo_diff_vfilt_scale = CLIP(bayertnr_lodiff_vfilt_scale, 0, 0x7f);
    }
    tmp = (0) * (1 << 4);
    pCfg->lo_diff_first_line_vfilt_wgt = CLIP(tmp, 0, 0x3f);
    // REG: BAY3D_ME0
    tmp = (pmdDyn->motionEst.hw_btnrT_upMvxCost_offset) * (1 << 10);
    pCfg->motion_est_up_mvx_cost_offset = CLIP(tmp, 0, 0x3fff);
    tmp = (pmdDyn->motionEst.hw_btnrT_upMvxCost_scale) * (1 << 10);
    pCfg->motion_est_up_mvx_cost_scale = CLIP(tmp, 0, 0x7ff);
    tmp = (pmdDyn->motionEst.hw_btnrT_sadFilt_wgt[0]) * (1 << 1);
    pCfg->motion_est_sad_vert_wgt0 = CLIP(tmp, 0, 0x3);
    // REG: BAY3D_ME1
    tmp = (pmdDyn->motionEst.hw_btnrT_upLeftMvxCost_offset) * (1 << 10);
    pCfg->motion_est_up_left_mvx_cost_offset = CLIP(tmp, 0, 0x3fff);
    tmp = (pmdDyn->motionEst.hw_btnrT_upLeftMvxCost_scale) * (1 << 10);
    pCfg->motion_est_up_left_mvx_cost_scale = CLIP(tmp, 0, 0x7ff);
    tmp = (pmdDyn->motionEst.hw_btnrT_sadFilt_wgt[1]) * (1 << 1);
    pCfg->motion_est_sad_vert_wgt1 = CLIP(tmp, 0, 0x3);
    // REG: BAY3D_ME2
    tmp = (pmdDyn->motionEst.hw_btnrT_upRightMvxCost_offset) * (1 << 10);
    pCfg->motion_est_up_right_mvx_cost_offset = CLIP(tmp, 0, 0x3fff);
    tmp = (pmdDyn->motionEst.hw_btnrT_upRightMvxCost_scale) * (1 << 10);
    pCfg->motion_est_up_right_mvx_cost_scale = CLIP(tmp, 0, 0x7ff);
    tmp = (pmdDyn->motionEst.hw_btnrT_sadFilt_wgt[2]) * (1 << 1);
    pCfg->motion_est_sad_vert_wgt2 = CLIP(tmp, 0, 0x3);
    // REG: BAY3D_WGTMAX
    pCfg->lo_wgt_clip_motion_max_limit = pCfg->lo_wgt_clip_max_limit;
    // REG: BAY3D_WGT1MAX
    tmp = (pmdDyn->loAsBias_hiMd1.hw_btnrT_mdWgt_maxLimit) * (1 << 10);
    pCfg->mode1_wgt_max_limit = CLIP(tmp, 0, 0x3fff);
    // REG: BAY3D_WGTM0
    tmp = (pmdDyn->loAsRatio_hiMd0.hw_btnrT_mdWgtOut_maxLimit) * (1 << 10);
    pCfg->mode0_wgt_out_max_limit = CLIP(tmp, 0, 0x7ff);
    tmp = (pmdDyn->loAsRatio_hiMd0.hw_btnrT_mdWgtOut_offset) * (1 << 10);
    pCfg->mode0_wgt_out_offset = CLIP(tmp, 0, 0x3ff);
    // REG: BAY3D_PRELOWGT
    tmp = (pdyn->preSpNr.loNr.hw_btnrT_ratAvgRgeWgt_alpha) * (1 << 7);
    pCfg->pre_spnr_lo_val_wgt_out_wgt = CLIP(tmp, 0, 0xff);
    tmp = (pdyn->preSpNr.loNr.hw_btnrT_loNrOut_alpha) * (1 << 7);
    pCfg->pre_spnr_lo_filter_out_wgt = CLIP(tmp, 0, 0xff);
    tmp = (pdyn->preSpNr.loNr.hw_btnrT_centerWgt_minLimit) * (1 << 8);
    pCfg->pre_spnr_lo_filter_wgt_min = CLIP(tmp, 0, 0xff);
    // REG: BAY3D_MIDBIG0
    tmp = (pmdDyn->subExtLoMd.hw_btnrT_wgt_offset) * (1 << 8);
    pCfg->md_large_lo_md_wgt_offset = CLIP(tmp, 0, 0xff);
    tmp = (pmdDyn->subExtLoMd.sw_btnrT_wgt2FusionLmt_scale) * (1 << 8);
    pCfg->md_large_lo_md_wgt_scale = CLIP(tmp, 0, 0xfff);
    // REG: BAY3D_MIDBIG1
    tmp = (pmdDyn->subExtLoMd.sw_btnrT_wgt2FusionLmt_negOff) * (1 << 8);
    pCfg->md_large_lo_wgt_cut_offset = CLIP(tmp, 0, 0xfff);
    tmp = (pmdDyn->subExtLoMd.hw_btnrT_wgt2FussionLmt_offset) * (1 << 12);
    pCfg->md_large_lo_wgt_add_offset = CLIP(tmp, 0, 0xfff);
    // REG: BAY3D_MIDBIG2
    tmp = (pmdDyn->subExtLoMd.hw_btnrT_wgt_scale) * (1 << 8);
    pCfg->md_large_lo_wgt_scale = CLIP(tmp, 0, 0xfff);

    // tnr sigma curve must calculate before spnr sigma
    if(cvtinfo->isFirstFrame || (pTransParams->transf_mode != pCfg->transf_mode) ||
            (pTransParams->transf_mode_scale != pCfg->transf_mode_scale)) {
        pTransParams->transf_mode = pCfg->transf_mode;
        pTransParams->transf_mode_scale = pCfg->transf_mode_scale;
        pTransParams->transf_mode_offset = pCfg->transf_mode_offset;
        pTransParams->transf_data_max_limit = pCfg->transf_data_max_limit;
        pTransParams->itransf_mode_offset = pCfg->itransf_mode_offset;
        bayertnr_logtrans_init(pCfg->transf_mode, pCfg->transf_mode_scale, pTransParams);
    }
    // REG: BAY3D_PIXMAX
    if(pCfg->transf_bypass_en) {
        pCfg->pix_max_limit  = ((1 << 12) - 1);
    } else {
        if(pCfg->transf_mode_scale) {
            pCfg->pix_max_limit  = bayertnr_logtrans(((1 << 15) - 1), pTransParams);
        } else {
            pCfg->pix_max_limit  = bayertnr_logtrans(((1 << 20) - 1), pTransParams);;
        }
    }

    pTransParams->isTransfBypass = pCfg->transf_bypass_en;
    bayertnr_luma2sigmax_config_v41(pTransParams);

    sigbins = 20;
    for(i = 0; i < sigbins; i++) {
        pCfg->tnr_luma2sigma_x[i] = CLIP(pTransParams->tnr_luma_sigma_x[i], 0, 0xfff);
    }

    if(psta->sigmaEnv.sw_btnrCfg_sigma_mode == btnr_manualSigma_mode || cvtinfo->isFirstFrame || bayertnr_default_noise_curve_use) {
        pTransParams->bayertnr_auto_sig_count_en = 0;
        for(i = 0; i < sigbins; i++) {
            pTransParams->tnr_luma_sigma_y[i] = pdyn->sigmaEnv.hw_btnrC_mdSigma_curve.val[i];
        }
    } else {
        pTransParams->bayertnr_auto_sig_count_en = 1;
        pTransParams->bayertnr_auto_sig_count_filt_wgt = pdyn->sigmaEnv.sw_btnrT_autoSgmIIR_alpha * (1 << 10);
        pTransParams->bayertnr_auto_sig_count_max = cvtinfo->rawWidth * cvtinfo->rawHeight / 3;
        bayertnr_autosigma_config(btnr_stats, pTransParams);
    }
    int max_sig = ((1 << 12) - 1);
    for(i = 0; i < sigbins; i++) {
        pCfg->tnr_luma2sigma_y[i] = CLIP((int)(pTransParams->tnr_luma_sigma_y[i]), 0, max_sig);
    }

    // spnr sigma curve, cur bil and iir bil strength
    int spnrsigbins = 16;
    bool auto_sig_curve_spnruse = psta->sigmaEnv.sw_btnrCfg_sigma_mode == btnr_autoSigma_mode;
    int pix_max = pCfg->transf_bypass_en ? ((1 << 12) - 1) : bayertnr_logtrans((1 << 12) - 1, pTransParams);
    if(cvtinfo->frameNum == 2) {
        // hdr long bins
        int lgbins = 8;
        for(i = 0; i < lgbins; i++) {
            pCfg->pre_spnr_luma2sigma_x[i] = pix_max * (i + 1) / lgbins;
        }
        pCfg->pre_spnr_luma2sigma_x[lgbins-1] = pix_max * (i + 1) / lgbins;

        // hdr short bins, max gain 256
        int shbins = spnrsigbins - lgbins;
        i = 8;
        tmp0 = (1 << (12 + i)) - 1;
        tmp0 = pCfg->transf_bypass_en ? tmp0 : bayertnr_logtrans(tmp0, pTransParams);
        tmp0 = tmp0 - pix_max;
        for(i = lgbins; i < spnrsigbins; i++) {
            pCfg->pre_spnr_luma2sigma_x[i] = 256 * (i - lgbins + 1) + pCfg->pre_spnr_luma2sigma_x[lgbins - 1];
        }
        pCfg->pre_spnr_luma2sigma_x[spnrsigbins - 1] = pix_max;

    } else if (!pCfg->transf_bypass_en) {
        for(i = 0; i < spnrsigbins; i++) {
            pCfg->pre_spnr_luma2sigma_x[i] = 64 * (i + 1) ;
        }
        pCfg->pre_spnr_luma2sigma_x[spnrsigbins - 1] = pix_max;
    } else {
        for(i = 0; i < spnrsigbins; i++) {
            pCfg->pre_spnr_luma2sigma_x[i] = 256 * (i + 1) ;
        }
        pCfg->pre_spnr_luma2sigma_x[spnrsigbins - 1] = pix_max;
    }

    for(i = 0; i < spnrsigbins; i++) {
        if(!auto_sig_curve_spnruse || cvtinfo->isFirstFrame) {
            pCfg->pre_spnr_luma2sigma_y[i] = CLIP((int)(pdyn->sigmaEnv.hw_btnrC_preSpNrSgm_curve.val[i]), 0, max_sig);
        } else {
            tmp0 = bayertnr_tnr_noise_curve(pCfg->pre_spnr_luma2sigma_x[i], 0, pTransParams);
            pCfg->pre_spnr_luma2sigma_y[i] = CLIP((int)(tmp0), 0, max_sig);
        }
    }

    // REG: BAY3D_SIGORG
    pTransParams->bayertnr_lo_wgt_clip_min_limit = pCfg->lo_wgt_clip_min_limit ;
    pTransParams->bayertnr_lo_wgt_clip_max_limit = pCfg->lo_wgt_clip_max_limit;
    pCfg->tnr_out_sigma_sq  = bayertnr_update_sq(pTransParams);
    return;
}

