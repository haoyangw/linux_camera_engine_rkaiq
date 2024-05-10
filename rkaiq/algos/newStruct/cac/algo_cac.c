/*
 * rk_aiq_algo_debayer_itf.c
 *
 *  Copyright (c) 2019 Rockchip Corporation
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
 *
 */

#include "algo_types_priv.h"
#include "cac_types_prvt.h"
#include "xcam_log.h"

#include "RkAiqCalibDbTypes.h"
#include "RkAiqCalibDbTypesV2.h"
#include "RkAiqCalibDbV2Helper.h"
//#include "RkAiqHandle.h"

#include "interpolation.h"

//RKAIQ_BEGIN_DECLARE

XCamReturn CacSelectParam(CacContext_t* pCacCtx, cac_param_t* out, int iso);

static void LutBufferInit(LutBuffer *buf, const LutBufferConfig* config, const rk_aiq_cac_share_mem_info_t* mem_info)
{
    buf->Config = *config;
    buf->State = (LutBufferState)(*mem_info->state);
    buf->Fd    = mem_info->fd;
    buf->Addr  = mem_info->addr;
    buf->Size  = mem_info->size;
}

static void LutBufferManagerInit(LutBufferManager *man, const LutBufferConfig* config, const isp_drv_share_mem_ops_t* mem_ops)
{
    man->mem_ops_ = mem_ops;
    man->mem_ctx_ = NULL;
    man->config_ = *config;
}

static void LutBufferManagerImportHwBuffers(LutBufferManager *man, uint8_t isp_id) {
    assert(man->mem_ops_ != NULL);
    rk_aiq_share_mem_config_t hw_config;
    hw_config.mem_type             = MEM_TYPE_CAC;
    hw_config.alloc_param.width  = man->config_.Width;
    hw_config.alloc_param.height = man->config_.Height;
    hw_config.alloc_param.reserved[0] = 1;

    man->mem_ops_->alloc_mem(isp_id, (void*)(man->mem_ops_), &hw_config, &man->mem_ctx_);
}

static void LutBufferManagerReleaseHwBuffers(LutBufferManager *man, uint8_t isp_id) {
    if (man->mem_ctx_ != NULL && man->mem_ops_ != NULL)
        man->mem_ops_->release_mem(isp_id, man->mem_ctx_);
}

static LutBuffer* LutBufferManagerGetFreeHwBuffer(LutBufferManager *man, uint8_t isp_id) {
    if (man->mem_ops_ == NULL || man->mem_ctx_ == NULL) {
        return NULL;
    }

    const rk_aiq_cac_share_mem_info_t* mem_info = (const rk_aiq_cac_share_mem_info_t*)(
        man->mem_ops_->get_free_item(isp_id, man->mem_ctx_));
    if (mem_info != NULL) {
        LutBuffer* lut_buf = aiq_mallocz(sizeof(LutBuffer));
        if (lut_buf != NULL) {
            LutBufferInit(lut_buf, &man->config_, mem_info);
            return lut_buf;
        }
    }
    return NULL;
}

static void LutBufferManagerDeinit(LutBufferManager *man)
{
    LutBufferManagerReleaseHwBuffers(man, 0);
    LutBufferManagerReleaseHwBuffers(man, 1);
}

static inline bool IsIspBigMode(uint32_t width, uint32_t height, bool is_multi_sensor) {
    if (is_multi_sensor || width > IspBigModeWidthLimit || width * height > IspBigModeSizeLimit) {
        return true;
    }

    return false;
}

#if RKAIQ_HAVE_CAC_V12
static inline void CalcCacLutConfig(uint32_t width, uint32_t height, bool is_big_mode,
                                    LutBufferConfig* config) {
    //is_big_mode is useless;
    config->Width     = width;
    config->Height    = height;
    config->IsBigMode = true;
    config->ScaleFactor = CacScaleFactor256Mode;
    /**
     * CAC only processes R & B channels, that means for R or R channels,
     * which have only half size of full picture, only need to div round up by 32(scale==64) or
     * 64(scale==128). For calculate convinient, use full picture size to calculate
     */
    config->LutHCount   =(width + 254) >> 8 ;
    config->LutVCount   = (height  + 254) >> 8;
    config->PsfCfgCount = config->LutHCount * config->LutVCount;
    XCAM_ASSERT(config->PsfCfgCount <= CacPsfCountLimit);
    /**
     * CAC stores one PSF point's kernel in 9 words, one kernel size is 8 bytes.
     * (8bytes*8bits/byte + 32 - 1) / 32bits/word = 9 words.
     */
}
#else
static inline void CalcCacLutConfig(uint32_t width, uint32_t height, bool is_big_mode,
                                    LutBufferConfig* config) {
    config->Width     = width;
    config->Height    = height;
    config->IsBigMode = is_big_mode;
    if (config->IsBigMode) {
        config->ScaleFactor = CacScaleFactorBigMode;
    } else {
        config->ScaleFactor = CacScaleFactorDefault;
    }
    /**
     * CAC only processes R & B channels, that means for R or R channels,
     * which have only half size of full picture, only need to div round up by 32(scale==64) or
     * 64(scale==128). For calculate convinient, use full picture size to calculate
     */
    config->LutHCount   = is_big_mode ? (width + 126) >> 7 : (width + 62) >> 6;
    config->LutVCount   = is_big_mode ? (height + 126) >> 7 : (height + 62) >> 6;
    config->PsfCfgCount = config->LutHCount * config->LutVCount;
    XCAM_ASSERT(config->PsfCfgCount <= CacPsfCountLimit);
    /**
     * CAC stores one PSF point's kernel in 9 words, one kernel size is 8 bytes.
     * (8bytes*8bits/byte + 32 - 1) / 32bits/word = 9 words.
     */
}
#endif

static XCamReturn
create_context(RkAiqAlgoContext** context, const AlgoCtxInstanceCfg* cfg)
{
    XCamReturn result = XCAM_RETURN_NO_ERROR;
    CamCalibDbV2Context_t *pCalibDbV2 = cfg->calibv2;

    CacContext_t *ctx = aiq_mallocz(sizeof(CacContext_t));
    if (ctx == NULL) {
        LOGE_ACAC( "%s: create Cac context fail!\n", __FUNCTION__);
        return XCAM_RETURN_ERROR_MEM;
    }

    ctx->valid_ = true;
    ctx->isReCal_ = true;
    ctx->prepare_params = NULL;
    ctx->algo_config_ = NULL;
    ctx->prepare_success = false;
    ctx->cac_attrib =
        (cac_api_attrib_t*)(CALIBDBV2_GET_MODULE_PTR(pCalibDbV2, cac));

    *context = (RkAiqAlgoContext* )ctx;
    LOGV_ACAC("%s: (exit)\n", __FUNCTION__ );
    return result;
}

static XCamReturn
destroy_context(RkAiqAlgoContext* context)
{
    XCamReturn result = XCAM_RETURN_NO_ERROR;

    CacContext_t* pCacCtx = (CacContext_t*)context;
    aiq_free(pCacCtx->algo_config_);
    aiq_free(pCacCtx);
    for (int i = 0;i < current_lut_size;i++) {
        aiq_free(current_lut_[i]);
        current_lut_[i] = NULL;
    }
    current_lut_size = 0;
    aiq_free(lut_manger_);
    lut_manger_ = NULL;
    return result;
}
#if RKAIQ_HAVE_CAC_V11
static XCamReturn prepare_helper(CacContext_t* pCacCtx, RkAiqAlgoConfigCac* config) {
    LutBufferConfig lut_config;
    LutBufferConfig full_lut_config;
    uint32_t width   = config->width;
    uint32_t height  = config->height;
    bool is_big_mode = IsIspBigMode(width, height, config->is_multi_sensor);
    char cac_map_path[RKCAC_MAX_PATH_LEN] = {0};

    LOGD_ACAC("%s : en %d valid: %d Enter", __func__, pCacCtx->cac_attrib->en, pCacCtx->valid_);

    if (!pCacCtx->cac_attrib->en || !pCacCtx->valid_) {
        return XCAM_RETURN_BYPASS;
    }

    memset(&lut_config, 0, sizeof(lut_config));
    memset(&full_lut_config, 0, sizeof(full_lut_config));
    pCacCtx->width = config->width;
    pCacCtx->multi_isp_extended_pixel = config->multi_isp_extended_pixel;
    if (config->is_multi_isp) {
        CalcCacLutConfig(width, height, is_big_mode, &full_lut_config);
        width = width / 2 + config->multi_isp_extended_pixel;
        CalcCacLutConfig(width, height, is_big_mode, &lut_config);
    } else {
        CalcCacLutConfig(width, height, is_big_mode, &lut_config);
    }
    if (lut_manger_ == NULL) {
        lut_manger_ = aiq_mallocz(sizeof(LutBufferManager));
        if (lut_manger_ == NULL) {
            LOGE_ACAC("malloc failure");
            return XCAM_RETURN_ERROR_MEM;
        }

        LutBufferManagerInit(lut_manger_, &lut_config, config->mem_ops);
        LutBufferManagerImportHwBuffers(lut_manger_, 0);
        if (config->is_multi_isp) {
            LutBufferManagerImportHwBuffers(lut_manger_, 1);
        }
    }
    LutBuffer* buf = LutBufferManagerGetFreeHwBuffer(lut_manger_, 0);
    if (buf == NULL) {
        LOGW_ACAC("No buffer available, maybe only one buffer ?!");
        return XCAM_RETURN_NO_ERROR;
    }
    current_lut_size = 0;
    current_lut_[0] = buf;
    current_lut_size++;
    if (buf->State != kInitial) {
        LOGW_ACAC("Buffer in use, will not update lut!");
        return XCAM_RETURN_NO_ERROR;
    }
    if (config->is_multi_isp) {
        LutBuffer* buf2 = LutBufferManagerGetFreeHwBuffer(lut_manger_, 1);
        if (buf2 == NULL) {
            LOGW_ACAC("No buffer available, maybe only one buffer ?!");
            return XCAM_RETURN_NO_ERROR;
        }
        current_lut_[1] = buf2;
        current_lut_size++;
    }
    XCAM_ASSERT(current_lut_size == (uint32_t)(config->is_multi_isp + 1));

    if (pCacCtx->cac_attrib->stAuto.sta.psfParam.sw_cacT_psfMap_path[0] != '/') {
        strcpy(cac_map_path, config->iqpath);
        strcat(cac_map_path, "/");
    }
    strcat(cac_map_path, pCacCtx->cac_attrib->stAuto.sta.psfParam.sw_cacT_psfMap_path);

    FILE *fp = fopen(cac_map_path, "rb");
    if (fp == NULL) {
        LOGE_ACAC("Failed to open PSF file %s", cac_map_path);
        pCacCtx->valid_ = false;
        return XCAM_RETURN_ERROR_FILE;
    }

    if (!config->is_multi_isp) {
        uint32_t line_offset = lut_config.LutHCount * CacPsfKernelWordSizeInMemory * BYTES_PER_WORD;
        uint32_t size = lut_config.LutHCount * lut_config.LutVCount * CacPsfKernelWordSizeInMemory *
                        BYTES_PER_WORD;
        for (int ch = 0; ch < CacChannelCount; ch++) {
            char* addr0 = (char*)(current_lut_[0]->Addr) + ch * size;
            fread(addr0, 1, size, fp);
        }
    } else {
        XCAM_ASSERT(current_lut_size > 1);
        // Read and Split Memory
        //   a == line_size - line_offset
        //   b == line_offset
        //   c == line_offset - a = 2 * line_offset - line_size
        // For each line:
        //   read b size to left
        //   copy c from left to right
        //   read a' to right
        // - +---------------------------+
        // | |<---a---->|  |  |<---a'--->|
        // | |                 |<-c->|          |
        // v |<---b---------->|          |
        // | |          |  |  |          |
        // - +---------------------------+
        //   |<---------line_size------->|
        //
        uint32_t line_offset = lut_config.LutHCount * CacPsfKernelWordSizeInMemory * BYTES_PER_WORD;
        uint32_t line_size =
            full_lut_config.LutHCount * CacPsfKernelWordSizeInMemory * BYTES_PER_WORD;
        for (int ch = 0; ch < CacChannelCount; ch++) {
            char* addr0 = (char*)(current_lut_[0]->Addr) +
                          ch * line_offset * lut_config.LutVCount;
            char* addr1 = (char*)(current_lut_[1]->Addr) +
                          ch * line_offset * lut_config.LutVCount;
            for (uint32_t i = 0; i < full_lut_config.LutVCount; i++) {
                fread(addr0 + (i * line_offset), 1, line_offset, fp);
                memcpy(addr1 + (i * line_offset),
                       addr0 + (i * line_offset) + line_size - line_offset,
                       2 * line_offset - line_size);
                fread(addr1 + (i * line_size) + line_offset, 1, line_size - line_offset, fp);
            }
        }
    }
    fclose(fp);
    config->LutHCount = current_lut_[0]->Config.LutHCount;
    config->LutVCount = current_lut_[0]->Config.LutVCount;
    config->PsfCfgCount = current_lut_[0]->Config.PsfCfgCount;
    config->Fd0 = current_lut_[0]->Fd;
    if (config->is_multi_isp)
        config->Fd1 = current_lut_[1]->Fd;

    pCacCtx->prepare_success = true;
    return XCAM_RETURN_NO_ERROR;
}
#endif
static XCamReturn
prepare(RkAiqAlgoCom* params)
{
    XCamReturn result = XCAM_RETURN_NO_ERROR;
    CacContext_t* pCacCtx = (CacContext_t *)params->ctx;

    if(!!(params->u.prepare.conf_type & RK_AIQ_ALGO_CONFTYPE_UPDATECALIB )){
        // just update calib ptr
        if (params->u.prepare.conf_type & RK_AIQ_ALGO_CONFTYPE_UPDATECALIB_PTR) {
            pCacCtx->cac_attrib =
                (cac_api_attrib_t*)(CALIBDBV2_GET_MODULE_PTR(params->u.prepare.calibv2, cac));
            return XCAM_RETURN_NO_ERROR;
        }
    }

    pCacCtx->cac_attrib =
        (cac_api_attrib_t*)(CALIBDBV2_GET_MODULE_PTR(params->u.prepare.calibv2, cac));
    pCacCtx->prepare_params = &params->u.prepare;
    pCacCtx->isReCal_ = true;
#if RKAIQ_HAVE_CAC_V11
    RkAiqAlgoConfigCac* CacCfgParam = (RkAiqAlgoConfigCac*)params;

    if(pCacCtx->algo_config_ == NULL)
        pCacCtx->algo_config_ = aiq_mallocz(sizeof(RkAiqAlgoConfigCac));
    memcpy(pCacCtx->algo_config_, CacCfgParam, sizeof(RkAiqAlgoConfigCac));

    if (CacCfgParam->width != 0 && CacCfgParam->height != 0)
        return prepare_helper(pCacCtx, CacCfgParam);
    else
        return result;
#else
    return result;
#endif
}

static XCamReturn
processing(const RkAiqAlgoCom* inparams, RkAiqAlgoResCom* outparams)
{
    XCamReturn result = XCAM_RETURN_NO_ERROR;

    CacContext_t* pCacCtx = (CacContext_t *)inparams->ctx;
    cac_api_attrib_t* cac_attrib = pCacCtx->cac_attrib;
    RkAiqAlgoProcResCac* pCacProcResParams = (RkAiqAlgoProcResCac*)outparams;
    RkAiqAlgoProcCac* cac_proc_param = (RkAiqAlgoProcCac*)inparams;

    LOGV_ACAC("%s: Cac (enter)\n", __FUNCTION__ );

    if (!cac_attrib) {
        LOGE_ACAC("cac_attrib is NULL !");
        return XCAM_RETURN_ERROR_MEM;
    }

    pCacCtx->hdr_ratio = cac_proc_param->hdr_ratio;
    int iso = cac_proc_param->iso;
    bool init = inparams->u.proc.init;
    int delta_iso = abs(iso - pCacCtx->iso);
    pCacCtx->iso = iso;

    outparams->cfg_update = false;

    if (cac_attrib->opMode != RK_AIQ_OP_MODE_AUTO) {
        LOGE_ACAC("mode is %d, not auto mode, ignore", cac_attrib->opMode);
        return XCAM_RETURN_NO_ERROR;
    }

    if (inparams->u.proc.is_attrib_update) {
        pCacCtx->isReCal_ = true;
    }

    if (delta_iso > DEFAULT_RECALCULATE_DELTA_ISO) {
        pCacCtx->isReCal_ = true;
    }
    if (!pCacCtx->cac_attrib->en || !pCacCtx->valid_) {
        outparams->en = false;
        if (pCacCtx->isReCal_) {
            outparams->cfg_update = true;
            pCacCtx->isReCal_ = false;
        }
        return XCAM_RETURN_NO_ERROR;
    }
#if RKAIQ_HAVE_CAC_V11
    if (pCacCtx->isReCal_ && !pCacCtx->prepare_success) {
        prepare_helper(pCacCtx, pCacCtx->algo_config_);
        if (!pCacCtx->prepare_success) {
            LOGE_ACAC("prepare_helper failed");
            return XCAM_RETURN_ERROR_PARAM;
        }
    }
#endif
    if (pCacCtx->isReCal_) {
        CacSelectParam(pCacCtx, pCacProcResParams->cacRes, iso);
        outparams->cfg_update = true;
        outparams->en = cac_attrib->en;
        outparams->bypass = cac_attrib->bypass;
        LOGD_ACAC("cac en:%d, bypass:%d", outparams->en, outparams->bypass);
    }

    pCacCtx->isReCal_ = false;

    LOGV_ACAC("%s: Cac (exit)\n", __FUNCTION__ );
    return XCAM_RETURN_NO_ERROR;
}
#if RKAIQ_HAVE_CAC_V11
XCamReturn CacSelectParam(CacContext_t* pCacCtx, cac_param_t* out, int iso)
{
    LOGD_ACAC("%s(%d): enter!\n", __FUNCTION__, __LINE__);

    if(pCacCtx == NULL) {
        LOGE_ACAC("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return XCAM_RETURN_ERROR_PARAM;
    }
    cac_param_auto_t* paut = &pCacCtx->cac_attrib->stAuto;
    int i;
    int iso_low  = 50;
    int iso_high = 50;
    int gain_high, gain_low;
    float ratio      = 1.0;
    int iso_div      = 50;
    int max_iso_step = CAC_ISO_STEP_MAX;

    for (i = max_iso_step - 1; i >= 0; i--) {
        if (iso < iso_div * (2 << i)) {
            iso_low  = iso_div * (2 << (i)) / 2;
            iso_high = iso_div * (2 << i);
        }
    }
    ratio = (float)(iso - iso_low) / (iso_high - iso_low);
    uint16_t uratio = ratio * (1 << RATIO_FIXBIT);
    if (iso_low == iso) {
        iso_high = iso;
        ratio    = 0;
    }
    if (iso_high == iso) {
        iso_low = iso;
        ratio   = 1;
    }
    gain_high = (int)(log((float)(iso_high) / 50) / log(2.0));
    gain_low  = (int)(log((float)(iso_low) / 50) / log(2.0));

    gain_low  = MIN(MAX(gain_low, 0), max_iso_step - 1);
    gain_high = MIN(MAX(gain_high, 0), max_iso_step - 1);

    XCAM_ASSERT(gain_low >= 0 && gain_low < max_iso_step);
    XCAM_ASSERT(gain_high >= 0 && gain_high < max_iso_step);

    out->sta.strgCenter[0].hw_cacT_strgCenter_en
        = paut->sta.strgCenter[0].hw_cacT_strgCenter_en;
    out->sta.strgCenter[0].hw_cacT_strgCenter_x
        = paut->sta.strgCenter[0].hw_cacT_strgCenter_x;
    out->sta.strgCenter[0].hw_cacT_strgCenter_y
        = paut->sta.strgCenter[0].hw_cacT_strgCenter_y;
    out->sta.psfParam.hw_cacT_psfShift_bits
        = paut->sta.psfParam.hw_cacT_psfShift_bits;
    out->sta.psfParam.hw_cacCfg_psfBlock_num =
        current_lut_[0]->Config.PsfCfgCount;
    out->sta.lutBuf[0].sw_cacCfg_lutBuf_fd = current_lut_[0]->Fd;
    out->sta.lutBuf[0].sw_cacCfg_lutBufSize_height =
        current_lut_[0]->Config.LutHCount * CacPsfKernelWordSizeInMemory;
    out->sta.lutBuf[0].sw_cacCfg_lutBufSize_width =
        current_lut_[0]->Config.LutVCount * CacChannelCount;

    float strength[RKCAC_STRENGTH_TABLE_LEN] = {1.0f};
    float strenth_low = 0.0;
    float strenth_high = 0.0;
    for (i = 0; i < RKCAC_STRENGTH_TABLE_LEN; i++) {
        strenth_low = paut->dyn[gain_low].strgInterp.sw_cacT_globalCorr_strg > 0
            ? paut->dyn[gain_low].strgInterp.sw_cacT_globalCorr_strg
            : paut->dyn[gain_low].strgInterp.hw_cacT_corrStrength_table[i];
        strenth_high = paut->dyn[gain_high].strgInterp.sw_cacT_globalCorr_strg > 0
            ? paut->dyn[gain_high].strgInterp.sw_cacT_globalCorr_strg
            : paut->dyn[gain_high].strgInterp.hw_cacT_corrStrength_table[i];
        out->dyn.strgInterp.hw_cacT_corrStrength_table[i] =
            interpolation_f32(strenth_low, strenth_high, ratio);
    }

    int clip_g_mode1 = paut->dyn[gain_low].chromaAberrCorr.hw_cacT_clipG_mode;
    int clip_g_mode2 = paut->dyn[gain_high].chromaAberrCorr.hw_cacT_clipG_mode;
    out->dyn.chromaAberrCorr.hw_cacT_clipG_mode =
        (cac_clipGT_mode_t) interpolation_u32(clip_g_mode1, clip_g_mode1, ratio);
    out->dyn.chromaAberrCorr.hw_cacT_negClip0_en =
        interpolation_bool(paut->dyn[gain_low].chromaAberrCorr.hw_cacT_negClip0_en,
                    paut->dyn[gain_high].chromaAberrCorr.hw_cacT_negClip0_en, uratio);
    out->dyn.strgInterp.hw_cacT_edgeDetect_en =
        interpolation_bool(paut->dyn[gain_low].strgInterp.hw_cacT_edgeDetect_en,
                    paut->dyn[gain_high].strgInterp.hw_cacT_edgeDetect_en, uratio);
    out->dyn.strgInterp.hw_cacT_flatDctB_thred =
        interpolation_f32(paut->dyn[gain_low].strgInterp.hw_cacT_flatDctB_thred,
            paut->dyn[gain_high].strgInterp.hw_cacT_flatDctB_thred, ratio);
    out->dyn.strgInterp.hw_cacT_flatDctR_thred =
        interpolation_f32(paut->dyn[gain_low].strgInterp.hw_cacT_flatDctR_thred,
            paut->dyn[gain_high].strgInterp.hw_cacT_flatDctR_thred, ratio);
    out->dyn.strgInterp.hw_cacT_flatDctB_offset =
        interpolation_u32(paut->dyn[gain_low].strgInterp.hw_cacT_flatDctB_offset,
            paut->dyn[gain_high].strgInterp.hw_cacT_flatDctB_offset, ratio);
    out->dyn.strgInterp.hw_cacT_flatDctR_offset =
        interpolation_u32(paut->dyn[gain_low].strgInterp.hw_cacT_flatDctR_offset,
            paut->dyn[gain_high].strgInterp.hw_cacT_flatDctR_offset, ratio);
    out->dyn.chromaAberrCorr.sw_cacT_expoDctB_en =
        interpolation_bool(paut->dyn[gain_low].chromaAberrCorr.sw_cacT_expoDctB_en,
            paut->dyn[gain_high].chromaAberrCorr.sw_cacT_expoDctB_en, uratio);
    out->dyn.chromaAberrCorr.sw_cacT_expoDctR_en =
        interpolation_bool(paut->dyn[gain_low].chromaAberrCorr.sw_cacT_expoDctR_en,
            paut->dyn[gain_high].chromaAberrCorr.sw_cacT_expoDctR_en, uratio);

    uint32_t hw_cacT_overExpoB_thred =
        interpolation_u32(paut->dyn[gain_low].chromaAberrCorr.hw_cacT_overExpoB_thred,
            paut->dyn[gain_high].chromaAberrCorr.hw_cacT_overExpoB_thred, ratio);
    uint32_t hw_cacT_overExpoR_thred =
        interpolation_u32(paut->dyn[gain_low].chromaAberrCorr.hw_cacT_overExpoR_thred,
            paut->dyn[gain_high].chromaAberrCorr.hw_cacT_overExpoR_thred, ratio);
    uint32_t hw_cacT_overExpoB_adj =
        interpolation_u32(paut->dyn[gain_low].chromaAberrCorr.hw_cacT_overExpoB_adj,
            paut->dyn[gain_high].chromaAberrCorr.hw_cacT_overExpoB_adj, ratio);
    uint32_t hw_cacT_overExpoR_adj =
        interpolation_u32(paut->dyn[gain_low].chromaAberrCorr.hw_cacT_overExpoR_adj,
            paut->dyn[gain_high].chromaAberrCorr.hw_cacT_overExpoR_adj, ratio);
    out->dyn.chromaAberrCorr.hw_cacT_overExpoB_thred =
        pCacCtx->hdr_ratio * hw_cacT_overExpoB_thred;
    out->dyn.chromaAberrCorr.hw_cacT_overExpoR_thred =
        pCacCtx->hdr_ratio * hw_cacT_overExpoR_thred;
    out->dyn.chromaAberrCorr.hw_cacT_overExpoB_adj =
        pCacCtx->hdr_ratio * hw_cacT_overExpoB_adj;
    out->dyn.chromaAberrCorr.hw_cacT_overExpoR_adj =
        pCacCtx->hdr_ratio * hw_cacT_overExpoR_adj;

    memcpy(&out->sta.strgCenter[1], &out->sta.strgCenter[0], sizeof(out->sta.strgCenter[0]));
    memcpy(&out->sta.lutBuf[1], &out->sta.lutBuf[0], sizeof(out->sta.lutBuf[0]));
    if (current_lut_size > 1) {
        out->sta.lutBuf[1].sw_cacCfg_lutBuf_fd = current_lut_[1]->Fd;
        if (out->sta.strgCenter[0].hw_cacT_strgCenter_en) {
            uint16_t w                     = pCacCtx->width / 4;
            uint16_t e                     = pCacCtx->multi_isp_extended_pixel / 4;
            uint16_t x                     = paut->sta.strgCenter[0].hw_cacT_strgCenter_x;
            out->sta.strgCenter[1].hw_cacT_strgCenter_x = x - (w / 2 - e);
        }
    }
    return XCAM_RETURN_NO_ERROR;
}
#endif
#if RKAIQ_HAVE_CAC_V30
XCamReturn CacSelectParam(CacContext_t *pCacCtx, cac_param_t* out, int iso)
{
      if(pCacCtx == NULL) {
        LOGE_ACAC("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return XCAM_RETURN_ERROR_PARAM;
    }
    cac_param_auto_t* paut = &pCacCtx->cac_attrib->stAuto;
    int i;
    int iso_low  = 50;
    int iso_high = 50;
    int gain_high, gain_low;
    float ratio      = 1.0;
    int iso_div      = 50;
    int max_iso_step = CAC_ISO_STEP_MAX;
    for (i = max_iso_step - 1; i >= 0; i--) {
        if (iso < iso_div * (2 << i)) {
            iso_low  = iso_div * (2 << (i)) / 2;
            iso_high = iso_div * (2 << i);
        }
    }
    ratio = (float)(iso - iso_low) / (iso_high - iso_low);
    uint16_t uratio = ratio * (1 << RATIO_FIXBIT);
    if (iso_low == iso) {
        iso_high = iso;
        ratio    = 0;
    }
    if (iso_high == iso) {
        iso_low = iso;
        ratio   = 1;
    }
    LOGD_ACAC("%s:iso=%d,iso_high=%d,iso_low=%d,ratio=%f\n", __FUNCTION__, iso,iso_high,iso_low,ratio);
    gain_high = (int)(log((float)(iso_high) / 50) / log(2.0));
    gain_low  = (int)(log((float)(iso_low) / 50) / log(2.0));

    gain_low  = MIN(MAX(gain_low, 0), max_iso_step - 1);
    gain_high = MIN(MAX(gain_high, 0), max_iso_step - 1);

    XCAM_ASSERT(gain_low >= 0 && gain_low < max_iso_step);
    XCAM_ASSERT(gain_high >= 0 && gain_high < max_iso_step);

    out->sta.hw_cacT_psfTableFix_bit
        = paut->sta.hw_cacT_psfTableFix_bit;
    out->dyn.psfTable.hw_cacT_bluePsf_sigma = interpolation_f32(paut->dyn[gain_low].psfTable.hw_cacT_bluePsf_sigma,
            paut->dyn[gain_high].psfTable.hw_cacT_bluePsf_sigma, ratio);
    out->dyn.psfTable.hw_cacT_redPsf_sigma = interpolation_f32(paut->dyn[gain_low].psfTable.hw_cacT_redPsf_sigma,
            paut->dyn[gain_high].psfTable.hw_cacT_redPsf_sigma, ratio);
    out->dyn.gInterp.hw_cacT_hiDrct_ratio = interpolation_f32(paut->dyn[gain_low].gInterp.hw_cacT_hiDrct_ratio,
            paut->dyn[gain_high].gInterp.hw_cacT_hiDrct_ratio, ratio);


    out->dyn.chromaAberrCorr.hw_cacT_chromaLoFlt_coeff0 =
        interpolation_u32(paut->dyn[gain_low].chromaAberrCorr.hw_cacT_chromaLoFlt_coeff0,
            paut->dyn[gain_high].chromaAberrCorr.hw_cacT_chromaLoFlt_coeff0, ratio);

    out->dyn.chromaAberrCorr.hw_cacT_chromaLoFlt_coeff1 =
        interpolation_u32(paut->dyn[gain_low].chromaAberrCorr.hw_cacT_chromaLoFlt_coeff1,
            paut->dyn[gain_high].chromaAberrCorr.hw_cacT_chromaLoFlt_coeff1, ratio);

    out->dyn.chromaAberrCorr.hw_cacT_searchRange_ratio =
        interpolation_f32(paut->dyn[gain_low].chromaAberrCorr.hw_cacT_searchRange_ratio,
            paut->dyn[gain_high].chromaAberrCorr.hw_cacT_searchRange_ratio, ratio);


    out->dyn.chromaAberrCorr.hw_cacT_wgtColorB_minThred =
        interpolation_f32(paut->dyn[gain_low].chromaAberrCorr.hw_cacT_wgtColorB_minThred,
            paut->dyn[gain_high].chromaAberrCorr.hw_cacT_wgtColorB_minThred, ratio);

    out->dyn.chromaAberrCorr.hw_cacT_wgtColorB_maxThred =
        interpolation_f32(paut->dyn[gain_low].chromaAberrCorr.hw_cacT_wgtColorB_maxThred,
            paut->dyn[gain_high].chromaAberrCorr.hw_cacT_wgtColorB_maxThred, ratio);

    out->dyn.chromaAberrCorr.hw_cacT_wgtColorR_minThred =
        interpolation_f32(paut->dyn[gain_low].chromaAberrCorr.hw_cacT_wgtColorR_minThred,
            paut->dyn[gain_high].chromaAberrCorr.hw_cacT_wgtColorR_minThred, ratio);

    out->dyn.chromaAberrCorr.hw_cacT_wgtColorR_maxThred =
        interpolation_f32(paut->dyn[gain_low].chromaAberrCorr.hw_cacT_wgtColorR_maxThred,
            paut->dyn[gain_high].chromaAberrCorr.hw_cacT_wgtColorR_maxThred, ratio);
    out->dyn.chromaAberrCorr.hw_cacT_wgtColor_minLuma =
        interpolation_f32(paut->dyn[gain_low].chromaAberrCorr.hw_cacT_wgtColor_minLuma,
            paut->dyn[gain_high].chromaAberrCorr.hw_cacT_wgtColor_minLuma, ratio);

    out->dyn.chromaAberrCorr.hw_cacT_wgtColor_maxLuma =
        interpolation_f32(paut->dyn[gain_low].chromaAberrCorr.hw_cacT_wgtColor_maxLuma,
            paut->dyn[gain_high].chromaAberrCorr.hw_cacT_wgtColor_maxLuma, ratio);

    out->dyn.chromaAberrCorr.hw_cacT_residualChroma_ratio =
        interpolation_f32(paut->dyn[gain_low].chromaAberrCorr.hw_cacT_residualChroma_ratio,
            paut->dyn[gain_high].chromaAberrCorr.hw_cacT_residualChroma_ratio, ratio);

    out->dyn.chromaAberrCorr.hw_cacT_wgtColorLoFlt_coeff0 =
        interpolation_u32(paut->dyn[gain_low].chromaAberrCorr.hw_cacT_wgtColorLoFlt_coeff0,
            paut->dyn[gain_high].chromaAberrCorr.hw_cacT_wgtColorLoFlt_coeff0, ratio);

    out->dyn.chromaAberrCorr.hw_cacT_wgtColorLoFlt_coeff1 =
        interpolation_u32(paut->dyn[gain_low].chromaAberrCorr.hw_cacT_wgtColorLoFlt_coeff1,
            paut->dyn[gain_high].chromaAberrCorr.hw_cacT_wgtColorLoFlt_coeff1, ratio);

    out->dyn.chromaAberrCorr.hw_cacT_wgtOverExpo_minThred =
        interpolation_f32(paut->dyn[gain_low].chromaAberrCorr.hw_cacT_wgtOverExpo_minThred,
            paut->dyn[gain_high].chromaAberrCorr.hw_cacT_wgtOverExpo_minThred, ratio);
    out->dyn.chromaAberrCorr.hw_cacT_wgtOverExpo_maxThred =
        interpolation_f32(paut->dyn[gain_low].chromaAberrCorr.hw_cacT_wgtOverExpo_maxThred,
            paut->dyn[gain_high].chromaAberrCorr.hw_cacT_wgtOverExpo_maxThred, ratio);
    out->dyn.chromaAberrCorr.hw_cacT_contrast_offset =
        interpolation_u32(paut->dyn[gain_low].chromaAberrCorr.hw_cacT_contrast_offset,
            paut->dyn[gain_high].chromaAberrCorr.hw_cacT_contrast_offset, ratio);
    out->dyn.chromaAberrCorr.hw_cacT_wgtContrast_minThred =
        interpolation_f32(paut->dyn[gain_low].chromaAberrCorr.hw_cacT_wgtContrast_minThred,
            paut->dyn[gain_high].chromaAberrCorr.hw_cacT_wgtContrast_minThred, ratio);
    out->dyn.chromaAberrCorr.hw_cacT_wgtContrast_maxThred =
        interpolation_f32(paut->dyn[gain_low].chromaAberrCorr.hw_cacT_wgtContrast_maxThred,
            paut->dyn[gain_high].chromaAberrCorr.hw_cacT_wgtContrast_maxThred, ratio);
    out->dyn.chromaAberrCorr.hw_cacT_wgtDarkArea_minThred =
        interpolation_f32(paut->dyn[gain_low].chromaAberrCorr.hw_cacT_wgtDarkArea_minThred,
            paut->dyn[gain_high].chromaAberrCorr.hw_cacT_wgtDarkArea_minThred, ratio);
    out->dyn.chromaAberrCorr.hw_cacT_wgtDarkArea_maxThred =
        interpolation_f32(paut->dyn[gain_low].chromaAberrCorr.hw_cacT_wgtDarkArea_maxThred,
            paut->dyn[gain_high].chromaAberrCorr.hw_cacT_wgtDarkArea_maxThred, ratio);
    out->dyn.chromaAberrCorr.hw_cacT_wgtColor_en =
        interpolation_bool(paut->dyn[gain_low].chromaAberrCorr.hw_cacT_wgtColor_en,
            paut->dyn[gain_high].chromaAberrCorr.hw_cacT_wgtColor_en, ratio);


     out->dyn.edgeDetect.hw_cacT_edgeDetect_en =
        interpolation_bool(paut->dyn[gain_low].edgeDetect.hw_cacT_edgeDetect_en,
                    paut->dyn[gain_high].edgeDetect.hw_cacT_edgeDetect_en, ratio);
    out->dyn.edgeDetect.hw_cacT_flat_thred =
        interpolation_f32(paut->dyn[gain_low].edgeDetect.hw_cacT_flat_thred,
            paut->dyn[gain_high].edgeDetect.hw_cacT_flat_thred, ratio);
    out->dyn.edgeDetect.hw_cacT_flat_offset =
        interpolation_u32(paut->dyn[gain_low].edgeDetect.hw_cacT_flat_offset,
            paut->dyn[gain_high].edgeDetect.hw_cacT_flat_offset, ratio);
     out->dyn.hfCalc.hw_cacT_negClip0_en =
        interpolation_bool(paut->dyn[gain_low].hfCalc.hw_cacT_negClip0_en,
                    paut->dyn[gain_high].hfCalc.hw_cacT_negClip0_en, ratio);
     out->dyn.hfCalc.hw_cacT_overExpoDetect_en =
        interpolation_bool(paut->dyn[gain_low].hfCalc.hw_cacT_overExpoDetect_en,
                    paut->dyn[gain_high].hfCalc.hw_cacT_overExpoDetect_en, ratio);
     out->dyn.hfCalc.hw_cacT_overExpo_thred =
        interpolation_f32(paut->dyn[gain_low].hfCalc.hw_cacT_overExpo_thred,
                    paut->dyn[gain_high].hfCalc.hw_cacT_overExpo_thred, ratio);
     out->dyn.hfCalc.hw_cacT_overExpo_adj =
        interpolation_f32(paut->dyn[gain_low].hfCalc.hw_cacT_overExpo_adj,
                    paut->dyn[gain_high].hfCalc.hw_cacT_overExpo_adj, ratio);
    return XCAM_RETURN_NO_ERROR;
}
#endif
#if 0
XCamReturn
algo_cac_SetAttrib(RkAiqAlgoContext* ctx, cac_api_attrib_t* attr) {
    if(ctx == NULL || attr == NULL) {
        LOGE_ACAC("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return XCAM_RETURN_ERROR_PARAM;
    }
    CacContext_t* pCacCtx = (CacContext_t*)ctx;
    cac_api_attrib_t* cac_attrib = pCacCtx->cac_attrib;

    if (attr->opMode != RK_AIQ_OP_MODE_AUTO) {
        LOGE_ACAC("not auto mode: %d", attr->opMode);
        return XCAM_RETURN_ERROR_PARAM;
    }

    cac_attrib->opMode = attr->opMode;
    cac_attrib->en = attr->en;
    cac_attrib->bypass = attr->bypass;

    if (attr->opMode == RK_AIQ_OP_MODE_AUTO)
        cac_attrib->stAuto = attr->stAuto;
    else if (attr->opMode == RK_AIQ_OP_MODE_MANUAL)
        cac_attrib->stMan = attr->stMan;
    else {
        LOGW_ACAC("wrong mode: %d\n", attr->opMode);
    }

    pCacCtx->isReCal_ = true;

    return XCAM_RETURN_NO_ERROR;
}


XCamReturn
algo_cac_GetAttrib(RkAiqAlgoContext* ctx, cac_api_attrib_t* attr)
{
    if(ctx == NULL || attr == NULL) {
        LOGE_ACAC("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return XCAM_RETURN_ERROR_PARAM;
    }

    CacContext_t* pCacCtx = (CacContext_t*)ctx;
    cac_api_attrib_t* cac_attrib = pCacCtx->cac_attrib;

#if 0
    if (cac_attrib->opMode != RK_AIQ_OP_MODE_AUTO) {
        LOGE_ACAC("not auto mode: %d", cac_attrib->opMode);
        return XCAM_RETURN_ERROR_PARAM;
    }
#endif

    attr->opMode = cac_attrib->opMode;
    attr->en = cac_attrib->en;
    attr->bypass = cac_attrib->bypass;
    memcpy(&attr->stAuto, &cac_attrib->stAuto, sizeof(cac_param_auto_t));
    return XCAM_RETURN_NO_ERROR;
}
#endif
#define RKISP_ALGO_CAC_VERSION     "v0.0.1"
#define RKISP_ALGO_CAC_VENDOR      "Rockchip"
#define RKISP_ALGO_CAC_DESCRIPTION "Rockchip cac algo for ISP2.0"

RkAiqAlgoDescription g_RkIspAlgoDescCac = {
    .common = {
        .version = RKISP_ALGO_CAC_VERSION,
        .vendor  = RKISP_ALGO_CAC_VENDOR,
        .description = RKISP_ALGO_CAC_DESCRIPTION,
        .type    = RK_AIQ_ALGO_TYPE_ACAC,
        .id      = 0,
        .create_context  = create_context,
        .destroy_context = destroy_context,
    },
    .prepare = prepare,
    .pre_process = NULL,
    .processing = processing,
    .post_process = NULL,
};

//RKAIQ_END_DECLARE
