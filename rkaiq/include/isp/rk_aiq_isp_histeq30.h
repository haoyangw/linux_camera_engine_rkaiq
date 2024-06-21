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

#ifndef _RK_AIQ_PARAM_HISTEQ30_H_
#define _RK_AIQ_PARAM_HISTEQ30_H_

#define HIST_SIGMA_LUT_NUM     17

typedef struct {
    /* M4_GENERIC_DESC(
        M4_ALIAS(blocks_cols),
        M4_TYPE(u8),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(4,10),
        M4_DEFAULT(4),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(Even number only. The horizontal num of the block for histogram statistics.\n
        Freq of use: low))  */
    uint8_t hw_histc_blocks_cols;
    /* M4_GENERIC_DESC(
        M4_ALIAS(blocks_rows),
        M4_TYPE(u8),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(4,8),
        M4_DEFAULT(4),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(Even number only. The vertical num of the block for histogram statistics.\n
        Freq of use: low))  */
    uint8_t hw_histc_blocks_rows;
} histeq_stats_params_static_t;

typedef struct {
    /* M4_GENERIC_DESC(
    M4_ALIAS(stats),
    M4_TYPE(struct),
    M4_UI_MODULE(normal_ui_style),
    M4_HIDE_EX(0),
    M4_RO(0),
    M4_ORDER(2),
    M4_NOTES(All histeq stats params in static.))  */
    histeq_stats_params_static_t stats;
} histeq_params_static_t;

typedef struct {
    /* M4_GENERIC_DESC(
        M4_ALIAS(noiseCount_offset),
        M4_TYPE(u8),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,255),
        M4_DEFAULT(64),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The offset of noise curve.\n Freq of use: low))  */
    uint8_t hw_histc_noiseCount_offset;
    /* M4_GENERIC_DESC(
        M4_ALIAS(noiseCount_scale),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,7),
        M4_DEFAULT(2),
        M4_DIGIT_EX(2f4b),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The scale of noise curve.\n Freq of use: low))  */
    float sw_histc_noiseCount_scale;
    /* M4_GENERIC_DESC(
        M4_ALIAS(countWgt_minLimit),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,1),
        M4_DEFAULT(0.015),
        M4_DIGIT_EX(4f8b),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(The min limit of histogram statistics.\n Freq of use: low))  */
    float sw_histc_countWgt_minLimit;
} histeq_stats_params_t;

typedef struct {
    /* M4_GENERIC_DESC(
    M4_ALIAS(mapUserSet),
    M4_TYPE(f32),
    M4_SIZE_EX(1,1),
    M4_RANGE_EX(0,1),
    M4_DEFAULT(0.2),
    M4_DIGIT_EX(2f6b),
    M4_HIDE_EX(0),
    M4_RO(0),
    M4_ORDER(0),
    M4_NOTES(The user-config scale of histeq.\n Freq of use: high))  */
    float sw_hist_mapUserSet;
    /* M4_GENERIC_DESC(
    M4_ALIAS(mapCount_scale),
    M4_TYPE(f32),
    M4_SIZE_EX(1,1),
    M4_RANGE_EX(0,1),
    M4_DEFAULT(0.09),
    M4_DIGIT_EX(2f8b),
    M4_HIDE_EX(0),
    M4_RO(0),
    M4_ORDER(0),
    M4_NOTES(The auto scale of histeq.\n Freq of use: high))  */
    float sw_hist_mapCount_scale;
    /* M4_GENERIC_DESC(
    M4_ALIAS(mapMerge_alpha),
    M4_TYPE(f32),
    M4_SIZE_EX(1,1),
    M4_RANGE_EX(0,1),
    M4_DEFAULT(2),
    M4_DIGIT_EX(2f8b),
    M4_HIDE_EX(0),
    M4_RO(0),
    M4_ORDER(0),
    M4_NOTES(In the fusion operation of sw_hist_mapCount_scale and sw_hist_mapUserSet, the
    fusion weight value of sw_hist_mapUserSet.\n Freq of use: high))  */
    float sw_hist_mapMerge_alpha;
} histeq_mapping_params_t;

typedef struct {
    /* M4_GENERIC_DESC(
    M4_ALIAS(MapTflt_invSigma),
    M4_TYPE(f32),
    M4_SIZE_EX(1,1),
    M4_RANGE_EX(1,255),
    M4_DEFAULT(6),
    M4_DIGIT_EX(0f8b),
    M4_HIDE_EX(0),
    M4_RO(0),
    M4_ORDER(0),
    M4_NOTES(The iir coefficient of histeq mapping curve.\n
        Freq of use: low))  */
    uint8_t sw_hist_MapTflt_invSigma;
    /* M4_GENERIC_DESC(
    M4_ALIAS(paramTfilt_curWgt),
    M4_TYPE(f32),
    M4_SIZE_EX(1,1),
    M4_RANGE_EX(1,15),
    M4_DEFAULT(8),
    M4_DIGIT_EX(0f8b),
    M4_HIDE_EX(0),
    M4_RO(0),
    M4_ORDER(0),
    M4_NOTES(The bilateral sigma of dark channel.\n Freq of use: low))  */
    float hw_hist_paramTfilt_curWgt;
} histeq_iir_params_t;

typedef struct {
    /* M4_GENERIC_DESC(
    M4_ALIAS(gainRef_sel),
    M4_TYPE(f32),
    M4_SIZE_EX(1,1),
    M4_RANGE_EX(0,0.99),
    M4_DEFAULT(0.5),
    M4_DIGIT_EX(2f8b),
    M4_HIDE_EX(0),
    M4_RO(0),
    M4_ORDER(0),
    M4_NOTES(TODO.\n Freq of use: low))  */
    float hw_hist_gainRef_sel;
    /* M4_GENERIC_DESC(
    M4_ALIAS(hw_hist_globalMergeWeight_en),
    M4_TYPE(bool),
    M4_GROUP_CTRL(globalMergeWeight_en_group),
    M4_DEFAULT(1),
    M4_HIDE_EX(0),
    M4_RO(0),
    M4_ORDER(0),
    M4_NOTES(The enable of using sw_hist_globalMergePos_weight and sw_hist_globalMergeNeg_weight.\n
    Freq of use: low))	*/
    bool hw_hist_globalMergeWeight_en;
    /* M4_GENERIC_DESC(
    M4_ALIAS(globalMergePos_weight),
    M4_TYPE(f32),
    M4_GROUP(globalMergeWeight_en_group),
    M4_SIZE_EX(1,1),
    M4_RANGE_EX(0,4),
    M4_DEFAULT(0.5),
    M4_DIGIT_EX(2f8b),
    M4_HIDE_EX(0),
    M4_RO(0),
    M4_ORDER(0),
    M4_NOTES(TODO.\n Freq of use: low))  */
    float sw_hist_globalMergePos_weight;
    /* M4_GENERIC_DESC(
    M4_ALIAS(globalMergeNeg_weight),
    M4_TYPE(f32),
    M4_GROUP(globalMergeWeight_en_group),
    M4_SIZE_EX(1,1),
    M4_RANGE_EX(0,4),
    M4_DEFAULT(0.5),
    M4_DIGIT_EX(2f8b),
    M4_HIDE_EX(0),
    M4_RO(0),
    M4_ORDER(0),
    M4_NOTES(TODO.\n Freq of use: low))  */
    float sw_hist_globalMergeNeg_weight;
    /* M4_GENERIC_DESC(
    M4_ALIAS(outputMerge_pos_alpha),
    M4_TYPE(f32),
    M4_UI_MODULE(drc_curve),
    M4_SIZE_EX(1,17),
    M4_RANGE_EX(0.0,4.0),
    M4_DEFAULT([1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0]),
    M4_DATAX([0, 64, 128, 192, 256, 320, 384, 448, 512, 576, 640, 704, 768, 832, 896, 960, 1024]),
    M4_DIGIT_EX(2f6b),
    M4_HIDE_EX(0),
    M4_RO(0),
    M4_ORDER(0),
    M4_NOTES(TODO.\n Freq of use: low))  */
    float sw_hist_outputMerge_pos_alpha[17];
    /* M4_GENERIC_DESC(
    M4_ALIAS(outputMerge_neg_alpha),
    M4_TYPE(f32),
    M4_UI_MODULE(drc_curve),
    M4_SIZE_EX(1,17),
    M4_RANGE_EX(0.0,4.0),
    M4_DEFAULT([1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0]),
    M4_DATAX([0, 64, 128, 192, 256, 320, 384, 448, 512, 576, 640, 704, 768, 832, 896, 960, 1024]),
    M4_DIGIT_EX(2f6b),
    M4_HIDE_EX(0),
    M4_RO(0),
    M4_ORDER(0),
    M4_NOTES(TODO.\n Freq of use: low))  */
    float sw_hist_outputMerge_neg_alpha[17];
} histeq_mergeWeit_params_t;

typedef struct {
    /* M4_GENERIC_DESC(
    M4_ALIAS(stats),
    M4_TYPE(struct),
    M4_UI_MODULE(normal_ui_style),
    M4_HIDE_EX(0),
    M4_RO(0),
    M4_ORDER(2),
    M4_NOTES(All histeq stats params corresponded with iso array))  */
    histeq_stats_params_t stats;
    /* M4_GENERIC_DESC(
    M4_ALIAS(mapping),
    M4_TYPE(struct),
    M4_UI_MODULE(normal_ui_style),
    M4_HIDE_EX(0),
    M4_RO(0),
    M4_ORDER(2),
    M4_NOTES(All histeq stats params corresponded with iso array))  */
    histeq_mapping_params_t mapping;
    /* M4_GENERIC_DESC(
    M4_ALIAS(iir),
    M4_TYPE(struct),
    M4_UI_MODULE(normal_ui_style),
    M4_HIDE_EX(0),
    M4_RO(0),
    M4_ORDER(2),
    M4_NOTES(All histeq stats params corresponded with iso array))  */
    histeq_iir_params_t iir;
    /* M4_GENERIC_DESC(
    M4_ALIAS(mergeWeit),
    M4_TYPE(struct),
    M4_UI_MODULE(normal_ui_style),
    M4_HIDE_EX(0),
    M4_RO(0),
    M4_ORDER(2),
    M4_NOTES(All histeq stats params corresponded with iso array))  */
    histeq_mergeWeit_params_t mergeWeit;
    /* M4_GENERIC_DESC(
    M4_ALIAS(saturate_scale),
    M4_TYPE(f32),
    M4_SIZE_EX(1,1),
    M4_RANGE_EX(0,7.99),
    M4_DEFAULT(1),
    M4_DIGIT_EX(2f5b),
    M4_HIDE_EX(0),
    M4_RO(0),
    M4_ORDER(0),
    M4_NOTES(The saturate strength of histeq.\n Freq of use: high))  */
    float sw_hist_saturate_scale;
} histeq_params_dyn_t;

typedef struct {
    /* M4_GENERIC_DESC(
    M4_ALIAS(sta),
    M4_TYPE(struct),
    M4_UI_MODULE(static_ui),
    M4_HIDE_EX(0),
    M4_RO(0),
    M4_ORDER(1),
    M4_NOTES(The static params of histeq module))  */
    histeq_params_static_t sta;
    /* M4_GENERIC_DESC(
    M4_ALIAS(dyn),
    M4_TYPE(struct),
    M4_UI_MODULE(dynamic_ui),
    M4_HIDE_EX(0),
    M4_RO(0),
    M4_ORDER(2),
    M4_NOTES(All dynamic params array corresponded with iso array))  */
    histeq_params_dyn_t dyn;
} histeq_param_t;

#endif
