/*
 * rk_aiq_ldch_generate_mesh.cpp
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

#include "xcam_log.h"
#include "ldch_generate_mesh.h"

#define LDCH_CUSTOM_MESH "ldch_custom_mesh.bin"

RKAIQ_BEGIN_DECLARE


static XCamReturn alloc_ldch_buf(LdchContext_t* pldchCtx)
{
    if (!pldchCtx->hasAllocShareMem.load(std::memory_order_acquire)) {
        LOGD_ALDCH("alloc ldch buf");
        release_ldch_buf(pldchCtx, 0);
        if (pldchCtx->is_multi_isp) {
            release_ldch_buf(pldchCtx, 1);
        }

        rk_aiq_share_mem_config_t share_mem_config;
        share_mem_config.mem_type = MEM_TYPE_LDCH;
        share_mem_config.alloc_param.width =  pldchCtx->dst_width;
        share_mem_config.alloc_param.height = pldchCtx->dst_height;
        share_mem_config.mem_type = MEM_TYPE_LDCH;
        share_mem_config.alloc_param.reserved[0] = 1;
        pldchCtx->share_mem_ops->alloc_mem(0, pldchCtx->share_mem_ops,
                &share_mem_config,
                &pldchCtx->share_mem_ctx);
        if (pldchCtx->is_multi_isp) {
            pldchCtx->share_mem_ops->alloc_mem(1, pldchCtx->share_mem_ops,
                    &share_mem_config,
                    &pldchCtx->share_mem_ctx);
        }

        if (pldchCtx->share_mem_ctx) {
            pldchCtx->hasAllocShareMem.store(true, std::memory_order_release);
        } else {
            LOGE_ALDCH("Failed to alloc shared mem");
            return XCAM_RETURN_ERROR_MEM;
        }
    }

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn release_ldch_buf(LdchContext_t* pldchCtx, uint8_t isp_id)
{
    if (pldchCtx->hasAllocShareMem.load(std::memory_order_acquire)) {
        LOGD_ALDCH("isp_id %d, release ldch buf", isp_id);
        if (pldchCtx->share_mem_ctx)
            pldchCtx->share_mem_ops->release_mem(isp_id, pldchCtx->share_mem_ctx);
        pldchCtx->hasAllocShareMem.store(false, std::memory_order_release);
    }

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn get_ldch_buf(LdchContext_t* pldchCtx, uint8_t isp_id)
{
    if (alloc_ldch_buf(pldchCtx) != XCAM_RETURN_NO_ERROR) {
        LOGE_ALDCH("Failed to alloc ldch buf\n");
        return XCAM_RETURN_ERROR_MEM;
    }

    pldchCtx->ldch_mem_info[isp_id] = (rk_aiq_ldch_share_mem_info_t *)
            pldchCtx->share_mem_ops->get_free_item(isp_id, pldchCtx->share_mem_ctx);
    if (pldchCtx->ldch_mem_info[isp_id] == NULL || \
        (pldchCtx->ldch_mem_info[isp_id] && \
         pldchCtx->ldch_mem_info[isp_id]->state[0] != MESH_BUF_INIT)) {
        LOGE_ALDCH("%s: isp_id %d, no free ldch buf", __FUNCTION__, isp_id);
        return XCAM_RETURN_ERROR_MEM;
    }
    pldchCtx->ldch_mem_info[isp_id]->state[0] = MESH_BUF_WAIT2CHIP; //mark that this buf is using.

    LOGD_ALDCH("isp_id %d, LDCH get buf: fd %d, addr %p, size %d",
            isp_id,
            pldchCtx->ldch_mem_info[isp_id]->fd,
            pldchCtx->ldch_mem_info[isp_id]->addr,
            pldchCtx->ldch_mem_info[isp_id]->size);

    return XCAM_RETURN_NO_ERROR;
}

void put_ldch_buf(LdchContext_t* pldchCtx, uint8_t isp_id)
{
    if (pldchCtx->ldch_mem_info[isp_id] && pldchCtx->ldch_mem_info[isp_id]->state[0] == MESH_BUF_WAIT2CHIP) {
        pldchCtx->ldch_mem_info[isp_id]->state[0] = MESH_BUF_INIT;
        LOGD_ALDCH("isp_id %d, LDCH put buf: fd %d, addr %p, size %d",
                isp_id,
                pldchCtx->ldch_mem_info[isp_id]->fd,
                pldchCtx->ldch_mem_info[isp_id]->addr,
                pldchCtx->ldch_mem_info[isp_id]->size);

        pldchCtx->ldch_mem_info[isp_id] = NULL;
    }
}

bool
read_mesh_from_file(LdchContext_t* pldchCtx, const char* fileName)
{
    FILE* ofp;
    ofp = fopen(fileName, "rb");
    if (ofp != NULL) {
        unsigned short hpic, vpic, hsize, vsize, hstep, vstep = 0;
        uint32_t lut_size = 0;

        fread(&hpic, sizeof(unsigned short), 1, ofp);
        fread(&vpic, sizeof(unsigned short), 1, ofp);
        fread(&hsize, sizeof(unsigned short), 1, ofp);
        fread(&vsize, sizeof(unsigned short), 1, ofp);
        fread(&hstep, sizeof(unsigned short), 1, ofp);
        fread(&vstep, sizeof(unsigned short), 1, ofp);

        lut_size = hsize * vsize *  sizeof(uint16_t);
        LOGW_ALDCH("lut info: [%d-%d-%d-%d-%d-%d]", hpic, vpic, hsize, vsize, hstep, vstep);
        unsigned int num = fread(pldchCtx->ldch_mem_info[0]->addr, 1, lut_size, ofp);
        fclose(ofp);

        if (num != lut_size) {
            LOGE_ALDCH("mismatched lut calib file");
            return false;
        }

        if (pldchCtx->src_width != hpic || pldchCtx->src_height != vpic) {
            LOGE_ALDCH("mismatched lut pic resolution: src %dx%d, lut %dx%d",
                    pldchCtx->src_width, pldchCtx->src_height, hpic, vpic);
            return false;
        }

        pldchCtx->lut_h_size = hsize;
        pldchCtx->lut_v_size = vsize;
        pldchCtx->lut_mapxy_size = lut_size;
        pldchCtx->lut_h_size = hsize / 2; //word unit

        LOGW_ALDCH("check file, size: %d, num: %d", pldchCtx->lut_mapxy_size, num);
    } else {
        LOGE_ALDCH("lut file %s not exist", fileName);
        return false;
    }

    return true;
}


#if GENMESH_ONLINE
XCamReturn aiqGenLdchMeshInit(LdchContext_t* pldchCtx)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    if (pldchCtx->genLdchMeshInit) {
        LOG1_ALDCH("genLDCHMesh has been initialized!!\n");
        return XCAM_RETURN_BYPASS;
    }
#ifdef RKAIQ_HAVE_LDCH_V21
    pldchCtx->ldchParams.enLdchVersion = RKALGO_LDCH_VERSION_1;
#else
    pldchCtx->ldchParams.enLdchVersion = RKALGO_LDCH_VERSION_0;
#endif

    pldchCtx->ldchParams.saveMeshX = false;
    if (pldchCtx->ldchParams.saveMeshX) {
        sprintf(pldchCtx->ldchParams.meshPath, "/tmp/");
    }

	if (pldchCtx->is_multi_isp) {
        pldchCtx->camCoeff.cx = (pldchCtx->src_width - 1) * 0.5;
        pldchCtx->camCoeff.cy = (pldchCtx->src_height - 1) * 0.5;

        genLdchMeshInit8kTo4k(pldchCtx->src_width, pldchCtx->src_height,
                              pldchCtx->dst_width, pldchCtx->dst_height,
                              pldchCtx->multi_isp_extended_pixel,
                              pldchCtx->camCoeff, pldchCtx->camCoeff_left, pldchCtx->camCoeff_right,
                              pldchCtx->ldchParams, pldchCtx->ldchParams_left, pldchCtx->ldchParams_right);

        LOGD_ALDCH("left len light center(%.16f, %.16f), maxLevel: %d\n",
                    pldchCtx->camCoeff_left.cx, pldchCtx->camCoeff_left.cy,
                    pldchCtx->ldchParams_left.maxLevel);
        LOGD_ALDCH("right len light center(%.16f, %.16f), maxLevel: %d\n",
                    pldchCtx->camCoeff_right.cx, pldchCtx->camCoeff_right.cy,
                    pldchCtx->ldchParams_right.maxLevel);
	} else {
		genLdchMeshInit(pldchCtx->src_width, pldchCtx->src_height,
				pldchCtx->dst_width, pldchCtx->dst_height,
				pldchCtx->ldchParams, pldchCtx->camCoeff);

	}

    pldchCtx->lut_h_size = (pldchCtx->ldchParams.meshSizeW + 1) / 2; //word unit
    pldchCtx->lut_v_size = pldchCtx->ldchParams.meshSizeH;
    pldchCtx->lut_mapxy_size = pldchCtx->ldchParams.meshSize * sizeof(unsigned short);

    LOGI_ALDCH("ldch en %d, h/v size(%dx%d), mapxy size(%d), correct_level: %d, genMeshLib ver: %d",
               pldchCtx->ldch_en,
               pldchCtx->lut_h_size,
               pldchCtx->lut_v_size,
               pldchCtx->lut_mapxy_size,
               pldchCtx->correct_level,
               pldchCtx->ldchParams.enLdchVersion);

    pldchCtx->genLdchMeshInit = true;

    return ret;
}

XCamReturn aiqGenLdchMeshDeInit(LdchContext_t* pldchCtx)
{
    if (!pldchCtx->genLdchMeshInit) {
        return XCAM_RETURN_NO_ERROR;
    }

	genLdchMeshDeInit(pldchCtx->ldchParams);

    pldchCtx->genLdchMeshInit = false;

    return XCAM_RETURN_NO_ERROR;
}

bool aiqGenMesh(LdchContext_t* pldchCtx, int32_t level, uint8_t isp_id)
{
    bool success = false;

    LOGD_ALDCH("isp_id %d, LDCH gen mesh level: %d", isp_id, level);

    if (level == 0) {
        char filename[512];
        sprintf(filename, "%s/%s",
                pldchCtx->resource_path,
                LDCH_CUSTOM_MESH);
        success = read_mesh_from_file(pldchCtx, filename);
        if (success) {
            LOGW_ALDCH("read mesh from %s", filename);
        }
    }

    if (!success) {
        if (pldchCtx->is_multi_isp) {
            LdchParams* ldchParams = nullptr;
            struct CameraCoeff* camCoeff = nullptr;
            uint16_t* mesh_buf = nullptr;

            if (!isp_id) {
                ldchParams = &pldchCtx->ldchParams_left;
                camCoeff = &pldchCtx->camCoeff_left;
            } else {
                ldchParams = &pldchCtx->ldchParams_right;
                camCoeff = &pldchCtx->camCoeff_right;
            }

            if (level > ldchParams->maxLevel) {
                LOGW_ALDCH("Out max level: %d, change level %d to %d",
                           ldchParams->maxLevel, level, level - 50);
                level -= 50;
            }

            if (pldchCtx->ldch_mem_info[isp_id]) {
                mesh_buf = (uint16_t *)pldchCtx->ldch_mem_info[isp_id]->addr;
                success = genLDCMeshNLevel(*ldchParams, *camCoeff, level, mesh_buf);
            }
        } else {
            if (pldchCtx->ldch_mem_info[0]) {
                if (level > pldchCtx->ldchParams.maxLevel) {
                    LOGW_ALDCH("Out max level, change level %d to %d",
                               level, pldchCtx->ldchParams.maxLevel);
                    level = pldchCtx->ldchParams.maxLevel;
                }

                success = genLDCMeshNLevel(pldchCtx->ldchParams, pldchCtx->camCoeff,
                        level, (uint16_t *)pldchCtx->ldch_mem_info[0]->addr);
            }
        }
    }

    return success;
}
#endif

RKAIQ_END_DECLARE
