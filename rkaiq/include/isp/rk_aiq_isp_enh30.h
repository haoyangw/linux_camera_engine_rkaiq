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

#ifndef _RK_AIQ_PARAM_ENH30_H_
#define _RK_AIQ_PARAM_ENH30_H_

typedef struct {
    /* M4_GENERIC_DESC(
        M4_ALIAS(iir_inv_sigma),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,1),
        M4_DEFAULT(0.2),
        M4_DIGIT_EX(2f8b),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO.\n Freq of use: low))  */
    float hw_enhT_iir_inv_sigma;
	/* M4_GENERIC_DESC(
        M4_ALIAS(iir_soft_thed),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,31),
        M4_DEFAULT(5),
        M4_DIGIT_EX(0f8b),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO.\n Freq of use: low))  */
    float hw_enhT_iir_soft_thed;
	/* M4_GENERIC_DESC(
        M4_ALIAS(iir_cur_wgt),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,1),
        M4_DEFAULT(0.5),
        M4_DIGIT_EX(2f8b),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO.\n Freq of use: low))  */
    float hw_enhT_iir_cur_wgt;
} enh_iirFlt_params_t;

typedef struct {
    /* M4_GENERIC_DESC(
    M4_ALIAS(blf3_bypass),
    M4_GROUP_CTRL(blf3_bypass_group),
    M4_TYPE(bool),
    M4_DEFAULT(0),
    M4_HIDE_EX(0),
    M4_RO(0),
    M4_ORDER(1),
    M4_NOTES(The enable of color protection.\n
    Freq of use: low))  */
    bool hw_enhT_blf3_bypass;
    /* M4_GENERIC_DESC(
        M4_ALIAS(blf3_inv_sigma),
        M4_GROUP(blf3_bypass_group),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,1),
        M4_DEFAULT(0.04),
        M4_DIGIT_EX(2f8b),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO.\n Freq of use: low))  */
    float hw_enhT_blf3_inv_sigma;
    /* M4_GENERIC_DESC(
    M4_ALIAS(blf3_cur_wgt),
    M4_GROUP(blf3_bypass_group),
    M4_TYPE(f32),
    M4_SIZE_EX(1,1),
    M4_RANGE_EX(0,1),
    M4_DEFAULT(0.5),
    M4_DIGIT_EX(2f8b),
    M4_HIDE_EX(0),
    M4_RO(0),
    M4_ORDER(0),
    M4_NOTES(TODO.\n Freq of use: low))  */
    float hw_enhT_blf3_cur_wgt;
    /* M4_GENERIC_DESC(
    M4_ALIAS(blf3_thumb_cur_wgt),
    M4_GROUP(blf3_bypass_group),
    M4_TYPE(f32),
    M4_SIZE_EX(1,1),
    M4_RANGE_EX(1,15),
    M4_DEFAULT(4),
    M4_DIGIT_EX(0f8b),
    M4_HIDE_EX(0),
    M4_RO(0),
    M4_ORDER(0),
    M4_NOTES(TODO.\n Freq of use: low))  */
    float hw_enhT_blf3_thumb_cur_wgt;
} enh_blf3Flt_params_t;

typedef struct {
    /* M4_GENERIC_DESC(
        M4_ALIAS(blf5_inv_sigma),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,1),
        M4_DEFAULT(0.04),
        M4_DIGIT_EX(2f8b),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO.\n Freq of use: low))  */
    float hw_enhT_blf5_inv_sigma;
	/* M4_GENERIC_DESC(
        M4_ALIAS(blf5_cur_wgt),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,1),
        M4_DEFAULT(0.01),
        M4_DIGIT_EX(2f8b),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO.\n Freq of use: low))  */
    float hw_enhT_blf5_cur_wgt;
} enh_blf5Flt_params_t;

typedef struct enh_detail2strg_curve_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(detail2strg_idx),
        M4_TYPE(u16),
        M4_UI_PARAM(data_x),
        M4_SIZE_EX(1,8),
        M4_RANGE_EX(0,1024),
        M4_DEFAULT([16,32,64,128,256,512,640,1024]),
        M4_DIGIT_EX(0f6b),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The contrast strength of enhance when hw_dhazT_luma2strg_en == 1.\n
    Freq of use: high))  */
    float idx[8];
    /* M4_GENERIC_DESC(
        M4_ALIAS(detail2strg_val),
        M4_TYPE(f32),
        M4_UI_PARAM(data_y),
        M4_SIZE_EX(1,8),
        M4_RANGE_EX(0,1),
        M4_DEFAULT([1, 1, 1, 1, 1, 1, 1, 1]),
        M4_DIGIT_EX(2f6b),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(The contrast strength of enhance when hw_dhazT_luma2strg_en == 1.\n
    Freq of use: high))  */
    float val[8];
} enh_detail2strg_curve_t;

typedef struct {
    /* M4_GENERIC_DESC(
    M4_ALIAS(global_strg),
    M4_TYPE(f32),
    M4_SIZE_EX(1,1),
    M4_RANGE_EX(0,15.99),
    M4_DEFAULT(1),
    M4_DIGIT_EX(2f8b),
    M4_HIDE_EX(0),
    M4_RO(0),
    M4_ORDER(0),
    M4_NOTES(TODO.\n Freq of use: low))  */
    float hw_enhT_global_strg;
    /* M4_GENERIC_DESC(
    M4_ALIAS(diff2strg_en),
    M4_GROUP_CTRL(detail2strg_en_group),
    M4_TYPE(bool),
    M4_DEFAULT(0),
    M4_HIDE_EX(0),
    M4_RO(0),
    M4_ORDER(1),
    M4_NOTES(The enable of using luma as index to control the enhance strength.\n
    Freq of use: low))  */
    bool hw_enhT_detail2strg_en;
    /* M4_GENERIC_DESC(
    M4_ALIAS(hw_enhT_detail2strg_curve),
    M4_GROUP(detail2strg_en_group),
    M4_TYPE(struct),
    M4_UI_MODULE(curve_ui),
    M4_HIDE_EX(0),
    M4_RO(0),
    M4_ORDER(2),
    M4_NOTES(The contrast strength of enhance when hw_dhazT_luma2strg_en == 1.\n
    Freq of use: high))  */
    enh_detail2strg_curve_t hw_enhT_detail2strg_curve;
    /* M4_GENERIC_DESC(
    M4_ALIAS(luma2strg_en),
    M4_GROUP_CTRL(luma2strg_en_group),
    M4_TYPE(bool),
    M4_DEFAULT(0),
    M4_HIDE_EX(0),
    M4_RO(0),
    M4_ORDER(1),
    M4_NOTES(The enable of using luma as index to control the enhance strength.\n
    Freq of use: low))  */
    bool hw_enhT_luma2strg_en;
    /* M4_GENERIC_DESC(
    M4_ALIAS(lum2strg),
    M4_GROUP(luma2strg_en_group),
    M4_TYPE(f32),
    M4_UI_MODULE(drc_curve),
    M4_SIZE_EX(1,17),
    M4_RANGE_EX(0,1),
    M4_DEFAULT([1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1]),
    M4_DATAX([0, 64, 128, 192, 256, 320, 384, 448, 512, 576, 640, 704, 768, 832, 896, 960, 1024]),
    M4_DIGIT_EX(2f6b),
    M4_HIDE_EX(0),
    M4_RO(0),
    M4_ORDER(0),
    M4_NOTES(The contrast strength of enhance when hw_dhazT_luma2strg_en == 1.\n
    Freq of use: high))  */
    float hw_enhT_lum2strg[17];
} enh_strg_params_t;

typedef struct {
    /* M4_GENERIC_DESC(
    M4_ALIAS(iir),
    M4_TYPE(struct),
    M4_UI_MODULE(normal_ui_style),
    M4_HIDE_EX(0),
    M4_RO(0),
    M4_ORDER(2),
    M4_NOTES(All bifilt_filter params corresponded with iso array))  */
    enh_iirFlt_params_t iir;
    /* M4_GENERIC_DESC(
    M4_ALIAS(blf3),
    M4_TYPE(struct),
    M4_UI_MODULE(normal_ui_style),
    M4_HIDE_EX(0),
    M4_RO(0),
    M4_ORDER(2),
    M4_NOTES(All bifilt_filter params corresponded with iso array))  */
    enh_blf3Flt_params_t blf3;
    /* M4_GENERIC_DESC(
    M4_ALIAS(blf5),
    M4_TYPE(struct),
    M4_UI_MODULE(normal_ui_style),
    M4_HIDE_EX(0),
    M4_RO(0),
    M4_ORDER(2),
    M4_NOTES(All bifilt_filter params corresponded with iso array))  */
    enh_blf5Flt_params_t blf5;
    /* M4_GENERIC_DESC(
    M4_ALIAS(strg),
    M4_TYPE(struct),
    M4_UI_MODULE(normal_ui_style),
    M4_HIDE_EX(0),
    M4_RO(0),
    M4_ORDER(2),
    M4_NOTES(All bifilt_filter params corresponded with iso array))  */
    enh_strg_params_t strg;
} enh_params_dyn_t;

typedef struct {
    /* M4_GENERIC_DESC(
        M4_ALIAS(dynamic_param),
        M4_TYPE(struct),
        M4_UI_MODULE(dynamic_ui),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(All dynamic params array corresponded with iso array))  */
    enh_params_dyn_t dyn;
} enh_param_t;

#endif
