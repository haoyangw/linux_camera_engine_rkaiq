/*
 * Copyright (c) 2024 Rockchip Eletronics Co., Ltd.
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

#include "algo_types_priv.h"
#include "rk_aiq_isp39_modules.h"

#define MAX_AE_DRC_GAIN     (256.0f)
#define GAINMIN             (1.0f)
#define ISP_HDR_BIT_NUM_MAX (20)
#define ISP_HDR_BIT_NUM_MIN (12)
#define ISP_RAW_BIT         (12)
#define FUNCTION_DISABLE    (0)
#define FUNCTION_ENABLE     (1)

void rk_aiq_drc40_params_dump(void* attr, isp_params_t* isp_params) {
    LOG1_ATMO(
        "%s: bypass_en:%d gainx32_en:%d raw_dly_dis:%d sw_drc_position:%d sw_drc_compres_scl:%d "
        "sw_drc_offset_pow2:%d\n",
        __FUNCTION__, isp_params->isp_cfg->others.drc_cfg.bypass_en,
        isp_params->isp_cfg->others.drc_cfg.gainx32_en,
        isp_params->isp_cfg->others.drc_cfg.raw_dly_dis,
        isp_params->isp_cfg->others.drc_cfg.position,
        isp_params->isp_cfg->others.drc_cfg.compres_scl,
        isp_params->isp_cfg->others.drc_cfg.offset_pow2);
    LOG1_ATMO("%s: sw_drc_lpdetail_ratio:%d sw_drc_hpdetail_ratio:%d sw_drc_delta_scalein:%d\n",
              __FUNCTION__, isp_params->isp_cfg->others.drc_cfg.lpdetail_ratio,
              isp_params->isp_cfg->others.drc_cfg.hpdetail_ratio,
              isp_params->isp_cfg->others.drc_cfg.delta_scalein);
    LOG1_ATMO(
        "%s: sw_drc_bilat_wt_off:%d thumb_thd_neg:%d thumb_thd_enable:%d sw_drc_weicur_pix:%d\n",
        __FUNCTION__, isp_params->isp_cfg->others.drc_cfg.bilat_wt_off,
        isp_params->isp_cfg->others.drc_cfg.thumb_thd_neg,
        isp_params->isp_cfg->others.drc_cfg.thumb_thd_enable,
        isp_params->isp_cfg->others.drc_cfg.weicur_pix);
    LOG1_ATMO(
        "%s: cmps_byp_en:%d cmps_offset_bits_int:%d cmps_fixbit_mode:%d thumb_clip:%d "
        "thumb_scale:%d "
        "\n",
        __FUNCTION__, isp_params->isp_cfg->others.drc_cfg.cmps_byp_en,
        isp_params->isp_cfg->others.drc_cfg.cmps_offset_bits_int,
        isp_params->isp_cfg->others.drc_cfg.cmps_fixbit_mode,
        isp_params->isp_cfg->others.drc_cfg.thumb_clip,
        isp_params->isp_cfg->others.drc_cfg.thumb_scale);
    LOG1_ATMO(
        "%s: sw_drc_range_sgm_inv0:%d sw_drc_range_sgm_inv1:%d weig_bilat:%d weight_8x8thumb:%d\n",
        __FUNCTION__, isp_params->isp_cfg->others.drc_cfg.range_sgm_inv0,
        isp_params->isp_cfg->others.drc_cfg.range_sgm_inv1,
        isp_params->isp_cfg->others.drc_cfg.weig_bilat,
        isp_params->isp_cfg->others.drc_cfg.weight_8x8thumb);
    LOG1_ATMO(
        "%s: sw_drc_bilat_soft_thd:%d "
        "sw_drc_enable_soft_thd:%d sw_drc_min_ogain:%d\n",
        __FUNCTION__, isp_params->isp_cfg->others.drc_cfg.bilat_soft_thd,
        isp_params->isp_cfg->others.drc_cfg.enable_soft_thd,
        isp_params->isp_cfg->others.drc_cfg.min_ogain);
    LOG1_ATMO("%s: sw_drc_gain_y: %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d\n",
              __FUNCTION__, isp_params->isp_cfg->others.drc_cfg.gain_y[0],
              isp_params->isp_cfg->others.drc_cfg.gain_y[1],
              isp_params->isp_cfg->others.drc_cfg.gain_y[2],
              isp_params->isp_cfg->others.drc_cfg.gain_y[3],
              isp_params->isp_cfg->others.drc_cfg.gain_y[4],
              isp_params->isp_cfg->others.drc_cfg.gain_y[5],
              isp_params->isp_cfg->others.drc_cfg.gain_y[6],
              isp_params->isp_cfg->others.drc_cfg.gain_y[7],
              isp_params->isp_cfg->others.drc_cfg.gain_y[8],
              isp_params->isp_cfg->others.drc_cfg.gain_y[9],
              isp_params->isp_cfg->others.drc_cfg.gain_y[10],
              isp_params->isp_cfg->others.drc_cfg.gain_y[11],
              isp_params->isp_cfg->others.drc_cfg.gain_y[12],
              isp_params->isp_cfg->others.drc_cfg.gain_y[13],
              isp_params->isp_cfg->others.drc_cfg.gain_y[14],
              isp_params->isp_cfg->others.drc_cfg.gain_y[15],
              isp_params->isp_cfg->others.drc_cfg.gain_y[16]);
    LOG1_ATMO("%s: sw_drc_scale_y: %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d\n",
              __FUNCTION__, isp_params->isp_cfg->others.drc_cfg.scale_y[0],
              isp_params->isp_cfg->others.drc_cfg.scale_y[1],
              isp_params->isp_cfg->others.drc_cfg.scale_y[2],
              isp_params->isp_cfg->others.drc_cfg.scale_y[3],
              isp_params->isp_cfg->others.drc_cfg.scale_y[4],
              isp_params->isp_cfg->others.drc_cfg.scale_y[5],
              isp_params->isp_cfg->others.drc_cfg.scale_y[6],
              isp_params->isp_cfg->others.drc_cfg.scale_y[7],
              isp_params->isp_cfg->others.drc_cfg.scale_y[8],
              isp_params->isp_cfg->others.drc_cfg.scale_y[9],
              isp_params->isp_cfg->others.drc_cfg.scale_y[10],
              isp_params->isp_cfg->others.drc_cfg.scale_y[11],
              isp_params->isp_cfg->others.drc_cfg.scale_y[12],
              isp_params->isp_cfg->others.drc_cfg.scale_y[13],
              isp_params->isp_cfg->others.drc_cfg.scale_y[14],
              isp_params->isp_cfg->others.drc_cfg.scale_y[15],
              isp_params->isp_cfg->others.drc_cfg.scale_y[16]);
    LOG1_ATMO(
        "%s: sw_drc_compres_y: %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d "
        "%d\n",
        __FUNCTION__, isp_params->isp_cfg->others.drc_cfg.compres_y[0],
        isp_params->isp_cfg->others.drc_cfg.compres_y[1],
        isp_params->isp_cfg->others.drc_cfg.compres_y[2],
        isp_params->isp_cfg->others.drc_cfg.compres_y[3],
        isp_params->isp_cfg->others.drc_cfg.compres_y[4],
        isp_params->isp_cfg->others.drc_cfg.compres_y[5],
        isp_params->isp_cfg->others.drc_cfg.compres_y[6],
        isp_params->isp_cfg->others.drc_cfg.compres_y[7],
        isp_params->isp_cfg->others.drc_cfg.compres_y[8],
        isp_params->isp_cfg->others.drc_cfg.compres_y[9],
        isp_params->isp_cfg->others.drc_cfg.compres_y[10],
        isp_params->isp_cfg->others.drc_cfg.compres_y[11],
        isp_params->isp_cfg->others.drc_cfg.compres_y[12],
        isp_params->isp_cfg->others.drc_cfg.compres_y[13],
        isp_params->isp_cfg->others.drc_cfg.compres_y[14],
        isp_params->isp_cfg->others.drc_cfg.compres_y[15],
        isp_params->isp_cfg->others.drc_cfg.compres_y[16]);
    LOG1_ATMO(
        "%s: sfthd_y: %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d "
        "%d\n",
        __FUNCTION__, isp_params->isp_cfg->others.drc_cfg.sfthd_y[0],
        isp_params->isp_cfg->others.drc_cfg.sfthd_y[1],
        isp_params->isp_cfg->others.drc_cfg.sfthd_y[2],
        isp_params->isp_cfg->others.drc_cfg.sfthd_y[3],
        isp_params->isp_cfg->others.drc_cfg.sfthd_y[4],
        isp_params->isp_cfg->others.drc_cfg.sfthd_y[5],
        isp_params->isp_cfg->others.drc_cfg.sfthd_y[6],
        isp_params->isp_cfg->others.drc_cfg.sfthd_y[7],
        isp_params->isp_cfg->others.drc_cfg.sfthd_y[8],
        isp_params->isp_cfg->others.drc_cfg.sfthd_y[9],
        isp_params->isp_cfg->others.drc_cfg.sfthd_y[10],
        isp_params->isp_cfg->others.drc_cfg.sfthd_y[11],
        isp_params->isp_cfg->others.drc_cfg.sfthd_y[12],
        isp_params->isp_cfg->others.drc_cfg.sfthd_y[13],
        isp_params->isp_cfg->others.drc_cfg.sfthd_y[14],
        isp_params->isp_cfg->others.drc_cfg.sfthd_y[15],
        isp_params->isp_cfg->others.drc_cfg.sfthd_y[16]);
}

void rk_aiq_drc40_params_cvt(void* attr, isp_params_t* isp_params, common_cvt_info_t* cvtinfo) {
#if ISP_HW_V39
    struct isp39_drc_cfg* phwcfg = &isp_params->isp_cfg->others.drc_cfg;
#elif ISP_HW_V33
    struct isp33_drc_cfg* phwcfg = &isp_params->isp_cfg->others.drc_cfg;
#endif
    drc_param_t* drc_param         = &((rk_aiq_isp_drc_v39_t*)attr)->drc_param;
    drc_params_dyn_t* pdyn         = &drc_param->dyn;
    trans_api_attrib_t* trans_attr = &((rk_aiq_isp_drc_v39_t*)attr)->trans_attr;
    trans_params_static_t* psta    = &trans_attr->stMan.sta;
    float L2S_Ratio                = ((rk_aiq_isp_drc_v39_t*)attr)->L2S_Ratio;
    cvtinfo->L2S_Ratio             = L2S_Ratio;
    unsigned char compr_bit        = ((rk_aiq_isp_drc_v39_t*)attr)->compr_bit;

    bool LongFrmMode = false;
    if (cvtinfo->frameNum == 1) {
        LongFrmMode = cvtinfo->ae_exp->LinearExp.exp_real_params.longfrm_mode;
    } else {
        LongFrmMode = cvtinfo->ae_exp->HdrExp[cvtinfo->frameNum - 1].exp_real_params.longfrm_mode;
    }

    // clip drc gain
    float drc_gain = 1.0f;
    if (pdyn->preProc.sw_drcT_toneCurve_mode == drc_cfgCurveCtrlCoeff_mode)
        drc_gain = pdyn->preProc.toneCurveCtrl.sw_drcT_toneGain_maxLimit;
    else if (pdyn->preProc.sw_drcT_toneCurve_mode == drc_cfgCurveDirect_mode)
        drc_gain = pdyn->preProc.hw_drcT_luma2ToneGain_val[16];

    if (compr_bit) {
        if (pow(2.0f, (float)(compr_bit - ISP_HDR_BIT_NUM_MIN)) * drc_gain > MAX_AE_DRC_GAIN) {
            if (pow(2.0f, (float)(compr_bit - ISP_HDR_BIT_NUM_MIN)) > MAX_AE_DRC_GAIN)
                LOGE_ATMO("%s:  SensorMgeRatio > 256x!!!\n", __FUNCTION__);
            else
                drc_gain = MAX(
                    MAX_AE_DRC_GAIN / pow(2.0f, (float)(compr_bit - ISP_HDR_BIT_NUM_MIN)), GAINMIN);
            LOGI_ATMO(
                "%s:  SensorMgeRatio*sw_drcT_toneGain_maxLimit > 256x, sw_drcT_toneGain_maxLimit "
                "clip to %f!!!\n",
                __FUNCTION__, drc_gain);
        }
    } else if (cvtinfo->frameNum > 1) {
        if (L2S_Ratio * drc_gain > MAX_AE_DRC_GAIN) {
            LOGE_ATMO("%s:  AERatio*sw_drcT_toneGain_maxLimit > 256!!!\n", __FUNCTION__);
            drc_gain = MAX(MAX_AE_DRC_GAIN / L2S_Ratio, GAINMIN);
        }
    } else {
        drc_gain = CLIP(drc_gain, GAINMIN, 8.0f);
    }

    if (pdyn->preProc.sw_drcT_toneCurve_mode == drc_cfgCurveCtrlCoeff_mode)
        pdyn->preProc.toneCurveCtrl.sw_drcT_toneGain_maxLimit = drc_gain;
    else if (pdyn->preProc.sw_drcT_toneCurve_mode == drc_cfgCurveDirect_mode) {
        for (int i = 0; i < 17; i++) {
            pdyn->preProc.hw_drcT_luma2ToneGain_val[i] =
                pdyn->preProc.hw_drcT_luma2ToneGain_val[i] > drc_gain
                    ? drc_gain
                    : pdyn->preProc.hw_drcT_luma2ToneGain_val[i];
        }
    }

    uint16_t tmp;
    phwcfg->cmps_byp_en          = (trans_attr->en == 0 || trans_attr->bypass > 0);
    phwcfg->offset_pow2          = CLIP(psta->hw_transCfg_transOfDrc_offset, 0, (1 << 4) - 1);
    tmp                          = psta->hw_transCfg_lscOutTrans_offset;
    phwcfg->cmps_offset_bits_int = tmp > 15 ? 15 : tmp;
    phwcfg->cmps_fixbit_mode     = psta->hw_transCfg_trans_mode == trans_lgi3f9_mode ? 1 : 0;
    LOGD_ATMO("phwcfg->cmps_byp_en %d, phwcfg->offset_pow2 %d, phwcfg->cmps_offset_bits_int %d\n",
              phwcfg->cmps_byp_en, phwcfg->offset_pow2, phwcfg->cmps_offset_bits_int);

    // phwcfg->bypass_en   = 0;
    phwcfg->gainx32_en  = 0;
    phwcfg->raw_dly_dis = 0;

    phwcfg->position = CLIP(pdyn->preProc.hw_drcT_toneCurveIdx_scale * (1 << 8), 0, (1 << 14) - 1);
    // get sw_drc_compres_scl
    float log_ratio2     = log(L2S_Ratio * drc_gain) / log(2.0f) + 12.0f;
    float offsetbits_int = CLIP(psta->hw_transCfg_transOfDrc_offset, 0, 15);

    unsigned char cmps_fixbit_mode =
        (psta->hw_transCfg_trans_mode == trans_lgi3f9_mode ? FUNCTION_ENABLE : FUNCTION_DISABLE);
    LOGD_ATMO("pDrcCtx->hw_transCfg_transOfDrc_offset %d", psta->hw_transCfg_transOfDrc_offset);
    int cmps_fix_bit                        = 8 + cmps_fixbit_mode;
    float offsetbits                        = offsetbits_int * (1 << cmps_fix_bit);
    float hdrbits                           = log_ratio2 * (1 << cmps_fix_bit);
    float hdrvalidbits                      = hdrbits - offsetbits;
    pdyn->drcProc.hw_drcT_drcCurveIdx_scale = (12.0f * (1 << (cmps_fix_bit + 11))) / hdrvalidbits;
    phwcfg->compres_scl = (unsigned short)pdyn->drcProc.hw_drcT_drcCurveIdx_scale;

    // get hw_drc_comps_gain_minLimit
    if (LongFrmMode) {
        pdyn->drcProc.hw_drcT_drcGain_minLimit = 1.0f;
    } else if (pdyn->drcProc.sw_drcT_drcGainLimit_mode == drc_drcGainLmt_auto_mode) {
        pdyn->drcProc.hw_drcT_drcGain_minLimit = 1.0f / (L2S_Ratio * drc_gain);
    }

    // get sw_drc_compres_y
    if (pdyn->drcProc.sw_drcT_drcCurve_mode == drc_vendorDefault_mode) {
        float luma2[DRC_CURVE_LEN] = {0.0f,     1024.0f,  2048.0f,  3072.0f,  4096.0f,  5120.0f,
                                      6144.0f,  7168.0f,  8192.0f,  10240.0f, 12288.0f, 14336.0f,
                                      16384.0f, 18432.0f, 20480.0f, 22528.0f, 24576.0f};
        float dstbits              = (float)(ISP_RAW_BIT << cmps_fix_bit);
        float validbits            = dstbits - offsetbits;
        float curveparam  = (float)(validbits - 0.0f) / (hdrvalidbits - validbits + 0.0156f);
        float curveparam2 = validbits * (1.0f + curveparam);
        float curveparam3 = hdrvalidbits * curveparam;
        for (int i = 0; i < DRC_CURVE_LEN; ++i) {
            float tmp        = luma2[i] * hdrvalidbits / 24576.0f;
            float curveTable = tmp * curveparam2 / (tmp + curveparam3);
            pdyn->drcProc.hw_drcT_hdr2Sdr_curve[i] =
                ((unsigned short)(curveTable)) >> cmps_fixbit_mode;
        }
    }

    // get sw_drc_gain_y
    if (pdyn->preProc.sw_drcT_toneCurve_mode == drc_cfgCurveCtrlCoeff_mode) {
        float tmp = 0.0f;
        /*luma[i] = pow((1.0f - luma[i] / 4096.0f), 2.0f)*/
        float luma[DRC_CURVE_LEN] = {1.0f,    0.8789f, 0.7656f, 0.6602f, 0.5625f, 0.4727f,
                                     0.3906f, 0.3164f, 0.2500f, 0.1914f, 0.1406f, 0.0977f,
                                     0.0625f, 0.0352f, 0.0156f, 0.0039f, 0.0f};
        float DrcGain             = pdyn->preProc.toneCurveCtrl.sw_drcT_toneGain_maxLimit;
        float alpha               = pdyn->preProc.toneCurveCtrl.sw_drcT_toneCurveK_coeff;
        for (int i = 0; i < DRC_CURVE_LEN; ++i) {
            tmp                                        = 1 - alpha * luma[i];
            tmp                                        = pow(DrcGain, tmp);
            pdyn->preProc.hw_drcT_luma2ToneGain_val[i] = tmp;
        }
    }

    tmp                    = 2048.0f * pdyn->drcProc.hw_drcT_loDetail_strg;
    phwcfg->lpdetail_ratio = tmp > 2048 ? 2048 : tmp;
    tmp                    = 2048.0f * pdyn->drcProc.hw_drcT_drcStrg_alpha;
    phwcfg->hpdetail_ratio = tmp > 2048 ? 2048 : tmp;
    tmp                    = 256.0f * pdyn->drcProc.hw_drcT_drcStrgLutLuma_scale;
    phwcfg->delta_scalein  = tmp > 255 ? 255 : tmp;
    phwcfg->min_ogain =
        CLIP((int)(pdyn->drcProc.hw_drcT_drcGain_minLimit * (1 << 15)), 0, (1 << 15));
    tmp               = 256.0f * pdyn->preProc.hw_drcT_lpfSoftThd_thred;
    phwcfg->drc_gas_t = tmp > 1023 ? 1023 : tmp;

    tmp                  = 128.0f * pdyn->bifilt_filter.hw_drcT_rgeWgt_negOff;
    phwcfg->bilat_wt_off = tmp > 255 ? 255 : tmp;
    tmp                  = 16.0f * pdyn->bifilt_filter.hw_drcT_centerPixel_wgt;
    phwcfg->weicur_pix   = tmp > 255 ? 255 : tmp;
    if (pdyn->bifilt_filter.hw_drcT_midRgeSgm_val != 0.0f)
        tmp = 256.0f * pdyn->bifilt_filter.hw_drcT_midRgeSgm_val;
    else
        tmp = 256.0f / 0.25f;
    phwcfg->range_sgm_inv0 = tmp > 1023 ? 1023 : tmp;
    if (pdyn->bifilt_filter.hw_drcT_loRgeSgm_val != 0.0f)
        tmp = 256.0f / pdyn->bifilt_filter.hw_drcT_loRgeSgm_val;
    else
        tmp = 256.0f / 0.25f;
    phwcfg->range_sgm_inv1  = tmp > 1023 ? 1023 : tmp;
    tmp                     = 16.0f * pdyn->bifilt_filter.hw_drcT_bifiltOut_alpha;
    phwcfg->weig_bilat      = tmp > 16 ? 16 : tmp;
    tmp                     = 256.0f * pdyn->bifilt_filter.hw_drcT_midWgt_alpha;
    phwcfg->weight_8x8thumb = tmp > 255 ? 255 : tmp;
    tmp                     = 2048.0f * pdyn->bifilt_filter.hw_drcT_softThd_thred;
    phwcfg->bilat_soft_thd  = tmp > 0x3ff ? 0x3ff : tmp;
    phwcfg->enable_soft_thd = pdyn->bifilt_filter.hw_drcT_softThd_en;

    tmp                      = 256.0f * pdyn->bifilt_guideDiff.hw_drcT_guideDiff_minLimit;
    phwcfg->thumb_thd_neg    = tmp > 511 ? 511 : tmp;
    phwcfg->thumb_thd_enable = pdyn->bifilt_guideDiff.hw_drcT_guideDiffLmt_en ? 1 : 0;
    tmp                      = 256.0f * pdyn->bifilt_guideDiff.hw_drcT_guideLuma_maxLimit;
    phwcfg->thumb_clip       = tmp > 4095 ? 4095 : tmp;
    tmp                      = 16.0f * pdyn->bifilt_guideDiff.hw_drcT_maxLutIdx_scale;
    phwcfg->thumb_scale      = tmp > 255 ? 255 : tmp;
    if (pdyn->bifilt_guideDiff.sw_drcT_gdDiffMaxLut_mode == drc_cfgCurveCtrlCoeff_mode) {
        float tmp = 0.0f;
        for (int i = 0; i < DRC_CURVE_LEN; ++i) {
            tmp = pdyn->bifilt_guideDiff.gdDiffMaxCurveCtrl.sw_drcT_maxLutCreate_maxLimit -
                  (pdyn->bifilt_guideDiff.gdDiffMaxCurveCtrl.sw_drcT_maxLutCreate_maxLimit -
                   pdyn->bifilt_guideDiff.gdDiffMaxCurveCtrl.sw_drcT_maxLutCreate_minLimit) /
                      (1.0f +
                       pow(2.718f,
                           -pdyn->bifilt_guideDiff.gdDiffMaxCurveCtrl.sw_drcT_maxLutCreate_slope *
                               (i / 16.0f - pdyn->bifilt_guideDiff.gdDiffMaxCurveCtrl
                                                .sw_drcT_maxLutCreate_offset)));
            phwcfg->sfthd_y[i] = (unsigned short)(tmp * (255 + 1));
        }
    } else if (pdyn->bifilt_guideDiff.sw_drcT_gdDiffMaxLut_mode == drc_cfgCurveDirect_mode) {
        for (int i = 0; i < DRC_CURVE_LEN; ++i) {
            tmp                = 256.0f * pdyn->bifilt_guideDiff.hw_drcT_gdLuma2DiffMax_lut[i];
            phwcfg->sfthd_y[i] = tmp > 511 ? 511 : tmp;
        }
    }

    for (int i = 0; i < DRC_CURVE_LEN; ++i) {
        tmp               = 1024.0f * pdyn->preProc.hw_drcT_luma2ToneGain_val[i];
        phwcfg->gain_y[i] = tmp > 0x1fff ? 0x1fff : tmp;

        tmp                  = pdyn->drcProc.hw_drcT_hdr2Sdr_curve[i];
        phwcfg->compres_y[i] = tmp > 0x400 ? 0x400 : tmp;

        tmp                = 2048.0f * pdyn->drcProc.hw_drcT_luma2DrcStrg_val[i];
        phwcfg->scale_y[i] = tmp > 0x800 ? 0x800 : tmp;
    }
    if (!cvtinfo->use_aiisp)
        cvtinfo->preDGain = CLIP(pdyn->preProc.hw_drcT_luma2ToneGain_val[16], 0, 4);
    LOGD_ATMO("%s: cvtinfo->preDGain %f\n", __FUNCTION__, cvtinfo->preDGain);

    if ((cvtinfo->frameNum > 1 || cvtinfo->preDGain > 1) && phwcfg->cmps_byp_en == 1) {
        phwcfg->cmps_byp_en = 0;
    }
    cvtinfo->cmps_on = phwcfg->cmps_byp_en == 0 ? 1 : 0;
#if ISP_HW_V33
    phwcfg->position &= 0x3ff0;
    for (int i = 0; i < DRC_CURVE_LEN; ++i) {
        phwcfg->gain_y[i] &= 0x1ff0;
        phwcfg->scale_y[i] &= 0xfe0;
    }
#endif
    rk_aiq_drc40_params_dump(attr, isp_params);
}
