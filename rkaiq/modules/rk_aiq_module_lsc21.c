#include "rk_aiq_isp39_modules.h"

bool rk_aiq_lsc21_params_check(void* attr, common_cvt_info_t *cvtinfo)
{
    //TODO: check if sum of x_size_tbl == raw width
    //TODO: check if sum of y_size_tbl == raw height

    return true;
}

void rk_aiq_lsc21_params_cvt(void* attr, isp_params_t* isp_params, common_cvt_info_t *cvtinfo)
{
    int i;
    struct isp3x_lsc_cfg * pFix = &isp_params->isp_cfg->others.lsc_cfg;
    lsc_param_t *lsc_param = (lsc_param_t *) attr;
    lsc_param_dyn_t * pdyn = &lsc_param->dyn;
    lsc_param_static_t * psta = &lsc_param->sta;

    pFix->sector_16x16 = 1;

    for (i=0; i<LSC_MESHGRID_SIZE; i++) {
        pFix->x_size_tbl[i] = psta->meshGrid.width[i];
        pFix->y_size_tbl[i] = psta->meshGrid.height[i];

        pFix->x_grad_tbl[i] = (uint16_t)((double)(1UL << 15) / pFix->x_size_tbl[i] + 0.5);
        pFix->y_grad_tbl[i] = (uint16_t)((double)(1UL << 15) / pFix->y_size_tbl[i] + 0.5);
    }

    for (i=0; i<LSC_LSCTABLE_SIZE; i++) {
        pFix->r_data_tbl[i] = pdyn->meshGain.hw_lscC_gainR_val[i];
        pFix->gr_data_tbl[i] = pdyn->meshGain.hw_lscC_gainGr_val[i];
        pFix->gb_data_tbl[i] = pdyn->meshGain.hw_lscC_gainGb_val[i];
        pFix->b_data_tbl[i] = pdyn->meshGain.hw_lscC_gainB_val[i];
    }

    return;
}
