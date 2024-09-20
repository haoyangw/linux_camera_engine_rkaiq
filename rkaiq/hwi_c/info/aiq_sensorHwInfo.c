/*
 *   Copyright (c) 2024 Rockchip Corporation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include "aiq_sensorHwInfo.h"

#include <string.h>

#include "c_base/aiq_map.h"
#include "dumpcam_server/info/include/rk_info_utils.h"
#include "dumpcam_server/info/include/st_string.h"

void sensor_dump_mod_param(AiqSensorHw_t* self, st_string* result) {
    char buffer[MAX_LINE_LENGTH] = {0};

    aiq_info_dump_title(result, "sensor module param");

    snprintf(buffer, MAX_LINE_LENGTH, "%-10s%-9s%-9s", "dev", "phy_chn", "i2c_exp");
    string_printf(result, buffer);
    string_printf(result, "\n");

    // sensor entity name format SHOULD be like this:
    // m00_b_ov13850 1-0010
    char name[32]       = {0};
    char* real_name_end = strchr(self->_sns_entity_name, ' ');
    int size            = (int)(real_name_end - self->_sns_entity_name) - 6;
    if (real_name_end) strncpy(name, self->_sns_entity_name + 6, size);

    memset(buffer, 0, MAX_LINE_LENGTH);
    snprintf(buffer, MAX_LINE_LENGTH, "%-10s%-9d%-9d", name, self->mCamPhyId, self->_is_i2c_exp);
    string_printf(result, buffer);
    string_printf(result, "\n\n");
}

void sensor_dump_dev_attr1(AiqSensorHw_t* self, st_string* result) {
    char buffer[MAX_LINE_LENGTH] = {0};

    aiq_info_dump_title(result, "sensor dev attr 1");

    snprintf(buffer, MAX_LINE_LENGTH, "%-9s%-8s%-14s%-7s%-8s%-5s%-8s%-6s%-5s", "phy_chn", "mode",
             "pixel_format", "width", "height", "fps", "mirror", "flip", "dcg");
    string_printf(result, buffer);
    string_printf(result, "\n");

    const char* mode = "linear";
    if (RK_AIQ_HDR_GET_WORKING_MODE(self->_working_mode) == RK_AIQ_WORKING_MODE_ISP_HDR2) {
        mode = "HDR2";
    } else if (RK_AIQ_HDR_GET_WORKING_MODE(self->_working_mode) == RK_AIQ_WORKING_MODE_ISP_HDR3) {
        mode = "HDR3";
    }

    char fmt_str[5];
    fmt_str[0] = self->desc.sensor_pixelformat & 0xFF;
    fmt_str[1] = (self->desc.sensor_pixelformat >> 8) & 0xFF;
    fmt_str[2] = (self->desc.sensor_pixelformat >> 16) & 0xFF;
    fmt_str[3] = (self->desc.sensor_pixelformat >> 24) & 0xFF;
    fmt_str[4] = '\0';

    const char* dcg = "N";
    if (self->dcg_mode == GAIN_MODE_HCG)
        dcg = "HCG";
    else if (self->dcg_mode == GAIN_MODE_LCG)
        dcg = "LCG";

    memset(buffer, 0, MAX_LINE_LENGTH);
    snprintf(buffer, MAX_LINE_LENGTH, "%-9d%-8s%-14s%-7d%-8d%-5d%-8s%-6s%-5s", self->mCamPhyId,
             mode, fmt_str, self->desc.sensor_output_width, self->desc.sensor_output_height,
             self->fps, self->_mirror ? "Y" : "N", self->_flip ? "Y" : "N", dcg);
    string_printf(result, buffer);
    string_printf(result, "\n\n");
}

void sensor_dump_dev_attr2(AiqSensorHw_t* self, st_string* result) {
    char buffer[MAX_LINE_LENGTH] = {0};

    aiq_info_dump_title(result, "sensor dev attr 2");

    snprintf(buffer, MAX_LINE_LENGTH, "%-9s%-14s%-14s%-11s%-8s%-10s%-10s%-9s", "phy_chn",
             "pixel_period", "line_period", "pixel_clk", "vBlank", "time_del", "gain_del",
             "dcg_del");
    string_printf(result, buffer);
    string_printf(result, "\n");

    memset(buffer, 0, MAX_LINE_LENGTH);
    snprintf(buffer, MAX_LINE_LENGTH, "%-9d%-14d%-14d%-11.3f%-8d%-10d%-10d%-9d", self->mCamPhyId,
             self->desc.pixel_periods_per_line, self->desc.line_periods_per_field,
             self->desc.pixel_clock_freq_mhz,
             self->desc.frame_length_lines - self->desc.sensor_output_height, self->_time_delay,
             self->_gain_delay, MAX(self->_dcg_gain_mode_delay, 0));
    string_printf(result, buffer);
    string_printf(result, "\n\n");
}

void sensor_dump_reg_upd_delay(AiqSensorHw_t* self, st_string* result) {
    char buffer[MAX_LINE_LENGTH] = {0};

    aiq_info_dump_title(result, "sensor reg delay");

    snprintf(buffer, MAX_LINE_LENGTH, "%-6s%-6s%-6s", "time", "gain", "dcg");
    string_printf(result, buffer);
    string_printf(result, "\n");

    memset(buffer, 0, MAX_LINE_LENGTH);
    snprintf(buffer, MAX_LINE_LENGTH, "%-6d%-6d%-6d", self->_time_delay, self->_gain_delay,
             MAX(self->_dcg_gain_mode_delay, 0));
    string_printf(result, buffer);
    string_printf(result, "\n\n");
}

void sensor_dump_exp_list_size(AiqSensorHw_t* self, st_string* result) {
    char buffer[MAX_LINE_LENGTH] = {0};

    aiq_info_dump_title(result, "sensor exp config debug info");

    snprintf(buffer, MAX_LINE_LENGTH, "%-10s%-9s%-10s%-16s%-15s%-14s", "seq", "set_cnt", "exp_list",
             "effect_exp_map", "gain_del_list", "dcg_del_list");
    string_printf(result, buffer);
    string_printf(result, "\n");

    memset(buffer, 0, MAX_LINE_LENGTH);
    aiqMutex_lock(&self->_mutex);
    snprintf(buffer, MAX_LINE_LENGTH, "%-10d%-9d%-10d%-16d%-15d%-14d", self->_frame_sequence,
             self->set_exp_cnt, aiqList_size(self->_exp_list),
             aiqMap_size(self->_effecting_exp_map), aiqList_size(self->_delayed_gain_list),
             aiqList_size(self->_delayed_dcg_gain_mode_list));
    aiqMutex_unlock(&self->_mutex);
    string_printf(result, buffer);
    string_printf(result, "\n\n");
}

void __dump_configured_linear_exp(AiqSensorHw_t* self, st_string* result) {
    char buffer[MAX_LINE_LENGTH] = {0};

    aiq_info_dump_title(result, "sensor configured linear exp");

    AiqMapItem_t* pItem = NULL;
    bool rm             = false;
    snprintf(buffer, MAX_LINE_LENGTH, "%-10s%-6s%-6s%-7s%-11s%-10s%-10s%-10s%-12s%-10s", "seq",
             "time", "again", "dgain", "isp_dgain", "time_f", "again_f", "dgain_f", "ispdgain_f",
             "iso");
    string_printf(result, buffer);
    string_printf(result, "\n");

    AIQ_MAP_FOREACH(self->_effecting_exp_map, pItem, rm) {
        if (!pItem) goto out;

        memset(buffer, 0, MAX_LINE_LENGTH);
        AiqSensorExpInfo_t* pExp = *((AiqSensorExpInfo_t**)(pItem->_pData));
        snprintf(buffer, MAX_LINE_LENGTH, "%-10d%-6d%-6d%-7d%-11d%-10.6f%-10.2f%-10.2f%-12.2f%-10d",
                 pExp->_base.frame_id,
                 pExp->aecExpInfo.LinearExp.exp_sensor_params.coarse_integration_time,
                 pExp->aecExpInfo.LinearExp.exp_sensor_params.analog_gain_code_global,
                 pExp->aecExpInfo.LinearExp.exp_sensor_params.digital_gain_global,
                 pExp->aecExpInfo.LinearExp.exp_sensor_params.isp_digital_gain,
                 pExp->aecExpInfo.LinearExp.exp_real_params.integration_time,
                 pExp->aecExpInfo.LinearExp.exp_real_params.analog_gain,
                 pExp->aecExpInfo.LinearExp.exp_real_params.digital_gain,
                 pExp->aecExpInfo.LinearExp.exp_real_params.isp_dgain,
                 pExp->aecExpInfo.LinearExp.exp_real_params.iso);
        string_printf(result, buffer);
        string_printf(result, "\n");
    }

out:
    string_printf(result, "\n");
}

void __dump_configured_hdr_short_exp(AiqSensorHw_t* self, st_string* result) {
    char buffer[MAX_LINE_LENGTH] = {0};

    aiq_info_dump_title(result, "sensor configured hdr-short exp");

    AiqMapItem_t* pItem = NULL;
    bool rm             = false;
    snprintf(buffer, MAX_LINE_LENGTH, "%-10s%-6s%-6s%-7s%-11s%-10s%-10s%-10s%-12s%-10s", "id",
             "time", "again", "dgain", "isp_dgain", "time_f", "again_f", "dgain_f", "ispdgain_f",
             "iso");
    string_printf(result, buffer);
    string_printf(result, "\n");

    AIQ_MAP_FOREACH(self->_effecting_exp_map, pItem, rm) {
        if (!pItem) goto out;

        memset(buffer, 0, MAX_LINE_LENGTH);

        AiqSensorExpInfo_t* pExp = *((AiqSensorExpInfo_t**)(pItem->_pData));
        snprintf(buffer, MAX_LINE_LENGTH, "%-10d%-6d%-6d%-7d%-11d%-10.6f%-10.2f%-10.2f%-12.2f%-10d",
                 pExp->_base.frame_id,
                 pExp->aecExpInfo.HdrExp[0].exp_sensor_params.coarse_integration_time,
                 pExp->aecExpInfo.HdrExp[0].exp_sensor_params.analog_gain_code_global,
                 pExp->aecExpInfo.HdrExp[0].exp_sensor_params.digital_gain_global,
                 pExp->aecExpInfo.HdrExp[0].exp_sensor_params.isp_digital_gain,
                 pExp->aecExpInfo.HdrExp[0].exp_real_params.integration_time,
                 pExp->aecExpInfo.HdrExp[0].exp_real_params.analog_gain,
                 pExp->aecExpInfo.HdrExp[0].exp_real_params.digital_gain,
                 pExp->aecExpInfo.HdrExp[0].exp_real_params.isp_dgain,
                 pExp->aecExpInfo.HdrExp[0].exp_real_params.iso);
        string_printf(result, buffer);
        string_printf(result, "\n");
    }

out:
    string_printf(result, "\n");
}

void __dump_configured_hdr_middle_exp(AiqSensorHw_t* self, st_string* result) {
    char buffer[MAX_LINE_LENGTH] = {0};

    aiq_info_dump_title(result, "sensor configured hdr-middle exp");

    AiqMapItem_t* pItem = NULL;
    bool rm             = false;
    snprintf(buffer, MAX_LINE_LENGTH, "%-10s%-6s%-6s%-7s%-11s%-10s%-10s%-10s%-12s%-10s", "id",
             "time", "again", "dgain", "isp_dgain", "time_f", "again_f", "dgain_f", "ispdgain_f",
             "iso");
    string_printf(result, buffer);
    string_printf(result, "\n");

    AIQ_MAP_FOREACH(self->_effecting_exp_map, pItem, rm) {
        if (!pItem) goto out;

        memset(buffer, 0, MAX_LINE_LENGTH);

        AiqSensorExpInfo_t* pExp = *((AiqSensorExpInfo_t**)(pItem->_pData));
        snprintf(buffer, MAX_LINE_LENGTH, "%-10d%-6d%-6d%-7d%-11d%-10.6f%-10.2f%-10.2f%-12.2f%-10d",
                 pExp->_base.frame_id,
                 pExp->aecExpInfo.HdrExp[1].exp_sensor_params.coarse_integration_time,
                 pExp->aecExpInfo.HdrExp[1].exp_sensor_params.analog_gain_code_global,
                 pExp->aecExpInfo.HdrExp[1].exp_sensor_params.digital_gain_global,
                 pExp->aecExpInfo.HdrExp[1].exp_sensor_params.isp_digital_gain,
                 pExp->aecExpInfo.HdrExp[1].exp_real_params.integration_time,
                 pExp->aecExpInfo.HdrExp[1].exp_real_params.analog_gain,
                 pExp->aecExpInfo.HdrExp[1].exp_real_params.digital_gain,
                 pExp->aecExpInfo.HdrExp[1].exp_real_params.isp_dgain,
                 pExp->aecExpInfo.HdrExp[1].exp_real_params.iso);
        string_printf(result, buffer);
        string_printf(result, "\n");
    }

out:
    string_printf(result, "\n");
}

void __dump_configured_hdr_long_exp(AiqSensorHw_t* self, st_string* result) {
    char buffer[MAX_LINE_LENGTH] = {0};

    aiq_info_dump_title(result, "sensor configured hdr-long exp");

    AiqMapItem_t* pItem = NULL;
    bool rm             = false;
    snprintf(buffer, MAX_LINE_LENGTH, "%-10s%-6s%-6s%-7s%-11s%-10s%-10s%-10s%-12s%-10s", "id",
             "time", "again", "dgain", "isp_dgain", "time_f", "again_f", "dgain_f", "ispdgain_f",
             "iso");
    string_printf(result, buffer);
    string_printf(result, "\n");

    AIQ_MAP_FOREACH(self->_effecting_exp_map, pItem, rm) {
        if (!pItem) goto out;

        memset(buffer, 0, MAX_LINE_LENGTH);

        AiqSensorExpInfo_t* pExp = *((AiqSensorExpInfo_t**)(pItem->_pData));
        snprintf(buffer, MAX_LINE_LENGTH, "%-10d%-6d%-6d%-7d%-11d%-10.6f%-10.2f%-10.2f%-12.2f%-10d",
                 pExp->_base.frame_id,
                 pExp->aecExpInfo.HdrExp[2].exp_sensor_params.coarse_integration_time,
                 pExp->aecExpInfo.HdrExp[2].exp_sensor_params.analog_gain_code_global,
                 pExp->aecExpInfo.HdrExp[2].exp_sensor_params.digital_gain_global,
                 pExp->aecExpInfo.HdrExp[2].exp_sensor_params.isp_digital_gain,
                 pExp->aecExpInfo.HdrExp[2].exp_real_params.integration_time,
                 pExp->aecExpInfo.HdrExp[2].exp_real_params.analog_gain,
                 pExp->aecExpInfo.HdrExp[2].exp_real_params.digital_gain,
                 pExp->aecExpInfo.HdrExp[2].exp_real_params.isp_dgain,
                 pExp->aecExpInfo.HdrExp[2].exp_real_params.iso);
        string_printf(result, buffer);
        string_printf(result, "\n");
    }

out:
    string_printf(result, "\n");
}

void sensor_dump_configured_exp(AiqSensorHw_t* self, st_string* result) {
    if (self->_working_mode == RK_AIQ_WORKING_MODE_NORMAL) {
        __dump_configured_linear_exp(self, result);
    } else {
        __dump_configured_hdr_short_exp(self, result);
        __dump_configured_hdr_middle_exp(self, result);
        if (RK_AIQ_HDR_GET_WORKING_MODE(self->_working_mode) == RK_AIQ_WORKING_MODE_ISP_HDR3)
            __dump_configured_hdr_long_exp(self, result);
    }
}
