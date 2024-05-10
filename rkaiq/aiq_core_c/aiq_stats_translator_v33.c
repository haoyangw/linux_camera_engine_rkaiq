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

#include "aiq_stats_translator.h"
#include "xcore_c/aiq_v4l2_buffer.h"
#include "hwi_c/aiq_sensorHw.h"
#include "hwi_c/aiq_lensHw.h"

// #define AE_STATS_DEBUG
#define MAX_8BITS  ((1 << 8) - 1)
#define MAX_10BITS (1 << 10) - 1
#define MAX_12BITS (1 << 12) - 1
#define MAX_29BITS (1 << 29) - 1
#define MAX_32BITS 0xffffffff

#if defined(ISP_HW_V33) & defined(USE_NEWSTRUCT)


void calcAecBigWinStatsV33(
    struct isp33_rawae_stat*    stats_in,
    aeStats_mainWinStats_t*     stats_out,
    struct isp2x_bls_fixed_val  bls1_val,
    struct isp2x_bls_fixed_val  awb1_gain
) {

    //copy all channel
    for (int i = 0; i < ISP32_RAWAEBIG_MEAN_NUM; i++) {
        stats_out->hw_ae_meanBayerR_val[i] = CLIP((int)(stats_in->blk_y[i / 15].blk_x[i % 15].r * awb1_gain.r / 256 - bls1_val.r), 0, MAX_10BITS);
        stats_out->hw_ae_meanBayerGrGb_val[i] = CLIP((int)(stats_in->blk_y[i / 15].blk_x[i % 15].g * awb1_gain.gr / 256 - bls1_val.gr), 0, MAX_12BITS);
        stats_out->hw_ae_meanBayerB_val[i] = CLIP((int)(stats_in->blk_y[i / 15].blk_x[i % 15].b * awb1_gain.b / 256 - bls1_val.b), 0, MAX_10BITS);
    }
}

void calcAecHistBinStatsV33(
    u32*                        hist_bin_in,
    u32*                        hist_bin_out,
    u8                          hist_mode,
    u16                         ob_offset_rb,
    u16                         ob_offset_g,
    struct isp2x_bls_fixed_val  bls1_val,
    struct isp2x_bls_fixed_val  awb1_gain,
    bool                        is_hdr
) {

    if (!is_hdr) {

        memset(hist_bin_out, 0, ISP39_HIST_BIN_N_MAX * sizeof(u32));

        s16 bls1, bls_part;
        u16 ob_part, awb_part, div_part, round_part;
        int tmp;
        switch (hist_mode) {
        case 2:
            ob_part = ob_offset_rb >> 2;
            bls1 = bls1_val.r >> 2;
            awb_part = awb1_gain.r;
            bls_part = (ob_part + bls1) * awb_part;
            div_part = 256;
            round_part = 128;
            break;
        case 3:
            ob_part = ob_offset_g >> 4;
            bls1 = bls1_val.gr >> 4;
            awb_part = awb1_gain.gr;
            bls_part = (ob_part + bls1) * awb_part;
            div_part = 256;
            round_part = 128;
            break;
        case 4:
            ob_part = ob_offset_rb >> 2;
            bls1 = bls1_val.b >> 2;
            awb_part = awb1_gain.b;
            bls_part = (ob_part + bls1) * awb_part;
            div_part = 256;
            round_part = 128;
            break;
        case 5:
        default:
            ob_part = (u16)((ob_offset_g >> 4) * 587 + (ob_offset_rb >> 2) * 299 + (ob_offset_rb >> 2) * 114 + 500) / 1000;
            bls1 = (s16)((bls1_val.gr >> 4) * 587 + (bls1_val.r >> 2) * 299 + (bls1_val.b >> 2) * 114 + 500) / 1000;
            awb_part = 100;
            bls_part = (ob_part + bls1) * awb_part;
            div_part = 7655.0f / (float)awb1_gain.r + 15027.0f / (float)awb1_gain.gr + 2919.0f / (float)awb1_gain.b;
            round_part = div_part / 2;
            break;
        }

        for (int i = 0; i < ISP39_HIST_BIN_N_MAX; i++) {
            tmp = (i - ob_part - bls1 > 0) ? (i * awb_part - bls_part + round_part) / div_part : 0;
            tmp = (tmp > ISP39_HIST_BIN_N_MAX - 1) ? (ISP39_HIST_BIN_N_MAX - 1) : tmp;
            hist_bin_out[tmp] += hist_bin_in[i];
        }


    } else {

        memcpy(hist_bin_out, hist_bin_in, ISP39_HIST_BIN_N_MAX * sizeof(u32));
    }

}

XCamReturn translateAecStatsV33(AiqStatsTranslator_t* pStatsTrans, const aiq_VideoBuffer_t* from,
                                aiq_stats_base_t* to) {
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

#if defined(ISP_HW_V33)

    // 0) blc awb cfg
    struct isp33_isp_meas_cfg* isp_params = &pStatsTrans->_ispParams->meas;
    uint8_t AeSwapMode, AeSelMode, Bnr20bitEn;
    AeSwapMode             = isp_params->rawae0.rawae_sel & 0x0f;
    AeSelMode              = isp_params->rawae3.rawae_sel & 0x0f;
    Bnr20bitEn             = isp_params->rawae0.rawae_sel & 0xf0;
    unsigned int meas_type = 0;

    // ae_stats = (ae_ori_stats_u12/10 - ob_offset_u9 - bls1_val_u12) * awb1_gain_u16 * range_ratio
    struct isp32_bls_cfg* bls_cfg           = &pStatsTrans->_ispParams->bls_cfg;
    struct isp32_awb_gain_cfg* awb_gain_cfg = &pStatsTrans->_ispParams->awb_gain_cfg;

    struct isp2x_bls_fixed_val bls1_ori_val;
    struct isp2x_bls_fixed_val bls1_val;  // bls1_val = blc1_ori_val * awb * range_ratio
    struct isp2x_bls_fixed_val awb1_gain;

    u16 isp_ob_offset_rb, isp_ob_offset_g, isp_ob_predgain;
    u32 pixel_num      = 0;
    u8  rawhist_mode   = 0;
    bool is_hdr = ((pStatsTrans->mWorkingMode > 0) ? true : false) | (Bnr20bitEn > 0 ? true : false);
    bool is_bls1_en = bls_cfg->bls1_en && !is_hdr;

    isp_ob_offset_rb = bls_cfg->isp_ob_offset >> 2;
    isp_ob_offset_g  = bls_cfg->isp_ob_offset;
    isp_ob_predgain =  MAX(bls_cfg->isp_ob_predgain >> 8, 1);

    if (is_bls1_en) {
        bls1_ori_val.r  = (bls_cfg->bls1_val.r / isp_ob_predgain) >> 2;
        bls1_ori_val.gr = bls_cfg->bls1_val.gr / isp_ob_predgain;
        bls1_ori_val.gb = bls_cfg->bls1_val.gb / isp_ob_predgain;
        bls1_ori_val.b  = (bls_cfg->bls1_val.b / isp_ob_predgain) >> 2;
    } else {
        bls1_ori_val.r  = 0;
        bls1_ori_val.gr = 0;
        bls1_ori_val.gb = 0;
        bls1_ori_val.b  = 0;
    }

    //awb1_gain have adapted to the range of bls1_lvl
    awb1_gain.r = MAX(256, awb_gain_cfg->awb1_gain_r);
    awb1_gain.gr = MAX(256, awb_gain_cfg->awb1_gain_gr);
    awb1_gain.gb = MAX(256, awb_gain_cfg->awb1_gain_gb);
    awb1_gain.b = MAX(256, awb_gain_cfg->awb1_gain_b);

#ifdef AE_STATS_DEBUG
    LOGE("bls1[%d-%d-%d-%d]", bls1_ori_val.r, bls1_ori_val.gr, bls1_ori_val.gb, bls1_ori_val.b);
    LOGE("isp_ob_offset_rb, isp_ob_offset_g, isp_ob_predgain [%d-%d-%d]",
         isp_ob_offset_rb, isp_ob_offset_g, isp_ob_predgain);
    LOGE("awb1_gain[%d-%d-%d-%d]", awb1_gain.r, awb1_gain.gr, awb1_gain.gb, awb1_gain.b);
#endif

    // bls1_val = (bls1_ori_val + ob) * awb * range_ratio
    bls1_val.r = ((isp_ob_offset_rb + bls1_ori_val.r) * awb1_gain.r + 128) / 256;
    bls1_val.gr = ((isp_ob_offset_g + bls1_ori_val.gr) * awb1_gain.gr + 128) / 256;
    bls1_val.gb = ((isp_ob_offset_g + bls1_ori_val.gb) * awb1_gain.gb + 128) / 256;
    bls1_val.b = ((isp_ob_offset_rb + bls1_ori_val.b) * awb1_gain.b + 128) / 256;

    aiq_ae_stats_wrapper_t* statsInt = (aiq_ae_stats_wrapper_t*)to->_data;
    struct rkisp33_stat_buffer* stats =
        (struct rkisp33_stat_buffer*)(AiqV4l2Buffer_getExpbufUsrptr((AiqV4l2Buffer_t*)from));

    if (stats == NULL) {
        LOGE("fail to get stats ,ignore\n");
        return XCAM_RETURN_BYPASS;
    }

    LOGI_ANALYZER("camId: %d, stats: frame_id: %d,  meas_type; 0x%x", pStatsTrans->mCamPhyId,
                  stats->frame_id, stats->meas_type);

    AiqIrisInfoWrapper_t irisParamsTmp;
    AiqIrisInfoWrapper_t* irisParams = NULL;
    if (pStatsTrans->mIrishw) {
        XCamReturn ret = AiqLensHw_getIrisInfoParams(pStatsTrans->mIrishw, stats->frame_id, &irisParamsTmp);
        if (ret == XCAM_RETURN_NO_ERROR) irisParams = &irisParamsTmp;
    }

    // ae stats v3.3
    to->frame_id = stats->frame_id;

    if (AeSwapMode == AEC_RAWSWAP_MODE_S_LITE) {
        meas_type = ((stats->meas_type >> 7) & (0x01)) & ((stats->meas_type >> 11) & (0x01));
    } else {
        LOGE("wrong AeSwapMode=%d\n", AeSwapMode);
        return XCAM_RETURN_ERROR_PARAM;
    }

    to->bValid = (meas_type & 0x01) ? true : false;
    if (!to->bValid) {
        LOGE("aec_stats_valid=false,meas_type=0x%x", stats->meas_type);
        return XCAM_RETURN_BYPASS;
    }

    statsInt->aec_stats_v25.ae_data.hw_ae_entityGroup_mode = pStatsTrans->_ispParams->ae_cfg_v39.hw_aeCfg_entityGroup_mode;

    if(pStatsTrans->_ispParams->ae_cfg_v39.hw_aeCfg_entityGroup_mode == aeStats_entity03_indWk_mode || \
            pStatsTrans->_ispParams->ae_cfg_v39.hw_aeCfg_entityGroup_mode == aeStats_entity0_wkOnly_mode) {

        //RAWAE0 BIG-MODE
        pixel_num = pStatsTrans->_ispParams->ae_cfg_v39.entityGroup.entities.entity0.subWin[0].hw_aeCfg_win_width \
                    * pStatsTrans->_ispParams->ae_cfg_v39.entityGroup.entities.entity0.subWin[0].hw_aeCfg_win_height;

        calcAecBigWinStatsV33(&stats->stat.rawae0, &statsInt->aec_stats_v25.ae_data.entityGroup.entities.entity0.mainWin,
                              bls1_val, awb1_gain);
        //For isp33, only has one sub win
        statsInt->aec_stats_v25.ae_data.entityGroup.entities.entity0.subWin[0].hw_ae_sumBayerR_val = CLIP((s64)((u64)stats->stat.rawae0.wnd1_sumr * awb1_gain.r / 256 - (pixel_num >> 2) * bls1_val.r), 0, MAX_29BITS);
        statsInt->aec_stats_v25.ae_data.entityGroup.entities.entity0.subWin[0].hw_ae_sumBayerGrGb_val = CLIP((s64)((u64)stats->stat.rawae0.wnd1_sumg * awb1_gain.gr / 256 - (pixel_num >> 1) * bls1_val.gb), 0, MAX_32BITS);
        statsInt->aec_stats_v25.ae_data.entityGroup.entities.entity0.subWin[0].hw_ae_sumBayerB_val = CLIP((s64)((u64)stats->stat.rawae0.wnd1_sumb * awb1_gain.b / 256 - (pixel_num >> 2) * bls1_val.b), 0, MAX_29BITS);

        //RAWHIST0 BIG-MODE
        rawhist_mode = isp_params->rawhist0.mode;

        calcAecHistBinStatsV33(stats->stat.rawhist0.bin,
                               statsInt->aec_stats_v25.ae_data.entityGroup.entities.entity0.hist.hw_ae_histBin_val,
                               rawhist_mode, isp_ob_offset_rb, isp_ob_offset_g,
                               bls1_ori_val, awb1_gain, is_hdr);
    }

    if(pStatsTrans->_ispParams->ae_cfg_v39.hw_aeCfg_entityGroup_mode == aeStats_entity03_indWk_mode || \
            pStatsTrans->_ispParams->ae_cfg_v39.hw_aeCfg_entityGroup_mode == aeStats_entity3_wkOnly_mode) {

        switch (AeSelMode) {
        case AEC_RAWSEL_MODE_CHN_0:
        case AEC_RAWSEL_MODE_CHN_1:

            //RAWAE3 BIG-MODE
            pixel_num = pStatsTrans->_ispParams->ae_cfg_v39.entityGroup.entities.entity3.subWin[0].hw_aeCfg_win_width \
                        * pStatsTrans->_ispParams->ae_cfg_v39.entityGroup.entities.entity3.subWin[0].hw_aeCfg_win_height;

            calcAecBigWinStatsV33(&stats->stat.rawae3, &statsInt->aec_stats_v25.ae_data.entityGroup.entities.entity3.mainWin,
                                  bls1_val, awb1_gain);

            //For isp33, only has one sub win
            statsInt->aec_stats_v25.ae_data.entityGroup.entities.entity3.subWin[0].hw_ae_sumBayerR_val = CLIP((s64)((u64)stats->stat.rawae3.wnd1_sumr * awb1_gain.r / 256 - (pixel_num >> 2) * bls1_val.r), 0, MAX_29BITS);
            statsInt->aec_stats_v25.ae_data.entityGroup.entities.entity3.subWin[0].hw_ae_sumBayerGrGb_val = CLIP((s64)((u64)stats->stat.rawae3.wnd1_sumg * awb1_gain.gr / 256 - (pixel_num >> 1) * bls1_val.gb), 0, MAX_32BITS);
            statsInt->aec_stats_v25.ae_data.entityGroup.entities.entity3.subWin[0].hw_ae_sumBayerB_val = CLIP((s64)((u64)stats->stat.rawae3.wnd1_sumb * awb1_gain.b / 256 - (pixel_num >> 2) * bls1_val.b), 0, MAX_29BITS);

            //RAWHIST3 BIG-MODE
            rawhist_mode = isp_params->rawhist3.mode;
            calcAecHistBinStatsV33(stats->stat.rawhist3.bin,
                                   statsInt->aec_stats_v25.ae_data.entityGroup.entities.entity3.hist.hw_ae_histBin_val,
                                   rawhist_mode, isp_ob_offset_rb, isp_ob_offset_g,
                                   bls1_ori_val, awb1_gain, is_hdr);
            break;
        case AEC_RAWSEL_MODE_TMO:

            bls1_val.r = 0;
            bls1_val.gr = 0;
            bls1_val.gb = 0;
            bls1_val.b = 0;

            awb1_gain.r = 256;
            awb1_gain.gr = 256;
            awb1_gain.gb = 256;
            awb1_gain.b = 256;

            pixel_num = pStatsTrans->_ispParams->ae_cfg_v39.entityGroup.entities.entity3.subWin[0].hw_aeCfg_win_width \
                        * pStatsTrans->_ispParams->ae_cfg_v39.entityGroup.entities.entity3.subWin[0].hw_aeCfg_win_height;

            calcAecBigWinStatsV33(&stats->stat.rawae3, &statsInt->aec_stats_v25.ae_data.entityGroup.entities.entity3.mainWin,
                                  bls1_val, awb1_gain);

            //For isp33, only has one sub win
            statsInt->aec_stats_v25.ae_data.entityGroup.entities.entity3.subWin[0].hw_ae_sumBayerR_val = stats->stat.rawae3.wnd1_sumr;
            statsInt->aec_stats_v25.ae_data.entityGroup.entities.entity3.subWin[0].hw_ae_sumBayerGrGb_val = stats->stat.rawae3.wnd1_sumg;
            statsInt->aec_stats_v25.ae_data.entityGroup.entities.entity3.subWin[0].hw_ae_sumBayerB_val = stats->stat.rawae3.wnd1_sumb;

            memcpy(statsInt->aec_stats_v25.ae_data.entityGroup.entities.entity3.hist.hw_ae_histBin_val,
                   stats->stat.rawhist3.bin, ISP33_HIST_BIN_N_MAX * sizeof(u32));

            break;

        default:
            LOGE("wrong AeSelMode=%d\n", AeSelMode);
            return XCAM_RETURN_ERROR_PARAM;
        }
    }

#ifdef AE_STATS_DEBUG


    for (int i = 0; i < 15; i++) {
        for (int j = 0; j < 15; j++) {
            printf("entity0[%d,%d]:r 0x%x, g 0x%x, b 0x%x\n", i, j,
                   statsInt->aec_stats_v25.ae_data.entityGroup.entities.entity0.mainWin.hw_ae_meanBayerR_val[i * 15 + j],
                   statsInt->aec_stats_v25.ae_data.entityGroup.entities.entity0.mainWin.hw_ae_meanBayerGrGb_val[i * 15 + j],
                   statsInt->aec_stats_v25.ae_data.entityGroup.entities.entity0.mainWin.hw_ae_meanBayerB_val[i * 15 + j]);
        }
    }

    printf("====================sub-win-result======================\n");

    printf("entity0 subwin[0]:sumr 0x%x, sumg 0x%x, sumb 0x%x\n",
           statsInt->aec_stats_v25.ae_data.entityGroup.entities.entity0.subWin[0].hw_ae_sumBayerR_val,
           statsInt->aec_stats_v25.ae_data.entityGroup.entities.entity0.subWin[0].hw_ae_sumBayerGrGb_val,
           statsInt->aec_stats_v25.ae_data.entityGroup.entities.entity0.subWin[0].hw_ae_sumBayerB_val);

    printf("====================hist_result========================\n");

    for (int i = 0; i < 256; i++)
        printf("entity0 bin[%d]= 0x%08x\n", i, statsInt->aec_stats_v25.ae_data.entityGroup.entities.entity0.hist.hw_ae_histBin_val[i]);

    for (int i = 0; i < 15; i++) {
        for (int j = 0; j < 15; j++) {
            printf("entity3[%d,%d]:r 0x%x, g 0x%x, b 0x%x\n", i, j,
                   statsInt->aec_stats_v25.ae_data.entityGroup.entities.entity3.mainWin.hw_ae_meanBayerR_val[i * 15 + j],
                   statsInt->aec_stats_v25.ae_data.entityGroup.entities.entity3.mainWin.hw_ae_meanBayerGrGb_val[i * 15 + j],
                   statsInt->aec_stats_v25.ae_data.entityGroup.entities.entity3.mainWin.hw_ae_meanBayerB_val[i * 15 + j]);
        }
    }
    printf("====================sub-win-result======================\n");

    printf("entity3 subwin[0]:sumr 0x%x, sumg 0x%x, sumb 0x%x\n",
           statsInt->aec_stats_v25.ae_data.entityGroup.entities.entity3.subWin[0].hw_ae_sumBayerR_val,
           statsInt->aec_stats_v25.ae_data.entityGroup.entities.entity3.subWin[0].hw_ae_sumBayerGrGb_val,
           statsInt->aec_stats_v25.ae_data.entityGroup.entities.entity3.subWin[0].hw_ae_sumBayerB_val);

    printf("====================hist_result========================\n");

    for (int i = 0; i < 256; i++)
        printf("entity3 bin[%d]= 0x%08x\n", i, statsInt->aec_stats_v25.ae_data.entityGroup.entities.entity3.hist.hw_ae_histBin_val[i]);


#endif


    /*
     *         unsigned long chn0_mean = 0, chn1_mean = 0;
     *         for(int i = 0; i < ISP3X_RAWAEBIG_MEAN_NUM; i++) {
     *             chn0_mean += stats->params.rawae1.data[i].channelg_xy;
     *             chn1_mean += stats->params.rawae3.data[i].channelg_xy;
     *         }
     *
     *
     *         printf("frame[%d]: chn[0-1]_g_mean_xy: %ld-%ld\n",
     *                 stats->frame_id, chn0_mean/ISP3X_RAWAEBIG_MEAN_NUM,
     *                 chn1_mean/ISP3X_RAWAEBIG_MEAN_NUM);
     */

    // expsoure params
    if (pStatsTrans->_expParams) {
        statsInt->aec_stats_v25.ae_exp = pStatsTrans->_expParams->aecExpInfo;
        /*printf("frame[%d],gain=%d,time=%d\n", stats->frame_id,
               expParams->aecExpInfo.LinearExp.exp_sensor_params.analog_gain_code_global,
               expParams->aecExpInfo.LinearExp.exp_sensor_params.coarse_integration_time);*/

        /*
         * printf("%s: L: [0x%x-0x%x], M: [0x%x-0x%x], S: [0x%x-0x%x]\n",
         *        __func__,
         *        expParams->aecExpInfo.HdrExp[2].exp_sensor_params.coarse_integration_time,
         *        expParams->aecExpInfo.HdrExp[2].exp_sensor_params.analog_gain_code_global,
         *        expParams->aecExpInfo.HdrExp[1].exp_sensor_params.coarse_integration_time,
         *        expParams->aecExpInfo.HdrExp[1].exp_sensor_params.analog_gain_code_global,
         *        expParams->aecExpInfo.HdrExp[0].exp_sensor_params.coarse_integration_time,
         *        expParams->aecExpInfo.HdrExp[0].exp_sensor_params.analog_gain_code_global);
         */
    }

    // iris params
    if (irisParams) {
        float sof_time   = (float)irisParams->sofTime / 1000000000.0f;
        float start_time = (float)irisParams->PIris.StartTim.tv_sec +
                           (float)irisParams->PIris.StartTim.tv_usec / 1000000.0f;
        float end_time = (float)irisParams->PIris.EndTim.tv_sec +
                         (float)irisParams->PIris.EndTim.tv_usec / 1000000.0f;
        float frm_intval = 1 / (statsInt->aec_stats_v25.ae_exp.pixel_clock_freq_mhz * 1000000.0f /
                                (float)statsInt->aec_stats_v25.ae_exp.line_length_pixels /
                                (float)statsInt->aec_stats_v25.ae_exp.frame_length_lines);

        /*printf("%s: step=%d,last-step=%d,start-tim=%f,end-tim=%f,sof_tim=%f\n",
            __func__,
            statsInt->aec_stats_v25.ae_exp.Iris.PIris.step,
            irisParams->data()->PIris.laststep,start_time,end_time,sof_time);
        */

        if (sof_time < end_time + frm_intval)
            statsInt->aec_stats_v25.ae_exp.Iris.PIris.step = irisParams->PIris.laststep;
        else
            statsInt->aec_stats_v25.ae_exp.Iris.PIris.step = irisParams->PIris.step;
    }

    to->frame_id = stats->frame_id;
#endif

    return ret;
}
#endif

XCamReturn translateAwbStatsV33(AiqStatsTranslator_t* pStatsTrans, const aiq_VideoBuffer_t* from,
                                aiq_stats_base_t* to) {
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
#if defined(ISP_HW_V33)

    aiq_awb_stats_wrapper_t* statsInt = (aiq_awb_stats_wrapper_t*)to->_data;
    struct rkisp33_stat_buffer* stats =
        (struct rkisp33_stat_buffer*)(AiqV4l2Buffer_getExpbufUsrptr((AiqV4l2Buffer_t*)from));

    if (stats == NULL) {
        LOGE("fail to get stats ,ignore\n");
        return XCAM_RETURN_BYPASS;
    }
    LOGI_ANALYZER("awb stats: camId:%d, frame_id: %d,  meas_type; 0x%x", pStatsTrans->mCamPhyId,
                  stats->frame_id, stats->meas_type);

    to->bValid = stats->meas_type >> 5 & 1;
    if (!to->bValid) {
        LOGE_ANALYZER("AWB stats invalid, ignore");
        return XCAM_RETURN_BYPASS;
    }
    memset(&statsInt->awb_stats_v39, 0, sizeof(statsInt->awb_stats_v39));

    if (stats->stat.info2ddr.owner == RKISP_INFO2DRR_OWNER_AWB) {
        statsInt->awb_stats_v39.dbginfo_fd = stats->stat.info2ddr.buf_fd;
    } else {
        statsInt->awb_stats_v39.dbginfo_fd = -1;
    }
    statsInt->awb_stats_v39.awb_cfg_effect.blkMeasureMode =
        (rk_aiq_awb_blk_stat_mode_v201_t)(pStatsTrans->_ispParams->awb_cfg_v39.com.pixEngine.hw_awbCfg_zoneStatsSrc_mode > awbStats_pixAll_mode);
    statsInt->awb_stats_v39.awb_cfg_effect.mode = pStatsTrans->_ispParams->awb_cfg_v39.mode;
    statsInt->awb_stats_v39.awb_cfg_effect.lightNum = pStatsTrans->_ispParams->awb_cfg_v39.com.wpEngine.hw_awbCfg_lightSrcNum_val;
    statsInt->awb_stats_v39.awb_cfg_effect.groupIllIndxCurrent = 0;//pStatsTrans->_ispParams->awb_cfg_v39.groupIllIndxCurrent;
    memcpy(statsInt->awb_stats_v39.awb_cfg_effect.IllIndxSetCurrent, pStatsTrans->_ispParams->awb_cfg_v39.IllIndxSetCurrent,
           sizeof(statsInt->awb_stats_v39.awb_cfg_effect.IllIndxSetCurrent));
    memcpy(statsInt->awb_stats_v39.awb_cfg_effect.timeSign, pStatsTrans->_ispParams->awb_cfg_v39.timeSign,
           sizeof(statsInt->awb_stats_v39.awb_cfg_effect.timeSign));
    memcpy(statsInt->awb_stats_v39.awb_cfg_effect.preWbgainSw, pStatsTrans->_ispParams->awb_cfg_v39.preWbgainSw,
           sizeof(pStatsTrans->_ispParams->awb_cfg_v39.preWbgainSw));
    statsInt->awb_cfg_effect_valid = true;
    to->frame_id             = stats->frame_id;

    awbStats_stats_t *awb_stats_v39 = &statsInt->awb_stats_v39.com ;

    for(int i = 0; i < statsInt->awb_stats_v39.awb_cfg_effect.lightNum; i++) {
        awb_stats_v39->wpEngine.norWp[i].hw_awbCfg_rGainSum_val =
            stats->stat.rawawb.sum[i].rgain_nor;
        awb_stats_v39->wpEngine.norWp[i].hw_awbCfg_bGainSum_val =
            stats->stat.rawawb.sum[i].bgain_nor;
        awb_stats_v39->wpEngine.norWp[i].hw_awbCfg_statsWp_count =
            stats->stat.rawawb.sum[i].wp_num_nor;
        awb_stats_v39->wpEngine.bigWp[i].hw_awbCfg_rGainSum_val =
            stats->stat.rawawb.sum[i].rgain_big;
        awb_stats_v39->wpEngine.bigWp[i].hw_awbCfg_bGainSum_val =
            stats->stat.rawawb.sum[i].bgain_big;
        awb_stats_v39->wpEngine.bigWp[i].hw_awbCfg_statsWp_count =
            stats->stat.rawawb.sum[i].wp_num_big;
    }
    memset(&statsInt->awb_stats_v39.sumBlkRGB, 0, sizeof(statsInt->awb_stats_v39.sumBlkRGB));
    int index = 0;
    for (int i = 0; i < ISP39_MEAN_BLK_Y_NUM; i++) {
        for (int j = 0; j < ISP39_MEAN_BLK_X_NUM; j++) {
            awb_stats_v39->pixEngine.zonePix[index].hw_awbCfg_rSum_val = stats->stat.rawawb.ramdata_blk_y[i].ramdata_blk_x[j].r;
            awb_stats_v39->pixEngine.zonePix[index].hw_awbCfg_gSum_val = stats->stat.rawawb.ramdata_blk_y[i].ramdata_blk_x[j].g;
            awb_stats_v39->pixEngine.zonePix[index].hw_awbCfg_bSum_val = stats->stat.rawawb.ramdata_blk_y[i].ramdata_blk_x[j].b;
            awb_stats_v39->pixEngine.zonePix[index].hw_awbCfg_statsPix_count = stats->stat.rawawb.ramdata_blk_y[i].ramdata_blk_x[j].wp;
            statsInt->awb_stats_v39.sumBlkRGB.Rvalue += awb_stats_v39->pixEngine.zonePix[index].hw_awbCfg_rSum_val;
            statsInt->awb_stats_v39.sumBlkRGB.Gvalue += awb_stats_v39->pixEngine.zonePix[index].hw_awbCfg_gSum_val;
            statsInt->awb_stats_v39.sumBlkRGB.Bvalue += awb_stats_v39->pixEngine.zonePix[index].hw_awbCfg_bSum_val;
            index++;
        }
    }

    for(int i = 0; i < RK_AIQ_AWB_WP_HIST_BIN_NUM; i++) {
        awb_stats_v39->wpEngine.hw_awb_wpHistBin_val[i] = stats->stat.rawawb.yhist[i];
        // move the shift code here to make WpNoHist merged by several cameras easily
        if( stats->stat.rawawb.yhist[i]  & 0x8000 ) {
            awb_stats_v39->wpEngine.hw_awb_wpHistBin_val[i] = stats->stat.rawawb.yhist[i] & 0x7FFF;
            awb_stats_v39->wpEngine.hw_awb_wpHistBin_val[i] *=    (1 << 3);
        }
    }

    for(int i = 0; i < statsInt->awb_stats_v39.awb_cfg_effect.lightNum; i++) {
        awb_stats_v39->wpEngine.hw_awbCfg_wpXyUvSpcRaw_cnt[i] =  stats->stat.rawawb.sum[i].wp_num2;
    }
    for(int i = 0; i < RK_AIQ_AWB_STAT_WP_RANGE_NUM_V201; i++) {
        awb_stats_v39->wpFltOutFullEngine.fltPix[i].hw_awbCfg_rGainSum_val = stats->stat.rawawb.sum_exc[i].rgain_exc;
        awb_stats_v39->wpFltOutFullEngine.fltPix[i].hw_awbCfg_bGainSum_val = stats->stat.rawawb.sum_exc[i].bgain_exc;
        awb_stats_v39->wpFltOutFullEngine.fltPix[i].hw_awbCfg_statsWp_count =    stats->stat.rawawb.sum_exc[i].wp_num_exc;

    }
    to->frame_id = stats->frame_id;
#endif
    return ret;
}

#if RKAIQ_HAVE_DEHAZE_V14
XCamReturn translateAdehazeStatsV14(AiqStatsTranslator_t* pStatsTrans,
                                    const aiq_VideoBuffer_t* from, aiq_stats_base_t* to) {
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    rkisp_adehaze_stats_t* statsInt = (rkisp_adehaze_stats_t*)to->_data;
    struct rkisp33_stat_buffer* stats =
        (struct rkisp33_stat_buffer*)(AiqV4l2Buffer_getExpbufUsrptr((AiqV4l2Buffer_t*)from));
    if (stats == NULL) {
        LOGE("fail to get stats ,ignore\n");
        return XCAM_RETURN_BYPASS;
    }
    LOGI_ANALYZER("stats: frame_id: %d,  meas_type; 0x%x", stats->frame_id, stats->meas_type);

    // dehaze
    to->bValid = stats->meas_type & ISP39_STAT_DHAZ;
    if (!to->bValid) return XCAM_RETURN_BYPASS;

    statsInt->dehaze_stats_v14.adp_wt       = stats->stat.dhaz.adp_wt;
    statsInt->dehaze_stats_v14.adp_air_base = stats->stat.dhaz.adp_air_base;
    statsInt->dehaze_stats_v14.adp_tmax     = stats->stat.dhaz.adp_tmax;
    for (int j = 0; j < ISP39_DHAZ_HIST_IIR_NUM; j++)
        for (int i = 0; i < ISP39_DHAZ_HIST_IIR_BLK_MAX; i++)
            statsInt->dehaze_stats_v14.hist_iir[i][j] = stats->stat.dhaz.hist_iir[i][j];

    return ret;
}

#endif

#if defined(RKAIQ_HAVE_BAYERTNR_V30)
XCamReturn translateBay3dStatsV30(AiqStatsTranslator_t* pStatsTrans, const aiq_VideoBuffer_t* from,
                                  AiqList_t* to, AiqPool_t* pool, AiqMutex_t* lock) {
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    struct rkisp33_stat_buffer* stats =
        (struct rkisp33_stat_buffer*)(AiqV4l2Buffer_getExpbufUsrptr((AiqV4l2Buffer_t*)from));

    if (stats == NULL) {
        LOGE_ANALYZER("Fail to get isp stats, ignore");
        return XCAM_RETURN_BYPASS;
    }

    LOGI_ANALYZER("camId: %d, isp stats: frame_id: %d, meas_type; 0x%x", pStatsTrans->mCamPhyId, stats->frame_id,
                  stats->meas_type);

    if (!(stats->meas_type & ISP39_STAT_BAY3D)) {
        LOGE_ANALYZER("bay3d stat is invalid, ignore");
        return XCAM_RETURN_BYPASS;
    }

    AiqPoolItem_t* pItem = aiqPool_getFree(pool);
    if (!pItem) {
        LOGE_ANALYZER("fid:%d no free dhazStats buf", stats->frame_id);
        return XCAM_RETURN_BYPASS;
    }

    aiq_stats_base_t* pbase        = (aiq_stats_base_t*)pItem->_pData;
    rk_aiq_bay3d_stat_t* bay3dStat = (rk_aiq_bay3d_stat_t*)pbase->_data;

    pbase->bValid   = true;
    pbase->frame_id = stats->frame_id;
    *bay3dStat      = stats->stat.bay3d;

    LOGV_ANALYZER(
        "camId: %d, tnr stats: frame_id: %d, sigma count: %d, sigma_calc[0-3]: %d, %d, %d, %d",
        pStatsTrans->mCamPhyId, stats->frame_id, bay3dStat->tnr_auto_sigma_count,
        bay3dStat->tnr_auto_sigma_calc[0], bay3dStat->tnr_auto_sigma_calc[1],
        bay3dStat->tnr_auto_sigma_calc[2], bay3dStat->tnr_auto_sigma_calc[3]);

    aiqMutex_lock(lock);
    if (aiqList_size(to) > 4) {
        AiqListItem_t* pItem    = aiqList_get_item(to, NULL);
        aiq_stats_base_t* pStat = *(aiq_stats_base_t**)pItem->_pData;
        AIQ_REF_BASE_UNREF(&pStat->_ref_base);
        aiqList_erase_item_locked(to, pItem);
    }

    if (aiqList_push(to, &pbase)) {
        LOGE_ANALYZER("push err !");
        AIQ_REF_BASE_UNREF(&pbase->_ref_base);
    }
    aiqMutex_unlock(lock);

    return ret;
}
#endif

#if defined(ISP_HW_V33)
XCamReturn AiqStatsTranslator_init(AiqStatsTranslator_t* pStatsTrans) {
    pStatsTrans->translateAecStats          = translateAecStatsV33;
    pStatsTrans->translateAwbStats          = translateAwbStatsV33;
#if RKAIQ_HAVE_DEHAZE_V14
    pStatsTrans->translateAdehazeStats      = translateAdehazeStatsV14;
#endif
    pStatsTrans->translateAgainStats        = NULL;
#if RKAIQ_HAVE_PDAF
    pStatsTrans->translatePdafStats         = AiqStatsTranslator_translatePdafStats;
#endif
#if defined(RKAIQ_HAVE_BAYERTNR_V30)
    pStatsTrans->translateBay3dStats        = translateBay3dStatsV30;
#endif
#if defined(RKAIQ_HAVE_MULTIISP)
    pStatsTrans->translateMultiAecStats     = NULL;
    pStatsTrans->translateMultiAwbStats     = NULL;
    pStatsTrans->translateMultiAfStats      = NULL;
    pStatsTrans->translateMultiAdehazeStats = NULL;
#endif
    return XCAM_RETURN_NO_ERROR;
}

void AiqStatsTranslator_deinit(AiqStatsTranslator_t* pStatsTrans) {
    return;
}

#endif
