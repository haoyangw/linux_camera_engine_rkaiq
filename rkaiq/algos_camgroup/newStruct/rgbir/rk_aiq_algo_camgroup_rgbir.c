/*
 * rk_aiq_algo_camgroup_rgbir.c
 *
 *  Copyright (c) 2024 Rockchip Corporation
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

#include "rk_aiq_algo_camgroup_types.h"
#include "newStruct/rgbir/rgbir_types_prvt.h"
#include "c_base/aiq_base.h"
#include "newStruct/rk_aiq_algo_camgroup_common.h"

#define DEFAULT_RECALCULATE_DELTA_ISO (0.01)

typedef RgbirContext_t RgbirGroupContext_t;

static XCamReturn groupRgbirProcessing(const RkAiqAlgoCom* inparams, RkAiqAlgoResCom* outparams)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    LOGD_ARGBIR("%s enter", __FUNCTION__ );

    RgbirGroupContext_t* pRgbirGroupCtx = (RgbirGroupContext_t *)inparams->ctx;
    RkAiqAlgoCamGroupProcIn* procParaGroup = (RkAiqAlgoCamGroupProcIn*)inparams;
    RkAiqAlgoCamGroupProcOut* procResParaGroup = (RkAiqAlgoCamGroupProcOut*)outparams;

    //group empty
    if(procParaGroup == NULL || procParaGroup->camgroupParmasArray == NULL) {
        LOGE_ARGBIR("procParaGroup or camgroupParmasArray is null");
        return(XCAM_RETURN_ERROR_FAILED);
    }

    // skip group algo if in MANUAL mode
    if (pRgbirGroupCtx->rgbir_attrib->opMode != RK_AIQ_OP_MODE_AUTO) {
        return XCAM_RETURN_NO_ERROR;
    }

    int iso = pRgbirGroupCtx->iso;
    float blc_ob_predgain = procParaGroup->stAblcV32_proc_res.isp_ob_predgain;
    rk_aiq_singlecam_3a_result_t* scam_3a_res = procParaGroup->camgroupParmasArray[0];
    if(scam_3a_res->aec._bEffAecExpValid) {
        RKAiqAecExpInfo_t* pCurExp = &scam_3a_res->aec._effAecExpInfo;
        if((rk_aiq_working_mode_t)procParaGroup->working_mode == RK_AIQ_WORKING_MODE_NORMAL) {
            iso =  blc_ob_predgain * scam_3a_res->hdrIso;
        } else {
            iso = scam_3a_res->hdrIso;
        }
    }

    if (procParaGroup->attribUpdated) {
        LOGI("%s attribUpdated", __func__);
        pRgbirGroupCtx->isReCal_ = true;
    }

    int delta_iso = abs(iso - pRgbirGroupCtx->iso);
    if (delta_iso > DEFAULT_RECALCULATE_DELTA_ISO)
        pRgbirGroupCtx->isReCal_ = true;

    rgbir_param_t *rgbir_param = procResParaGroup->camgroupParmasArray[0]->rgbir;

    if (pRgbirGroupCtx->isReCal_) {
        RgbirSelectParam(&pRgbirGroupCtx->rgbir_attrib->stAuto, rgbir_param, iso);
		outparams->cfg_update = true;
        outparams->en         = pRgbirGroupCtx->rgbir_attrib->en;
        outparams->bypass     = pRgbirGroupCtx->rgbir_attrib->bypass;
    } else {
		outparams->cfg_update = false;
	}

	void* gp_ptrs[procResParaGroup->arraySize];
	int gp_size = sizeof(*procResParaGroup->camgroupParmasArray[0]->rgbir);
	for (int i = 0; i < procResParaGroup->arraySize; i++)
		gp_ptrs[i] = procResParaGroup->camgroupParmasArray[i]->rgbir;

	algo_camgroup_update_results(inparams, outparams, gp_ptrs, gp_size);

    pRgbirGroupCtx->iso = iso;
    pRgbirGroupCtx->isReCal_ = false;

    LOGD_ARGBIR("%s exit\n", __FUNCTION__);
    return ret;
}

#define RKISP_ALGO_CAMGROUP_RGBIR_VERSION     "v0.0.1"
#define RKISP_ALGO_CAMGROUP_RGBIR_VENDOR      "Rockchip"
#define RKISP_ALGO_CAMGROUP_RGBIR_DESCRIPTION "Rockchip Rgbir camgroup algo for ISP2.0"

RkAiqAlgoDescription g_RkIspAlgoDescCamgroupRgbir = {
    .common = {
        .version = RKISP_ALGO_CAMGROUP_RGBIR_VERSION,
        .vendor  = RKISP_ALGO_CAMGROUP_RGBIR_VENDOR,
        .description = RKISP_ALGO_CAMGROUP_RGBIR_DESCRIPTION,
        .type    = RK_AIQ_ALGO_TYPE_ARGBIR,
        .id      = 0,
        .create_context  = algo_camgroup_CreateCtx,
        .destroy_context = algo_camgroup_DestroyCtx,
    },
    .prepare = algo_camgroup_Prepare,
    .pre_process = NULL,
    .processing = groupRgbirProcessing,
    .post_process = NULL,
};
