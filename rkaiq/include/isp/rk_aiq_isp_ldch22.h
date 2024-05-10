/*
 * sw_param_ldch22.h
 *
 *  Copyright (c) 2023 Rockchip Corporation
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
#ifndef _RK_AIQ_PARAM_LDCH22_H_
#define _RK_AIQ_PARAM_LDCH22_H_

#ifndef ISP32_LDCH_BIC_NUM
#define ISP32_LDCH_BIC_NUM		36
#endif

typedef struct ldch_customLutCfg_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_ldchCfg_lutExtUpdate_en),
        M4_TYPE(bool),
        M4_DEFAULT(1),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO.
		Freq of use: low))  */
    bool sw_ldchCfg_lutExtUpdate_en;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_ldchCfg_lutExtFile_dir),
        M4_TYPE(char),
        M4_SIZE_EX(1,255),
        M4_DEFAULT("default_meshfile"),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO.
		Freq of use: low))  */
    char sw_ldchCfg_lutExtFile_dir[64];
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_ldchCfg_lutExtMeshFile_name),
        M4_TYPE(char),
        M4_SIZE_EX(1,255),
        M4_DEFAULT("default_meshfile"),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO.
		Freq of use: low))  */
    char sw_ldchCfg_lutExtMeshFile_name[32];
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_ldchCfg_lutExtBuf_addr),
        M4_TYPE(char),
        M4_SIZE_EX(1,255),
        M4_DEFAULT("default_meshfile"),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO.
		Freq of use: low))  */
    char* sw_ldchCfg_lutExtBuf_addr;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_ldchCfg_lutExtBuf_size),
        M4_TYPE(u32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,4095),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO.
        Freq of use: low))  */
    uint32_t sw_ldchCfg_lutExtBuf_size;
} ldch_customLutCfg_t;

typedef struct ldch_userCfg_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_ldch_interp_mode),
        M4_TYPE(u8),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,1),
        M4_DEFAULT(2),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO.
        Freq of use: low))  */
    // @reg: hw_ldch_interp_mode
    uint8_t hw_ldchCfg_bicMode_en;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_ldch_interp_mode),
        M4_TYPE(u8),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,1),
        M4_DEFAULT(2),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO.
        Freq of use: low))  */
    // @reg: hw_ldch_interp_mode
    uint8_t hw_ldchCfg_zeroInterp_en;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_ldch_uvDs_mode),
        M4_TYPE(u8),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,1),
        M4_DEFAULT(2),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO.
        Freq of use: low))  */
    // @reg: hw_ldch_uvDs_mode
    uint8_t hw_ldchCfg_sampleAvr_en;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_ldc_bicubic_wgt),
        M4_TYPE(u8),
        M4_SIZE_EX(1,36),
        M4_RANGE_EX(0,10),
        M4_DEFAULT(2),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO.
        Freq of use: low))  */
    // @reg: hw_ldc_bicubic_wgt0 ~ hw_ldc_bicubic_wgt8
    uint8_t hw_ldchCfg_bicWeight_table[ISP32_LDCH_BIC_NUM];
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_ldch_frm_end_dis),
        M4_TYPE(u8),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,1),
        M4_DEFAULT(2),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO.
        Freq of use: low))  */
    // @reg: sw_ldch_frm_end_dis
    uint8_t hw_ldchCfg_frmEndDis_mode;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sys_itself_lut_mode),
        M4_TYPE(u8),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,3),
        M4_DEFAULT(2),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO.
        Freq of use: low))  */
    // @reg: sys_itself_lut_mode
    uint8_t hw_ldchCfg_forceMap_en;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_ldch_map3BitFix_en),
        M4_TYPE(u8),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,1),
        M4_DEFAULT(2),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO.
        Freq of use: low))  */
    // @reg: hw_ldch_map3BitFix_en
    uint8_t hw_ldchCfg_mapFix3Bit_en;
} ldch_userCfg_t;

typedef struct ldch_lutMapCfg_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_ldchT_lutMap_height),
        M4_TYPE(u8),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,255),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO.
        Freq of use: low))  */
    unsigned int sw_ldchT_lutMap_height;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_ldchT_lutMap_width),
        M4_TYPE(u32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,4095),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO.
        Freq of use: low))  */
    unsigned int sw_ldchT_lutMap_width;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_ldchT_lutMap_height),
        M4_TYPE(u32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,4095),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO.
        Freq of use: low))  */
    unsigned int sw_ldchT_lutMap_size;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_ldchT_lutMapBuf_fd),
        M4_TYPE(u32),
        M4_SIZE_EX(1,2),
        M4_RANGE_EX(0,4095),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO.
        Freq of use: low))  */
    unsigned int sw_ldchT_lutMapBuf_fd[2];
} ldch_lutMapCfg_t;

typedef struct ldch_baseCtrl_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_ldchT_meshfile_path),
        M4_TYPE(char),
        M4_SIZE_EX(1,255),
        M4_DEFAULT("default_meshfile"),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO.
		Freq of use: low))  */
    char sw_ldchT_meshfile_path[256];
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_ldchT_correct_level),
        M4_TYPE(u8),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,255),
        M4_DEFAULT(255),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO.
        Freq of use: low))  */
    unsigned char sw_ldchT_correct_strg;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_ldchT_correctLevel_max),
        M4_TYPE(u8),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,255),
        M4_DEFAULT(255),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO.
        Freq of use: low))  */
    unsigned char sw_ldchT_correctStrg_max;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_ldchT_lightCenter_val),
        M4_TYPE(f64),
        M4_SIZE_EX(1,2),
        M4_RANGE_EX(-10000000000000000,10000000000000000),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO.
        Freq of use: low))  */
    double sw_ldchT_lightCenter_val[2];
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_ldchT_lensDistor_coeff),
        M4_TYPE(f64),
        M4_SIZE_EX(1,4),
        M4_RANGE_EX(-10000000000000000,10000000000000000),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(the distortion coefficient of the fisheye lens.
        Freq of use: low))  */
    double sw_ldchT_lensDistor_coeff[4];
} ldch_baseCtrl_t;

typedef enum ldch_updateLutCfg_mode_e{
    ldch_online_mode        = 0,    // generate lut inside rkaiq
    ldch_externalFile_mode,          // external file import lut
    ldch_externalBuf_mode,        // external buffer import lut
} ldch_updateLutCfg_mode_t;

typedef struct {
    /* M4_GENERIC_DESC(
        M4_ALIAS(baseCtrl),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO\n
        Freq of use: low))  */
    ldch_baseCtrl_t baseCtrl;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_ldchCfg_updateLut_mode),
        M4_TYPE(enum),
        M4_ENUM_DEF(ldch_updateLutCfg_mode_t),
        M4_DEFAULT(ldch_online_mode),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(Reference enum types.\n
		Freq of use: low))  */
    ldch_updateLutCfg_mode_t sw_ldchCfg_updateLut_mode;
    /* M4_GENERIC_DESC(
        M4_ALIAS(userCfg),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO\n
        Freq of use: low))  */
    ldch_userCfg_t userCfg;
    /* M4_GENERIC_DESC(
        M4_ALIAS(customLutCfg),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO\n
        Freq of use: low))  */
    ldch_customLutCfg_t customLutCfg;
    /* M4_GENERIC_DESC(
        M4_ALIAS(lutMapCfg),
        M4_TYPE(struct),
        M4_UI_MODULE(normal_ui_style),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO\n
        Freq of use: low))  */
    ldch_lutMapCfg_t lutMapCfg;
} ldch_params_static_t;

typedef struct {
    /* M4_GENERIC_DESC(
        M4_ALIAS(static_param),
        M4_TYPE(struct),
        M4_UI_MODULE(static_ui),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(The static params of demosaic module))  */
    ldch_params_static_t sta;
} ldch_param_t;

#endif