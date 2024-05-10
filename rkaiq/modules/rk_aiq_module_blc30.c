/*
 * Copyright (c) 2021-2022 Rockchip Eletronics Co., Ltd.
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
 */

#include "rk_aiq_isp39_modules.h"

void rk_aiq_blc30_params_cvt(void* attr, isp_params_t* isp_params, common_cvt_info_t *cvtinfo)
{
    struct isp32_bls_cfg *phwcfg = &isp_params->isp_cfg->others.bls_cfg;
    blc_params_dyn_t *pdyn = &((blc_param_t*)attr)->dyn;
    int isp_ob_max = 0xfff;
    float pre_dgain = 1.0;
    float isp_dgain = 1.0;

    pre_dgain = cvtinfo->preDGain;
    isp_dgain = cvtinfo->frameDGain[0];

    phwcfg->enable_auto = 0;
    phwcfg->en_windows  = 0;

    phwcfg->bls_window1.h_offs = 0;
    phwcfg->bls_window1.v_offs = 0;
    phwcfg->bls_window1.h_size = 0;
    phwcfg->bls_window1.v_size = 0;

    phwcfg->bls_window2.h_offs = 0;
    phwcfg->bls_window2.v_offs = 0;
    phwcfg->bls_window2.h_size = 0;
    phwcfg->bls_window2.v_size = 0;

    phwcfg->bls_samples = 0;

    // blc0
    phwcfg->fixed_val.r  = (short)pdyn->obcPreTnr.hw_blcC_obR_val;
    phwcfg->fixed_val.gr = (short)pdyn->obcPreTnr.hw_blcC_obGr_val;
    phwcfg->fixed_val.gb = (short)pdyn->obcPreTnr.hw_blcC_obGb_val;
    phwcfg->fixed_val.b  = (short)pdyn->obcPreTnr.hw_blcC_obB_val;

    // pre_dgain
    phwcfg->isp_ob_predgain = CLIP((int)(pre_dgain * (1 << 8)), 0, 65535);

    // blc1 aiq 6.0 does not support manualOBC, set to 0.
    phwcfg->bls1_val.r = 0;
    phwcfg->bls1_val.gr = 0;
    phwcfg->bls1_val.gb = 0;
    phwcfg->bls1_val.b = 0;

    if (pdyn->obcPostTnr.sw_blcT_obcPostTnr_en) {
        phwcfg->bls1_en = 1;
        // phwcfg->bls1_val.r = CLIP(pdyn->obcPostTnr.hw_blcT_manualOBR_val * isp_dgain * pre_dgain, 0, 32767);
        // phwcfg->bls1_val.gr = CLIP(pdyn->obcPostTnr.hw_blcT_manualOBGr_val * isp_dgain * pre_dgain, 0, 32767);
        // phwcfg->bls1_val.gb = CLIP(pdyn->obcPostTnr.hw_blcT_manualOBGb_val * isp_dgain * pre_dgain, 0, 32767);
        // phwcfg->bls1_val.b = CLIP(pdyn->obcPostTnr.hw_blcT_manualOBB_val * isp_dgain * pre_dgain, 0, 32767);
        phwcfg->isp_ob_offset = pdyn->obcPostTnr.sw_blcT_autoOB_offset;
    } else {
        phwcfg->bls1_en = 0;
        phwcfg->isp_ob_offset = 0;
    }

    if (cvtinfo->frameNum > 1) {
        if (phwcfg->bls1_en == 1 || phwcfg->isp_ob_offset > 0) {
            LOGE_ABLC("When using HDR mode, obcPostTnr and ob_offset should be off");
        }
        // hdr won't use blc1 and blc_ob
        phwcfg->bls1_en = 0;
        phwcfg->isp_ob_offset = 0;
        phwcfg->isp_ob_predgain = CLIP((int)(1 * (1 << 8)), 0, 65535);
    }

    if (phwcfg->isp_ob_predgain > 0x100) {
        phwcfg->bls1_en = 1;
        phwcfg->isp_ob_max = CLIP((int)(4096 * pre_dgain) - phwcfg->isp_ob_offset, 0, 1048575);
    } else {
        if (cvtinfo->frameNum == 1)
            phwcfg->isp_ob_max = 0xfff;
        else
            phwcfg->isp_ob_max = 0xfffff;
    }

    return;
}
