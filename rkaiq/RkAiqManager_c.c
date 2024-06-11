/*
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

#include "RkAiqManager_c.h"
#include "hwi_c/aiq_fake_camhw.h"
#include "hwi_c/isp39/aiq_CamHwIsp39.h"
#include "hwi_c/isp33/aiq_CamHwIsp33.h"

#define RKAIQMNG_CHECK_RET(ret, format, ...) \
    if (ret) { \
        LOGE(format, ##__VA_ARGS__); \
        return ret; \
    }

extern sensor_info_share_t g_rk1608_share_inf;

static XCamReturn hwResCb(void* pCtx, AiqHwEvt_t* hwres)
{
    ENTER_XCORE_FUNCTION();
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
	AiqManager_t* pAiqManager = (AiqManager_t*)pCtx;
    if (hwres->type == ISP_POLL_3A_STATS) {
		AiqHwStatsEvt_t* pStatsEvt = (AiqHwStatsEvt_t*)hwres;
        uint32_t seq = hwres->frame_id;
#if defined(ISP_HW_V21)
        struct rkisp_isp21_stat_buffer* stats =
            (struct rkisp_isp21_stat_buffer*)(AiqV4l2Buffer_getExpbufUsrptr((AiqV4l2Buffer_t*)hwres->vb));
#elif defined(ISP_HW_V30)
        struct rkisp3x_isp_stat_buffer* stats =
            (struct rkisp3x_isp_stat_buffer*)(AiqV4l2Buffer_getExpbufUsrptr((AiqV4l2Buffer_t*)hwres->vb));
#elif defined(ISP_HW_V32)
        struct rkisp32_isp_stat_buffer* stats =
            (struct rkisp32_isp_stat_buffer*)(AiqV4l2Buffer_getExpbufUsrptr((AiqV4l2Buffer_t*)hwres->vb));
#elif defined(ISP_HW_V32_LITE)
        struct rkisp32_lite_stat_buffer* stats =
            (struct rkisp32_lite_stat_buffer*)(AiqV4l2Buffer_getExpbufUsrptr((AiqV4l2Buffer_t*)hwres->vb));
#elif defined(ISP_HW_V39)
        struct rkisp39_stat_buffer* stats =
            (struct rkisp39_stat_buffer*)(AiqV4l2Buffer_getExpbufUsrptr((AiqV4l2Buffer_t*)hwres->vb));
#elif defined(ISP_HW_V33)
        struct rkisp33_stat_buffer* stats =
            (struct rkisp33_stat_buffer*)(AiqV4l2Buffer_getExpbufUsrptr((AiqV4l2Buffer_t*)hwres->vb));
#else
#error "wrong isp hw version !"
        void * stats = NULL;
#endif
        if (stats == NULL) {
            LOGE("fail to get stats ,ignore\n");
            return XCAM_RETURN_BYPASS;
        }

        if ((stats->meas_type & ISP32_STAT_RTT_FST) && (seq != pAiqManager->mLastAweekId)) {
            AiqCore_awakenClean(pAiqManager->mRkAiqAnalyzer, seq);
			//TODO
            //ret = AiqCamHw_setFastAeExp(pAiqManager->mmCamHw, seq);
            pAiqManager->mLastAweekId = seq;

            // push sof msg
            struct timespec tp;
            clock_gettime(CLOCK_MONOTONIC_RAW, &tp);

			Aiqisp20Evt_t hw_evt;
			AiqCamHw_make_ispHwEvt(pAiqManager->mCamHw,
					&hw_evt, seq, V4L2_EVENT_FRAME_SYNC,
					tp.tv_sec * 1000 * 1000 * 1000 + tp.tv_nsec);
			AiqCore_pushEvts(pAiqManager->mRkAiqAnalyzer, (AiqHwEvt_t*)&hw_evt);

            LOGI_ANALYZER("stats meas is special, buf frame id %d", seq);
        } else if (seq == pAiqManager->mLastAweekId) {
            return ret;
        } else if (pAiqManager->mTbInfo.is_fastboot && !pAiqManager->mTBStatsCnt && seq) {
            pAiqManager->mTBStatsCnt++;
        }

        ret = AiqCore_pushStats(pAiqManager->mRkAiqAnalyzer, hwres);

    } else if (hwres->type == ISP_POLL_PARAMS) {
        rk_aiq_err_msg_t msg;
        msg.err_code = XCAM_RETURN_BYPASS;
        if (pAiqManager->mTbInfo.is_fastboot && !pAiqManager->mTBStatsCnt) {
            if (pAiqManager->mErrCb) {
                (*pAiqManager->mErrCb)(&msg);
            }
            pAiqManager->mTBStatsCnt++;
        }

        if (pAiqManager->mHwEvtCb) {
            rk_aiq_hwevt_t hwevt;

            memset(&hwevt, 0, sizeof(hwevt));
            hwevt.cam_id = AiqCamHw_getCamPhyId(pAiqManager->mCamHw);
            hwevt.aiq_status = RK_AIQ_STATUS_PREAIQ_DONE;
            hwevt.ctx = pAiqManager->mHwEvtCbCtx;

            (*pAiqManager->mHwEvtCb)(&hwevt);
        }

        if (GlobalParamsManager_isFullManualMode(&pAiqManager->mGlobalParamsManager))
            AiqManager_applyAnalyzerResult(pAiqManager,
					GlobalParamsManager_getFullManParamsProxy(&pAiqManager->mGlobalParamsManager),
					false);
    } else if (hwres->type == ISPP_POLL_NR_STATS) {
        ret = AiqCore_pushStats(pAiqManager->mRkAiqAnalyzer, hwres);
    } else if (hwres->type == ISP_POLL_SOF) {
        if (pAiqManager->mTbInfo.is_fastboot && !pAiqManager->mTBStatsCnt) {
            return ret;
        }
		AiqCamHw_notify_sof(pAiqManager->mCamHw, hwres);

		Aiqisp20Evt_t hw_evt;
		AiqCamHw_make_ispHwEvt(pAiqManager->mCamHw,
				&hw_evt, hwres->frame_id, V4L2_EVENT_FRAME_SYNC,
				hwres->mTimestamp);
		AiqCore_pushEvts(pAiqManager->mRkAiqAnalyzer, (AiqHwEvt_t*)&hw_evt);

        if (hwres->frame_id % 100 == 0)
            xcam_get_runtime_log_level();

        // TODO: moved to aiq core ?
        if (pAiqManager->mMetasCb) {
            rk_aiq_metas_t metas;
            memset(&metas, 0, sizeof(metas));
            metas.frame_id = hwres->frame_id ;
            metas.cam_id = AiqCamHw_getCamPhyId(pAiqManager->mCamHw);
            metas.sensor_name = pAiqManager->mSnsEntName;
            (*pAiqManager->mMetasCb)(&metas);
        }
    }
    else if (hwres->type == ISP_POLL_AIISP) {
        AiqHwAiispEvt_t* aiisp_data = (AiqHwAiispEvt_t *)hwres;
        if (pAiqManager->mAiispCtx.mAiispEvtcb) {
            rk_aiq_aiisp_t aiisp_evt;
            memset(&aiisp_evt, 0, sizeof(aiisp_evt));
            aiisp_evt.wr_linecnt = pAiqManager->mCamHw->mAiisp_cfg.wr_linecnt;
            aiisp_evt.rd_linecnt = pAiqManager->mCamHw->mAiisp_cfg.rd_linecnt;
            aiisp_evt.height = aiisp_data->_height;
            aiisp_evt.sequence = aiisp_data->_base.frame_id;
            aiisp_evt.bay3dbuf = aiisp_data->bay3dbuf;
            aiisp_evt.iir_address = aiisp_data->iir_address;
            aiisp_evt.gain_address = aiisp_data->gain_address;
            aiisp_evt.aiisp_address = aiisp_data->aiisp_address;
            LOGD_ANALYZER("aiisp params: wr_linecnt %d rd_linecnt %d _height %d _frameid %d bay3dbuf.iir_fd  %d bay3dbuf.iir_size %d",
                          aiisp_evt.wr_linecnt, aiisp_evt.rd_linecnt, aiisp_evt.height, aiisp_evt.sequence, aiisp_evt.bay3dbuf.iir_fd,
                          aiisp_evt.bay3dbuf.iir_size);
            LOGD_ANALYZER("bay3dbuf.aiisp_fd  %d bay3dbuf.aiisp_size %d", aiisp_evt.bay3dbuf.u.v39.aiisp_fd, aiisp_evt.bay3dbuf.u.v39.aiisp_size);
            (*pAiqManager->mAiispCtx.mAiispEvtcb)(&aiisp_evt, pAiqManager->mAiispCtx.ctx);
        }
        else {
            LOGE_ANALYZER("mAiispEvtcb is NULL");
        }
    } else if (hwres->type == ISP_POLL_TX) {
        ret = AiqCore_pushStats(pAiqManager->mRkAiqAnalyzer, hwres);
    } else if (hwres->type == ISP_POLL_SP) {
        LOGD_ANALYZER("ISP_IMG");
        ret = AiqCore_pushStats(pAiqManager->mRkAiqAnalyzer, hwres);
    } else if (hwres->type == ISP_NR_IMG) {
        ret = AiqCore_pushStats(pAiqManager->mRkAiqAnalyzer, hwres);
    } else if (hwres->type == ISP_GAIN) {
        LOGD_ANALYZER("ISP_GAIN");
        ret = AiqCore_pushStats(pAiqManager->mRkAiqAnalyzer, hwres);
#if RKAIQ_HAVE_PDAF
    } else if (hwres->type == ISP_POLL_PDAF_STATS) {
        LOGD_ANALYZER("ISP_POLL_PDAF_STATS");
        ret = AiqCore_pushStats(pAiqManager->mRkAiqAnalyzer, hwres);
#endif
    } else if (hwres->type == VICAP_STREAM_ON_EVT) {
        LOGD_ANALYZER("VICAP_STREAM_ON_EVT ... ");
        if (pAiqManager->mHwEvtCb) {
            rk_aiq_hwevt_t hwevt;
            memset(&hwevt, 0, sizeof(hwevt));
            hwevt.cam_id = AiqCamHw_getCamPhyId(pAiqManager->mCamHw);
#ifdef RKAIQ_ENABLE_CAMGROUP
            if (pAiqManager->mCamGroupCoreManager) {
                AiqCamGroupManager_setVicapReady(pAiqManager->mCamGroupCoreManager, &hwevt);
                if (AiqCamGroupManager_isAllVicapReady(pAiqManager->mCamGroupCoreManager))
                    hwevt.aiq_status = RK_AIQ_STATUS_VICAP_READY;
                else
                    hwevt.aiq_status = 0;
            } else
                hwevt.aiq_status = RK_AIQ_STATUS_VICAP_READY;
#else
            hwevt.aiq_status = RK_AIQ_STATUS_VICAP_READY;
#endif
            hwevt.ctx = pAiqManager->mHwEvtCbCtx;
            (*pAiqManager->mHwEvtCb)(&hwevt);
        }
    } else if (hwres->type == VICAP_RESET_EVT) {
        LOGD_ANALYZER(" VICAP_RESET_EVT... ");
        if (pAiqManager->mHwEvtCb) {
            rk_aiq_hwevt_t hwevt;

            memset(&hwevt, 0, sizeof(hwevt));
            hwevt.cam_id = AiqCamHw_getCamPhyId(pAiqManager->mCamHw);
            hwevt.aiq_status = RK_AIQ_STATUS_VICAP_RESET;
            hwevt.ctx = pAiqManager->mHwEvtCbCtx;

            LOGE_ANALYZER("cam: %d, VICAP_RESET_EVT...", hwevt.cam_id);
            (*pAiqManager->mHwEvtCb)(&hwevt);
        }
    } else if (hwres->type == VICAP_WITH_RK1608_RESET_EVT) {
        LOGD_ANALYZER(" VICAP_WITH_RK1608_RESET_EVT... ");
        if (pAiqManager->mHwEvtCb && pAiqManager->mCamHw) {
            rk_aiq_hwevt_t hwevt;

            memset(&hwevt, 0, sizeof(hwevt));
            for(int id = 0; id < 8; id++)
                hwevt.multi_cam.multi_cam_id[id] = -1;

            int i = 0;
            for(int camPhyId = 0; camPhyId < CAM_INDEX_FOR_1608; camPhyId++) {
                if (g_rk1608_share_inf.raw_proc_unit[camPhyId]) {
                    hwevt.multi_cam.multi_cam_id[i++] = camPhyId;
                }
            }
            hwevt.multi_cam.cam_count = i;
            hwevt.cam_id = -1;
            hwevt.aiq_status = RK_AIQ_STATUS_VICAP_WITH_MULTI_CAM_RESET;
            hwevt.ctx = pAiqManager->mHwEvtCbCtx;

            for (i = 0; i < 8; i++) {
                LOGV_ANALYZER("multi_cam_id[%d]: %d \n", i, hwevt.multi_cam.multi_cam_id[i]);
            }

            (*pAiqManager->mHwEvtCb)(&hwevt);
        }
    } else if (hwres->type == VICAP_POLL_SCL) {
        ret = AiqCore_pushStats(pAiqManager->mRkAiqAnalyzer, hwres);
    }

exit:
    EXIT_XCORE_FUNCTION();

    return ret;
}

XCamReturn AiqManager_applyAnalyzerResult(AiqManager_t* pAiqManager, AiqFullParams_t* results, bool ignoreIsUpdate)
{
    ENTER_XCORE_FUNCTION();
    //xcam_get_runtime_log_level();
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    AiqFullParams_t* aiqParams = results;

    if (!results) {
        LOGW_ANALYZER("empty aiq params results!");
        return ret;
    }

    if (aiqParams->_base.frame_id == pAiqManager->mLastAweekId)
        ignoreIsUpdate = true;

    if (aiqParams->pParamsArray[RESULT_TYPE_EXPOSURE_PARAM]) {
		aiqList_push(pAiqManager->mParamsList, &aiqParams->pParamsArray[RESULT_TYPE_EXPOSURE_PARAM]);
    }

    if (aiqParams->pParamsArray[RESULT_TYPE_IRIS_PARAM]) {
		aiqList_push(pAiqManager->mParamsList, &aiqParams->pParamsArray[RESULT_TYPE_IRIS_PARAM]);
    }

    if (aiqParams->pParamsArray[RESULT_TYPE_FOCUS_PARAM] &&
		(ignoreIsUpdate || aiqParams->pParamsArray[RESULT_TYPE_FOCUS_PARAM]->is_update)) {
        aiqParams->pParamsArray[RESULT_TYPE_FOCUS_PARAM]->is_update = false;
		aiqList_push(pAiqManager->mParamsList, &aiqParams->pParamsArray[RESULT_TYPE_FOCUS_PARAM]);
    }

#define APPLY_ANALYZER_RESULT(lc, BC) \
    { \
        if (GlobalParamsManager_isFullManualMode(&pAiqManager->mGlobalParamsManager)) { \
            aiq_params_base_t* manual_params = \
                GlobalParamsManager_getAndClearPending2(&pAiqManager->mGlobalParamsManager, RESULT_TYPE_##BC##_PARAM); \
            if (manual_params) { \
                LOGD("new manual result type: %s", #BC);\
                manual_params->frame_id = aiqParams->_base.frame_id; \
				aiqList_push(pAiqManager->mParamsList, &manual_params); \
            } \
        } else if (aiqParams->pParamsArray[RESULT_TYPE_##BC##_PARAM] && \
				(ignoreIsUpdate || aiqParams->pParamsArray[RESULT_TYPE_##BC##_PARAM]->is_update)) { \
            aiqParams->pParamsArray[RESULT_TYPE_##BC##_PARAM]->is_update = false; \
			aiqList_push(pAiqManager->mParamsList, &aiqParams->pParamsArray[RESULT_TYPE_##BC##_PARAM]); \
        } \
    } \

#if RKAIQ_HAVE_ASD
    APPLY_ANALYZER_RESULT(Cpsl, CPSL);
#endif

#if RKAIQ_HAVE_AE
#if USE_NEWSTRUCT
    APPLY_ANALYZER_RESULT(AESTATS, AESTATS);
#else
    APPLY_ANALYZER_RESULT(Aec, AEC);
    APPLY_ANALYZER_RESULT(Hist, HIST);
#endif
#endif
#if RKAIQ_HAVE_AWB
    APPLY_ANALYZER_RESULT(AwbGain, AWBGAIN);
    APPLY_ANALYZER_RESULT(Awb, AWB);// call AWBGAIN  before AWB

#endif

#if RKAIQ_HAVE_AF
    APPLY_ANALYZER_RESULT(Af, AF);
#endif
#if RKAIQ_HAVE_DPCC
    APPLY_ANALYZER_RESULT(Dpcc, DPCC);
#endif
#if RKAIQ_HAVE_MERGE
    APPLY_ANALYZER_RESULT(Merge, MERGE);
#endif
#if RKAIQ_HAVE_TMO
    APPLY_ANALYZER_RESULT(Tmo, TMO);
#endif
#if RKAIQ_HAVE_CCM
    APPLY_ANALYZER_RESULT(Ccm, CCM);
#endif
#if RKAIQ_HAVE_BLC
    APPLY_ANALYZER_RESULT(Blc, BLC);
#endif
#if RKAIQ_HAVE_ANR
    APPLY_ANALYZER_RESULT(Rawnr, RAWNR);
#endif
#if RKAIQ_HAVE_GIC
    APPLY_ANALYZER_RESULT(Gic, GIC);
#endif
#if RKAIQ_HAVE_DEBAYER
#if USE_NEWSTRUCT
    APPLY_ANALYZER_RESULT(Dm, DEBAYER);
#else
    APPLY_ANALYZER_RESULT(Debayer, DEBAYER);
#endif
#endif
#if RKAIQ_HAVE_LDCH
    APPLY_ANALYZER_RESULT(Ldch, LDCH);
#endif
#if RKAIQ_HAVE_3DLUT
    APPLY_ANALYZER_RESULT(Lut3d, LUT3D);
#endif
#if RKAIQ_HAVE_DEHAZE
    APPLY_ANALYZER_RESULT(Dehaze, DEHAZE);
    APPLY_ANALYZER_RESULT(Histeq, HISTEQ);
#endif
#if RKAIQ_HAVE_HISTEQ
    APPLY_ANALYZER_RESULT(Histeq, HISTEQ);
#endif
#if RKAIQ_HAVE_ENHANCE
    APPLY_ANALYZER_RESULT(Enh, ENH);
#endif
#if RKAIQ_HAVE_HSV
    APPLY_ANALYZER_RESULT(Hsv, HSV);
#endif
#if RKAIQ_HAVE_GAMMA
#if USE_NEWSTRUCT
    APPLY_ANALYZER_RESULT(Gamma, AGAMMA);
#else
    APPLY_ANALYZER_RESULT(Agamma, AGAMMA);
#endif
#endif
#if RKAIQ_HAVE_DEGAMMA
    APPLY_ANALYZER_RESULT(Adegamma, ADEGAMMA);
#endif
#if RKAIQ_HAVE_WDR
    APPLY_ANALYZER_RESULT(Wdr, WDR);
#endif
#if RKAIQ_HAVE_CSM
    APPLY_ANALYZER_RESULT(Csm, CSM);
#endif
#if RKAIQ_HAVE_CGC
    APPLY_ANALYZER_RESULT(Cgc, CGC);
#endif
    APPLY_ANALYZER_RESULT(Conv422, CONV422);
    APPLY_ANALYZER_RESULT(Yuvconv, YUVCONV);
#if RKAIQ_HAVE_GAIN
    APPLY_ANALYZER_RESULT(Gain, GAIN);
#endif
#if RKAIQ_HAVE_ACP
    APPLY_ANALYZER_RESULT(Cp, CP);
#endif
#if RKAIQ_HAVE_AIE
    APPLY_ANALYZER_RESULT(Ie, IE);
#endif
#if RKAIQ_HAVE_AMD
    APPLY_ANALYZER_RESULT(Motion, MOTION);
#endif
#if RKAIQ_HAVE_ANR
    APPLY_ANALYZER_RESULT(Tnr, TNR);
    APPLY_ANALYZER_RESULT(Ynr, YNR);
    APPLY_ANALYZER_RESULT(Uvnr, UVNR);
    APPLY_ANALYZER_RESULT(Sharpen, SHARPEN);
    APPLY_ANALYZER_RESULT(Edgeflt, EDGEFLT);
#endif
#if RKAIQ_HAVE_SHARP_V40
    APPLY_ANALYZER_RESULT(texEst, TEXEST);
#endif
#if RKAIQ_HAVE_FEC
    APPLY_ANALYZER_RESULT(Fec, FEC);
#endif
#if RKAIQ_HAVE_ORB
    APPLY_ANALYZER_RESULT(Orb, ORB);
#endif
    // ispv21
#if RKAIQ_HAVE_DRC
    APPLY_ANALYZER_RESULT(Drc, DRC);

#endif
#if RKAIQ_HAVE_YNR
    APPLY_ANALYZER_RESULT(Ynr, YNR);
#endif
#if RKAIQ_HAVE_CNR
    APPLY_ANALYZER_RESULT(Cnr, UVNR);
#endif
#if RKAIQ_HAVE_SHARP
#if USE_NEWSTRUCT
    APPLY_ANALYZER_RESULT(Sharp, SHARPEN);
#else
    APPLY_ANALYZER_RESULT(Sharpen, SHARPEN);
#endif
#endif
#if RKAIQ_HAVE_BAYERNR || RKAIQ_HAVE_BAYER2DNR
    APPLY_ANALYZER_RESULT(Baynr, RAWNR);
#endif
    // ispv3x
#if RKAIQ_HAVE_LSC
    APPLY_ANALYZER_RESULT(Lsc, LSC);
#endif
#if RKAIQ_HAVE_CAC
    APPLY_ANALYZER_RESULT(Cac, CAC);
#endif
#if RKAIQ_HAVE_BAYERTNR
#if USE_NEWSTRUCT
    APPLY_ANALYZER_RESULT(Btnr, TNR);
#else
    APPLY_ANALYZER_RESULT(Tnr, TNR);
#endif
#endif
    // ispv32
#if RKAIQ_HAVE_AFD
    APPLY_ANALYZER_RESULT(Afd, AFD);
#endif
#if RKAIQ_HAVE_YUVME
    APPLY_ANALYZER_RESULT(Yuvme, MOTION);
#endif
#if RKAIQ_HAVE_RGBIR_REMOSAIC
    APPLY_ANALYZER_RESULT(Rgbir, RGBIR);
#endif
#if RKAIQ_HAVE_LDC
    APPLY_ANALYZER_RESULT(Ldc, LDC);
#endif
	ret = AiqCamHw_applyAnalyzerResultList(pAiqManager->mCamHw, pAiqManager->mParamsList);
	if (ret) {
        LOGE_ANALYZER("cid:%d, fid:%d apply to hw failed", aiqParams->_base.frame_id,
				AiqCamHw_getCamPhyId(pAiqManager->mCamHw));
	}

    AiqListItem_t* pItem = NULL;
    bool rm              = false;
    AIQ_LIST_FOREACH(pAiqManager->mParamsList, pItem, rm) {
        aiq_params_base_t* params = *(aiq_params_base_t**)(pItem->_pData);
        pItem = aiqList_erase_item_locked(pAiqManager->mParamsList, pItem);
        rm    = true;
        LOGE_ANALYZER("cid:%d, fid:%d apply %d failed", aiqParams->_base.frame_id,
				AiqCamHw_getCamPhyId(pAiqManager->mCamHw), params->type);
        AIQ_REF_BASE_UNREF(&params->_ref_base);
    }

    EXIT_XCORE_FUNCTION();

    return ret;
}

static void
rkAiqCalcDone(void* pCtx, AiqFullParams_t* results)
{
    ENTER_XCORE_FUNCTION();
    AiqManager_applyAnalyzerResult((AiqManager_t*)pCtx, results, false);
    EXIT_XCORE_FUNCTION();
}

XCamReturn AiqManager_setMirrorFlip(AiqManager_t* pAiqManager, bool mirror, bool flip, int skip_frm_cnt)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    ENTER_XCORE_FUNCTION();
    if (pAiqManager->_state == AIQ_STATE_INVALID) {
        LOGE_ANALYZER("wrong aiq state !");
        return XCAM_RETURN_ERROR_FAILED;
    }
    ret = AiqCamHw_setSensorFlip(pAiqManager->mCamHw, mirror, flip, skip_frm_cnt);
    if (ret == XCAM_RETURN_NO_ERROR) {
        // notify aiq sensor flip is changed
        AiqCore_setSensorFlip(pAiqManager->mRkAiqAnalyzer, mirror, flip);
        pAiqManager->mCurMirror = mirror;
        pAiqManager->mCurFlip = flip;
    } else {
        LOGW_ANALYZER("set mirror %d, flip %d error", mirror, flip);
    }
    return ret;
    EXIT_XCORE_FUNCTION();
}

XCamReturn AiqManager_getMirrorFlip(AiqManager_t* pAiqManager, bool* mirror, bool* flip)
{
    ENTER_XCORE_FUNCTION();
    if (pAiqManager->_state == AIQ_STATE_INVALID) {
        LOGE_ANALYZER("wrong aiq state !");
        return XCAM_RETURN_ERROR_FAILED;
    }

    *mirror = pAiqManager->mCurMirror;
    *flip = pAiqManager->mCurFlip;

    EXIT_XCORE_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}


static void setDefMirrorFlip(AiqManager_t* pAiqManager)
{
    /* set defalut mirror & flip from iq*/
    CalibDb_Sensor_ParaV2_t* sensor =
        (CalibDb_Sensor_ParaV2_t*)(CALIBDBV2_GET_MODULE_PTR(pAiqManager->mCalibDbV2, sensor_calib));

    bool def_mirr = sensor->CISFlip & 0x1 ? true : false;
    bool def_flip = sensor->CISFlip & 0x2 ? true : false;
    AiqManager_setMirrorFlip(pAiqManager, def_mirr, def_flip, 0);
}

XCamReturn AiqManager_init(AiqManager_t* pAiqManager, const char* sns_ent_name, rk_aiq_error_cb err_cb, rk_aiq_metas_cb metas_cb)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    pAiqManager->mErrCb = err_cb;
    pAiqManager->mMetasCb = metas_cb;
	pAiqManager->mSnsEntName = sns_ent_name;

    XCAM_ASSERT (pAiqManager->mSnsEntName);
    XCAM_ASSERT (pAiqManager->mRkAiqAnalyzer);
    XCAM_ASSERT (pAiqManager->mCamHw);
    XCAM_ASSERT (pAiqManager->mCalibDbV2);

	pAiqManager->mLastAweekId = -1;
	pAiqManager->mAnalyzeCb.pCtx = pAiqManager;
	pAiqManager->mAnalyzeCb.rkAiqCalcDone = rkAiqCalcDone;
	pAiqManager->mAnalyzeCb.rkAiqCalcFailed = NULL;
	AiqCore_setAnalyzeResultCb(pAiqManager->mRkAiqAnalyzer, &pAiqManager->mAnalyzeCb);

    GlobalParamsManager_setManager(&pAiqManager->mGlobalParamsManager, pAiqManager);
    CamHW_setManager(pAiqManager->mCamHw, pAiqManager);
    ret = GlobalParamsManager_init(&pAiqManager->mGlobalParamsManager, false, pAiqManager->mCalibDbV2);
	AiqCore_setGlobalParamsManager(pAiqManager->mRkAiqAnalyzer, &pAiqManager->mGlobalParamsManager);

    ret |= AiqCore_init(pAiqManager->mRkAiqAnalyzer, pAiqManager->mSnsEntName, pAiqManager->mCalibDbV2);
    RKAIQMNG_CHECK_RET(ret, "analyzer init error %d !", ret);

	pAiqManager->mHwResCb._pCtx = pAiqManager;
	pAiqManager->mHwResCb.hwResCb = hwResCb;
	AiqCamHw_setHwResListener(pAiqManager->mCamHw, &pAiqManager->mHwResCb);
	if (pAiqManager->mCamHw->mIsFake) {
		AiqCamHwFake_init((AiqCamHwFake_t*)pAiqManager->mCamHw, pAiqManager->mSnsEntName);
	} else {
#if defined(ISP_HW_V39)
        ret = AiqCamHwIsp39_init(pAiqManager->mCamHw, pAiqManager->mSnsEntName);
#elif defined(ISP_HW_V33)
        ret = AiqCamHwIsp33_init(pAiqManager->mCamHw, pAiqManager->mSnsEntName);
#else
		XCAM_ASSERT(0);
#endif
	}
    RKAIQMNG_CHECK_RET(ret, "camHw init error %d !", ret);
    pAiqManager->_state = AIQ_STATE_INITED;

    isp_drv_share_mem_ops_t *mem_ops = NULL;
    AiqCamHw_getShareMemOps(pAiqManager->mCamHw, &mem_ops);
    AiqCore_setShareMemOps(pAiqManager->mRkAiqAnalyzer, mem_ops);
    // set default mirror & flip
    setDefMirrorFlip(pAiqManager);

	AiqListConfig_t paramsListCfg;
	paramsListCfg._name      = "paramsList";
	paramsListCfg._item_nums = RESULT_TYPE_MAX_PARAM;
	paramsListCfg._item_size = sizeof(aiq_params_base_t*);
	pAiqManager->mParamsList = aiqList_init(&paramsListCfg);
	if (!pAiqManager->mParamsList)
		LOGE_ANALYZER("init %s error", paramsListCfg._name);

	return ret;
}

XCamReturn AiqManager_prepare(AiqManager_t* pAiqManager, uint32_t width, uint32_t height, rk_aiq_working_mode_t mode)
{
    ENTER_XCORE_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    rk_aiq_exposure_sensor_descriptor sensor_des;

    XCAM_ASSERT (pAiqManager->mCalibDbV2);
#ifdef RUNTIME_MODULE_DEBUG
#ifndef RK_SIMULATOR_HW
    get_dbg_force_disable_mods_env();
#endif
#endif
    int working_mode_hw = RK_AIQ_WORKING_MODE_NORMAL;
    if (mode == RK_AIQ_WORKING_MODE_NORMAL) {
        working_mode_hw = mode;
    } else {
        if (mode == RK_AIQ_WORKING_MODE_ISP_HDR2)
            working_mode_hw = RK_AIQ_ISP_HDR_MODE_2_FRAME_HDR;
        else if (mode == RK_AIQ_WORKING_MODE_ISP_HDR3)
            working_mode_hw = RK_AIQ_ISP_HDR_MODE_3_FRAME_HDR;
        else
            LOGE_ANALYZER("Not supported HDR mode !");
    }
    AiqCamHw_setCalib(pAiqManager->mCamHw, pAiqManager->mCalibDbV2);
    CalibDb_Sensor_ParaV2_t* sensor_calib =
        (CalibDb_Sensor_ParaV2_t*)(CALIBDBV2_GET_MODULE_PTR(pAiqManager->mCalibDbV2, sensor_calib));

#ifdef RKAIQ_ENABLE_CAMGROUP
    AiqCamHw_setGroupMode(pAiqManager->mCamHw, pAiqManager->mCamGroupCoreManager ? true : false, pAiqManager->mIsMain);
#endif

    if(mode != RK_AIQ_WORKING_MODE_NORMAL)
        ret = pAiqManager->mCamHw->prepare(pAiqManager->mCamHw, width, height, working_mode_hw,
                                           sensor_calib->CISExpUpdate.Hdr.time_update,
                                           sensor_calib->CISExpUpdate.Hdr.gain_update);
    else
        ret = pAiqManager->mCamHw->prepare(pAiqManager->mCamHw, width, height, working_mode_hw,
                                           sensor_calib->CISExpUpdate.Linear.time_update,
                                           sensor_calib->CISExpUpdate.Linear.gain_update);

    RKAIQMNG_CHECK_RET(ret, "camhw prepare error %d", ret);

    xcam_mem_clear(sensor_des);
    ret = AiqCamHw_getSensorModeData(pAiqManager->mCamHw, pAiqManager->mSnsEntName, &sensor_des);

    pAiqManager->sensor_output_width = sensor_des.sensor_output_width;
    pAiqManager->sensor_output_height = sensor_des.sensor_output_height;
    int w, h, aligned_w, aligned_h;
    ret = AiqCamHw_get_sp_resolution(pAiqManager->mCamHw, &w, &h, &aligned_w, &aligned_h);
    ret = AiqCore_set_sp_resolution(pAiqManager->mRkAiqAnalyzer, &w, &h, &aligned_w, &aligned_h);
#if RKAIQ_HAVE_PDAF
    ret = AiqCore_set_pdaf_support(pAiqManager->mRkAiqAnalyzer,
			AiqCamHw_get_pdaf_support(pAiqManager->mCamHw));
    ret = AiqCore_set_pdaf_type(pAiqManager->mRkAiqAnalyzer,
			AiqCamHw_get_pdaf_type(pAiqManager->mCamHw));
#endif

    RKAIQMNG_CHECK_RET(ret, "getSensorModeData error %d", ret);
    AiqCore_notifyIspStreamMode(pAiqManager->mRkAiqAnalyzer,
		AiqCamHw_getIspStreamMode(pAiqManager->mCamHw));
    ret = AiqCore_prepare(pAiqManager->mRkAiqAnalyzer, &sensor_des, working_mode_hw);
    RKAIQMNG_CHECK_RET(ret, "analyzer prepare error %d", ret);

    AiqFullParams_t* initParams = AiqCore_getAiqFullParams(pAiqManager->mRkAiqAnalyzer);

#ifdef RKAIQ_ENABLE_CAMGROUP
    if (!pAiqManager->mCamGroupCoreManager) {
#endif
        ret = AiqManager_applyAnalyzerResult(pAiqManager, initParams, true);
        RKAIQMNG_CHECK_RET(ret, "set initial params error %d", ret);
#ifdef RKAIQ_ENABLE_CAMGROUP
    }
#endif

    pAiqManager->mCamHw->get_aiisp_bay3dbuf(pAiqManager->mCamHw);
    pAiqManager->mWorkingMode = mode;
    pAiqManager->mOldWkModeForGray = RK_AIQ_WORKING_MODE_NORMAL;
    pAiqManager->mWidth = width;
    pAiqManager->mHeight = height;
    pAiqManager->_state = AIQ_STATE_PREPARED;

    EXIT_XCORE_FUNCTION();

    return ret;
}

XCamReturn AiqManager_start(AiqManager_t* pAiqManager)
{
    ENTER_XCORE_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    // restart
    if (pAiqManager->_state == AIQ_STATE_STOPED) {
        AiqFullParams_t* initParams = AiqCore_getAiqFullParams(pAiqManager->mRkAiqAnalyzer);
        AiqManager_applyAnalyzerResult(pAiqManager, initParams, true);
    } else if (pAiqManager->_state == AIQ_STATE_STARTED) {
        return ret;
    }

    ret = AiqCore_start(pAiqManager->mRkAiqAnalyzer);
    RKAIQMNG_CHECK_RET(ret, "analyzer start error %d", ret);

    ret = AiqCamHw_start(pAiqManager->mCamHw);
    RKAIQMNG_CHECK_RET(ret, "camhw start error %d", ret);

    pAiqManager->_state = AIQ_STATE_STARTED;

    EXIT_XCORE_FUNCTION();

    return ret;
}

XCamReturn AiqManager_stop(AiqManager_t* pAiqManager, bool keep_ext_hw_st)
{
    ENTER_XCORE_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    if (pAiqManager->_state  == AIQ_STATE_STOPED) {
        return ret;
    }

    pAiqManager->_state = AIQ_STATE_STOPED;

    ret = AiqCore_stop(pAiqManager->mRkAiqAnalyzer);
    RKAIQMNG_CHECK_RET(ret, "analyzer stop error %d", ret);

    AiqCamHw_keepHwStAtStop(pAiqManager->mCamHw, keep_ext_hw_st);
    ret = AiqCamHw_stop(pAiqManager->mCamHw);
    RKAIQMNG_CHECK_RET(ret, "camhw stop error %d", ret);

	AiqCore_clean(pAiqManager->mRkAiqAnalyzer);
	AiqCamHw_clean(pAiqManager->mCamHw);

    EXIT_XCORE_FUNCTION();

    return ret;
}

XCamReturn AiqManager_deinit(AiqManager_t* pAiqManager)
{
    ENTER_XCORE_FUNCTION();

    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    // stop first in prepared status, some resources and process were
    // done at prepare stage
    if (pAiqManager->_state == AIQ_STATE_PREPARED)
        AiqManager_stop(pAiqManager, false);

    ret = AiqCore_deinit(pAiqManager->mRkAiqAnalyzer);
    RKAIQMNG_CHECK_RET(ret, "analyzer deinit error %d", ret);

	if (pAiqManager->mCamHw->mIsFake) {
		AiqCamHwFake_deinit((AiqCamHwFake_t*)pAiqManager->mCamHw);
	} else {
		AiqCamHwBase_deinit(pAiqManager->mCamHw);
	}
    if (pAiqManager->mCalibDbV2) {
        aiq_free(pAiqManager->mCalibDbV2);
        pAiqManager->mCalibDbV2 = NULL;
    }
    if (pAiqManager->tuningCalib) {
        CamCalibDbFreeCalibByJ2S(pAiqManager->tuningCalib);
        pAiqManager->mCalibDbV2 = NULL;
    }

	if (pAiqManager->mParamsList) {
		aiqList_deinit(pAiqManager->mParamsList);
		pAiqManager->mParamsList = NULL;
	}

    pAiqManager->_state = AIQ_STATE_INVALID;

    EXIT_XCORE_FUNCTION();

    return ret;
}

void AiqManager_setAiqCalibDb(AiqManager_t* pAiqManager, const CamCalibDbV2Context_t* calibDb)
{
    ENTER_XCORE_FUNCTION();
    XCAM_ASSERT (!pAiqManager->mCalibDbV2);
    pAiqManager->mCalibDbV2 = (CamCalibDbV2Context_t*)aiq_mallocz(sizeof(CamCalibDbV2Context_t));
    *pAiqManager->mCalibDbV2 = *calibDb;
    EXIT_XCORE_FUNCTION();
}

XCamReturn AiqManager_updateCalibDb(AiqManager_t* pAiqManager, const CamCalibDbV2Context_t* newCalibDb)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    *pAiqManager->mCalibDbV2 = *(CamCalibDbV2Context_t*)newCalibDb;
    AiqCamHw_setCalib(pAiqManager->mCamHw, newCalibDb);

    ret = AiqCore_setCalib(pAiqManager->mRkAiqAnalyzer, pAiqManager->mCalibDbV2);

    if (!AiqCore_isRunningState(pAiqManager->mRkAiqAnalyzer)) {
        AiqCore_updateCalibDbBrutal(pAiqManager->mRkAiqAnalyzer, pAiqManager->mCalibDbV2);
    } else {
		TuningCalib update_list;
		update_list.calib = (CamCalibDbV2Context_t*)newCalibDb;
		strcpy(update_list.moduleNames[0], "colorAsGrey");
		strcpy(update_list.moduleNames[1], "ALL");
		update_list.moduleNamesSize = 2;
        AiqCore_calibTuning(pAiqManager->mRkAiqAnalyzer, pAiqManager->mCalibDbV2, &update_list);
    }

    EXIT_XCORE_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn AiqManager_syncSofEvt(AiqManager_t* pAiqManager, AiqHwEvt_t* hwres)
{
    ENTER_XCORE_FUNCTION();

    if (hwres->type == ISP_POLL_SOF) {
        xcam_get_runtime_log_level();
		AiqCamHw_notify_sof(pAiqManager->mCamHw, hwres);

		Aiqisp20Evt_t hw_evt;
		AiqCamHw_make_ispHwEvt(pAiqManager->mCamHw,
				&hw_evt, hwres->frame_id, V4L2_EVENT_FRAME_SYNC,
				hwres->mTimestamp);
		AiqCore_pushEvts(pAiqManager->mRkAiqAnalyzer, (AiqHwEvt_t*)&hw_evt);

        // TODO: moved to aiq core ?
        if (pAiqManager->mMetasCb) {
            rk_aiq_metas_t metas;
            memset(&metas, 0, sizeof(metas));
            metas.frame_id = hwres->frame_id;
            metas.cam_id = AiqCamHw_getCamPhyId(pAiqManager->mCamHw);
            metas.sensor_name = pAiqManager->mSnsEntName;
            (*pAiqManager->mMetasCb)(&metas);
        }
    }

    EXIT_XCORE_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn AiqManager_setModuleCtl(AiqManager_t* pAiqManager, rk_aiq_module_id_t mId, bool mod_en)
{
    ENTER_XCORE_FUNCTION();
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    ret = AiqCamHw_setModuleCtl(pAiqManager->mCamHw, mId, mod_en);
    EXIT_XCORE_FUNCTION();
    return ret;
}

XCamReturn AiqManager_getModuleCtl(AiqManager_t* pAiqManager, rk_aiq_module_id_t mId, bool* mod_en)
{
    ENTER_XCORE_FUNCTION();
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    ret = AiqCamHw_getModuleCtl(pAiqManager->mCamHw, mId, mod_en);
    EXIT_XCORE_FUNCTION();
    return ret;
}

XCamReturn AiqManager_rawdataPrepare(AiqManager_t* pAiqManager, rk_aiq_raw_prop_t prop)
{
    ENTER_XCORE_FUNCTION();
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    ret = AiqCamHwFake_rawdataPrepare((AiqCamHwFake_t*)pAiqManager->mCamHw, prop);
    EXIT_XCORE_FUNCTION();
    return ret;
}

XCamReturn AiqManager_enqueueRawBuffer(AiqManager_t* pAiqManager, void *rawdata, bool sync)
{
    ENTER_XCORE_FUNCTION();
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    ret = AiqCamHwFake_enqueueRawBuffer((AiqCamHwFake_t*)pAiqManager->mCamHw, rawdata, sync);
    EXIT_XCORE_FUNCTION();
    return ret;

}

XCamReturn AiqManager_enqueueRawFile(AiqManager_t* pAiqManager, const char *path)
{
    ENTER_XCORE_FUNCTION();
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
	// TODO
    ret = AiqCamHwFake_enqueueRawFile((AiqCamHwFake_t*)pAiqManager->mCamHw, path);
    EXIT_XCORE_FUNCTION();
    return ret;
}

XCamReturn AiqManager_registRawdataCb(AiqManager_t* pAiqManager, void (*callback)(void *))
{
    ENTER_XCORE_FUNCTION();
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    ret = AiqCamHwFake_registRawdataCb((AiqCamHwFake_t*)pAiqManager->mCamHw, callback);
    EXIT_XCORE_FUNCTION();
    return ret;
}

void AiqManager_setMulCamConc(AiqManager_t* pAiqManager, bool cc)
{
	AiqCamHw_setMulCamConc(pAiqManager->mCamHw, cc);
	AiqCore_setMulCamConc(pAiqManager->mRkAiqAnalyzer, cc);
}

XCamReturn AiqManager_calibTuning(AiqManager_t* pAiqManager, CamCalibDbV2Context_t* aiqCalib,
                       TuningCalib* change_list)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    AiqCamHw_setCalib(pAiqManager->mCamHw, aiqCalib);
    bool need_check = true;
    if (change_list->moduleNamesSize == 1 &&
        (strstr(change_list->moduleNames[0], "ae") || strstr(change_list->moduleNames[0], "wb") || strstr(change_list->moduleNames[0], "af")))
        need_check = false;
    GlobalParamsManager_switchCalibDb(&pAiqManager->mGlobalParamsManager, aiqCalib, need_check);
    ret = AiqCore_setCalib(pAiqManager->mRkAiqAnalyzer, aiqCalib);

	AiqCore_calibTuning(pAiqManager->mRkAiqAnalyzer, aiqCalib, change_list);

    // Won't free calib witch from iqfiles
    *pAiqManager->mCalibDbV2 = *aiqCalib;
    if (pAiqManager->mNeedFreeCalib) {
        CamCalibDbFreeCalibByJ2S(pAiqManager->tuningCalib);
        pAiqManager->tuningCalib = aiqCalib;
    }
    else {
        pAiqManager->mNeedFreeCalib = true;
    }
    EXIT_XCORE_FUNCTION();

    return ret;
}

XCamReturn AiqManager_setVicapStreamMode(AiqManager_t* pAiqManager, int on, bool isSingleMode)
{
    return AiqCamHw_setVicapStreamMode(pAiqManager->mCamHw, on, isSingleMode);
}
