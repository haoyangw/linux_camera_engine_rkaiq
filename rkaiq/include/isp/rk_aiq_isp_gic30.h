/*
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

#ifndef _RK_AIQ_PARAM_GIC21_H_
#define _RK_AIQ_PARAM_GIC21_H_

#define GIC_SIGMACURVE_SEGMENT_MAX         15

typedef struct gic_params_dyn_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_gicT_gain_bypass),
        M4_TYPE(bool),
        M4_DEFAULT(0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(TODO.\n
        Freq of use: low))  */
    bool hw_gicT_gain_bypass;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_gicT_pro_mode),
        M4_TYPE(u8),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,1),
        M4_DEFAULT(0),
        M4_DIGIT_EX(0),
        M4_FP_EX(0,11,0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO\n
        Freq of use: low))  */
    uint8_t hw_gicT_pro_mode;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_gicT_medFlt_minThred),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(1,7),
        M4_DEFAULT(2),
        M4_DIGIT_EX(0),
        M4_FP_EX(0,11,0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO\n
        Freq of use: low))  */
    float hw_gicT_medFlt_minThred;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_gicT_medFlt_maxThred),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(1,7),
        M4_DEFAULT(6),
        M4_DIGIT_EX(0),
        M4_FP_EX(0,11,0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO\n
        Freq of use: low))  */
    float hw_gicT_medFlt_maxThred;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_gicT_medFlt_ratio),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,1),
        M4_DEFAULT(1),
        M4_DIGIT_EX(0),
        M4_FP_EX(0,11,0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO\n
        Freq of use: low))  */
    float sw_gicT_medFlt_ratio;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_gicT_medFltUV_minThred),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(1,7),
        M4_DEFAULT(2),
        M4_DIGIT_EX(0),
        M4_FP_EX(0,11,0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO\n
        Freq of use: low))  */
    float hw_gicT_medFltUV_minThred;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_gicT_medFltUV_maxThred),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(1,7),
        M4_DEFAULT(6),
        M4_DIGIT_EX(0),
        M4_FP_EX(0,11,0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO\n
        Freq of use: low))  */
    float hw_gicT_medFltUV_maxThred;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_gicT_medFltUV_ratio),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,1),
        M4_DEFAULT(1),
        M4_DIGIT_EX(0),
        M4_FP_EX(0,11,0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO\n
        Freq of use: low))  */
    float sw_gicT_medFltUV_ratio;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_gicT_manualNoiseCurve_en),
        M4_TYPE(bool),
        M4_DEFAULT(1),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(TODO.\n
        Freq of use: low))  */
    bool  hw_gicT_manualNoiseCurve_en;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_gicT_noiseCurve_scale),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,8),
        M4_DEFAULT(1),
        M4_DIGIT_EX(0),
        M4_FP_EX(0,11,0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO\n
        Freq of use: low))  */
    float sw_gicT_noiseCurve_scale;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_gicT_bfFltWgt_minThred),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,4),
        M4_DEFAULT(0.23),
        M4_DIGIT_EX(0),
        M4_FP_EX(0,11,0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO\n
        Freq of use: low))  */
    float sw_gicT_bfFltWgt_minThred;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_gicT_bfFltWgt_maxThred),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,4),
        M4_DEFAULT(1.43),
        M4_DIGIT_EX(0),
        M4_FP_EX(0,11,0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO\n
        Freq of use: low))  */
    float sw_gicT_bfFltWgt_maxThred;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_gicT_bfFlt_ratio),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,1),
        M4_DEFAULT(1),
        M4_DIGIT_EX(0),
        M4_FP_EX(0,11,0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO\n
        Freq of use: low))  */
    // M4_ARRAY_DESC("sw_gicT_bfFlt_ratio", "f32", M4_SIZE(1,1), M4_RANGE(0,1.0), "1.0",M4_DIGIT(2), M4_DYNAMIC(0))
    float sw_gicT_bfFlt_ratio;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_gicT_bfFlt_rsigma),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,100),
        M4_DEFAULT(1),
        M4_DIGIT_EX(0),
        M4_FP_EX(0,11,0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO\n
        Freq of use: low))  */
    // M4_ARRAY_DESC("sw_gicT_bfFlt_rsigma", "f32", M4_SIZE(1,1), M4_RANGE(0,100.0), "1.0",M4_DIGIT(2), M4_DYNAMIC(0))
    float sw_gicT_bfFlt_rsigma;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_gicT_bfFlt_rsigma),
        M4_TYPE(u32),
        M4_SIZE_EX(1,17),
        M4_RANGE_EX(0, 1023),
        M4_DEFAULT(32),
        M4_DIGIT_EX(0),
        M4_FP_EX(0,11,0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO\n
        Freq of use: low))  */
    uint32_t sw_gicT_bfFlt_vsigma[17];
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_gicT_curve_idx),
        M4_TYPE(u32),
        M4_SIZE_EX(1,8),
        M4_RANGE_EX(0, 1024),
        M4_DEFAULT([0,64,128,256,384,640,896,1024]),
        M4_DIGIT_EX(0),
        M4_FP_EX(0,11,0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO\n
        Freq of use: low))  */
    uint32_t sw_gicT_curve_idx[8];
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_gicT_noise_thred),
        M4_TYPE(u32),
        M4_SIZE_EX(1,8),
        M4_RANGE_EX(0, 511),
        M4_DEFAULT(16),
        M4_DIGIT_EX(0),
        M4_FP_EX(0,11,0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO\n
        Freq of use: low))  */
    uint32_t hw_gicT_noise_thred[8];
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_gicT_noise_minThred),
        M4_TYPE(u32),
        M4_SIZE_EX(1,8),
        M4_RANGE_EX(0, 511),
        M4_DEFAULT(0),
        M4_DIGIT_EX(0),
        M4_FP_EX(0,11,0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO\n
        Freq of use: low))  */
    uint32_t hw_gicT_noise_minThred[8];
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_gicT_manualNoiseThred_en),
        M4_TYPE(bool),
        M4_DEFAULT(1),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(1),
        M4_NOTES(TODO.\n
        Freq of use: low))  */
    bool hw_gicT_manualNoiseThred_en;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_gicT_autoNoiseThred_scale),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,4),
        M4_DEFAULT(0.23),
        M4_DIGIT_EX(0),
        M4_FP_EX(0,11,0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO\n
        Freq of use: low))  */
    // M4_ARRAY_DESC("sw_gicT_autoNoiseThred_scale", "f32", M4_SIZE(1,1), M4_RANGE(0,16.0), "1.0",M4_DIGIT(2), M4_DYNAMIC(0))
    float sw_gicT_autoNoiseThred_scale;
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_gicT_loFltGr_coeff),
        M4_TYPE(u32),
        M4_SIZE_EX(1,4),
        M4_RANGE_EX(0, 31),
        M4_DEFAULT([4,4,4,4]),
        M4_DIGIT_EX(0),
        M4_FP_EX(0,11,0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO\n
        Freq of use: low))  */
    uint32_t hw_gicT_loFltGr_coeff[4];
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_gicT_loFltGb_coeff),
        M4_TYPE(u32),
        M4_SIZE_EX(1,2),
        M4_RANGE_EX(0, 31),
        M4_DEFAULT([6,6]),
        M4_DIGIT_EX(0),
        M4_FP_EX(0,11,0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO\n
        Freq of use: low))  */
    uint32_t hw_gicT_loFltGb_coeff[2];
    /* M4_GENERIC_DESC(
        M4_ALIAS(hw_gicT_loFltThed_coeff),
        M4_TYPE(u32),
        M4_SIZE_EX(1,2),
        M4_RANGE_EX(0, 31),
        M4_DEFAULT([1,2]),
        M4_DIGIT_EX(0),
        M4_FP_EX(0,11,0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO\n
        Freq of use: low))  */
    uint32_t hw_gicT_loFltThed_coeff[2];
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_gicT_global_gain),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,64.0),
        M4_DEFAULT(1.0),
        M4_DIGIT_EX(0),
        M4_FP_EX(0,11,0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO\n
        Freq of use: low))  */
    float sw_gicT_global_gain;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_gicT_globalGain_alpha),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,1.0),
        M4_DEFAULT(0),
        M4_DIGIT_EX(0),
        M4_FP_EX(0,11,0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO\n
        Freq of use: low))  */
    float sw_gicT_globalGain_alpha;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_gicT_localGain_scale),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,1.0),
        M4_DEFAULT(1),
        M4_DIGIT_EX(0),
        M4_FP_EX(0,11,0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO\n
        Freq of use: low))  */
    float sw_gicT_localGain_scale;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_gicT_gain_minThred),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,64.0),
        M4_DEFAULT(0.25),
        M4_DIGIT_EX(0),
        M4_FP_EX(0,11,0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO\n
        Freq of use: low))  */
    float sw_gicT_gain_minThred;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_gicT_gain_maxThred),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,64.0),
        M4_DEFAULT(1),
        M4_DIGIT_EX(0),
        M4_FP_EX(0,11,0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO\n
        Freq of use: low))  */
    float sw_gicT_gain_maxThred;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_gicT_BfFltStrg_minThred),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,8.0),
        M4_DEFAULT(1),
        M4_DIGIT_EX(0),
        M4_FP_EX(0,11,0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO\n
        Freq of use: low))  */
    float sw_gicT_BfFltStrg_minThred;
    /* M4_GENERIC_DESC(
        M4_ALIAS(sw_gicT_BfFltStrg_maxThred),
        M4_TYPE(f32),
        M4_SIZE_EX(1,1),
        M4_RANGE_EX(0,8),
        M4_DEFAULT(1.0),
        M4_DIGIT_EX(0),
        M4_FP_EX(0,11,0),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(0),
        M4_NOTES(TODO\n
        Freq of use: low))  */
    float sw_gicT_BfFltStrg_maxThred;
} gic_params_dyn_t;

typedef struct gic_param_s {
    /* M4_GENERIC_DESC(
        M4_ALIAS(dyn),
        M4_TYPE(struct),
        M4_UI_MODULE(dynamic_ui),
        M4_HIDE_EX(0),
        M4_RO(0),
        M4_ORDER(2),
        M4_NOTES(TODO))  */
    gic_params_dyn_t dyn;
} gic_param_t;

#endif

