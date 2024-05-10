#include "rk_aiq_isp39_modules.h"
#define RKCAC_EDGE_DETECT_FIX_BITS 4
#define RKCAC_STRENGTH_FIX_BITS    7
static void cac21_multi_cvt(struct isp32_cac_cfg* phwcfg, cac_params_dyn_t* pdyn,
    cac_params_static_t* psta, bool is_multi_isp);

void rk_aiq_cac21_params_cvt(void* attr, isp_params_t* isp_params,
    isp_params_t* isp_cfg_right, bool is_multi_isp)
{
    struct isp32_cac_cfg* phwcfg = &isp_params->isp_cfg->others.cac_cfg;
    cac_param_t *cac_param = (cac_param_t*)attr;
    cac_params_dyn_t* pdyn = &cac_param->dyn;
    cac_params_static_t* psta = &cac_param->sta;
    cac21_multi_cvt(phwcfg, pdyn, psta, false);
    if (is_multi_isp) {
        phwcfg = &(isp_params->isp_cfg + 1)->others.cac_cfg;
        cac21_multi_cvt(phwcfg, pdyn, psta, is_multi_isp);
    }
}

static void cac21_multi_cvt(struct isp32_cac_cfg* phwcfg, cac_params_dyn_t* pdyn,
    cac_params_static_t* psta, bool is_multi_isp) {

    int index = (int)is_multi_isp;
    phwcfg->center_en = psta->strgCenter[index].hw_cacT_strgCenter_en;
    phwcfg->center_width  = psta->strgCenter[index].hw_cacT_strgCenter_x;
    phwcfg->center_height = psta->strgCenter[index].hw_cacT_strgCenter_y;
    phwcfg->psf_sft_bit   = psta->psfParam.hw_cacT_psfShift_bits;
    phwcfg->cfg_num       = psta->psfParam.hw_cacCfg_psfBlock_num;
    phwcfg->buf_fd        = psta->lutBuf[index].sw_cacCfg_lutBuf_fd;
    phwcfg->hsize         = psta->lutBuf[index].sw_cacCfg_lutBufSize_height;
    phwcfg->vsize         = psta->lutBuf[index].sw_cacCfg_lutBufSize_width;
    for (int i = 0; i < RKCAC_STRENGTH_TABLE_LEN; i++) {
        phwcfg->strength[i] =
            ROUND_F(pdyn->strgInterp.hw_cacT_corrStrength_table[i] * (1 << RKCAC_STRENGTH_FIX_BITS));
        phwcfg->strength[i] =
                phwcfg->strength[i] > 2047 ? 2047 : phwcfg->strength[i];
    }
    phwcfg->clip_g_mode = (int)pdyn->chromaAberrCorr.hw_cacT_clipG_mode;
    phwcfg->neg_clip0_en = pdyn->chromaAberrCorr.hw_cacT_negClip0_en;
    phwcfg->edge_detect_en = pdyn->strgInterp.hw_cacT_edgeDetect_en;
    phwcfg->flat_thed_b =
        ROUND_F(pdyn->strgInterp.hw_cacT_flatDctB_thred * (1 << RKCAC_EDGE_DETECT_FIX_BITS));
    phwcfg->flat_thed_r =
        ROUND_F(pdyn->strgInterp.hw_cacT_flatDctR_thred * (1 << RKCAC_EDGE_DETECT_FIX_BITS));
    phwcfg->offset_b = CLIP(
        ROUND_F(pdyn->strgInterp.hw_cacT_flatDctB_offset * (1 << RKCAC_EDGE_DETECT_FIX_BITS)), 0, 1 << 16);
    phwcfg->offset_r = CLIP(
        ROUND_F(pdyn->strgInterp.hw_cacT_flatDctR_offset * (1 << RKCAC_EDGE_DETECT_FIX_BITS)), 0, 1 << 16);
    int expoDctB_en = (int) !pdyn->chromaAberrCorr.sw_cacT_expoDctB_en;
    int expoDctR_en = (int) !pdyn->chromaAberrCorr.sw_cacT_expoDctR_en;
    phwcfg->expo_thed_b =
        (expoDctB_en << 20) | CLIP((int)pdyn->chromaAberrCorr.hw_cacT_overExpoB_thred, 0, (1 << 20) - 1);
    phwcfg->expo_thed_r =
        (expoDctR_en << 20) | CLIP((int)pdyn->chromaAberrCorr.hw_cacT_overExpoR_thred, 0, (1 << 20) - 1);
    phwcfg->expo_adj_b = CLIP((int)pdyn->chromaAberrCorr.hw_cacT_overExpoB_adj, 0, (1 << 20) - 1);
    phwcfg->expo_adj_r = CLIP((int)pdyn->chromaAberrCorr.hw_cacT_overExpoR_adj, 0, (1 << 20) - 1);

    LOGD_ACAC("Dump CAC config: ");
    LOGD_ACAC("by en: %d",             phwcfg->bypass_en);
    LOGD_ACAC("center en: %d",         phwcfg->center_en);
    LOGD_ACAC("center x: %d",          phwcfg->center_width);
    LOGD_ACAC("center y: %d",          phwcfg->center_height);
    LOGD_ACAC("psf shift bits: %d",    phwcfg->psf_sft_bit);
    LOGD_ACAC("psf cfg num: %d",       phwcfg->cfg_num);
    LOGD_ACAC("psf buf fd: %d",        phwcfg->buf_fd);
    LOGD_ACAC("psf hwsize: %d",        phwcfg->hsize);
    LOGD_ACAC("psf vsize: %d",          phwcfg->vsize);
    for (int i = 0; i < RKCAC_STRENGTH_TABLE_LEN; i++) {
        LOGD_ACAC("strength %d: %d", i, phwcfg->strength[i]);
    }
    LOGD_ACAC("clip_g_mode : %d",     phwcfg->clip_g_mode);
    LOGD_ACAC("edge_detect_en : %d",  phwcfg->edge_detect_en);
    LOGD_ACAC("neg_clip0_en : %d",    phwcfg->neg_clip0_en);
    LOGD_ACAC("flat_thed_b : %d",     phwcfg->flat_thed_b);
    LOGD_ACAC("flat_thed_r : %d",     phwcfg->flat_thed_r);
    LOGD_ACAC("offset_b : %d",        phwcfg->offset_b);
    LOGD_ACAC("offset_r : %d",        phwcfg->offset_r);
    LOGD_ACAC("expo_thed_b : %d",     phwcfg->expo_thed_b);
    LOGD_ACAC("expo_thed_r : %d",     phwcfg->expo_thed_r);
    LOGD_ACAC("expo_adj_b : %d",      phwcfg->expo_adj_b);
    LOGD_ACAC("expo_adj_r : %d",      phwcfg->expo_adj_r);
}
