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

    switch (psta->sw_lscT_meshGrid_mode) {
        case lsc_usrConfig_mode: {
            uint16_t x0, x1, y0, y1;
            x0 = (uint16_t)(psta->meshGrid.posX_f[0] * (float)cvtinfo->rawWidth);
            y0 = (uint16_t)(psta->meshGrid.posY_f[0] * (float)cvtinfo->rawHeight);
            for (i=0; i<LSC_MESHGRID_SIZE; i++) {
                x1 = (uint16_t)(psta->meshGrid.posX_f[i+1] * (float)cvtinfo->rawWidth);
                y1 = (uint16_t)(psta->meshGrid.posY_f[i+1] * (float)cvtinfo->rawHeight);
                pFix->x_size_tbl[i] = x1 - x0;
                pFix->y_size_tbl[i] = y1 - y0;
                x0 = x1;
                y0 = y1;

                pFix->x_grad_tbl[i] = (uint16_t)((double)(1UL << 15) / pFix->x_size_tbl[i] + 0.5);
                pFix->y_grad_tbl[i] = (uint16_t)((double)(1UL << 15) / pFix->y_size_tbl[i] + 0.5);
            }
        } break;
        case lsc_vendorDefault_mode:
        case lsc_equalSector_mode: {
            float sampPos[17] = {
                0, 0.0625, 0.125, 0.1875, 0.25, 0.3125, 0.375, 0.4375, 0.5,
		        0.5625, 0.625, 0.6875, 0.75, 0.8125, 0.875, 0.9375, 1};
            for (i=0; i<LSC_MESHGRID_SIZE; i++) {
                pFix->x_size_tbl[i] = (uint16_t)(sampPos[i+1] * (float)cvtinfo->rawWidth) - (uint16_t)(sampPos[i] * (float)cvtinfo->rawWidth);
                pFix->y_size_tbl[i] = (uint16_t)(sampPos[i+1] * (float)cvtinfo->rawHeight) - (uint16_t)(sampPos[i] * (float)cvtinfo->rawHeight);
                pFix->x_grad_tbl[i] = (uint16_t)((double)(1UL << 15) / pFix->x_size_tbl[i] + 0.5);
                pFix->y_grad_tbl[i] = (uint16_t)((double)(1UL << 15) / pFix->y_size_tbl[i] + 0.5);
            }
        } break;
        default:
            break;
    }

    for (i=0; i<LSC_LSCTABLE_SIZE; i++) {
        pFix->r_data_tbl[i] = pdyn->meshGain.hw_lscC_gainR_val[i];
        pFix->gr_data_tbl[i] = pdyn->meshGain.hw_lscC_gainGr_val[i];
        pFix->gb_data_tbl[i] = pdyn->meshGain.hw_lscC_gainGb_val[i];
        pFix->b_data_tbl[i] = pdyn->meshGain.hw_lscC_gainB_val[i];
    }

    return;
}
