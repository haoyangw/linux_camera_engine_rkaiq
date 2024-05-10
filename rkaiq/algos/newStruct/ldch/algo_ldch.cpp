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

#include "rk_aiq_algo_types.h"
#include "ldch_types_prvt.h"
#include "xcam_log.h"

#include "RkAiqCalibDbTypes.h"
#include "RkAiqCalibDbTypesV2.h"
#include "RkAiqCalibDbV2Helper.h"
#include "RkAiqHandle.h"

#include "interpolation.h"
#include "ldch_generate_mesh.h"

//RKAIQ_BEGIN_DECLARE
static const uint8_t default_bic_table[9][4] = {
    {0x00, 0x80, 0x00, 0x00}, // table0: 0, 0, 128, 0
    {0xfc, 0x7f, 0x05, 0x00}, // table1: 0, 5, 127, -4
    {0xfa, 0x7b, 0x0c, 0xff}, // table2: -1, 12, 123, -6
    {0xf8, 0x76, 0x14, 0xfe}, // table3: -2, 20, 118, -8
    {0xf7, 0x6f, 0x1d, 0xfd}, // table4: -3, 29, 111, -9
    {0xf7, 0x66, 0x27, 0xfc}, // table4: -4, 39, 102, -9
    {0xf7, 0x5d, 0x32, 0xfa}, // table4: -6, 50, 93, -9
    {0xf7, 0x53, 0x3d, 0xf9}, // table4: -7, 61, 83, -9
    {0xf8, 0x48, 0x48, 0xf8}, // table4: -8, 72, 72, -8
};

static XCamReturn
updateCalibConfig(RkAiqAlgoCom* params)
{
    LdchContext_t* pLdchCtx = (LdchContext_t*)params->ctx;
    RkAiqAlgoConfigAldch* rkaiqAldchConfig = (RkAiqAlgoConfigAldch*)params;

    
    ldch_api_attrib_t* calib_ldch =
        (ldch_api_attrib_t*)(CALIBDBV2_GET_MODULE_PTR(params->u.prepare.calibv2, ldch));
    pLdchCtx->ldch_attrib = calib_ldch;

    pLdchCtx->camCoeff.cx = calib_ldch->stAuto.sta.baseCtrl.sw_ldchT_lightCenter_val[0];
    pLdchCtx->camCoeff.cy = calib_ldch->stAuto.sta.baseCtrl.sw_ldchT_lightCenter_val[1];
    pLdchCtx->camCoeff.a0 = calib_ldch->stAuto.sta.baseCtrl.sw_ldchT_lensDistor_coeff[0];
    pLdchCtx->camCoeff.a2 = calib_ldch->stAuto.sta.baseCtrl.sw_ldchT_lensDistor_coeff[1];
    pLdchCtx->camCoeff.a3 = calib_ldch->stAuto.sta.baseCtrl.sw_ldchT_lensDistor_coeff[2];
    pLdchCtx->camCoeff.a4 = calib_ldch->stAuto.sta.baseCtrl.sw_ldchT_lensDistor_coeff[3];
    memcpy(pLdchCtx->meshfile, calib_ldch->stAuto.sta.baseCtrl.sw_ldchT_meshfile_path, sizeof(pLdchCtx->meshfile));

    if (calib_ldch->en) {
        if ((!pLdchCtx->ldch_en || calib_ldch->stAuto.sta.baseCtrl.sw_ldchT_correct_strg != pLdchCtx->correct_level) && \
            pLdchCtx->ldch_attrib->stAuto.sta.sw_ldchCfg_updateLut_mode == ldch_online_mode) {
            if (aiqGenLdchMeshInit(pLdchCtx) < 0) {
                LOGE_ALDCH("Failed to init gen mesh");
                return XCAM_RETURN_ERROR_FAILED;
            }

            for (uint8_t i = 0; i < pLdchCtx->multi_isp_number; i++) {
                if (get_ldch_buf(pLdchCtx, i) != XCAM_RETURN_NO_ERROR) {
                    LOGE_ALDCH("Failed to get ldch buf\n");
                    return XCAM_RETURN_ERROR_FAILED;
                }

                bool success = aiqGenMesh(pLdchCtx, calib_ldch->stAuto.sta.baseCtrl.sw_ldchT_correct_strg, i);
                if (!success) {
                    LOGW_ALDCH("lut is not exist");
                    put_ldch_buf(pLdchCtx, i);
                    return XCAM_RETURN_ERROR_FAILED;
                }

                if (pLdchCtx->ldch_mem_info[i])
                    pLdchCtx->ready_lut_mem_fd[i] = pLdchCtx->ldch_mem_info[i]->fd;
            }

            pLdchCtx->isLutUpdated.store(true, std::memory_order_release);
        }
    }
    

    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn update_custom_lut_from_file(LdchContext_t* pLdchCtx)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    if (get_ldch_buf(pLdchCtx, 0) != XCAM_RETURN_NO_ERROR) {
        LOGE_ALDCH("Failed to get ldch buf\n");
        ret = XCAM_RETURN_ERROR_MEM;
    }
    else {
        char filename[200] = {0};
        sprintf(filename, "%s/%s",
                pLdchCtx->ldch_attrib->stAuto.sta.customLutCfg.sw_ldchCfg_lutExtFile_dir,
                pLdchCtx->ldch_attrib->stAuto.sta.customLutCfg.sw_ldchCfg_lutExtMeshFile_name);

        LOGD_ALDCH("read lut file name: %s/%s\n",
                pLdchCtx->ldch_attrib->stAuto.sta.customLutCfg.sw_ldchCfg_lutExtFile_dir,
                pLdchCtx->ldch_attrib->stAuto.sta.customLutCfg.sw_ldchCfg_lutExtMeshFile_name);

        bool ret1 = read_mesh_from_file(pLdchCtx, filename);
        if (!ret1) {
            LOGE_ALDCH("Failed to read mesh, disable ldch!");
            pLdchCtx->ldch_en = pLdchCtx->ldch_attrib->en = false;
            put_ldch_buf(pLdchCtx, 0);
            ret = XCAM_RETURN_ERROR_FILE;
        } else {
            uint16_t *addr = (uint16_t *)pLdchCtx->ldch_mem_info[0]->addr;
            LOGD_ALDCH("lut[0:15]: %d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d",
                    addr[0], addr[1], addr[2], addr[3],
                    addr[4], addr[5], addr[6], addr[7],
                    addr[8], addr[9], addr[10], addr[11],
                    addr[12], addr[13], addr[14], addr[15]);
        }

        if (pLdchCtx->ldch_mem_info[0]) {
            pLdchCtx->ready_lut_mem_fd[0] = pLdchCtx->ldch_mem_info[0]->fd;
            pLdchCtx->update_lut_mem_fd[0] = pLdchCtx->ready_lut_mem_fd[0];
        }

        LOGD_ALDCH("update custom lut from external file, lut_mem_fd %d\n", pLdchCtx->update_lut_mem_fd[0]);
    }

    return ret;
}

static XCamReturn update_custom_lut_from_external_buffer(LdchContext_t* pLdchCtx)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    if (!pLdchCtx->_lutCache->GetBuffer()) {
        LOGE_ALDCH("Failed to get ldch lut cache\n");
        ret = XCAM_RETURN_ERROR_MEM;
    }

    if (get_ldch_buf(pLdchCtx, 0) != XCAM_RETURN_NO_ERROR) {
        LOGE_ALDCH("Failed to get ldch buf\n");
        ret = XCAM_RETURN_ERROR_MEM;
    } else {
        uint16_t hpic, vpic, hsize, vsize, hstep, vstep;
        uint32_t lut_size = 0;
        uint16_t *addr = (uint16_t *)pLdchCtx->_lutCache->GetBuffer();

        hpic    = *addr++;
        vpic    = *addr++;
        hsize   = *addr++;
        vsize   = *addr++;
        hstep   = *addr++;
        vstep   = *addr++;

        lut_size = hsize * vsize *  sizeof(uint16_t);
        LOGD_ALDCH("lut info: [%d-%d-%d-%d-%d-%d]", hpic, vpic, hsize, vsize, hstep, vstep);

        if (pLdchCtx->src_width != hpic || pLdchCtx->src_height != vpic || \
            lut_size > (uint32_t)pLdchCtx->ldch_mem_info[0]->size) {
            LOGE_ALDCH("mismatched lut pic resolution: src %dx%d, lut %dx%d, disable ldch",
                    pLdchCtx->src_width, pLdchCtx->src_height, hpic, vpic);
            LOGE_ALDCH("Invalid lut buffer size %zu, ldch drv bufer size is %u, disable ldch",
                       lut_size, pLdchCtx->ldch_mem_info[0]->size);
            pLdchCtx->ldch_en = pLdchCtx->ldch_attrib->en = false;
            put_ldch_buf(pLdchCtx, 0);
            ret = XCAM_RETURN_ERROR_PARAM;
        } else {
            pLdchCtx->lut_h_size = hsize;
            pLdchCtx->lut_v_size = vsize;
            pLdchCtx->lut_mapxy_size = lut_size;
            pLdchCtx->lut_h_size = hsize / 2; //word unit

            memcpy(pLdchCtx->ldch_mem_info[0]->addr, addr, pLdchCtx->lut_mapxy_size);
            LOGD_ALDCH("lut[0:15]: %d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d",
                    addr[0], addr[1], addr[2], addr[3],
                    addr[4], addr[5], addr[6], addr[7],
                    addr[8], addr[9], addr[10], addr[11],
                    addr[12], addr[13], addr[14], addr[15]);
            pLdchCtx->_lutCache.release();
            pLdchCtx->_lutCache = nullptr;
        }

        if (pLdchCtx->ldch_mem_info[0]) {
            pLdchCtx->ready_lut_mem_fd[0] = pLdchCtx->ldch_mem_info[0]->fd;
            pLdchCtx->update_lut_mem_fd[0] = pLdchCtx->ready_lut_mem_fd[0];
        }

        LOGD_ALDCH("update custom lut from external buffer, lut_mem_fd %d\n", pLdchCtx->update_lut_mem_fd[0]);
    }

    return ret;
}

static XCamReturn update_uapi_attribute(LdchContext_t* pLdchCtx)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    if (pLdchCtx->ldch_attrib->stAuto.sta.sw_ldchCfg_updateLut_mode == ldch_externalFile_mode) {
        if (pLdchCtx->ldch_attrib->en) {
            if (pLdchCtx->ldch_attrib->stAuto.sta.customLutCfg.sw_ldchCfg_lutExtUpdate_en) {
                if (update_custom_lut_from_file(pLdchCtx) < 0) {
                    LOGE_ALDCH("Failed update custom lut\n");
                    return XCAM_RETURN_ERROR_FAILED;
                } else {
                    pLdchCtx->ldch_attrib->stAuto.sta.customLutCfg.sw_ldchCfg_lutExtUpdate_en = false;
                }
            }
        } else {
            pLdchCtx->ldch_en = false;
            LOGD_ALDCH("disable ldch by user api\n");
        }

    } else if (pLdchCtx->ldch_attrib->stAuto.sta.sw_ldchCfg_updateLut_mode == ldch_online_mode) {
        if (pLdchCtx->ldch_attrib->en) {
            SmartPtr<ldch_api_attrib_t> attrPtr = new ldch_api_attrib_t;
            if (attrPtr.ptr()) {
                memcpy(attrPtr.ptr(), pLdchCtx->ldch_attrib, sizeof(ldch_api_attrib_t));
                pLdchCtx->ldchReadMeshThread->push_attr(attrPtr);
            } else {
                LOGE_ALDCH("Failed to new ldch attr, don't update attrib\n");
            }
        } else {
            pLdchCtx->ldch_en = false;
        }
    } else if (pLdchCtx->ldch_attrib->stAuto.sta.sw_ldchCfg_updateLut_mode == ldch_externalBuf_mode) {
        if (pLdchCtx->ldch_attrib->en) {
            if (pLdchCtx->ldch_attrib->stAuto.sta.customLutCfg.sw_ldchCfg_lutExtUpdate_en) {
                if (update_custom_lut_from_external_buffer(pLdchCtx) < 0) {
                    LOGE_ALDCH("Failed update custom lut from external buffer\n");
                } else {
                    pLdchCtx->ldch_attrib->stAuto.sta.customLutCfg.sw_ldchCfg_lutExtUpdate_en = false;
                }
            }
        } else {
            pLdchCtx->ldch_en = false;
            LOGD_ALDCH("disable ldch by user api\n");
        }
    } else {
        LOGE_ALDCH("unknow updating lut mode %d\n", pLdchCtx->ldch_attrib->stAuto.sta.sw_ldchCfg_updateLut_mode);
        ret = XCAM_RETURN_ERROR_PARAM;
    }

    // update user params after lut is generated by RKAiqLdchThread in online mode
    if (ret ==  XCAM_RETURN_NO_ERROR && \
        pLdchCtx->ldch_attrib->stAuto.sta.sw_ldchCfg_updateLut_mode != ldch_online_mode) {
        pLdchCtx->ldch_en          = pLdchCtx->ldch_attrib->en;
        pLdchCtx->correct_level    = pLdchCtx->ldch_attrib->stAuto.sta.baseCtrl.sw_ldchT_correct_strg;
        pLdchCtx->zero_interp_en   = pLdchCtx->ldch_attrib->stAuto.sta.userCfg.hw_ldchCfg_zeroInterp_en;
        pLdchCtx->sample_avr_en    = pLdchCtx->ldch_attrib->stAuto.sta.userCfg.hw_ldchCfg_sampleAvr_en;
        pLdchCtx->bic_mode_en      = pLdchCtx->ldch_attrib->stAuto.sta.userCfg.hw_ldchCfg_bicMode_en;
        memcpy(pLdchCtx->bicubic, pLdchCtx->ldch_attrib->stAuto.sta.userCfg.hw_ldchCfg_bicWeight_table, sizeof(pLdchCtx->bicubic));
    }

    return ret;
}

static XCamReturn LdchSelectParam(LdchContext_t* pLdchCtx, ldch_param_t* out);

static XCamReturn
create_context
(
    RkAiqAlgoContext** context,
    const AlgoCtxInstanceCfg* cfg
)
{
    XCamReturn result = XCAM_RETURN_NO_ERROR;
    CamCalibDbV2Context_t *pCalibDbV2 = cfg->calibv2;

    LdchContext_t *ctx = new LdchContext_t();
    if (ctx == NULL) {
        LOGE_ALDCH( "%s: create Ldch context fail!\n", __FUNCTION__);
        return XCAM_RETURN_ERROR_MEM;
    }
    /* setup config */
    memset((void*)(ctx), 0, sizeof(LdchContext_t));

    ctx->ldchReadMeshThread = new RKAiqLdchThread(ctx);
    ctx->isLutUpdated.store(false, std::memory_order_release);

    ldch_api_attrib_t* calib_ldch = (ldch_api_attrib_t*)(CALIBDBV2_GET_MODULE_PTR(pCalibDbV2, ldch));
    ctx->ldch_en = calib_ldch->en;
    memcpy(ctx->meshfile, calib_ldch->stAuto.sta.baseCtrl.sw_ldchT_meshfile_path, sizeof(ctx->meshfile));

    ctx->camCoeff.cx = calib_ldch->stAuto.sta.baseCtrl.sw_ldchT_lightCenter_val[0];
    ctx->camCoeff.cy = calib_ldch->stAuto.sta.baseCtrl.sw_ldchT_lightCenter_val[1];
    ctx->camCoeff.a0 = calib_ldch->stAuto.sta.baseCtrl.sw_ldchT_lensDistor_coeff[0];
    ctx->camCoeff.a2 = calib_ldch->stAuto.sta.baseCtrl.sw_ldchT_lensDistor_coeff[1];
    ctx->camCoeff.a3 = calib_ldch->stAuto.sta.baseCtrl.sw_ldchT_lensDistor_coeff[2];
    ctx->camCoeff.a4 = calib_ldch->stAuto.sta.baseCtrl.sw_ldchT_lensDistor_coeff[3];
    LOGD_ALDCH("(%s) len light center(%.16f, %.16f)\n",
            __FUNCTION__,
            ctx->camCoeff.cx, ctx->camCoeff.cy);
    LOGD_ALDCH("(%s) len coefficient(%.16f, %.16f, %.16f, %.16f)\n",
            __FUNCTION__,
            ctx->camCoeff.a0, ctx->camCoeff.a2,
            ctx->camCoeff.a3, ctx->camCoeff.a4);

    ctx->correct_level = calib_ldch->stAuto.sta.baseCtrl.sw_ldchT_correct_strg;
    ctx->correct_level_max = calib_ldch->stAuto.sta.baseCtrl.sw_ldchT_correctStrg_max;
    
    ctx->isReCal_ = true;
    ctx->prepare_params = NULL;
    ctx->ldch_attrib = calib_ldch;
    memcpy(ctx->ldch_attrib->stAuto.sta.userCfg.hw_ldchCfg_bicWeight_table, default_bic_table, sizeof(default_bic_table));

    ctx->frm_end_dis    = 0;
    ctx->zero_interp_en = 0;
    ctx->sample_avr_en  = 0;
    ctx->bic_mode_en    = 1;
    ctx->force_map_en   = 0;
    ctx->map13p3_en     = 0;
    memcpy(ctx->bicubic, default_bic_table, sizeof(default_bic_table));

    for (int i = 0; i < 2; i++) {
        ctx->update_lut_mem_fd[i] = -1;
        ctx->ready_lut_mem_fd[i] = -1;
    }

    ctx->hasAllocShareMem.store(false, std::memory_order_release);
    ctx->_lutCache = nullptr;
    ctx->is_multi_isp = false;
    ctx->multi_isp_extended_pixel = 0;
    ctx->multi_isp_number = 1;

    *context = (RkAiqAlgoContext* )ctx;
    LOGV_ALDCH("%s: (exit)\n", __FUNCTION__ );
    return result;
}

static XCamReturn
destroy_context
(
    RkAiqAlgoContext* context
)
{
    XCamReturn result = XCAM_RETURN_NO_ERROR;

    LdchContext_t* pLdchCtx = (LdchContext_t*)context;

    if (pLdchCtx->ldchReadMeshThread->is_running()) {
        pLdchCtx->ldchReadMeshThread->triger_stop();
        pLdchCtx->ldchReadMeshThread->stop();
    }

    if (!pLdchCtx->ldchReadMeshThread->is_empty()) {
        pLdchCtx->ldchReadMeshThread->clear_attr();
    }

    if (pLdchCtx->ldch_attrib->stAuto.sta.sw_ldchCfg_updateLut_mode == ldch_online_mode) {
        genLdchMeshDeInit(pLdchCtx->ldchParams);
    }

    release_ldch_buf(pLdchCtx, 0);
    if (pLdchCtx->is_multi_isp)
        release_ldch_buf(pLdchCtx, 1);
    
    delete pLdchCtx;
    return result;
}

static XCamReturn
prepare
(
    RkAiqAlgoCom* params
)
{
    XCamReturn result = XCAM_RETURN_NO_ERROR;
    LdchContext_t* pLdchCtx = (LdchContext_t*)params->ctx;
    RkAiqAlgoConfigLdch* rkaiqLdchConfig = (RkAiqAlgoConfigLdch*)params;
    pLdchCtx->src_width = params->u.prepare.sns_op_width;
    pLdchCtx->src_height = params->u.prepare.sns_op_height;
    pLdchCtx->dst_width = params->u.prepare.sns_op_width;
    pLdchCtx->dst_height = params->u.prepare.sns_op_height;
    pLdchCtx->resource_path = rkaiqLdchConfig->resource_path;
    pLdchCtx->share_mem_ops = rkaiqLdchConfig->mem_ops_ptr;
    pLdchCtx->is_multi_isp = rkaiqLdchConfig->is_multi_isp;
    pLdchCtx->multi_isp_extended_pixel = rkaiqLdchConfig->multi_isp_extended_pixel;
    if (pLdchCtx->is_multi_isp)
        pLdchCtx->multi_isp_number = 2;
    else
        pLdchCtx->multi_isp_number = 1;

    LOGD_ALDCH("update_lut_mode %d\n", pLdchCtx->ldch_attrib->stAuto.sta.sw_ldchCfg_updateLut_mode);
    LOGD_ALDCH("is_multi_isp %d, multi_isp_extended_pixel %d\n", pLdchCtx->is_multi_isp,
               pLdchCtx->multi_isp_extended_pixel);

	if(!!(params->u.prepare.conf_type & RK_AIQ_ALGO_CONFTYPE_UPDATECALIB )){

        if(updateCalibConfig(params) != XCAM_RETURN_NO_ERROR) {
            LOGW_ALDCH("Failed to update calib config");
        }
        return XCAM_RETURN_NO_ERROR;
    }

    pLdchCtx->ldch_attrib =
        (ldch_api_attrib_t*)(CALIBDBV2_GET_MODULE_PTR(params->u.prepare.calibv2, ldch));
    pLdchCtx->prepare_params = &params->u.prepare;
    pLdchCtx->isReCal_ = true;

    // 2.process the new attrib set before prepare
    if (pLdchCtx->ldchReadMeshThread->is_running()) {
        pLdchCtx->ldchReadMeshThread->triger_stop();
        pLdchCtx->ldchReadMeshThread->stop();
    }

    if (!pLdchCtx->ldchReadMeshThread->is_empty()) {
        pLdchCtx->ldchReadMeshThread->clear_attr();
    }

    // discard the lut generated ReadMeshThread in before aiq prepare
    if (pLdchCtx->isLutUpdated.load(std::memory_order_acquire)) {
        put_ldch_buf(pLdchCtx, 0);
        pLdchCtx->isLutUpdated.store(false, std::memory_order_release);
    }

    // 3.update uapi attribute
    pLdchCtx->ldch_en            = pLdchCtx->ldch_attrib->en;
    pLdchCtx->correct_level      = pLdchCtx->ldch_attrib->stAuto.sta.baseCtrl.sw_ldchT_correct_strg;
    pLdchCtx->zero_interp_en     = pLdchCtx->ldch_attrib->stAuto.sta.userCfg.hw_ldchCfg_zeroInterp_en;
    pLdchCtx->sample_avr_en      = pLdchCtx->ldch_attrib->stAuto.sta.userCfg.hw_ldchCfg_sampleAvr_en;
    pLdchCtx->bic_mode_en        = pLdchCtx->ldch_attrib->stAuto.sta.userCfg.hw_ldchCfg_bicMode_en;
    memcpy(pLdchCtx->bicubic, pLdchCtx->ldch_attrib->stAuto.sta.userCfg.hw_ldchCfg_bicWeight_table, sizeof(pLdchCtx->bicubic));

    // 4.update ldch result
    if (pLdchCtx->ldch_en) {
        if (pLdchCtx->ldch_attrib->stAuto.sta.sw_ldchCfg_updateLut_mode == ldch_externalFile_mode) {
            if (pLdchCtx->ldch_attrib->stAuto.sta.customLutCfg.sw_ldchCfg_lutExtUpdate_en) {
                if (update_custom_lut_from_file(pLdchCtx) < 0) {
                    LOGE_ALDCH("Failed update custom lut from file\n");
                    pLdchCtx->ldch_en = false;
                } else {
                    pLdchCtx->ldch_attrib->stAuto.sta.customLutCfg.sw_ldchCfg_lutExtUpdate_en = false;
                }
            }
        } else if (pLdchCtx->ldch_attrib->stAuto.sta.sw_ldchCfg_updateLut_mode == ldch_online_mode) {
            if (aiqGenLdchMeshInit(pLdchCtx) >= 0) {
                for (uint8_t i = 0; i < pLdchCtx->multi_isp_number; i++) {
                    if (get_ldch_buf(pLdchCtx, i) != XCAM_RETURN_NO_ERROR) {
                        LOGE_ALDCH("Failed to get mesh buf, disable LDCH\n");
                        pLdchCtx->ldch_en = pLdchCtx->ldch_attrib->en = false;
                    }

                    bool success = aiqGenMesh(pLdchCtx, pLdchCtx->correct_level, i);
                    if (!success) {
                        LOGE_ALDCH("Failed to gen mesh, disable LDCH\n");
                        put_ldch_buf(pLdchCtx, i);
                        pLdchCtx->ldch_en = pLdchCtx->ldch_attrib->en = false;
                    }

                    if (pLdchCtx->ldch_mem_info[i])
                        pLdchCtx->ready_lut_mem_fd[i] = pLdchCtx->ldch_mem_info[i]->fd;
                }
            }
        } else if (pLdchCtx->ldch_attrib->stAuto.sta.sw_ldchCfg_updateLut_mode == ldch_externalBuf_mode) {
            if (pLdchCtx->ldch_attrib->stAuto.sta.customLutCfg.sw_ldchCfg_lutExtUpdate_en) {
                if (update_custom_lut_from_external_buffer(pLdchCtx) < 0) {
                    LOGE_ALDCH("Failed update custom lut from external buffer\n");
                    pLdchCtx->ldch_en = false;
                } else {
                    pLdchCtx->ldch_attrib->stAuto.sta.customLutCfg.sw_ldchCfg_lutExtUpdate_en = false;
                }
            }
        } else {
            LOGE_ALDCH("unknow updating lut mode %d\n", pLdchCtx->ldch_attrib->stAuto.sta.sw_ldchCfg_updateLut_mode);
        }
    }

    if (pLdchCtx->ldch_attrib->stAuto.sta.sw_ldchCfg_updateLut_mode == ldch_online_mode) {
        pLdchCtx->ldchReadMeshThread->triger_start();
        pLdchCtx->ldchReadMeshThread->start();
    }

    return result;
}

static XCamReturn
processing
(
    const RkAiqAlgoCom* inparams,
    RkAiqAlgoResCom* outparams
)
{
    XCamReturn result = XCAM_RETURN_NO_ERROR;

    LdchContext_t* pLdchCtx = (LdchContext_t *)inparams->ctx;
    ldch_api_attrib_t* ldch_attrib = pLdchCtx->ldch_attrib;
    RkAiqAlgoProcResLdch* pLdchProcResParams = (RkAiqAlgoProcResLdch*)outparams;

    LOGV_ALDCH("%s: (enter)\n", __FUNCTION__ );

    if (!ldch_attrib) {
        LOGE_ALDCH("ldch_attrib is NULL !");
        return XCAM_RETURN_ERROR_MEM;
    }

    bool init = inparams->u.proc.init;
    outparams->cfg_update = false;

    bool update_params = false;
    // 1.initial state processing or updating uapi attrib after lut is generated by RKAiqAldchThread
    if (inparams->u.proc.init) {
        for (uint8_t i = 0; i < pLdchCtx->multi_isp_number; i++) {
            if (pLdchCtx->ready_lut_mem_fd[i] >= 0)
                pLdchCtx->update_lut_mem_fd[i] = pLdchCtx->ready_lut_mem_fd[i];
            LOGD_ALDCH("isp_id: %d, update update_lut_mem_fd %d\n",
                        i, pLdchCtx->update_lut_mem_fd[i]);
        }

        update_params = true;
    } else if (pLdchCtx->isLutUpdated.load(std::memory_order_acquire)) {
        // update user params after lut is generated by RKAiqAldchThread
        pLdchCtx->ldch_en = pLdchCtx->ldch_attrib->en;
        pLdchCtx->correct_level    = pLdchCtx->ldch_attrib->stAuto.sta.baseCtrl.sw_ldchT_correct_strg;
        pLdchCtx->zero_interp_en   = pLdchCtx->ldch_attrib->stAuto.sta.userCfg.hw_ldchCfg_zeroInterp_en;
        pLdchCtx->sample_avr_en    = pLdchCtx->ldch_attrib->stAuto.sta.userCfg.hw_ldchCfg_sampleAvr_en;
        pLdchCtx->bic_mode_en      = pLdchCtx->ldch_attrib->stAuto.sta.userCfg.hw_ldchCfg_bicMode_en;
        memcpy(pLdchCtx->bicubic, pLdchCtx->ldch_attrib->stAuto.sta.userCfg.hw_ldchCfg_bicWeight_table, sizeof(pLdchCtx->bicubic));

        for (uint8_t i = 0; i < pLdchCtx->multi_isp_number; i++) {
            if (pLdchCtx->ready_lut_mem_fd[i] >= 0)
                pLdchCtx->update_lut_mem_fd[i] = pLdchCtx->ready_lut_mem_fd[i];
            LOGD_ALDCH("isp_id: %d, update update_lut_mem_fd %d\n", i, pLdchCtx->update_lut_mem_fd[i]);
        }

        update_params = true;
        pLdchCtx->isReCal_ = true;
        pLdchCtx->isLutUpdated.store(false, std::memory_order_release);
    }


    if (ldch_attrib->opMode != RK_AIQ_OP_MODE_AUTO) {
        LOGE_ALDCH("mode is %d, not auto mode, ignore", ldch_attrib->opMode);
        return XCAM_RETURN_NO_ERROR;
    }

    if (inparams->u.proc.is_attrib_update) {
        pLdchCtx->isReCal_ = true;
    }

    if (pLdchCtx->isReCal_) {
        // 2.update uapi attribute
        if (update_uapi_attribute(pLdchCtx) < 0) {
            LOGE_ALDCH("Failed to update uapi attribute %d", pLdchCtx->update_lut_mem_fd[0]);
        }

        // update params asynchronously after lut is generated by RKAiqAldchThread
        // in online mode
        auto update_lut_mode = pLdchCtx->ldch_attrib->stAuto.sta.sw_ldchCfg_updateLut_mode;
        if (update_lut_mode != ldch_online_mode ||
            (update_lut_mode == ldch_online_mode && !pLdchCtx->ldch_en)) {
            update_params = true;
        }

        ldch_param_t* out = pLdchProcResParams->ldchRes;
        // 3.update ldch result
        if (pLdchCtx->ldch_en) {
            for (uint8_t i = 0; i < pLdchCtx->multi_isp_number; i++) {
                if (pLdchCtx->update_lut_mem_fd[i] < 0) {
                    LOGE_ALDCH("isp_id: %d, invalid mesh buf!", i);
                    if (inparams->u.proc.init) {
                        LOGE_ALDCH("mesh buf is invalid, disable LDCH!");
                        pLdchCtx->ldch_en = pLdchCtx->ldch_attrib->en = false;
                        outparams->cfg_update = true;
                    }
                    else {
                        outparams->cfg_update = false;
                    }
                    return XCAM_RETURN_NO_ERROR;
                }

                out->sta.lutMapCfg.sw_ldchT_lutMapBuf_fd[i] = pLdchCtx->update_lut_mem_fd[i];
            }

            LdchSelectParam(pLdchCtx, out);
        }
        outparams->cfg_update = update_params;
        outparams->en = ldch_attrib->en;
        outparams->bypass = ldch_attrib->bypass;
        LOGD_ALDCH("ldch en:%d, bypass:%d, cfg_update:%d", outparams->en, outparams->bypass, outparams->cfg_update);
    }

    pLdchCtx->isReCal_ = false;

    LOGV_ALDCH("%s: (exit)\n", __FUNCTION__ );
    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn LdchSelectParam
(
    LdchContext_t *pLdchCtx,
    ldch_param_t* out)
{
    LOGI_ALDCH("%s(%d): enter!\n", __FUNCTION__, __LINE__);

    if(pLdchCtx == NULL) {
        LOGE_ALDCH("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return XCAM_RETURN_ERROR_PARAM;
    }
    ldch_param_auto_t* paut = &pLdchCtx->ldch_attrib->stAuto;

    out->sta.lutMapCfg.sw_ldchT_lutMap_height = pLdchCtx->lut_h_size;
    out->sta.lutMapCfg.sw_ldchT_lutMap_width = pLdchCtx->lut_v_size;
    out->sta.lutMapCfg.sw_ldchT_lutMap_size = pLdchCtx->lut_mapxy_size;

    out->sta.userCfg.hw_ldchCfg_frmEndDis_mode = pLdchCtx->frm_end_dis;
    out->sta.userCfg.hw_ldchCfg_zeroInterp_en = pLdchCtx->zero_interp_en;
    out->sta.userCfg.hw_ldchCfg_sampleAvr_en = pLdchCtx->sample_avr_en;
    out->sta.userCfg.hw_ldchCfg_bicMode_en = pLdchCtx->bic_mode_en;
    out->sta.userCfg.hw_ldchCfg_forceMap_en = pLdchCtx->force_map_en;
    out->sta.userCfg.hw_ldchCfg_mapFix3Bit_en = pLdchCtx->map13p3_en;
    memcpy(out->sta.userCfg.hw_ldchCfg_bicWeight_table, pLdchCtx->bicubic, sizeof(pLdchCtx->bicubic));


    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
algo_ldch_SetAttrib
(
    RkAiqAlgoContext* ctx,
    ldch_api_attrib_t *attr
) {
    if(ctx == NULL || attr == NULL) {
        LOGE_ALDCH("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return XCAM_RETURN_ERROR_PARAM;
    }

    LdchContext_t* pLdchCtx = (LdchContext_t*)ctx;
    ldch_api_attrib_t* ldch_attrib = pLdchCtx->ldch_attrib;

    if (attr->opMode != RK_AIQ_OP_MODE_AUTO) {
        LOGE_ALDCH("not auto mode: %d", attr->opMode);
        return XCAM_RETURN_ERROR_PARAM;
    }

    ldch_attrib->opMode = attr->opMode;
    ldch_attrib->en = attr->en;
    ldch_attrib->bypass = attr->bypass;

    if (attr->opMode == RK_AIQ_OP_MODE_AUTO)
        ldch_attrib->stAuto = attr->stAuto;
    else{
        LOGW_ALDCH("wrong mode: %d\n", attr->opMode);
    }

    pLdchCtx->isReCal_ = true;

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
algo_ldch_GetAttrib
(
    RkAiqAlgoContext* ctx,
    ldch_api_attrib_t* attr
)
{
    if(ctx == NULL || attr == NULL) {
        LOGE_ALDCH("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return XCAM_RETURN_ERROR_PARAM;
    }

    LdchContext_t* pLdchCtx = (LdchContext_t*)ctx;
    ldch_api_attrib_t* ldch_attrib = pLdchCtx->ldch_attrib;

#if 0
    if (ldch_attrib->opMode != RK_AIQ_OP_MODE_AUTO) {
        LOGE_ALDCH("not auto mode: %d", ldch_attrib->opMode);
        return XCAM_RETURN_ERROR_PARAM;
    }
#endif

    attr->opMode = ldch_attrib->opMode;
    attr->en = ldch_attrib->en;
    attr->bypass = ldch_attrib->bypass;
    memcpy(&attr->stAuto, &ldch_attrib->stAuto, sizeof(ldch_param_auto_t));

    return XCAM_RETURN_NO_ERROR;
}

#define RKISP_ALGO_ALDCH_VERSION     "v0.0.1"
#define RKISP_ALGO_ALDCH_VENDOR      "Rockchip"
#define RKISP_ALGO_ALDCH_DESCRIPTION "Rockchip ldch algo for ISP2.0"

RkAiqAlgoDescription g_RkIspAlgoDescLdch = {
    .common = {
        .version = RKISP_ALGO_ALDCH_VERSION,
        .vendor  = RKISP_ALGO_ALDCH_VENDOR,
        .description = RKISP_ALGO_ALDCH_DESCRIPTION,
        .type    = RK_AIQ_ALGO_TYPE_ALDCH,
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
bool RKAiqLdchThread::loop()
{
    ENTER_ANALYZER_FUNCTION();

    const static int32_t timeout = -1;
    SmartPtr<ldch_api_attrib_t> attrib = mAttrQueue.pop (timeout);

    if (!attrib.ptr()) {
        LOGW_ALDCH("RKAiqLdchThread got empty attrib, stop thread");
        return false;
    }
    if (!hLDCH->ldch_en || attrib->stAuto.sta.baseCtrl.sw_ldchT_correct_strg != hLDCH->correct_level) {
        if (aiqGenLdchMeshInit(hLDCH) < 0) {
            LOGE_ALDCH("Failed to init gen mesh");
            return true;
        }

        for (uint8_t i = 0; i < hLDCH->multi_isp_number; i++) {
            if (get_ldch_buf(hLDCH, i) != XCAM_RETURN_NO_ERROR) {
                LOGE_ALDCH("Failed to get ldch buf\n");
                return true;
            }

            bool success = aiqGenMesh(hLDCH, attrib->stAuto.sta.baseCtrl.sw_ldchT_correct_strg, i);
            if (!success) {
                LOGE_ALDCH("Failed to gen mesh\n");
                put_ldch_buf(hLDCH, i);
                return true;
            }

            if (hLDCH->ldch_mem_info[i])
                hLDCH->ready_lut_mem_fd[i] = hLDCH->ldch_mem_info[i]->fd;

            LOGD_ALDCH("ldch gen mesh: level %d, ready lut fd %d : %d\n",
                       attrib->stAuto.sta.baseCtrl.sw_ldchT_correct_strg, hLDCH->ready_lut_mem_fd[i]);
        }

        hLDCH->isLutUpdated.store(true, std::memory_order_release);
    }

    EXIT_ANALYZER_FUNCTION();

    return true;
}