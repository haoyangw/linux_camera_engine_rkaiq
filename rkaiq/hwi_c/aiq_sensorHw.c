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

#include "aiq_sensorHw.h"

#include "common/rk-camera-module.h"
#include "iq_parser_v2/RkAiqCalibDbTypesV2.h"

static uint16_t SENSORHW_DEFAULT_POOL_SIZE = 20;

static XCamReturn SensorHw_getSensorDescriptor(AiqSensorHw_t* pBaseSns,
                                               rk_aiq_exposure_sensor_descriptor* sns_des);
static XCamReturn _SensorHw_set_sync_mode(AiqSensorHw_t* pSnsHw, uint32_t mode);
static XCamReturn _SensorHw_set_working_mode(AiqSensorHw_t* pSnsHw, int mode);
static XCamReturn _SensorHw_setI2cDAta(AiqSensorHw_t* pSnsHw, aiq_pending_split_exps_t* exps);
static XCamReturn _set_mirror_flip(AiqSensorHw_t* pSnsHw);

static int _SensorHw_getSensorFps(AiqV4l2SubDevice_t* sd, float* fps) {
    struct v4l2_subdev_frame_interval finterval;

    memset(&finterval, 0, sizeof(finterval));
    finterval.pad = 0;

    if (AiqV4l2SubDevice_ioctl(sd, VIDIOC_SUBDEV_G_FRAME_INTERVAL, &finterval) < 0) return -errno;

    *fps = (float)(finterval.interval.denominator) / finterval.interval.numerator;

    return 0;
}

static XCamReturn _SensorHw_setLinearSensorExposure(AiqSensorHw_t* pSnsHw,
                                                    RKAiqAecExpInfo_t* expPar) {
    ENTER_CAMHW_FUNCTION();
    int frame_line_length;
    struct v4l2_control ctrl;
    rk_aiq_exposure_sensor_descriptor sensor_desc;

    LOGD_CAMHW_SUBM(SENSOR_SUBM, "camId: %d, frameId: %d: a-gain: %d, time: %d, dcg: %d, snr: %d\n",
                    pSnsHw->mCamPhyId, pSnsHw->_frame_sequence,
                    expPar->LinearExp.exp_sensor_params.analog_gain_code_global,
                    expPar->LinearExp.exp_sensor_params.coarse_integration_time,
                    expPar->LinearExp.exp_real_params.dcg_mode, expPar->CISFeature.SNR);

    // set vts before exposure time firstly
    SensorHw_getSensorDescriptor(pSnsHw, &sensor_desc);

    frame_line_length = expPar->frame_length_lines > sensor_desc.line_periods_per_field
                            ? expPar->frame_length_lines
                            : sensor_desc.line_periods_per_field;

    memset(&ctrl, 0, sizeof(ctrl));
    ctrl.id    = V4L2_CID_VBLANK;
    ctrl.value = frame_line_length - sensor_desc.sensor_output_height;
    if (AiqV4l2SubDevice_ioctl(pSnsHw->mSd, VIDIOC_S_CTRL, &ctrl) < 0) {
        LOGE_CAMHW_SUBM(SENSOR_SUBM, "cam%d failed to set vblank result(val: %d)",
                        pSnsHw->mCamPhyId, ctrl.value);
        return XCAM_RETURN_ERROR_IOCTL;
    }

    int dcg_mode = expPar->LinearExp.exp_real_params.dcg_mode;
    int dcg_mode_drv;

    if (dcg_mode == 1 /*AEC_DCG_MODE_HCG*/)
        dcg_mode_drv = GAIN_MODE_HCG;
    else if (dcg_mode == 0 /*AEC_DCG_MODE_LCG*/)
        dcg_mode_drv = GAIN_MODE_LCG;
    else  // default
        dcg_mode_drv = -1;

    if (dcg_mode_drv != -1) {
        if (AiqV4l2SubDevice_ioctl(pSnsHw->mSd, RKMODULE_SET_CONVERSION_GAIN, &dcg_mode_drv) < 0) {
            LOGD_CAMHW_SUBM(SENSOR_SUBM, "cam%d failed to set conversion gain !",
                            pSnsHw->mCamPhyId);
            return XCAM_RETURN_ERROR_IOCTL;
        }
    }

    memset(&ctrl, 0, sizeof(ctrl));
    ctrl.id    = V4L2_CID_ANALOGUE_GAIN;
    ctrl.value = expPar->LinearExp.exp_sensor_params.analog_gain_code_global;
    if (AiqV4l2SubDevice_ioctl(pSnsHw->mSd, VIDIOC_S_CTRL, &ctrl) < 0) {
        LOGD_CAMHW_SUBM(SENSOR_SUBM, "cam%d failed to  set again result(val: %d)",
                        pSnsHw->mCamPhyId, ctrl.value);
        return XCAM_RETURN_ERROR_IOCTL;
    }

    if (expPar->LinearExp.exp_sensor_params.digital_gain_global != 0) {
        memset(&ctrl, 0, sizeof(ctrl));
        ctrl.id    = V4L2_CID_GAIN;
        ctrl.value = expPar->LinearExp.exp_sensor_params.digital_gain_global;
        if (AiqV4l2SubDevice_ioctl(pSnsHw->mSd, VIDIOC_S_CTRL, &ctrl) < 0) {
            LOGD_CAMHW_SUBM(SENSOR_SUBM, "cam%d failed to set dgain result(val: %d)",
                            pSnsHw->mCamPhyId, ctrl.value);
            return XCAM_RETURN_ERROR_IOCTL;
        }
    }

    if (expPar->LinearExp.exp_sensor_params.coarse_integration_time != 0) {
        memset(&ctrl, 0, sizeof(ctrl));
        ctrl.id    = V4L2_CID_EXPOSURE;
        ctrl.value = expPar->LinearExp.exp_sensor_params.coarse_integration_time;
        if (AiqV4l2SubDevice_ioctl(pSnsHw->mSd, VIDIOC_S_CTRL, &ctrl) < 0) {
            LOGD_CAMHW_SUBM(SENSOR_SUBM, "cam%d failed to set dgain result(val: %d)",
                            pSnsHw->mCamPhyId, ctrl.value);
            return XCAM_RETURN_ERROR_IOCTL;
        }
    }

    EXIT_CAMHW_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn _SensorHw_setHdrSensorExposure(AiqSensorHw_t* pSnsHw, RKAiqAecExpInfo_t* expPar) {
    ENTER_CAMHW_FUNCTION();
    struct hdrae_exp_s hdrExp;
    int frame_line_length;
    struct v4l2_control ctrl;
    rk_aiq_exposure_sensor_descriptor sensor_desc;

    LOGD_CAMHW_SUBM(SENSOR_SUBM,
                    "camId: %d, frameId: %d: lexp: 0x%x-0x%x, mexp: 0x%x-0x%x, sexp: 0x%x-0x%x, "
                    "l-dcg %d, m-dcg %d, s-dcg %d\n",
                    pSnsHw->mCamPhyId, pSnsHw->_frame_sequence,
                    expPar->HdrExp[2].exp_sensor_params.analog_gain_code_global,
                    expPar->HdrExp[2].exp_sensor_params.coarse_integration_time,
                    expPar->HdrExp[1].exp_sensor_params.analog_gain_code_global,
                    expPar->HdrExp[1].exp_sensor_params.coarse_integration_time,
                    expPar->HdrExp[0].exp_sensor_params.analog_gain_code_global,
                    expPar->HdrExp[0].exp_sensor_params.coarse_integration_time,
                    expPar->HdrExp[2].exp_real_params.dcg_mode,
                    expPar->HdrExp[1].exp_real_params.dcg_mode,
                    expPar->HdrExp[0].exp_real_params.dcg_mode);

    SensorHw_getSensorDescriptor(pSnsHw, &sensor_desc);

    frame_line_length = expPar->frame_length_lines > sensor_desc.line_periods_per_field
                            ? expPar->frame_length_lines
                            : sensor_desc.line_periods_per_field;

    memset(&ctrl, 0, sizeof(ctrl));
    ctrl.id    = V4L2_CID_VBLANK;
    ctrl.value = frame_line_length - sensor_desc.sensor_output_height;
    if (AiqV4l2SubDevice_ioctl(pSnsHw->mSd, VIDIOC_S_CTRL, &ctrl) < 0) {
        LOGE_CAMHW_SUBM(SENSOR_SUBM, "failed to set vblank result(val: %d)", ctrl.value);
        return XCAM_RETURN_ERROR_IOCTL;
    }

    memset(&ctrl, 0, sizeof(ctrl));
    ctrl.id    = V4L2_CID_ANALOGUE_GAIN;
    ctrl.value = expPar->LinearExp.exp_sensor_params.analog_gain_code_global;
    if (AiqV4l2SubDevice_ioctl(pSnsHw->mSd, VIDIOC_S_CTRL, &ctrl) < 0) {
        LOGD_CAMHW_SUBM(SENSOR_SUBM, "failed to  set again result(val: %d)", ctrl.value);
        return XCAM_RETURN_ERROR_IOCTL;
    }

    memset(&hdrExp, 0, sizeof(hdrExp));
    hdrExp.long_exp_reg    = expPar->HdrExp[2].exp_sensor_params.coarse_integration_time;
    hdrExp.long_gain_reg   = expPar->HdrExp[2].exp_sensor_params.analog_gain_code_global;
    hdrExp.middle_exp_reg  = expPar->HdrExp[1].exp_sensor_params.coarse_integration_time;
    hdrExp.middle_gain_reg = expPar->HdrExp[1].exp_sensor_params.analog_gain_code_global;
    hdrExp.short_exp_reg   = expPar->HdrExp[0].exp_sensor_params.coarse_integration_time;
    hdrExp.short_gain_reg  = expPar->HdrExp[0].exp_sensor_params.analog_gain_code_global;

    int dcg_mode = expPar->HdrExp[2].exp_real_params.dcg_mode;

    if (dcg_mode == 1 /*AEC_DCG_MODE_HCG*/)
        hdrExp.long_cg_mode = GAIN_MODE_HCG;
    else if (dcg_mode == 0 /*AEC_DCG_MODE_LCG*/)
        hdrExp.long_cg_mode = GAIN_MODE_LCG;
    else  // default
        hdrExp.long_cg_mode = GAIN_MODE_LCG;

    dcg_mode = expPar->HdrExp[1].exp_real_params.dcg_mode;

    if (dcg_mode == 1 /*AEC_DCG_MODE_HCG*/)
        hdrExp.middle_cg_mode = GAIN_MODE_HCG;
    else if (dcg_mode == 0 /*AEC_DCG_MODE_LCG*/)
        hdrExp.middle_cg_mode = GAIN_MODE_LCG;
    else  // default
        hdrExp.middle_cg_mode = GAIN_MODE_LCG;

    dcg_mode = expPar->HdrExp[0].exp_real_params.dcg_mode;

    if (dcg_mode == 1 /*AEC_DCG_MODE_HCG*/)
        hdrExp.short_cg_mode = GAIN_MODE_HCG;
    else if (dcg_mode == 0 /*AEC_DCG_MODE_LCG*/)
        hdrExp.short_cg_mode = GAIN_MODE_LCG;
    else  // default
        hdrExp.short_cg_mode = GAIN_MODE_LCG;

    if (AiqV4l2SubDevice_ioctl(pSnsHw->mSd, SENSOR_CMD_SET_HDRAE_EXP, &hdrExp) < 0) {
        LOGE_CAMHW_SUBM(SENSOR_SUBM, "failed to set hdrExp exp");
        return XCAM_RETURN_ERROR_IOCTL;
    }

    EXIT_CAMHW_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn _SensorHw_setLinearSensorExposure2(AiqSensorHw_t* pSnsHw,
                                                     aiq_pending_split_exps_t* expPar) {
    ENTER_CAMHW_FUNCTION();
    int frame_line_length;
    struct v4l2_control ctrl;
    rk_aiq_exposure_sensor_descriptor sensor_desc;

    LOGD_CAMHW_SUBM(SENSOR_SUBM, "%s: cam%d frameid: %u, a-gain: %d, time: %d, dcg: %d\n",
                    __FUNCTION__, pSnsHw->mCamPhyId, pSnsHw->_frame_sequence,
                    expPar->rk_exp_res.sensor_params[0].analog_gain_code_global,
                    expPar->rk_exp_res.sensor_params[0].coarse_integration_time,
                    expPar->rk_exp_res.dcg_mode[0]);

    // set vts before exposure time firstly
    SensorHw_getSensorDescriptor(pSnsHw, &sensor_desc);

    frame_line_length = expPar->rk_exp_res.frame_length_lines > sensor_desc.line_periods_per_field
                            ? expPar->rk_exp_res.frame_length_lines
                            : sensor_desc.line_periods_per_field;

    memset(&ctrl, 0, sizeof(ctrl));
    ctrl.id    = V4L2_CID_VBLANK;
    ctrl.value = frame_line_length - sensor_desc.sensor_output_height;
    if (AiqV4l2SubDevice_ioctl(pSnsHw->mSd, VIDIOC_S_CTRL, &ctrl) < 0) {
        LOGE_CAMHW_SUBM(SENSOR_SUBM, "cid: %d failed to set vblank result(val: %d)",
                        pSnsHw->mCamPhyId, ctrl.value);
        return XCAM_RETURN_ERROR_IOCTL;
    }

    if (expPar->rk_exp_res.update_bits & (1 << RK_EXP_UPDATE_DCG)) {
        int dcg_mode = expPar->rk_exp_res.dcg_mode[0];
        int dcg_mode_drv;

        if (dcg_mode == 1 /*AEC_DCG_MODE_HCG*/)
            dcg_mode_drv = GAIN_MODE_HCG;
        else if (dcg_mode == 0 /*AEC_DCG_MODE_LCG*/)
            dcg_mode_drv = GAIN_MODE_LCG;
        else  // default
            dcg_mode_drv = -1;

        if (dcg_mode_drv != -1) {
            if (AiqV4l2SubDevice_ioctl(pSnsHw->mSd, RKMODULE_SET_CONVERSION_GAIN, &dcg_mode_drv) <
                0) {
                LOGD_CAMHW_SUBM(SENSOR_SUBM, "failed to set conversion gain !");
                return XCAM_RETURN_ERROR_IOCTL;
            }
        }
    }

    if (expPar->rk_exp_res.update_bits & (1 << RK_EXP_UPDATE_GAIN)) {
        memset(&ctrl, 0, sizeof(ctrl));
        ctrl.id    = V4L2_CID_ANALOGUE_GAIN;
        ctrl.value = expPar->rk_exp_res.sensor_params[0].analog_gain_code_global;
        if (AiqV4l2SubDevice_ioctl(pSnsHw->mSd, VIDIOC_S_CTRL, &ctrl) < 0) {
            LOGD_CAMHW_SUBM(SENSOR_SUBM, "cid:%d failed to  set again result(val: %d)",
                            pSnsHw->mCamPhyId, ctrl.value);
            return XCAM_RETURN_ERROR_IOCTL;
        }

        if (expPar->rk_exp_res.sensor_params[0].digital_gain_global != 0) {
            memset(&ctrl, 0, sizeof(ctrl));
            ctrl.id    = V4L2_CID_GAIN;
            ctrl.value = expPar->rk_exp_res.sensor_params[0].digital_gain_global;
            if (AiqV4l2SubDevice_ioctl(pSnsHw->mSd, VIDIOC_S_CTRL, &ctrl) < 0) {
                LOGD_CAMHW_SUBM(SENSOR_SUBM, "cid:%d failed to set dgain result(val: %d)",
                                pSnsHw->mCamPhyId, ctrl.value);
                return XCAM_RETURN_ERROR_IOCTL;
            }
        }
    }

    if (expPar->rk_exp_res.update_bits & (1 << RK_EXP_UPDATE_TIME)) {
        if (expPar->rk_exp_res.sensor_params[0].coarse_integration_time != 0) {
            memset(&ctrl, 0, sizeof(ctrl));
            ctrl.id    = V4L2_CID_EXPOSURE;
            ctrl.value = expPar->rk_exp_res.sensor_params[0].coarse_integration_time;
            if (AiqV4l2SubDevice_ioctl(pSnsHw->mSd, VIDIOC_S_CTRL, &ctrl) < 0) {
                LOGD_CAMHW_SUBM(SENSOR_SUBM, "cid:%d failed to set dgain result(val: %d)",
                                pSnsHw->mCamPhyId, ctrl.value);
                return XCAM_RETURN_ERROR_IOCTL;
            }
        }
    }

    EXIT_CAMHW_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn _SensorHw_setHdrSensorExposure2(AiqSensorHw_t* pSnsHw,
                                                  aiq_pending_split_exps_t* expPar) {
    ENTER_CAMHW_FUNCTION();
    struct hdrae_exp_s hdrExp;
    int frame_line_length;
    struct v4l2_control ctrl;
    rk_aiq_exposure_sensor_descriptor sensor_desc;

    LOGD_CAMHW_SUBM(SENSOR_SUBM,
                    "camId: %d, frameId: %d: lexp: 0x%x-0x%x, mexp: 0x%x-0x%x, sexp: 0x%x-0x%x, "
                    "l-dcg %d, m-dcg %d, s-dcg %d\n",
                    pSnsHw->mCamPhyId, pSnsHw->_frame_sequence,
                    expPar->rk_exp_res.sensor_params[2].analog_gain_code_global,
                    expPar->rk_exp_res.sensor_params[2].coarse_integration_time,
                    expPar->rk_exp_res.sensor_params[1].analog_gain_code_global,
                    expPar->rk_exp_res.sensor_params[1].coarse_integration_time,
                    expPar->rk_exp_res.sensor_params[0].analog_gain_code_global,
                    expPar->rk_exp_res.sensor_params[0].coarse_integration_time,
                    expPar->rk_exp_res.dcg_mode[2], expPar->rk_exp_res.dcg_mode[1],
                    expPar->rk_exp_res.dcg_mode[0]);

    SensorHw_getSensorDescriptor(pSnsHw, &sensor_desc);

    frame_line_length = expPar->rk_exp_res.frame_length_lines > sensor_desc.line_periods_per_field
                            ? expPar->rk_exp_res.frame_length_lines
                            : sensor_desc.line_periods_per_field;

    memset(&ctrl, 0, sizeof(ctrl));
    ctrl.id    = V4L2_CID_VBLANK;
    ctrl.value = frame_line_length - sensor_desc.sensor_output_height;
    if (AiqV4l2SubDevice_ioctl(pSnsHw->mSd, VIDIOC_S_CTRL, &ctrl) < 0) {
        LOGE_CAMHW_SUBM(SENSOR_SUBM, "cid:%d failed to set vblank result(val: %d)",
                        pSnsHw->mCamPhyId, ctrl.value);
        return XCAM_RETURN_ERROR_IOCTL;
    }

    memset(&hdrExp, 0, sizeof(hdrExp));
    hdrExp.long_exp_reg    = expPar->rk_exp_res.sensor_params[2].coarse_integration_time;
    hdrExp.long_gain_reg   = expPar->rk_exp_res.sensor_params[2].analog_gain_code_global;
    hdrExp.middle_exp_reg  = expPar->rk_exp_res.sensor_params[1].coarse_integration_time;
    hdrExp.middle_gain_reg = expPar->rk_exp_res.sensor_params[1].analog_gain_code_global;
    hdrExp.short_exp_reg   = expPar->rk_exp_res.sensor_params[0].coarse_integration_time;
    hdrExp.short_gain_reg  = expPar->rk_exp_res.sensor_params[0].analog_gain_code_global;

    int dcg_mode = expPar->rk_exp_res.dcg_mode[2];

    if (dcg_mode == 1 /*AEC_DCG_MODE_HCG*/)
        hdrExp.long_cg_mode = GAIN_MODE_HCG;
    else if (dcg_mode == 0 /*AEC_DCG_MODE_LCG*/)
        hdrExp.long_cg_mode = GAIN_MODE_LCG;
    else  // default
        hdrExp.long_cg_mode = GAIN_MODE_LCG;

    dcg_mode = expPar->rk_exp_res.dcg_mode[1];

    if (dcg_mode == 1 /*AEC_DCG_MODE_HCG*/)
        hdrExp.middle_cg_mode = GAIN_MODE_HCG;
    else if (dcg_mode == 0 /*AEC_DCG_MODE_LCG*/)
        hdrExp.middle_cg_mode = GAIN_MODE_LCG;
    else  // default
        hdrExp.middle_cg_mode = GAIN_MODE_LCG;

    dcg_mode = expPar->rk_exp_res.dcg_mode[0];

    if (dcg_mode == 1 /*AEC_DCG_MODE_HCG*/)
        hdrExp.short_cg_mode = GAIN_MODE_HCG;
    else if (dcg_mode == 0 /*AEC_DCG_MODE_LCG*/)
        hdrExp.short_cg_mode = GAIN_MODE_LCG;
    else  // default
        hdrExp.short_cg_mode = GAIN_MODE_LCG;

    if (AiqV4l2SubDevice_ioctl(pSnsHw->mSd, SENSOR_CMD_SET_HDRAE_EXP, &hdrExp) < 0) {
        LOGD_CAMHW_SUBM(SENSOR_SUBM, "cid:%d failed to set hdrExp exp", pSnsHw->mCamPhyId);
        return XCAM_RETURN_ERROR_IOCTL;
    }

    EXIT_CAMHW_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn _SensorHw_setSensorDpcc(AiqSensorHw_t* pSnsHw,
                                          Sensor_dpcc_res_t* SensorDpccInfo) {
    struct rkmodule_dpcc_cfg dpcc_cfg;

    dpcc_cfg.enable            = SensorDpccInfo->enable;
    dpcc_cfg.cur_single_dpcc   = SensorDpccInfo->cur_single_dpcc;
    dpcc_cfg.cur_multiple_dpcc = SensorDpccInfo->cur_multiple_dpcc;
    dpcc_cfg.total_dpcc        = SensorDpccInfo->total_dpcc;
    LOG1_CAMHW_SUBM(SENSOR_SUBM, "camId: %d, frameId: %d: enable:%d,single:%d,multi:%d,total:%d",
                    pSnsHw->mCamPhyId, pSnsHw->_frame_sequence, dpcc_cfg.enable,
                    dpcc_cfg.cur_single_dpcc, dpcc_cfg.cur_multiple_dpcc, dpcc_cfg.total_dpcc);
    if (AiqV4l2SubDevice_ioctl(pSnsHw->mSd, RKMODULE_SET_DPCC_CFG, &dpcc_cfg) < 0) {
        // LOGE_CAMHW_SUBM(SENSOR_SUBM,"failed to set sensor dpcc");
        return XCAM_RETURN_ERROR_IOCTL;
    }

    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn _SensorHw_composeExpParam(AiqSensorHw_t* pSnsHw, RKAiqAecExpInfo_t* timeValid,
                                            RKAiqAecExpInfo_t* gainValid,
                                            RKAiqAecExpInfo_t* dcgGainModeValid,
                                            RKAiqAecExpInfo_t* newExp) {
    *newExp = *timeValid;
    if (pSnsHw->_working_mode == RK_AIQ_WORKING_MODE_NORMAL) {
        newExp->LinearExp.exp_sensor_params.analog_gain_code_global =
            gainValid->LinearExp.exp_sensor_params.analog_gain_code_global;
        newExp->LinearExp.exp_sensor_params.coarse_integration_time =
            timeValid->LinearExp.exp_sensor_params.coarse_integration_time;
        newExp->LinearExp.exp_real_params.dcg_mode =
            dcgGainModeValid->LinearExp.exp_real_params.dcg_mode;
    } else {
        newExp->HdrExp[2].exp_sensor_params.analog_gain_code_global =
            gainValid->HdrExp[2].exp_sensor_params.analog_gain_code_global;
        newExp->HdrExp[2].exp_sensor_params.coarse_integration_time =
            timeValid->HdrExp[2].exp_sensor_params.coarse_integration_time;
        newExp->HdrExp[2].exp_real_params.dcg_mode =
            dcgGainModeValid->HdrExp[2].exp_real_params.dcg_mode;
        newExp->HdrExp[1].exp_sensor_params.analog_gain_code_global =
            gainValid->HdrExp[1].exp_sensor_params.analog_gain_code_global;
        newExp->HdrExp[1].exp_sensor_params.coarse_integration_time =
            timeValid->HdrExp[1].exp_sensor_params.coarse_integration_time;
        newExp->HdrExp[1].exp_real_params.dcg_mode =
            dcgGainModeValid->HdrExp[1].exp_real_params.dcg_mode;
        newExp->HdrExp[0].exp_sensor_params.analog_gain_code_global =
            gainValid->HdrExp[0].exp_sensor_params.analog_gain_code_global;
        newExp->HdrExp[0].exp_sensor_params.coarse_integration_time =
            timeValid->HdrExp[0].exp_sensor_params.coarse_integration_time;
        newExp->HdrExp[0].exp_real_params.dcg_mode =
            dcgGainModeValid->HdrExp[0].exp_real_params.dcg_mode;
    }
    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn _SensorHw_split_locked(AiqSensorHw_t* pSnsHw, AiqSensorExpInfo_t* exp_param,
                                         uint32_t sof_id) {
    ENTER_CAMHW_FUNCTION();
    XCamReturn ret = XCAM_RETURN_NO_ERROR;

    uint32_t dst_id = 0, max_dst_id = 0;
    // custom mode
    RKAiqExpI2cParam_t* i2c_param = exp_param->exp_i2c_params;
    if (i2c_param->bValid) {
        unsigned int num_regs = i2c_param->nNumRegs;
        uint32_t i            = 0;
        AiqMapItem_t* pItem   = NULL;

        LOG1_CAMHW_SUBM(SENSOR_SUBM, "i2c_exp_res num_regs %d!", num_regs);
        for (i = 0; i < num_regs; i++) {
            dst_id = sof_id + i2c_param->DelayFrames[i];
            LOG1_CAMHW_SUBM(SENSOR_SUBM, "i2c_exp_res delay: %d, dst_id %d",
                            i2c_param->DelayFrames[i], dst_id);
            if (max_dst_id < dst_id) max_dst_id = dst_id;

            pItem = aiqMap_get(pSnsHw->_pending_spilt_map, (void*)(intptr_t)dst_id);
            if (!pItem) {
                aiq_pending_split_exps_t new_exps;
                memset(&new_exps, 0, sizeof(aiq_pending_split_exps_t));
                new_exps.is_rk_exp_res               = false;
                new_exps.i2c_exp_res.RegAddr[0]      = i2c_param->RegAddr[i];
                new_exps.i2c_exp_res.RegValue[0]     = i2c_param->RegValue[i];
                new_exps.i2c_exp_res.AddrByteNum[0]  = i2c_param->AddrByteNum[i];
                new_exps.i2c_exp_res.ValueByteNum[0] = i2c_param->ValueByteNum[i];
                new_exps.i2c_exp_res.nNumRegs        = 1;
                pItem =
                    aiqMap_insert(pSnsHw->_pending_spilt_map, (void*)(intptr_t)dst_id, &new_exps);
                if (pItem == NULL) {
                    LOGE_CAMHW_SUBM(SENSOR_SUBM, "insert %d failed %d!", dst_id);
                }
            } else {
                aiq_pending_split_exps_t* tmp = (aiq_pending_split_exps_t*)(pItem->_pData);
                unsigned int num_regs         = tmp->i2c_exp_res.nNumRegs;

                if (num_regs >= MAX_I2CDATA_LEN) {
                    LOGE_CAMHW_SUBM(SENSOR_SUBM, "i2c_exp_res array overflow for frame %d!",
                                    dst_id);
                    return XCAM_RETURN_ERROR_FAILED;
                }
                tmp->i2c_exp_res.RegAddr[num_regs]      = i2c_param->RegAddr[i];
                tmp->i2c_exp_res.RegValue[num_regs]     = i2c_param->RegValue[i];
                tmp->i2c_exp_res.AddrByteNum[num_regs]  = i2c_param->AddrByteNum[i];
                tmp->i2c_exp_res.ValueByteNum[num_regs] = i2c_param->ValueByteNum[i];
                tmp->i2c_exp_res.nNumRegs++;
            }
        }

        if (max_dst_id < sof_id) max_dst_id = sof_id + 1;

        pItem = aiqMap_insert(pSnsHw->_effecting_exp_map, (void*)(intptr_t)(max_dst_id + 1),
                              &exp_param);
        if (pItem == NULL) {
            LOGE_CAMHW_SUBM(SENSOR_SUBM, "insert effMap %d failed!", max_dst_id + 1);
        }
        AIQ_REF_BASE_REF(&exp_param->_base._ref_base);

        LOGD_CAMHW_SUBM(SENSOR_SUBM, "cid: %d, num_reg:%d, efid:%d, isp_dgain:%0.3f \n", num_regs,
                        pSnsHw->mCamPhyId, max_dst_id + 1,
                        exp_param->aecExpInfo.LinearExp.exp_real_params.isp_dgain);
    } else {
        RKAiqAecExpInfo_t* exp_info = &exp_param->aecExpInfo;

        uint32_t dst_time_id = sof_id;
        uint32_t dst_gain_id = sof_id + pSnsHw->_time_delay - pSnsHw->_gain_delay;
        uint32_t dst_dcg_id  = sof_id + pSnsHw->_time_delay - pSnsHw->_dcg_gain_mode_delay;

        aiq_pending_split_exps_t new_exps;
        aiq_pending_split_exps_t* p_new_exps = NULL;
        bool is_id_exist                     = true;
        max_dst_id                           = sof_id + pSnsHw->_time_delay;

        int i = 0, ret1 = 0;

        struct {
            uint32_t dst_id;
            uint32_t type;
        } update_exps[3] = {{dst_time_id, RK_EXP_UPDATE_TIME},
                            {dst_gain_id, RK_EXP_UPDATE_GAIN},
                            {dst_dcg_id, RK_EXP_UPDATE_DCG}};

        AiqMapItem_t* pItem = NULL;

        for (i = 0; i < 3; i++) {
            dst_id = update_exps[i].dst_id;

            pItem = aiqMap_get(pSnsHw->_pending_spilt_map, (void*)(intptr_t)dst_id);
            if (!pItem) {
                p_new_exps = &new_exps;
                memset(p_new_exps, 0, sizeof(aiq_pending_split_exps_t));
                is_id_exist = false;
            } else {
                p_new_exps  = (aiq_pending_split_exps_t*)(pItem->_pData);
                is_id_exist = true;
            }

            p_new_exps->is_rk_exp_res = true;
            p_new_exps->rk_exp_res.update_bits |= 1 << update_exps[i].type;
            p_new_exps->rk_exp_res.line_length_pixels   = exp_info->line_length_pixels;
            p_new_exps->rk_exp_res.frame_length_lines   = exp_info->frame_length_lines;
            p_new_exps->rk_exp_res.pixel_clock_freq_mhz = exp_info->pixel_clock_freq_mhz;

            if (pSnsHw->_working_mode == RK_AIQ_WORKING_MODE_NORMAL) {
                if (update_exps[i].type == RK_EXP_UPDATE_TIME) {
                    p_new_exps->rk_exp_res.sensor_params[0].coarse_integration_time =
                        exp_info->LinearExp.exp_sensor_params.coarse_integration_time;
                    p_new_exps->rk_exp_res.sensor_params[0].fine_integration_time =
                        exp_info->LinearExp.exp_sensor_params.fine_integration_time;
                } else if (update_exps[i].type == RK_EXP_UPDATE_GAIN) {
                    p_new_exps->rk_exp_res.sensor_params[0].analog_gain_code_global =
                        exp_info->LinearExp.exp_sensor_params.analog_gain_code_global;
                    p_new_exps->rk_exp_res.sensor_params[0].digital_gain_global =
                        exp_info->LinearExp.exp_sensor_params.digital_gain_global;
                } else if (update_exps[i].type == RK_EXP_UPDATE_DCG) {
                    p_new_exps->rk_exp_res.dcg_mode[0] =
                        exp_info->LinearExp.exp_real_params.dcg_mode;
                } else {
                    LOGE_CAMHW_SUBM(SENSOR_SUBM, "wrong exposure params type %d!",
                                    update_exps[i].type);
                    return XCAM_RETURN_ERROR_FAILED;
                }
            } else {
                if (update_exps[i].type == RK_EXP_UPDATE_TIME) {
                    p_new_exps->rk_exp_res.sensor_params[0].coarse_integration_time =
                        exp_info->HdrExp[0].exp_sensor_params.coarse_integration_time;
                    p_new_exps->rk_exp_res.sensor_params[0].fine_integration_time =
                        exp_info->HdrExp[0].exp_sensor_params.fine_integration_time;

                    p_new_exps->rk_exp_res.sensor_params[1].coarse_integration_time =
                        exp_info->HdrExp[1].exp_sensor_params.coarse_integration_time;
                    p_new_exps->rk_exp_res.sensor_params[1].fine_integration_time =
                        exp_info->HdrExp[1].exp_sensor_params.fine_integration_time;

                    p_new_exps->rk_exp_res.sensor_params[2].coarse_integration_time =
                        exp_info->HdrExp[2].exp_sensor_params.coarse_integration_time;
                    p_new_exps->rk_exp_res.sensor_params[2].fine_integration_time =
                        exp_info->HdrExp[2].exp_sensor_params.fine_integration_time;
                } else if (update_exps[i].type == RK_EXP_UPDATE_GAIN) {
                    p_new_exps->rk_exp_res.sensor_params[0].analog_gain_code_global =
                        exp_info->HdrExp[0].exp_sensor_params.analog_gain_code_global;
                    p_new_exps->rk_exp_res.sensor_params[0].digital_gain_global =
                        exp_info->HdrExp[0].exp_sensor_params.digital_gain_global;

                    p_new_exps->rk_exp_res.sensor_params[1].analog_gain_code_global =
                        exp_info->HdrExp[1].exp_sensor_params.analog_gain_code_global;
                    p_new_exps->rk_exp_res.sensor_params[1].digital_gain_global =
                        exp_info->HdrExp[1].exp_sensor_params.digital_gain_global;

                    p_new_exps->rk_exp_res.sensor_params[2].analog_gain_code_global =
                        exp_info->HdrExp[2].exp_sensor_params.analog_gain_code_global;
                    p_new_exps->rk_exp_res.sensor_params[2].digital_gain_global =
                        exp_info->HdrExp[2].exp_sensor_params.digital_gain_global;
                } else if (update_exps[i].type == RK_EXP_UPDATE_DCG) {
                    p_new_exps->rk_exp_res.dcg_mode[0] =
                        exp_info->HdrExp[0].exp_real_params.dcg_mode;
                    p_new_exps->rk_exp_res.dcg_mode[1] =
                        exp_info->HdrExp[1].exp_real_params.dcg_mode;
                    p_new_exps->rk_exp_res.dcg_mode[2] =
                        exp_info->HdrExp[2].exp_real_params.dcg_mode;
                } else {
                    LOGE_CAMHW_SUBM(SENSOR_SUBM, "wrong exposure params type %d!",
                                    update_exps[i].type);
                    return XCAM_RETURN_ERROR_FAILED;
                }
            }

            if (!is_id_exist) {
                pItem =
                    aiqMap_insert(pSnsHw->_pending_spilt_map, (void*)(intptr_t)dst_id, p_new_exps);
                if (pItem == NULL) {
                    LOGE_CAMHW_SUBM(SENSOR_SUBM, "insert %d failed %d!", dst_id);
                }
            }
        }
        pItem = aiqMap_insert(pSnsHw->_effecting_exp_map, (void*)(intptr_t)(max_dst_id + 1),
                              &exp_param);
        if (pItem == NULL) {
            LOGE_CAMHW_SUBM(SENSOR_SUBM, "insert effMap %d failed!", max_dst_id + 1);
        }
        AIQ_REF_BASE_REF(&exp_param->_base._ref_base);
    }

    EXIT_CAMHW_FUNCTION();
    return ret;
}

static XCamReturn _SensorHw_handleSofInternal(AiqSensorHw_t* pSns, int64_t time, uint32_t frameid) {
    ENTER_CAMHW_FUNCTION();
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    SensorHwExpItem_t snsExpListItem;
    AiqSensorExpInfo_t* new_exp           = NULL;
    bool set_new_exp                      = false;
    AiqMapItem_t* pItem                   = NULL;
    aiq_pending_split_exps_t* pending_exp = NULL;

    aiqMutex_lock(&pSns->_mutex);
    if (pSns->_frame_sequence != (uint32_t)(-1) && (frameid - pSns->_frame_sequence > 1))
        LOGE_CAMHW_SUBM(SENSOR_SUBM,
                        "cam%d !!!!frame losed,last frameid:%u,current farmeid:%u!!!!\n",
                        pSns->mCamPhyId, pSns->_frame_sequence, frameid);

    pSns->_frame_sequence = frameid;

    LOGD_CAMHW_SUBM(SENSOR_SUBM, "%s: cam%d frameid=%u, exp_list size=%d, gain_list size=%d",
                    __FUNCTION__, pSns->mCamPhyId, frameid, aiqList_size(pSns->_exp_list),
                    aiqList_size(pSns->_delayed_gain_list));

    while (aiqMap_size(pSns->_effecting_exp_map) > 10) {
        pItem = aiqMap_begin(pSns->_effecting_exp_map);
        if (pItem) {
            AIQ_REF_BASE_UNREF(&(*((AiqSensorExpInfo_t**)(pItem->_pData)))->_base._ref_base);
			aiqMap_erase(pSns->_effecting_exp_map, pItem->_key);
        }
    }

    if (aiqList_size(pSns->_exp_list) > 0) {
        aiqList_get(pSns->_exp_list, &snsExpListItem);
        new_exp = snsExpListItem._pSnsExp;
        _SensorHw_split_locked(pSns, new_exp, frameid);
    }

    // update flip, skip _frame_sequence
    if (pSns->_update_mirror_flip) {
        _set_mirror_flip(pSns);
        pSns->_update_mirror_flip = false;
    }

    pItem = aiqMap_begin(pSns->_pending_spilt_map);
    while (pItem) {
        if ((unsigned long)(pItem->_key) <= (uint32_t)frameid) {
            pending_exp = (aiq_pending_split_exps_t*)(pItem->_pData);
            aiqMutex_unlock(&pSns->_mutex);
            if (pending_exp->is_rk_exp_res) {
                if (pSns->_working_mode == RK_AIQ_WORKING_MODE_NORMAL) {
                    ret = _SensorHw_setLinearSensorExposure2(pSns, pending_exp);
                } else {
                    ret = _SensorHw_setHdrSensorExposure2(pSns, pending_exp);
                }
            } else {
                _SensorHw_setI2cDAta(pSns, pending_exp);
            }
            aiqMutex_lock(&pSns->_mutex);
            pItem = aiqMap_erase(pSns->_pending_spilt_map, pItem->_key);
        } else {
            break;
        }
    }

    if (aiqMap_size(pSns->_pending_spilt_map) > 100) {
        LOGW_CAMHW_SUBM(SENSOR_SUBM, "cam%d _pending_spilt_map size %d > 100, may be error",
                        pSns->mCamPhyId, aiqMap_size(pSns->_pending_spilt_map));
    }

    aiqMutex_unlock(&pSns->_mutex);

    if (!pSns->_is_i2c_exp && new_exp) _SensorHw_setSensorDpcc(pSns, &new_exp->SensorDpccInfo);

    AIQ_REF_BASE_UNREF(&new_exp->_base._ref_base);

    EXIT_CAMHW_FUNCTION();

    return ret;
}

static XCamReturn _SensorHw_setI2cDAta(AiqSensorHw_t* pSnsHw, aiq_pending_split_exps_t* exps) {
    struct rkmodule_reg regs;

    regs.num_regs         = (__u64)(exps->i2c_exp_res.nNumRegs);
    regs.preg_addr        = (__u64)(ulong)(exps->i2c_exp_res.RegAddr);
    regs.preg_value       = (__u64)(ulong)(exps->i2c_exp_res.RegValue);
    regs.preg_addr_bytes  = (__u64)(ulong)(exps->i2c_exp_res.AddrByteNum);
    regs.preg_value_bytes = (__u64)(ulong)(exps->i2c_exp_res.ValueByteNum);

    LOG1_CAMHW_SUBM(SENSOR_SUBM, "set sensor reg array num %d ------", exps->i2c_exp_res.nNumRegs);
    if (exps->i2c_exp_res.nNumRegs <= 0) return XCAM_RETURN_NO_ERROR;

    for (uint32_t i = 0; i < regs.num_regs; i++) {
        LOG1_CAMHW_SUBM(SENSOR_SUBM, "reg:(0x%04x,%d,0x%04x,%d)", exps->i2c_exp_res.RegAddr[i],
                        exps->i2c_exp_res.AddrByteNum[i], exps->i2c_exp_res.RegValue[i],
                        exps->i2c_exp_res.ValueByteNum[i]);
    }

    if (AiqV4l2SubDevice_ioctl(pSnsHw, RKMODULE_SET_REGISTER, &regs) < 0) {
        LOGE_CAMHW_SUBM(SENSOR_SUBM, "failed to set i2c regs !");
        return XCAM_RETURN_ERROR_IOCTL;
    }

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn SensorHw_handle_sof(AiqSensorHw_t* pBaseSns, int64_t time, uint32_t frameid) {
    ENTER_CAMHW_FUNCTION();
    int effecting_frame_id            = 0;
    XCamReturn ret                    = XCAM_RETURN_NO_ERROR;
    AiqSensorHw_t* pSns               = (AiqSensorHw_t*)pBaseSns;
    AiqSensorExpInfo_t* exp_time      = NULL;
    AiqSensorExpInfo_t* exp_gain      = NULL;
    AiqSensorExpInfo_t* dcg_gain_mode = NULL;
    SensorHwExpItem_t snsExpListItem;
    bool set_time = false, set_gain = false, set_dcg_gain_mode = false;
    AiqMapItem_t* pItem = NULL;

    // TODO: only i2c exps using new handler now
    if (pSns->_is_i2c_exp) {
        return _SensorHw_handleSofInternal(pBaseSns, time, frameid);
    }

    aiqMutex_lock(&pSns->_mutex);
    if (pSns->_frame_sequence != (uint32_t)(-1) && frameid - pSns->_frame_sequence > 1)
        LOGE_CAMHW_SUBM(SENSOR_SUBM, "!!!!frame losed,last frameid:%u,current farmeid:%u!!!!\n",
                        pSns->_frame_sequence, frameid);

    pSns->_frame_sequence = frameid;
    LOGV_CAMHW_SUBM(SENSOR_SUBM, "%s:cam%d frameid=%u, exp_list size=%d, gain_list size=%d",
                    __FUNCTION__, pSns->mCamPhyId, frameid, aiqList_size(pSns->_exp_list),
                    aiqList_size(pSns->_delayed_gain_list));

    while (aiqMap_size(pSns->_effecting_exp_map) > 10) {
        pItem = aiqMap_begin(pSns->_effecting_exp_map);
        if (pItem) {
            AIQ_REF_BASE_UNREF(&(*((AiqSensorExpInfo_t**)(pItem->_pData)))->_base._ref_base);
            aiqMap_erase(pSns->_effecting_exp_map, pItem->_key);
        }
    }

    if (aiqList_size(pSns->_exp_list) > 0) {
        aiqList_get(pSns->_exp_list, &snsExpListItem);
        AIQ_REF_BASE_UNREF(&pSns->_last_exp_time->_base._ref_base);
        exp_time = pSns->_last_exp_time = snsExpListItem._pSnsExp;
        set_time                        = true;
    } else {
        exp_time = pSns->_last_exp_time;
    }

    if (aiqList_size(pSns->_delayed_gain_list) > 0) {
        aiqList_get(pSns->_delayed_gain_list, &exp_gain);
        AIQ_REF_BASE_UNREF(&pSns->_last_exp_gain->_base._ref_base);
        pSns->_last_exp_gain = exp_gain;
        set_gain             = true;
    } else {
        exp_gain = pSns->_last_exp_gain;
    }

    if (aiqList_size(pSns->_delayed_dcg_gain_mode_list) > 0) {
        aiqList_get(pSns->_delayed_dcg_gain_mode_list, &dcg_gain_mode);
        AIQ_REF_BASE_UNREF(&pSns->_last_dcg_gain_mode->_base._ref_base);
        pSns->_last_dcg_gain_mode = dcg_gain_mode;
        set_dcg_gain_mode         = true;
    } else {
        dcg_gain_mode = pSns->_last_dcg_gain_mode;
    }

    aiqMutex_unlock(&pSns->_mutex);
    // update flip, skip _frame_sequence
    if (pSns->_update_mirror_flip) {
        _set_mirror_flip(pSns);
        pSns->_update_mirror_flip = false;
    }

    LOGD_CAMHW_SUBM(
        SENSOR_SUBM, "%s: cam%d working_mode=%d,frameid=%u, status: set_time=%d,set_gain=%d\n",
        __FUNCTION__, pSns->mCamPhyId, pSns->_working_mode, frameid, set_time, set_gain);

    if (set_time || set_gain || set_dcg_gain_mode) {
        RKAiqAecExpInfo_t *ptr_new_exp = NULL, new_exp;
        if (pSns->_dcg_gain_mode_delayed) {
            // _gain_delayed should be false
            _SensorHw_composeExpParam(pSns, &exp_time->aecExpInfo, &exp_time->aecExpInfo,
                                      &dcg_gain_mode->aecExpInfo, &new_exp);
            ptr_new_exp = &new_exp;
        } else {
            if (pSns->_gain_delayed) {
                if (pSns->_dcg_gain_mode_with_time)
                    dcg_gain_mode = exp_time;
                else
                    dcg_gain_mode = exp_gain;
                _SensorHw_composeExpParam(pSns, &exp_time->aecExpInfo, &exp_gain->aecExpInfo,
                                          &dcg_gain_mode->aecExpInfo, &new_exp);
                ptr_new_exp = &new_exp;
            } else {
                ptr_new_exp = &exp_time->aecExpInfo;
            }
        }

        if (pSns->_working_mode == RK_AIQ_WORKING_MODE_NORMAL) {
            ret = _SensorHw_setLinearSensorExposure(pSns, ptr_new_exp);
        } else {
            ret = _SensorHw_setHdrSensorExposure(pSns, ptr_new_exp);
        }

        _SensorHw_setSensorDpcc(pSns, &exp_time->SensorDpccInfo);
    }
    if (ret != XCAM_RETURN_NO_ERROR) {
        LOGE_CAMHW_SUBM(SENSOR_SUBM, "%s: sof_id[%u]: set exposure failed!!!\n", __FUNCTION__,
                        frameid);
    }

    if (set_time) {
        aiqMutex_lock(&pSns->_mutex);

        if (pSns->_gain_delayed) {
            aiqList_push(pSns->_delayed_gain_list, &exp_time);
            AIQ_REF_BASE_REF(&exp_time->_base._ref_base);
        }

        if (pSns->_dcg_gain_mode_delayed) {
            aiqList_push(pSns->_delayed_dcg_gain_mode_list, &exp_time);
            AIQ_REF_BASE_REF(&exp_time->_base._ref_base);
        }
        effecting_frame_id = frameid + pSns->_time_delay;
        if (pSns->mPauseFlag && (pSns->mIsSingleMode || (frameid == pSns->mPauseId)) &&
            pSns->_time_delay > 1) {
            effecting_frame_id = frameid + 1;
        }

        aiqMap_insert(pSns->_effecting_exp_map, (void*)(intptr_t)effecting_frame_id, &exp_time);
        AIQ_REF_BASE_REF(&exp_time->_base._ref_base);

        if (pSns->_working_mode == RK_AIQ_WORKING_MODE_NORMAL) {
            LOGV_CAMHW_SUBM(
                SENSOR_SUBM,
                "%s:cam%d sof_id[%d], _effecting_exp_map: add %d, a-gain: %d, time: %d, snr: %d\n",
                __FUNCTION__, pSns->mCamPhyId, frameid, effecting_frame_id,
                exp_time->aecExpInfo.LinearExp.exp_sensor_params.analog_gain_code_global,
                exp_time->aecExpInfo.LinearExp.exp_sensor_params.coarse_integration_time,
                exp_time->aecExpInfo.CISFeature.SNR);
        } else {
            LOGV_CAMHW_SUBM(
                SENSOR_SUBM,
                "%s:cam%d sof_id[%d], _effecting_exp_map: add %d, lexp: 0x%x-0x%x, mexp: "
                "0x%x-0x%x, sexp: 0x%x-0x%x\n",
                __FUNCTION__, pSns->mCamPhyId, frameid, effecting_frame_id,
                exp_time->aecExpInfo.HdrExp[2].exp_sensor_params.analog_gain_code_global,
                exp_time->aecExpInfo.HdrExp[2].exp_sensor_params.coarse_integration_time,
                exp_time->aecExpInfo.HdrExp[1].exp_sensor_params.analog_gain_code_global,
                exp_time->aecExpInfo.HdrExp[1].exp_sensor_params.coarse_integration_time,
                exp_time->aecExpInfo.HdrExp[0].exp_sensor_params.analog_gain_code_global,
                exp_time->aecExpInfo.HdrExp[0].exp_sensor_params.coarse_integration_time);
        }

        aiqMutex_unlock(&pSns->_mutex);
    }

    EXIT_CAMHW_FUNCTION();
    return ret;
}

static XCamReturn _set_mirror_flip(AiqSensorHw_t* pSnsHw) {
    struct v4l2_control ctrl;

    memset(&ctrl, 0, sizeof(ctrl));
    ctrl.id    = V4L2_CID_HFLIP;
    ctrl.value = pSnsHw->_mirror ? 1 : 0;
    if (AiqV4l2SubDevice_ioctl(pSnsHw->mSd, VIDIOC_S_CTRL, &ctrl) < 0) {
        LOGE_CAMHW_SUBM(SENSOR_SUBM, "failed to set hflip (val: %d)", ctrl.value);
        return XCAM_RETURN_ERROR_IOCTL;
    }

    ctrl.id    = V4L2_CID_VFLIP;
    ctrl.value = pSnsHw->_flip ? 1 : 0;
    if (AiqV4l2SubDevice_ioctl(pSnsHw->mSd, VIDIOC_S_CTRL, &ctrl) < 0) {
        LOGE_CAMHW_SUBM(SENSOR_SUBM, "failed to set vflip (val: %d)", ctrl.value);
    }

    LOGD_CAMHW_SUBM(SENSOR_SUBM, "set mirror %d, flip %d", pSnsHw->_mirror, pSnsHw->_flip);

    return XCAM_RETURN_NO_ERROR;
}

static uint32_t sensorHw_get_v4l2_pixelformat(uint32_t pixelcode) {
    uint32_t pixelformat = -1;

    switch (pixelcode) {
        case MEDIA_BUS_FMT_SRGGB8_1X8:
            pixelformat = V4L2_PIX_FMT_SRGGB8;
            break;
        case MEDIA_BUS_FMT_SBGGR8_1X8:
            pixelformat = V4L2_PIX_FMT_SBGGR8;
            break;
        case MEDIA_BUS_FMT_SGBRG8_1X8:
            pixelformat = V4L2_PIX_FMT_SGBRG8;
            break;
        case MEDIA_BUS_FMT_SGRBG8_1X8:
            pixelformat = V4L2_PIX_FMT_SGRBG8;
            break;
        case MEDIA_BUS_FMT_SBGGR10_1X10:
            pixelformat = V4L2_PIX_FMT_SBGGR10;
            break;
        case MEDIA_BUS_FMT_SRGGB10_1X10:
            pixelformat = V4L2_PIX_FMT_SRGGB10;
            break;
        case MEDIA_BUS_FMT_SGBRG10_1X10:
            pixelformat = V4L2_PIX_FMT_SGBRG10;
            break;
        case MEDIA_BUS_FMT_SGRBG10_1X10:
            pixelformat = V4L2_PIX_FMT_SGRBG10;
            break;
        case MEDIA_BUS_FMT_SRGGB12_1X12:
            pixelformat = V4L2_PIX_FMT_SRGGB12;
            break;
        case MEDIA_BUS_FMT_SBGGR12_1X12:
            pixelformat = V4L2_PIX_FMT_SBGGR12;
            break;
        case MEDIA_BUS_FMT_SGBRG12_1X12:
            pixelformat = V4L2_PIX_FMT_SGBRG12;
            break;
        case MEDIA_BUS_FMT_SGRBG12_1X12:
            pixelformat = V4L2_PIX_FMT_SGRBG12;
            break;
        case MEDIA_BUS_FMT_Y8_1X8:
            pixelformat = V4L2_PIX_FMT_GREY;
            break;
        case MEDIA_BUS_FMT_Y10_1X10:
            pixelformat = V4L2_PIX_FMT_Y10;
            break;
        case MEDIA_BUS_FMT_Y12_1X12:
            pixelformat = V4L2_PIX_FMT_Y12;
            break;
        case MEDIA_BUS_FMT_SBGGR16_1X16:
            pixelformat = V4L2_PIX_FMT_SBGGR16;
            break;
        case MEDIA_BUS_FMT_SGBRG16_1X16:
            pixelformat = V4L2_PIX_FMT_SGBRG16;
            break;
        case MEDIA_BUS_FMT_SGRBG16_1X16:
            pixelformat = V4L2_PIX_FMT_SGRBG16;
            break;
        case MEDIA_BUS_FMT_SRGGB16_1X16:
            pixelformat = V4L2_PIX_FMT_SRGGB16;
            break;
        default:
            // TODO add other
            LOGD_CAMHW_SUBM(SENSOR_SUBM, "%s no support pixelcode:0x%x\n", __func__, pixelcode);
    }
    return pixelformat;
}

static int SensorHw_getSensorDesc(AiqSensorHw_t* pBaseSns,
                                  rk_aiq_exposure_sensor_descriptor* sns_des) {
    struct v4l2_subdev_format fmt;

    memset(&fmt, 0, sizeof(fmt));
    fmt.pad   = 0;
    fmt.which = V4L2_SUBDEV_FORMAT_ACTIVE;

    if (AiqV4l2SubDevice_ioctl(pBaseSns->mSd, VIDIOC_SUBDEV_G_FMT, &fmt) < 0) return -errno;

    sns_des->sensor_output_width  = fmt.format.width;
    sns_des->sensor_output_height = fmt.format.height;
    sns_des->sensor_pixelformat   = sensorHw_get_v4l2_pixelformat(fmt.format.code);
    return 0;
}

static int SensorHw_getDcgRatio(AiqSensorHw_t* pBaseSns, rk_aiq_sensor_dcg_ratio_t* dcg_ratio) {
    struct rkmodule_dcg_ratio dcg_ratio_drv;

    if (AiqV4l2SubDevice_ioctl(pBaseSns->mSd, RKMODULE_GET_DCG_RATIO, &dcg_ratio_drv) < 0) {
        // LOGD_CAMHW_SUBM(SENSOR_SUBM,"failed to get sensor dcg_ratio");
        dcg_ratio->valid = false;
        return XCAM_RETURN_ERROR_IOCTL;
    }

    dcg_ratio->valid     = true;
    dcg_ratio->integer   = dcg_ratio_drv.integer;
    dcg_ratio->decimal   = dcg_ratio_drv.decimal;
    dcg_ratio->div_coeff = dcg_ratio_drv.div_coeff;

    return 0;
}

static int SensorHw_getNrSwitch(AiqSensorHw_t* pBaseSns, rk_aiq_sensor_nr_switch_t* nr_switch) {
    struct rkmodule_nr_switch_threshold nr_switch_drv;

    if (AiqV4l2SubDevice_ioctl(pBaseSns->mSd, RKMODULE_GET_NR_SWITCH_THRESHOLD, &nr_switch_drv) <
        0) {
        // LOGE_CAMHW_SUBM(SENSOR_SUBM,"failed to get sensor nr switch");
        nr_switch->valid = false;
        return XCAM_RETURN_ERROR_IOCTL;
    }

    nr_switch->valid      = true;
    nr_switch->direct     = nr_switch_drv.direct;
    nr_switch->up_thres   = nr_switch_drv.up_thres;
    nr_switch->down_thres = nr_switch_drv.down_thres;
    nr_switch->div_coeff  = nr_switch_drv.div_coeff;

    return 0;
}

static int SensorHw_getExposureRange(AiqSensorHw_t* pBaseSns,
                                     rk_aiq_exposure_sensor_descriptor* sns_des) {
    struct v4l2_queryctrl ctrl;

    memset(&ctrl, 0, sizeof(ctrl));
    ctrl.id = V4L2_CID_EXPOSURE;

    if (AiqV4l2SubDevice_ioctl(pBaseSns->mSd, VIDIOC_QUERYCTRL, &ctrl) < 0) return -errno;

    sns_des->coarse_integration_time_min        = ctrl.minimum;
    sns_des->coarse_integration_time_max_margin = 10;

    return 0;
}

static int SensorHw_getPixel(AiqSensorHw_t* pBaseSns, rk_aiq_exposure_sensor_descriptor* sns_des) {
    struct v4l2_ext_controls controls;
    struct v4l2_ext_control ext_control;
    signed long pixel;

    memset(&controls, 0, sizeof(controls));
    memset(&ext_control, 0, sizeof(ext_control));

    ext_control.id      = V4L2_CID_PIXEL_RATE;
    controls.ctrl_class = V4L2_CTRL_ID2CLASS(ext_control.id);
    controls.count      = 1;
    controls.controls   = &ext_control;

    if (AiqV4l2SubDevice_ioctl(pBaseSns->mSd, VIDIOC_G_EXT_CTRLS, &controls) < 0) return -errno;

    pixel = ext_control.value64;

    sns_des->pixel_clock_freq_mhz = (float)pixel / 1000000;

    return 0;
}

static int SensorHw_getBlank(AiqSensorHw_t* pBaseSns, rk_aiq_exposure_sensor_descriptor* sns_des) {
    struct v4l2_queryctrl ctrl;
    int horzBlank, vertBlank;

    memset(&ctrl, 0, sizeof(ctrl));
    ctrl.id = V4L2_CID_HBLANK;
    if (AiqV4l2SubDevice_ioctl(pBaseSns->mSd, VIDIOC_QUERYCTRL, &ctrl) < 0) {
        return -errno;
    }
    horzBlank = ctrl.minimum;

    memset(&ctrl, 0, sizeof(ctrl));
    ctrl.id = V4L2_CID_VBLANK;
    if (AiqV4l2SubDevice_ioctl(pBaseSns->mSd, VIDIOC_QUERYCTRL, &ctrl) < 0) {
        return -errno;
    }
    vertBlank = ctrl.minimum;

    sns_des->pixel_periods_per_line = horzBlank + sns_des->sensor_output_width;
    sns_des->line_periods_per_field = vertBlank + sns_des->sensor_output_height;

    return 0;
}

XCamReturn SensorHw_getSensorModeData(AiqSensorHw_t* pBaseSns, const char* sns_ent_name,
                                      rk_aiq_exposure_sensor_descriptor* sns_des) {
    rk_aiq_exposure_sensor_descriptor sensor_desc;

    SensorHw_getSensorDescriptor(pBaseSns, &sensor_desc);

    pBaseSns->_sns_entity_name                  = sns_ent_name;
    sns_des->coarse_integration_time_min        = sensor_desc.coarse_integration_time_min;
    sns_des->coarse_integration_time_max_margin = sensor_desc.coarse_integration_time_max_margin;
    sns_des->fine_integration_time_min          = sensor_desc.fine_integration_time_min;
    sns_des->fine_integration_time_max_margin   = sensor_desc.fine_integration_time_max_margin;

    sns_des->frame_length_lines   = sensor_desc.line_periods_per_field;
    sns_des->line_length_pck      = sensor_desc.pixel_periods_per_line;
    sns_des->vt_pix_clk_freq_hz   = sensor_desc.pixel_clock_freq_mhz * 1000000;
    sns_des->pixel_clock_freq_mhz = sensor_desc.pixel_clock_freq_mhz /* * 1000000 */;

    // add nr_switch
    sns_des->nr_switch = sensor_desc.nr_switch;
    sns_des->dcg_ratio = sensor_desc.dcg_ratio;

    sns_des->sensor_output_width  = sensor_desc.sensor_output_width;
    sns_des->sensor_output_height = sensor_desc.sensor_output_height;
    sns_des->sensor_pixelformat   = sensor_desc.sensor_pixelformat;

    LOGD_CAMHW_SUBM(SENSOR_SUBM, "vts-hts-pclk: %d-%d-%d-%f, rect: [%dx%d]\n",
                    sns_des->frame_length_lines, sns_des->line_length_pck,
                    sns_des->vt_pix_clk_freq_hz, sns_des->pixel_clock_freq_mhz,
                    sns_des->sensor_output_width, sns_des->sensor_output_height);

    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn SensorHw_getSensorDescriptor(AiqSensorHw_t* pBaseSns,
                                               rk_aiq_exposure_sensor_descriptor* sns_des) {
    float fps = 0;
    memset(sns_des, 0, sizeof(rk_aiq_exposure_sensor_descriptor));

    if (SensorHw_getSensorDesc(pBaseSns, sns_des)) return XCAM_RETURN_ERROR_IOCTL;

    if (SensorHw_getBlank(pBaseSns, sns_des)) return XCAM_RETURN_ERROR_IOCTL;

    /*
     * pixel rate is not equal to pclk sometimes
     * prefer to use pclk = ppl * lpp * fps
     */
    if (_SensorHw_getSensorFps(pBaseSns->mSd, &fps) == 0)
        sns_des->pixel_clock_freq_mhz = (float)(sns_des->pixel_periods_per_line) *
                                        sns_des->line_periods_per_field * fps / 1000000.0;
    else if (SensorHw_getPixel(pBaseSns, sns_des))
        return XCAM_RETURN_ERROR_IOCTL;

    if (SensorHw_getExposureRange(pBaseSns, sns_des)) return XCAM_RETURN_ERROR_IOCTL;

    if (SensorHw_getNrSwitch(pBaseSns, &sns_des->nr_switch)) {
        // do nothing;
    }
    if (SensorHw_getDcgRatio(pBaseSns, &sns_des->dcg_ratio)) {
        // do nothing;
    }

    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn SensorHw_setExposureParams(AiqSensorHw_t* pBaseSns,
                                             AiqAecExpInfoWrapper_t* expPar) {
    ENTER_CAMHW_FUNCTION();

    AiqSensorHw_t* pSns             = (AiqSensorHw_t*)pBaseSns;
    AiqAecExpInfoWrapper_t* aec_exp = expPar;
    AiqSensorExpInfo_t* pSnsExp     = NULL;
    AiqMapItem_t* pItem             = NULL;
    int ret                         = 0;

    aiqMutex_lock(&pSns->_mutex);

    if (pSns->_first) {
        if (aec_exp->algo_id == 0) {
            if (aec_exp->ae_proc_res_rk.exp_set_cnt > 0) {
                int lastIdx         = aec_exp->ae_proc_res_rk.exp_set_cnt - 1;
                aec_exp->new_ae_exp = aec_exp->ae_proc_res_rk.exp_set_tbl[lastIdx];
            }
        }
        if (!aec_exp->exp_i2c_params.bValid) {
            pSns->_is_i2c_exp = false;
            if (pSns->_working_mode == RK_AIQ_WORKING_MODE_NORMAL)
                _SensorHw_setLinearSensorExposure(pSns, &aec_exp->new_ae_exp);
            else
                _SensorHw_setHdrSensorExposure(pSns, &aec_exp->new_ae_exp);
            _SensorHw_setSensorDpcc(pSns, &aec_exp->SensorDpccInfo);
        } else {
            pSns->_is_i2c_exp = true;
            aiq_pending_split_exps_t new_exps;
            uint32_t i = 0;
            memset(&new_exps, 0, sizeof(aiq_pending_split_exps_t));
            new_exps.i2c_exp_res.nNumRegs = aec_exp->exp_i2c_params.nNumRegs;
            for (i = 0; i < aec_exp->exp_i2c_params.nNumRegs; i++) {
                new_exps.i2c_exp_res.RegAddr[i]      = aec_exp->exp_i2c_params.RegAddr[i];
                new_exps.i2c_exp_res.RegValue[i]     = aec_exp->exp_i2c_params.RegValue[i];
                new_exps.i2c_exp_res.AddrByteNum[i]  = aec_exp->exp_i2c_params.AddrByteNum[i];
                new_exps.i2c_exp_res.ValueByteNum[i] = aec_exp->exp_i2c_params.ValueByteNum[i];
            }
            _SensorHw_setI2cDAta(pSns, &new_exps);
        }

        if (aiqPool_freeNums(pSns->_expParamsPool)) {
            AiqPoolItem_t* pItem = aiqPool_getFree(pSns->_expParamsPool);
            pSnsExp              = (AiqSensorExpInfo_t*)(pItem->_pData);
        } else {
            LOGE_CAMHW_SUBM(SENSOR_SUBM, "%s: no free params buffer!\n", __FUNCTION__);
            aiqMutex_unlock(&pSns->_mutex);
            return XCAM_RETURN_ERROR_MEM;
        }

        pSnsExp->aecExpInfo     = aec_exp->new_ae_exp;
        pSnsExp->SensorDpccInfo = aec_exp->SensorDpccInfo;
        pSnsExp->exp_i2c_params = &aec_exp->exp_i2c_params;

        pItem = aiqMap_insert(pSns->_effecting_exp_map, (void*)(intptr_t)0, &pSnsExp);
        if (!pItem) {
            AIQ_REF_BASE_UNREF(&pSnsExp->_base._ref_base);
            aiqMutex_unlock(&pSns->_mutex);
            return XCAM_RETURN_ERROR_FAILED;
        }
        pSns->_first = false;

        if (pSns->_last_exp_time) AIQ_REF_BASE_UNREF(&pSns->_last_exp_time->_base._ref_base);
        pSns->_last_exp_time = pSnsExp;
        AIQ_REF_BASE_REF(&pSnsExp->_base._ref_base);

        if (pSns->_last_exp_gain) AIQ_REF_BASE_UNREF(&pSns->_last_exp_gain->_base._ref_base);
        pSns->_last_exp_gain = pSnsExp;
        AIQ_REF_BASE_REF(&pSnsExp->_base._ref_base);

        if (pSns->_last_dcg_gain_mode)
            AIQ_REF_BASE_UNREF(&pSns->_last_dcg_gain_mode->_base._ref_base);
        pSns->_last_dcg_gain_mode = pSnsExp;
        AIQ_REF_BASE_REF(&pSnsExp->_base._ref_base);

        aec_exp->exp_i2c_params.bValid      = false;
        aec_exp->ae_proc_res_rk.exp_set_cnt = 0;
        LOGD_CAMHW_SUBM(SENSOR_SUBM,
                        "exp-sync: first set exp, add id[0] to the effected exp map\n");
    } else {
        if (aec_exp->algo_id == 0) {
            if (aec_exp->ae_proc_res_rk.exp_set_cnt > 0) {
                SensorHwExpItem_t tmp;
                aiq_memset(&tmp, 0, sizeof(SensorHwExpItem_t));

                LOGV_CAMHW_SUBM(SENSOR_SUBM, "%s: exp_tbl_size:%d, exp_list remain:%d\n",
                                __FUNCTION__, aec_exp->ae_proc_res_rk.exp_set_cnt,
                                aiqList_size(pSns->_exp_list));
                /* when new exp-table comes, remove elem until meet the first one of last exp-table
                 */
				AiqListItem_t* pItem = NULL;
				bool rm = false;
				AIQ_LIST_FOREACH(pSns->_exp_list, pItem, rm) {
					SensorHwExpItem_t* pSnsExp = (SensorHwExpItem_t*)pItem->_pData;
					if (!pSnsExp->_isFirst) {
						AIQ_REF_BASE_UNREF(&pSnsExp->_pSnsExp->_base._ref_base);
						pItem = aiqList_erase_item_locked(pSns->_exp_list, pItem);
						rm = true;
					}
				}

                for (int i = 0; i < aec_exp->ae_proc_res_rk.exp_set_cnt; i++) {
                    if (aiqPool_freeNums(pSns->_expParamsPool)) {
                        AiqPoolItem_t* pItem = aiqPool_getFree(pSns->_expParamsPool);
                        pSnsExp              = (AiqSensorExpInfo_t*)(pItem->_pData);
                    } else {
                        LOGE_CAMHW_SUBM(SENSOR_SUBM, "%s: no free params buffer!\n", __FUNCTION__);
                        aiqMutex_unlock(&pSns->_mutex);
                        return XCAM_RETURN_ERROR_MEM;
                    }

                    pSnsExp->aecExpInfo = aec_exp->new_ae_exp;
                    pSnsExp->aecExpInfo.LinearExp =
                        aec_exp->ae_proc_res_rk.exp_set_tbl[i].LinearExp;
                    pSnsExp->aecExpInfo.HdrExp[0] =
                        aec_exp->ae_proc_res_rk.exp_set_tbl[i].HdrExp[0];
                    pSnsExp->aecExpInfo.HdrExp[1] =
                        aec_exp->ae_proc_res_rk.exp_set_tbl[i].HdrExp[1];
                    pSnsExp->aecExpInfo.HdrExp[2] =
                        aec_exp->ae_proc_res_rk.exp_set_tbl[i].HdrExp[2];
                    pSnsExp->aecExpInfo.frame_length_lines =
                        aec_exp->ae_proc_res_rk.exp_set_tbl[i].frame_length_lines;
                    pSnsExp->aecExpInfo.CISFeature.SNR =
                        aec_exp->ae_proc_res_rk.exp_set_tbl[i].CISFeature.SNR;
                    pSnsExp->SensorDpccInfo = aec_exp->SensorDpccInfo;
                    pSnsExp->exp_i2c_params = &aec_exp->exp_i2c_params;

                    /* set a flag when it's fisrt elem of exp-table*/
                    tmp._pSnsExp = pSnsExp;
                    tmp._isFirst = (i == 0 ? true : false);
                    ret          = aiqList_push(pSns->_exp_list, &tmp);
                    if (ret) {
                        LOGE_CAMHW_SUBM(SENSOR_SUBM, "%s: pending list is full !\n", __FUNCTION__);
                        AIQ_REF_BASE_UNREF(&pSnsExp->_base._ref_base);
                        aiqMutex_unlock(&pSns->_mutex);
                        return XCAM_RETURN_ERROR_MEM;
                    }

                    if (pSns->_working_mode == RK_AIQ_WORKING_MODE_NORMAL) {
                        LOGV_CAMHW_SUBM(
                            SENSOR_SUBM,
                            "%s:cam%d add tbl[%d] to list: a-gain: %d, time: %d, snr: %d\n",
                            __FUNCTION__, pSns->mCamPhyId, i,
                            pSnsExp->aecExpInfo.LinearExp.exp_sensor_params.analog_gain_code_global,
                            pSnsExp->aecExpInfo.LinearExp.exp_sensor_params.coarse_integration_time,
                            pSnsExp->aecExpInfo.CISFeature.SNR);
                    } else {
                        LOGV_CAMHW_SUBM(
                            SENSOR_SUBM,
                            "%s:cam%d add tbl[%d] to list: lexp: 0x%x-0x%x, mexp: 0x%x-0x%x, sexp: "
                            "0x%x-0x%x\n",
                            __FUNCTION__, pSns->mCamPhyId, i,
                            pSnsExp->aecExpInfo.HdrExp[2].exp_sensor_params.analog_gain_code_global,
                            pSnsExp->aecExpInfo.HdrExp[2].exp_sensor_params.coarse_integration_time,
                            pSnsExp->aecExpInfo.HdrExp[1].exp_sensor_params.analog_gain_code_global,
                            pSnsExp->aecExpInfo.HdrExp[1].exp_sensor_params.coarse_integration_time,
                            pSnsExp->aecExpInfo.HdrExp[0].exp_sensor_params.analog_gain_code_global,
                            pSnsExp->aecExpInfo.HdrExp[0].exp_sensor_params.coarse_integration_time);
                    }
                }
                aec_exp->exp_i2c_params.bValid      = false;
                aec_exp->ae_proc_res_rk.exp_set_cnt = 0;
            }
        } else {
            LOGE_CAMHW_SUBM(SENSOR_SUBM, "unsurpported now !");
        }
    }
    aiqMutex_unlock(&pSns->_mutex);
    EXIT_CAMHW_FUNCTION();
    return XCAM_RETURN_NO_ERROR;
}

AiqSensorExpInfo_t* SensorHw_getEffectiveExpParams(AiqSensorHw_t* pSnsHw, uint32_t frame_id) {
    ENTER_CAMHW_FUNCTION();

    XCamReturn ret              = XCAM_RETURN_NO_ERROR;
    AiqSensorHw_t* pSns         = (AiqSensorHw_t*)pSnsHw;
    AiqSensorExpInfo_t* pSnsExp = NULL;
    AiqMapItem_t* pItem         = NULL;
    uint32_t search_id          = frame_id == (uint32_t)(-1) ? 0 : frame_id;
    aiqMutex_lock(&pSns->_mutex);

    pItem = aiqMap_get(pSns->_effecting_exp_map, (void*)(intptr_t)search_id);
    // havn't found
    if (!pItem) {
        /* use the latest */
		AiqMapItem_t* pLastItem         = NULL;
		bool rm = false;
		AIQ_MAP_FOREACH(pSns->_effecting_exp_map, pItem, rm) {
			if ((uint32_t)(long)pItem->_key >= search_id)
				break;
			pLastItem = pItem;
		}

		pItem = pLastItem;
        if (pItem) {
            LOGD_CAMHW_SUBM(SENSOR_SUBM,
                            "use effecting exposure of %d for %d, may be something wrong !",
                            (uint32_t)(long)(pItem->_key), search_id);
        } else {
            LOGE_CAMHW_SUBM(SENSOR_SUBM,
                            "can't find the latest effecting exposure for id %d, impossible case !",
                            search_id);
            aiqMutex_unlock(&pSns->_mutex);
            return NULL;
        }

        pSnsExp = *((AiqSensorExpInfo_t**)(pItem->_pData));
    } else {
        pSnsExp = *((AiqSensorExpInfo_t**)(pItem->_pData));
    }

    if (pSns->_working_mode == RK_AIQ_WORKING_MODE_NORMAL) {
        LOG1_CAMHW_SUBM(SENSOR_SUBM, "%s:cam%d search_id: %d, get-last %d, a-gain: %d, time: %d\n",
                        __FUNCTION__, pSns->mCamPhyId, search_id, (uint32_t)(long)(pItem->_key),
                        pSnsExp->aecExpInfo.LinearExp.exp_sensor_params.analog_gain_code_global,
                        pSnsExp->aecExpInfo.LinearExp.exp_sensor_params.coarse_integration_time);
    } else {
        LOG1_CAMHW_SUBM(SENSOR_SUBM,
                        "%s:cam%d search_id: %d, get-last %d, lexp: 0x%x-0x%x, mexp: 0x%x-0x%x, "
                        "sexp: 0x%x-0x%x\n",
                        __FUNCTION__, pSns->mCamPhyId, search_id, (uint32_t)(long)(pItem->_key),
                        pSnsExp->aecExpInfo.HdrExp[2].exp_sensor_params.analog_gain_code_global,
                        pSnsExp->aecExpInfo.HdrExp[2].exp_sensor_params.coarse_integration_time,
                        pSnsExp->aecExpInfo.HdrExp[1].exp_sensor_params.analog_gain_code_global,
                        pSnsExp->aecExpInfo.HdrExp[1].exp_sensor_params.coarse_integration_time,
                        pSnsExp->aecExpInfo.HdrExp[0].exp_sensor_params.analog_gain_code_global,
                        pSnsExp->aecExpInfo.HdrExp[0].exp_sensor_params.coarse_integration_time);
    }

    AIQ_REF_BASE_REF(&pSnsExp->_base._ref_base);
    aiqMutex_unlock(&pSns->_mutex);
    EXIT_CAMHW_FUNCTION();

    return pSnsExp;
}

static XCamReturn _SensorHw_set_working_mode(AiqSensorHw_t* pSnsHw, int mode) {
    struct rkmodule_hdr_cfg hdr_cfg;
    __u32 hdr_mode = NO_HDR;

    xcam_mem_clear(hdr_cfg);
    if (mode == RK_AIQ_WORKING_MODE_NORMAL) {
        hdr_mode = NO_HDR;
    } else if (mode == RK_AIQ_ISP_HDR_MODE_2_FRAME_HDR || mode == RK_AIQ_ISP_HDR_MODE_2_LINE_HDR) {
        hdr_mode = HDR_X2;
    } else if (mode == RK_AIQ_ISP_HDR_MODE_3_FRAME_HDR || mode == RK_AIQ_ISP_HDR_MODE_3_LINE_HDR) {
        hdr_mode = HDR_X3;
    } else {
        LOGW_CAMHW_SUBM(SENSOR_SUBM, "failed to set hdr mode to %d", mode);
        return XCAM_RETURN_ERROR_FAILED;
    }
    hdr_cfg.hdr_mode = hdr_mode;
    if (AiqV4l2SubDevice_ioctl(pSnsHw->mSd, RKMODULE_SET_HDR_CFG, &hdr_cfg) < 0) {
        LOGE_CAMHW_SUBM(SENSOR_SUBM, "failed to set hdr mode %d", hdr_mode);
        // return XCAM_RETURN_ERROR_IOCTL;
    }

    pSnsHw->_working_mode = mode;

    LOGD_CAMHW_SUBM(SENSOR_SUBM, "%s _working_mode: %d\n", __func__, pSnsHw->_working_mode);

    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn _SensorHw_set_exp_delay_info(AiqSensorHw_t* pSnsHw, int time_delay,
                                               int gain_delay, int hcg_lcg_mode_delay) {
    pSnsHw->_time_delay          = time_delay;
    pSnsHw->_gain_delay          = gain_delay;
    pSnsHw->_dcg_gain_mode_delay = hcg_lcg_mode_delay;

    LOG1_CAMHW_SUBM(SENSOR_SUBM, "%s _time_delay: %d, _gain_delay:%d, _dcg_delay:%d\n", __func__,
                    pSnsHw->_time_delay, pSnsHw->_gain_delay, pSnsHw->_dcg_gain_mode_delay);
    if (pSnsHw->_time_delay > pSnsHw->_gain_delay) {
        pSnsHw->_gain_delayed = true;
    } else if (pSnsHw->_time_delay == pSnsHw->_gain_delay) {
        pSnsHw->_gain_delayed = false;
    } else {
        LOGE_CAMHW_SUBM(SENSOR_SUBM, "Not support gain's delay greater than time's delay!");
        return XCAM_RETURN_ERROR_PARAM;
    }

    if (pSnsHw->_dcg_gain_mode_delay > time_delay) {
        LOGE_CAMHW_SUBM(SENSOR_SUBM, "Not support dcg gain's delay %d, greater than time_delay %d!",
                        pSnsHw->_dcg_gain_mode_delay, time_delay);
        return XCAM_RETURN_ERROR_PARAM;
    }

    if (pSnsHw->_dcg_gain_mode_delay > 0 && pSnsHw->_dcg_gain_mode_delay != time_delay &&
        pSnsHw->_dcg_gain_mode_delay != pSnsHw->_gain_delay) {
        pSnsHw->_dcg_gain_mode_delayed = true;
    } else {
        if (pSnsHw->_dcg_gain_mode_delay == time_delay)
            pSnsHw->_dcg_gain_mode_with_time = true;
        else
            pSnsHw->_dcg_gain_mode_with_time = false;
    }

    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn _SensorHw_set_mirror_flip(AiqSensorHw_t* pSnsHw, bool mirror, bool flip,
                                            int32_t* skip_frame_sequence) {
    aiqMutex_lock(&pSnsHw->_mutex);

    if (!AiqV4l2Device_isActivated((AiqV4l2Device_t*)pSnsHw->mSd)) {
        pSnsHw->_flip   = flip;
        pSnsHw->_mirror = mirror;
        _set_mirror_flip(pSnsHw);
        goto end;
    }

    if (pSnsHw->_mirror != mirror || pSnsHw->_flip != flip) {
        pSnsHw->_flip   = flip;
        pSnsHw->_mirror = mirror;
        // will be set at _frame_sequence + 1
        pSnsHw->_update_mirror_flip = true;
        // skip pre and current frame
        *skip_frame_sequence = pSnsHw->_frame_sequence;
        if (*skip_frame_sequence < 0) *skip_frame_sequence = 0;
    } else
        *skip_frame_sequence = -1;

end:
    aiqMutex_unlock(&pSnsHw->_mutex);

    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn _SensorHw_get_mirror_flip(AiqSensorHw_t* pSnsHw, bool* mirror, bool* flip) {
    struct v4l2_control ctrl;

    memset(&ctrl, 0, sizeof(ctrl));
    ctrl.id = V4L2_CID_HFLIP;
    if (AiqV4l2SubDevice_ioctl(pSnsHw->mSd, VIDIOC_G_CTRL, &ctrl) < 0) {
        LOGW_CAMHW_SUBM(SENSOR_SUBM, "failed to set hflip (val: %d)", ctrl.value);
        return XCAM_RETURN_ERROR_IOCTL;
    }

    *mirror = ctrl.value ? true : false;

    ctrl.id = V4L2_CID_VFLIP;
    if (AiqV4l2SubDevice_ioctl(pSnsHw->mSd, VIDIOC_G_CTRL, &ctrl) < 0) {
        LOGW_CAMHW_SUBM(SENSOR_SUBM, "failed to set vflip (val: %d)", ctrl.value);
        return XCAM_RETURN_ERROR_IOCTL;
    }

    *flip = ctrl.value ? true : false;

    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn _SensorHw_start(AiqSensorHw_t* pSnsHw, bool prepared) {
    AiqV4l2Device_t* v4l2_dev = (AiqV4l2Device_t*)pSnsHw->mSd;
    XCamReturn ret            = XCAM_RETURN_NO_ERROR;

    ENTER_CAMHW_FUNCTION();

    ret = (*v4l2_dev->start)(v4l2_dev, prepared);

    EXIT_CAMHW_FUNCTION();
    return ret;
}

XCamReturn _SensorHw_stop(AiqSensorHw_t* pSnsHw) {
    AiqV4l2Device_t* v4l2_dev = (AiqV4l2Device_t*)pSnsHw->mSd;
    XCamReturn ret            = XCAM_RETURN_NO_ERROR;

    ENTER_CAMHW_FUNCTION();
    aiqMutex_lock(&pSnsHw->_mutex);

    _SensorHw_set_sync_mode(pSnsHw, NO_SYNC_MODE);
    ret = (*v4l2_dev->stop)(v4l2_dev);

    EXIT_CAMHW_FUNCTION();
    aiqMutex_unlock(&pSnsHw->_mutex);

    return ret;
}

void AiqSensorHw_clean(AiqSensorHw_t* pSnsHw)
{
    pSnsHw->_frame_sequence = -1;
    pSnsHw->_first          = true;

    while (aiqList_size(pSnsHw->_exp_list) > 0) {
        AiqListItem_t* item = aiqList_get_item(pSnsHw->_exp_list, NULL);
        AIQ_REF_BASE_UNREF(&(((SensorHwExpItem_t*)(item->_pData))->_pSnsExp)->_base._ref_base);
        aiqList_erase_item(pSnsHw->_exp_list, item);
    }
    aiqList_reset(pSnsHw->_exp_list);

    if (pSnsHw->_last_exp_time) {
        AIQ_REF_BASE_UNREF(&pSnsHw->_last_exp_time->_base._ref_base);
        pSnsHw->_last_exp_time = NULL;
    }
    if (pSnsHw->_last_exp_gain) {
        AIQ_REF_BASE_UNREF(&pSnsHw->_last_exp_gain->_base._ref_base);
        pSnsHw->_last_exp_gain = NULL;
    }
    if (pSnsHw->_last_dcg_gain_mode) {
        AIQ_REF_BASE_UNREF(&pSnsHw->_last_dcg_gain_mode->_base._ref_base);
        pSnsHw->_last_dcg_gain_mode = NULL;
    }

    while (aiqMap_size(pSnsHw->_effecting_exp_map) > 0) {
        AiqMapItem_t* pItem = aiqMap_begin(pSnsHw->_effecting_exp_map);
        if (pItem) {
            AIQ_REF_BASE_UNREF(&(*((AiqSensorExpInfo_t**)(pItem->_pData)))->_base._ref_base);
            aiqMap_erase(pSnsHw->_effecting_exp_map, pItem->_key);
        }
    }
    aiqMap_reset(pSnsHw->_effecting_exp_map);

    while (aiqList_size(pSnsHw->_delayed_gain_list) > 0) {
        AiqListItem_t* item = aiqList_get_item(pSnsHw->_delayed_gain_list, NULL);
        AIQ_REF_BASE_UNREF(&(*((AiqSensorExpInfo_t**)(item->_pData)))->_base._ref_base);
        aiqList_erase_item(pSnsHw->_delayed_gain_list, item);
    }
    aiqList_reset(pSnsHw->_delayed_gain_list);

    while (aiqList_size(pSnsHw->_delayed_dcg_gain_mode_list) > 0) {
        AiqListItem_t* item = aiqList_get_item(pSnsHw->_delayed_dcg_gain_mode_list, NULL);
        AIQ_REF_BASE_UNREF(&(*((AiqSensorExpInfo_t**)(item->_pData)))->_base._ref_base);
        aiqList_erase_item(pSnsHw->_delayed_dcg_gain_mode_list, item);
    }
    aiqList_reset(pSnsHw->_delayed_dcg_gain_mode_list);

    while (aiqMap_size(pSnsHw->_pending_spilt_map) > 0) {
        AiqMapItem_t* pItem = aiqMap_begin(pSnsHw->_pending_spilt_map);
        if (pItem) {
            aiqMap_erase(pSnsHw->_effecting_exp_map, pItem->_key);
        }
    }
    aiqMap_reset(pSnsHw->_pending_spilt_map);

    pSnsHw->_frame_sequence = -1;
    pSnsHw->_first          = true;
}

static XCamReturn _SensorHw_open(AiqSensorHw_t* pBaseSns)
{
    AiqV4l2Device_t* v4l2_dev = (AiqV4l2Device_t*)pBaseSns->mSd;
    XCamReturn ret            = XCAM_RETURN_NO_ERROR;

    ENTER_CAMHW_FUNCTION();

    ret = (*v4l2_dev->open)(v4l2_dev, false);

    EXIT_CAMHW_FUNCTION();
    return ret;
}

static XCamReturn _SensorHw_close(AiqSensorHw_t* pBaseSns)
{
    AiqV4l2Device_t* v4l2_dev = (AiqV4l2Device_t*)pBaseSns->mSd;
    XCamReturn ret            = XCAM_RETURN_NO_ERROR;

    ENTER_CAMHW_FUNCTION();

    ret = (*v4l2_dev->close)(v4l2_dev);

    EXIT_CAMHW_FUNCTION();
	return XCAM_RETURN_NO_ERROR;
}

static XCamReturn _SensorHw_set_sync_mode(AiqSensorHw_t* pSnsHw, uint32_t mode) {
    if (AiqV4l2SubDevice_ioctl(pSnsHw->mSd, RKMODULE_SET_SYNC_MODE, &mode) < 0) {
        LOGW_CAMHW_SUBM(SENSOR_SUBM, "failed to set sync mode %d", mode);
        // return XCAM_RETURN_ERROR_IOCTL;
    }

    LOGI_CAMHW_SUBM(SENSOR_SUBM, "set sync mode %d", mode);

    return XCAM_RETURN_NO_ERROR;
}

static bool _SensorHw_getIsSingleMode(AiqSensorHw_t* pSnsHw) { return pSnsHw->mIsSingleMode; }

static XCamReturn _SensorHw_setEffExpMap(AiqSensorHw_t* pSns, uint32_t sequence, void* exp_ptr,
                                         int mode) {
    aiqMutex_lock(&pSns->_mutex);
    AiqMapItem_t* pItem         = NULL;
    AiqSensorExpInfo_t* pSnsExp = NULL;
    int ret                     = 0;
    rk_aiq_frame_info_t off_finfo;

    if (mode) {
        while (aiqMap_size(pSns->_effecting_exp_map) > 4) {
            pItem = aiqMap_begin(pSns->_effecting_exp_map);
            if (pItem) {
                AIQ_REF_BASE_UNREF(&(*((AiqSensorExpInfo_t**)(pItem->_pData)))->_base._ref_base);
				aiqMap_erase(pSns->_effecting_exp_map, pItem->_key);
            }
        }
        if (aiqPool_freeNums(pSns->_expParamsPool)) {
            AiqPoolItem_t* pItem = aiqPool_getFree(pSns->_expParamsPool);
            pSnsExp              = (AiqSensorExpInfo_t*)(pItem->_pData);
        } else {
            LOGE_CAMHW_SUBM(SENSOR_SUBM, "%s: no free params buffer!\n", __FUNCTION__);
            aiqMutex_unlock(&pSns->_mutex);
            return XCAM_RETURN_ERROR_MEM;
        }

        memcpy(&off_finfo, exp_ptr, sizeof(off_finfo));
        pSnsExp->aecExpInfo.LinearExp.exp_sensor_params.analog_gain_code_global =
            (uint32_t)off_finfo.normal_gain_reg;
        pSnsExp->aecExpInfo.LinearExp.exp_sensor_params.coarse_integration_time =
            (uint32_t)off_finfo.normal_exp_reg;
        pSnsExp->aecExpInfo.LinearExp.exp_real_params.analog_gain = (float)off_finfo.normal_gain;
        pSnsExp->aecExpInfo.LinearExp.exp_real_params.integration_time =
            (float)off_finfo.normal_exp;
        pSnsExp->aecExpInfo.LinearExp.exp_sensor_params.digital_gain_global = 1;
        pSnsExp->aecExpInfo.LinearExp.exp_sensor_params.isp_digital_gain    = 1;
        pSnsExp->aecExpInfo.LinearExp.exp_real_params.digital_gain          = 1.0f;
        pSnsExp->aecExpInfo.LinearExp.exp_real_params.isp_dgain             = 1.0f;

        pSnsExp->aecExpInfo.HdrExp[2].exp_sensor_params.analog_gain_code_global =
            (uint32_t)off_finfo.hdr_gain_l;
        pSnsExp->aecExpInfo.HdrExp[2].exp_sensor_params.coarse_integration_time =
            (uint32_t)off_finfo.hdr_exp_l;
        pSnsExp->aecExpInfo.HdrExp[2].exp_real_params.analog_gain = (float)off_finfo.hdr_gain_l_reg;
        pSnsExp->aecExpInfo.HdrExp[2].exp_real_params.integration_time =
            (float)off_finfo.hdr_exp_l_reg;
        pSnsExp->aecExpInfo.HdrExp[2].exp_sensor_params.digital_gain_global = 1;
        pSnsExp->aecExpInfo.HdrExp[2].exp_sensor_params.isp_digital_gain    = 1;
        pSnsExp->aecExpInfo.HdrExp[2].exp_real_params.digital_gain          = 1.0f;
        pSnsExp->aecExpInfo.HdrExp[2].exp_real_params.isp_dgain             = 1.0f;

        pSnsExp->aecExpInfo.HdrExp[1].exp_sensor_params.analog_gain_code_global =
            (uint32_t)off_finfo.hdr_gain_m;
        pSnsExp->aecExpInfo.HdrExp[1].exp_sensor_params.coarse_integration_time =
            (uint32_t)off_finfo.hdr_exp_m;
        pSnsExp->aecExpInfo.HdrExp[1].exp_real_params.analog_gain = (float)off_finfo.hdr_gain_m_reg;
        pSnsExp->aecExpInfo.HdrExp[1].exp_real_params.integration_time =
            (float)off_finfo.hdr_exp_m_reg;
        pSnsExp->aecExpInfo.HdrExp[1].exp_sensor_params.digital_gain_global = 1;
        pSnsExp->aecExpInfo.HdrExp[1].exp_sensor_params.isp_digital_gain    = 1;
        pSnsExp->aecExpInfo.HdrExp[1].exp_real_params.digital_gain          = 1.0f;
        pSnsExp->aecExpInfo.HdrExp[1].exp_real_params.isp_dgain             = 1.0f;

        pSnsExp->aecExpInfo.HdrExp[0].exp_sensor_params.analog_gain_code_global =
            (uint32_t)off_finfo.hdr_gain_s;
        pSnsExp->aecExpInfo.HdrExp[0].exp_sensor_params.coarse_integration_time =
            (uint32_t)off_finfo.hdr_exp_s;
        pSnsExp->aecExpInfo.HdrExp[0].exp_real_params.analog_gain = (float)off_finfo.hdr_gain_s_reg;
        pSnsExp->aecExpInfo.HdrExp[0].exp_real_params.integration_time =
            (float)off_finfo.hdr_exp_s_reg;
        pSnsExp->aecExpInfo.HdrExp[0].exp_sensor_params.digital_gain_global = 1;
        pSnsExp->aecExpInfo.HdrExp[0].exp_sensor_params.isp_digital_gain    = 1;
        pSnsExp->aecExpInfo.HdrExp[0].exp_real_params.digital_gain          = 1.0f;
        pSnsExp->aecExpInfo.HdrExp[0].exp_real_params.isp_dgain             = 1.0f;

        pItem = aiqMap_insert(pSns->_effecting_exp_map, (void*)(intptr_t)sequence, &pSnsExp);
        if (pItem) {
            AIQ_REF_BASE_UNREF(&pSnsExp->_base._ref_base);
            aiqMutex_unlock(&pSns->_mutex);
            return XCAM_RETURN_ERROR_FAILED;
        }
    } else {
        rk_aiq_exposure_params_t* senosrExp = (rk_aiq_exposure_params_t*)exp_ptr;

        while (aiqMap_size(pSns->_effecting_exp_map) > 0) {
            pItem = aiqMap_begin(pSns->_effecting_exp_map);
            if (pItem) {
                AIQ_REF_BASE_UNREF(&(*((AiqSensorExpInfo_t**)(pItem->_pData)))->_base._ref_base);
                aiqMap_erase(pSns->_effecting_exp_map, pItem->_key);
            }
        }

        while (aiqList_size(pSns->_exp_list) > 0) {
            AiqListItem_t* item = aiqList_get_item(pSns->_exp_list, NULL);
            AIQ_REF_BASE_UNREF(&(((SensorHwExpItem_t*)(item->_pData))->_pSnsExp)->_base._ref_base);
            aiqList_erase_item(pSns->_exp_list, item);
        }

        while (aiqMap_size(pSns->_pending_spilt_map) > 0) {
            pItem = aiqMap_begin(pSns->_pending_spilt_map);
			aiqMap_erase(pSns->_effecting_exp_map, pItem->_key);
        }

        if (aiqPool_freeNums(pSns->_expParamsPool)) {
            AiqPoolItem_t* pItem = aiqPool_getFree(pSns->_expParamsPool);
            pSnsExp              = (AiqSensorExpInfo_t*)(pItem->_pData);
        } else {
            LOGE_CAMHW_SUBM(SENSOR_SUBM, "%s: no free params buffer!\n", __FUNCTION__);
            aiqMutex_unlock(&pSns->_mutex);
            return XCAM_RETURN_ERROR_MEM;
        }

        memcpy(&pSnsExp->aecExpInfo, senosrExp, sizeof(rk_aiq_exposure_params_t));
        pItem = aiqMap_insert(pSns->_effecting_exp_map, (void*)(intptr_t)sequence, &pSnsExp);
        if (pItem) {
            AIQ_REF_BASE_UNREF(&pSnsExp->_base._ref_base);
            aiqMutex_unlock(&pSns->_mutex);
            return XCAM_RETURN_ERROR_FAILED;
        }
    }
    aiqMutex_unlock(&pSns->_mutex);

    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn _SensorHw_setPauseFlag(AiqSensorHw_t* pSnsHw, bool mode, uint32_t frameId,
                                         bool isSingleMode) {
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    uint32_t new_exp_id, last_exp_id;
    RKAiqAecExpInfo_t* ptr_new_exp = NULL;
    AiqMapItem_t* pItem            = NULL;
    pSnsHw->mPauseFlag             = mode;

    aiqMutex_lock(&pSnsHw->_mutex);
    if (pSnsHw->mPauseFlag && pSnsHw->_time_delay > 1) {
        pSnsHw->mPauseId = frameId;
        pSnsHw->mIsSingleMode = isSingleMode;
        new_exp_id       = frameId + pSnsHw->_time_delay;
        pItem            = aiqMap_get(pSnsHw->_effecting_exp_map, (void*)(intptr_t)new_exp_id);
        if (pItem) {
            AIQ_REF_BASE_UNREF(&(*((AiqSensorExpInfo_t**)(pItem->_pData)))->_base._ref_base);
            aiqMap_erase(pSnsHw->_effecting_exp_map, pItem->_key);
            pItem = aiqMap_rbegin(pSnsHw->_effecting_exp_map);
            if (pItem) {
                AiqSensorExpInfo_t* pSnsExp = *((AiqSensorExpInfo_t**)(pItem->_pData));
                ptr_new_exp                 = &pSnsExp->aecExpInfo;
                if (pSnsHw->_working_mode == RK_AIQ_WORKING_MODE_NORMAL) {
                    ret = _SensorHw_setLinearSensorExposure(pSnsHw, ptr_new_exp);
                } else {
                    ret = _SensorHw_setHdrSensorExposure(pSnsHw, ptr_new_exp);
                }
                LOGD_CAMHW("erase effect exp id %u, set new exp id is %u", new_exp_id,
                           (uint32_t)(long)(pItem->_key));
            }
        }
        LOGD_CAMHW_SUBM(SENSOR_SUBM,
                        "switch to %s mode, pauseId %u, handle sof id %u, _time_delay %d",
                        pSnsHw->mIsSingleMode ? "single" : "multi", pSnsHw->mPauseId,
                        pSnsHw->_frame_sequence, pSnsHw->_time_delay);
    }
    aiqMutex_unlock(&pSnsHw->_mutex);
    return XCAM_RETURN_NO_ERROR;
}

static void _SensorHw_dump(AiqSensorHw_t* pSnsHw) {
    // TODO
}

void AiqSensorHw_init(AiqSensorHw_t* pSnsHw, const char* name, int cid) {
    ENTER_CAMHW_FUNCTION();

    memset(pSnsHw, 0, sizeof(AiqSensorHw_t));
    pSnsHw->mSd = (AiqV4l2SubDevice_t*)aiq_malloc(sizeof(AiqV4l2SubDevice_t));
    AiqV4l2SubDevice_init(pSnsHw->mSd, name);
    pSnsHw->_working_mode   = RK_AIQ_WORKING_MODE_NORMAL;
    pSnsHw->_first          = true;
    pSnsHw->_frame_sequence = -1;
    pSnsHw->mCamPhyId       = cid;
    {
        // init pool
        AiqPoolConfig_t snsExpPoolCfg;
        AiqPoolItem_t* pItem     = NULL;
        int i                    = 0;
        snsExpPoolCfg._name      = "SensorLocalExpParams";
        snsExpPoolCfg._item_nums = SENSORHW_DEFAULT_POOL_SIZE;
        snsExpPoolCfg._item_size = sizeof(AiqSensorExpInfo_t);
        pSnsHw->_expParamsPool   = aiqPool_init(&snsExpPoolCfg);
        if (!pSnsHw->_expParamsPool)
            LOGE_CAMHW_SUBM(SENSOR_SUBM, "cId:%d init %s error", cid, snsExpPoolCfg._name);
        for (i = 0; i < pSnsHw->_expParamsPool->_item_nums; i++) {
            pItem = &pSnsHw->_expParamsPool->_item_array[i];
            AIQ_REF_BASE_INIT(&((AiqSensorExpInfo_t*)(pItem->_pData))->_base._ref_base, pItem,
                              aiqPoolItem_ref, aiqPoolItem_unref);
        }
    }
    {
        // init list
        AiqListConfig_t snsExpListCfg;
        snsExpListCfg._name      = "snsExpPendingList";
        snsExpListCfg._item_nums = SENSORHW_DEFAULT_POOL_SIZE;
        snsExpListCfg._item_size = sizeof(SensorHwExpItem_t);
        pSnsHw->_exp_list        = aiqList_init(&snsExpListCfg);
        if (!pSnsHw->_exp_list)
            LOGE_CAMHW_SUBM(SENSOR_SUBM, "cId:%d init %s error", cid, snsExpListCfg._name);

        snsExpListCfg._name        = "snsDelayGainList";
        snsExpListCfg._item_nums   = SENSORHW_DEFAULT_POOL_SIZE;
        snsExpListCfg._item_size   = sizeof(AiqSensorExpInfo_t*);
        pSnsHw->_delayed_gain_list = aiqList_init(&snsExpListCfg);
        if (!pSnsHw->_delayed_gain_list)
            LOGE_CAMHW_SUBM(SENSOR_SUBM, "cId:%d init %s error", cid, snsExpListCfg._name);

        snsExpListCfg._name                 = "snsDelayDcgGainModeList";
        snsExpListCfg._item_nums            = SENSORHW_DEFAULT_POOL_SIZE;
        snsExpListCfg._item_size            = sizeof(AiqSensorExpInfo_t*);
        pSnsHw->_delayed_dcg_gain_mode_list = aiqList_init(&snsExpListCfg);
        if (!pSnsHw->_delayed_dcg_gain_mode_list)
            LOGE_CAMHW_SUBM(SENSOR_SUBM, "cId:%d init %s error", cid, snsExpListCfg._name);
    }
    {
        // init map
        AiqMapConfig_t snsExpEffMapCfg;
        snsExpEffMapCfg._name      = "snsExpEffMap";

        snsExpEffMapCfg._key_type = AIQ_MAP_KEY_TYPE_UINT32;
        snsExpEffMapCfg._item_nums = SENSORHW_DEFAULT_POOL_SIZE;
        snsExpEffMapCfg._item_size = sizeof(AiqSensorExpInfo_t*);
        pSnsHw->_effecting_exp_map = aiqMap_init(&snsExpEffMapCfg);
        if (!pSnsHw->_effecting_exp_map)
            LOGE_CAMHW_SUBM(SENSOR_SUBM, "cId:%d init %s error", cid, snsExpEffMapCfg._name);
    }
    {
        AiqMapConfig_t snsExpPendingSplitMapCfg;
        snsExpPendingSplitMapCfg._name      = "snsExpPendingSplitMap";
        snsExpPendingSplitMapCfg._item_nums = SENSORHW_DEFAULT_POOL_SIZE;
        snsExpPendingSplitMapCfg._item_size = sizeof(aiq_pending_split_exps_t);
        snsExpPendingSplitMapCfg._key_type  = AIQ_MAP_KEY_TYPE_UINT32;
        pSnsHw->_pending_spilt_map          = aiqMap_init(&snsExpPendingSplitMapCfg);
        if (!pSnsHw->_pending_spilt_map)
            LOGE_CAMHW_SUBM(SENSOR_SUBM, "cId:%d init %s error", cid,
                            snsExpPendingSplitMapCfg._name);
    }

    aiqMutex_init(&pSnsHw->_mutex);

    pSnsHw->setExposureParams     = SensorHw_setExposureParams;
    pSnsHw->getSensorModeData     = SensorHw_getSensorModeData;
    pSnsHw->handle_sof            = SensorHw_handle_sof;
    pSnsHw->get_sensor_descriptor = SensorHw_getSensorDescriptor;
    pSnsHw->getEffectiveExpParams = SensorHw_getEffectiveExpParams;

    pSnsHw->set_working_mode      = _SensorHw_set_working_mode;
    pSnsHw->set_exp_delay_info    = _SensorHw_set_exp_delay_info;
    pSnsHw->set_mirror_flip       = _SensorHw_set_mirror_flip;
    pSnsHw->get_mirror_flip       = _SensorHw_get_mirror_flip;
    pSnsHw->start                 = _SensorHw_start;
    pSnsHw->stop                  = _SensorHw_stop;
    pSnsHw->close                 = _SensorHw_close;
    pSnsHw->open			 	  = _SensorHw_open;
    pSnsHw->set_sync_mode         = _SensorHw_set_sync_mode;
    pSnsHw->get_is_single_mode    = _SensorHw_getIsSingleMode;
    pSnsHw->set_effecting_exp_map = _SensorHw_setEffExpMap;
    pSnsHw->set_pause_flag        = _SensorHw_setPauseFlag;
    pSnsHw->dump                  = _SensorHw_dump;

    EXIT_CAMHW_FUNCTION();
}

void AiqSensorHw_deinit(AiqSensorHw_t* pSnsHw) {
    ENTER_CAMHW_FUNCTION();
    aiqMap_deinit(pSnsHw->_pending_spilt_map);
    aiqMap_deinit(pSnsHw->_effecting_exp_map);
    aiqList_deinit(pSnsHw->_exp_list);
    aiqList_deinit(pSnsHw->_delayed_gain_list);
    aiqList_deinit(pSnsHw->_delayed_dcg_gain_mode_list);
    aiqPool_deinit(pSnsHw->_expParamsPool);
    if (pSnsHw->mSd) {
        AiqV4l2SubDevice_deinit(pSnsHw->mSd);
        aiq_free(pSnsHw->mSd);
    }
    aiqMutex_deInit(&pSnsHw->_mutex);
    EXIT_CAMHW_FUNCTION();
}
