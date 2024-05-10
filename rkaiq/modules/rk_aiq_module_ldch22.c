#include "rk_aiq_isp39_modules.h"
#define RKCAC_EDGE_DETECT_FIX_BITS 4
#define RKCAC_STRENGTH_FIX_BITS    7

void rk_aiq_ldch22_params_cvt(void* attr, isp_params_t* isp_params,
    isp_params_t* isp_cfg_right, bool is_multi_isp)
{
    struct isp39_ldch_cfg* phwcfg = &isp_params->isp_cfg->others.ldch_cfg;
    ldch_param_t *ldch_param = (ldch_param_t*)attr;
    ldch_params_static_t* psta = &ldch_param->sta;

    phwcfg->hsize = psta->lutMapCfg.sw_ldchT_lutMap_height;
    phwcfg->vsize = psta->lutMapCfg.sw_ldchT_lutMap_width;
    phwcfg->buf_fd = psta->lutMapCfg.sw_ldchT_lutMapBuf_fd[0];

    phwcfg->frm_end_dis = psta->userCfg.hw_ldchCfg_frmEndDis_mode;
    phwcfg->sample_avr_en = psta->userCfg.hw_ldchCfg_sampleAvr_en;
    phwcfg->bic_mode_en = psta->userCfg.hw_ldchCfg_bicMode_en;
    phwcfg->force_map_en = psta->userCfg.hw_ldchCfg_forceMap_en;
    phwcfg->map13p3_en = psta->userCfg.hw_ldchCfg_mapFix3Bit_en;
    memcpy(phwcfg->bicubic, psta->userCfg.hw_ldchCfg_bicWeight_table, sizeof(psta->userCfg.hw_ldchCfg_bicWeight_table));

    if (is_multi_isp) {
        struct isp39_ldch_cfg* cfg_right = &(isp_params->isp_cfg + 1)->others.ldch_cfg;
        memcpy(cfg_right, phwcfg, sizeof(*cfg_right));
        cfg_right->buf_fd = psta->lutMapCfg.sw_ldchT_lutMapBuf_fd[1];
    }

    LOGD_ALDCH("hsize: %d",             phwcfg->hsize);
    LOGD_ALDCH("vsize: %d",         phwcfg->vsize);
    LOGD_ALDCH("buf_fd: %d",          phwcfg->buf_fd);
    LOGD_ALDCH("frm_end_dis: %d", phwcfg->frm_end_dis);
    LOGD_ALDCH("sample_avr_en: %d", phwcfg->sample_avr_en);
    LOGD_ALDCH("bic_mode_en: %d", phwcfg->bic_mode_en);
    LOGD_ALDCH("force_map_en: %d", phwcfg->force_map_en);
    LOGD_ALDCH("map13p3_en: %d", phwcfg->map13p3_en);
    LOGD_ALDCH("bicubic[0]: %d", phwcfg->bicubic[0]);
    LOGD_ALDCH("bicubic[1]: %d", phwcfg->bicubic[1]);
    LOGD_ALDCH("bicubic[2]: %d", phwcfg->bicubic[2]);
    LOGD_ALDCH("bicubic[3]: %d", phwcfg->bicubic[3]);
}

