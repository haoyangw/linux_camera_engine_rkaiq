/*
 * algo_bayertnr.cpp
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

#include "bayertnr_types_prvt.h"
#include "xcam_log.h"

#include "algo_types_priv.h"
#include "RkAiqCalibDbTypes.h"
#include "RkAiqCalibDbTypesV2.h"
#include "RkAiqCalibDbV2Helper.h"
#include "interpolation.h"
#include "c_base/aiq_base.h"
#include "newStruct/algo_common.h"

#if RKAIQ_HAVE_BAYERTNR_V23
XCamReturn BtnrSelectParam
(
    btnr_param_auto_t *pAuto,
    btnr_param_t* out,
    int iso)
{
    LOGD_ANR("%s(%d): enter!\n", __FUNCTION__, __LINE__);

    if(pAuto == NULL || out == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return XCAM_RETURN_ERROR_PARAM;
    }

    int i = 0;
    int iso_low = 0, iso_high = 0, ilow = 0, ihigh = 0;
    float ratio = 0.0f;
    uint16_t uratio;
    btnr_param_auto_t *paut = pAuto;

    pre_interp(iso, NULL, 0, &ilow, &ihigh, &ratio);
    uratio = ratio * (1 << RATIO_FIXBIT);

    out->dyn.loMd.hw_btnrT_loMedfilt_en = interpolation_bool(
            paut->dyn[ilow].loMd.hw_btnrT_loMedfilt_en, paut->dyn[ihigh].loMd.hw_btnrT_loMedfilt_en, uratio);
    out->dyn.loMd.hw_btnrT_loBayerFilt_en = interpolation_bool(
            paut->dyn[ilow].loMd.hw_btnrT_loBayerFilt_en, paut->dyn[ihigh].loMd.hw_btnrT_loBayerFilt_en, uratio);
    out->dyn.loMd.hw_btnrT_loLumaFilt_en = interpolation_bool(
            paut->dyn[ilow].loMd.hw_btnrT_loLumaFilt_en, paut->dyn[ihigh].loMd.hw_btnrT_loLumaFilt_en, uratio);
    out->dyn.loMd.hw_btnrT_loSigma_scale = interpolation_f32(
            paut->dyn[ilow].loMd.hw_btnrT_loSigma_scale, paut->dyn[ihigh].loMd.hw_btnrT_loSigma_scale, ratio);
    out->dyn.loMd.hw_btnrT_loSigma_offset = interpolation_f32(
            paut->dyn[ilow].loMd.hw_btnrT_loSigma_offset, paut->dyn[ihigh].loMd.hw_btnrT_loSigma_offset, ratio);
    for (i = 0; i < 16; i++) {
        out->dyn.loMd.hw_btnrC_luma2LoSigma_Curve.idx[i] = interpolation_u16(
                    paut->dyn[ilow].loMd.hw_btnrC_luma2LoSigma_Curve.idx[i], paut->dyn[ihigh].loMd.hw_btnrC_luma2LoSigma_Curve.idx[i], uratio);
    }
    for (i = 0; i < 16; i++) {
        out->dyn.loMd.hw_btnrC_luma2LoSigma_Curve.val[i] = interpolation_u16(
                    paut->dyn[ilow].loMd.hw_btnrC_luma2LoSigma_Curve.val[i], paut->dyn[ihigh].loMd.hw_btnrC_luma2LoSigma_Curve.val[i], uratio);
    }
    out->dyn.hiMd.hw_btnrT_hiMedfilt_en = interpolation_bool(
            paut->dyn[ilow].hiMd.hw_btnrT_hiMedfilt_en, paut->dyn[ihigh].hiMd.hw_btnrT_hiMedfilt_en, uratio);
    out->dyn.hiMd.hw_btnrT_hiFiltAbs_mode = ratio > 0.5 ?
                                            paut->dyn[ihigh].hiMd.hw_btnrT_hiFiltAbs_mode : paut->dyn[ilow].hiMd.hw_btnrT_hiFiltAbs_mode;
    out->dyn.hiMd.hw_btnrT_hiFilt_en = interpolation_bool(
                                           paut->dyn[ilow].hiMd.hw_btnrT_hiFilt_en, paut->dyn[ihigh].hiMd.hw_btnrT_hiFilt_en, uratio);
    out->dyn.hiMd.hw_btnrT_hiFilt_mode = ratio > 0.5 ?
                                         paut->dyn[ihigh].hiMd.hw_btnrT_hiFilt_mode : paut->dyn[ilow].hiMd.hw_btnrT_hiFilt_mode;
    out->dyn.hiMd.hw_btnrT_hiSigmaIdxPreFilt_en = interpolation_bool(
                paut->dyn[ilow].hiMd.hw_btnrT_hiSigmaIdxPreFilt_en, paut->dyn[ihigh].hiMd.hw_btnrT_hiSigmaIdxPreFilt_en, uratio);
    for (i = 0; i < 4; i++) {
        out->dyn.hiMd.hw_btnrT_SigmaIdxPreFilt_coeff[i] = interpolation_u8(
                    paut->dyn[ilow].hiMd.hw_btnrT_SigmaIdxPreFilt_coeff[i], paut->dyn[ihigh].hiMd.hw_btnrT_SigmaIdxPreFilt_coeff[i], uratio);
    }
    out->dyn.hiMd.hw_btnrT_hi0Sigma_scale = interpolation_f32(
            paut->dyn[ilow].hiMd.hw_btnrT_hi0Sigma_scale, paut->dyn[ihigh].hiMd.hw_btnrT_hi0Sigma_scale, ratio);
    out->dyn.hiMd.hw_btnrT_hi0Sigma_offset = interpolation_f32(
                paut->dyn[ilow].hiMd.hw_btnrT_hi0Sigma_offset, paut->dyn[ihigh].hiMd.hw_btnrT_hi0Sigma_offset, ratio);
    out->dyn.hiMd.hw_btnrT_hi1Sigma_scale = interpolation_f32(
            paut->dyn[ilow].hiMd.hw_btnrT_hi1Sigma_scale, paut->dyn[ihigh].hiMd.hw_btnrT_hi1Sigma_scale, ratio);
    out->dyn.hiMd.hw_btnrT_hi1Sigma_offset = interpolation_f32(
                paut->dyn[ilow].hiMd.hw_btnrT_hi1Sigma_offset, paut->dyn[ihigh].hiMd.hw_btnrT_hi1Sigma_offset, ratio);
    for (i = 0; i < 16; i++) {
        out->dyn.hiMd.hw_btnrC_luma2HiSigma_Curve.idx[i] = interpolation_u16(
                    paut->dyn[ilow].hiMd.hw_btnrC_luma2HiSigma_Curve.idx[i], paut->dyn[ihigh].hiMd.hw_btnrC_luma2HiSigma_Curve.idx[i], uratio);
    }
    for (i = 0; i < 16; i++) {
        out->dyn.hiMd.hw_btnrC_luma2HiSigma_Curve.val[i] = interpolation_u16(
                    paut->dyn[ilow].hiMd.hw_btnrC_luma2HiSigma_Curve.val[i], paut->dyn[ihigh].hiMd.hw_btnrC_luma2HiSigma_Curve.val[i], uratio);
    }
    out->dyn.hiMd.hw_btnrT_hiDiffStatics_minLimit = interpolation_u16(
                paut->dyn[ilow].hiMd.hw_btnrT_hiDiffStatics_minLimit, paut->dyn[ihigh].hiMd.hw_btnrT_hiDiffStatics_minLimit, uratio);
    out->dyn.frmAlphaWgtMd.sw_btnrT_wgtMdOpt_mode = ratio > 0.5 ?
            paut->dyn[ihigh].frmAlphaWgtMd.sw_btnrT_wgtMdOpt_mode : paut->dyn[ilow].frmAlphaWgtMd.sw_btnrT_wgtMdOpt_mode;
    out->dyn.frmAlphaWgtMd.hw_btnrT_loHi0Diff_scale = interpolation_f32(
                paut->dyn[ilow].frmAlphaWgtMd.hw_btnrT_loHi0Diff_scale, paut->dyn[ihigh].frmAlphaWgtMd.hw_btnrT_loHi0Diff_scale, ratio);
    out->dyn.frmAlphaWgtPk.sw_btnrT_glbPkSigma_scale = interpolation_f32(
                paut->dyn[ilow].frmAlphaWgtPk.sw_btnrT_glbPkSigma_scale, paut->dyn[ihigh].frmAlphaWgtPk.sw_btnrT_glbPkSigma_scale, ratio);
    out->dyn.frmAlphaWgtPk.hw_btnrT_pkSigmaIdxPreFilt_en = interpolation_bool(
                paut->dyn[ilow].frmAlphaWgtPk.hw_btnrT_pkSigmaIdxPreFilt_en, paut->dyn[ihigh].frmAlphaWgtPk.hw_btnrT_pkSigmaIdxPreFilt_en, uratio);
    out->dyn.frmAlphaWgtPk.hw_btnrT_wgtPk_minLimit = interpolation_f32(
                paut->dyn[ilow].frmAlphaWgtPk.hw_btnrT_wgtPk_minLimit, paut->dyn[ihigh].frmAlphaWgtPk.hw_btnrT_wgtPk_minLimit, ratio);
    for (i = 0; i < 16; i++) {
        out->dyn.frmAlphaWgtPk.hw_btnrC_luma2PkSigma_Curve.idx[i] = interpolation_u16(
                    paut->dyn[ilow].frmAlphaWgtPk.hw_btnrC_luma2PkSigma_Curve.idx[i], paut->dyn[ihigh].frmAlphaWgtPk.hw_btnrC_luma2PkSigma_Curve.idx[i], uratio);
    }
    for (i = 0; i < 16; i++) {
        out->dyn.frmAlphaWgtPk.hw_btnrC_luma2PkSigma_Curve.val[i] = interpolation_u16(
                    paut->dyn[ilow].frmAlphaWgtPk.hw_btnrC_luma2PkSigma_Curve.val[i], paut->dyn[ihigh].frmAlphaWgtPk.hw_btnrC_luma2PkSigma_Curve.val[i], uratio);
    }
    out->dyn.frmAlpha.hw_btnrT_frmAlpha_en = interpolation_bool(
                paut->dyn[ilow].frmAlpha.hw_btnrT_frmAlpha_en, paut->dyn[ihigh].frmAlpha.hw_btnrT_frmAlpha_en, uratio);
    out->dyn.frmAlpha.hw_bnrT_softThred_scale = interpolation_f32(
                paut->dyn[ilow].frmAlpha.hw_bnrT_softThred_scale, paut->dyn[ihigh].frmAlpha.hw_bnrT_softThred_scale, ratio);

    return XCAM_RETURN_NO_ERROR;
}
#endif

#if RKAIQ_HAVE_BAYERTNR_V30
XCamReturn BtnrSelectParam
(
    BtnrContext_t *pBtnrCtx,
    btnr_param_t* out,
    int iso)
{
    LOGD_ANR("%s(%d): enter!\n", __FUNCTION__, __LINE__);

    int i = 0;
    int iso_low = 0, iso_high = 0, ilow = 0, ihigh = 0, inear = 0;
    float ratio = 0.0f;
    uint16_t uratio;
    btnr_param_auto_t *paut = &pBtnrCtx->btnr_attrib->stAuto;

    if(paut == NULL || out == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return XCAM_RETURN_ERROR_PARAM;
    }

    pre_interp(iso, NULL, 0, &ilow, &ihigh, &ratio);
    uratio = ratio * (1 << RATIO_FIXBIT);

    if (ratio > 0.5)
        inear = ihigh;
    else
        inear = ilow;

    {
        out->dyn.sigmaEnv.hw_btnrT_statsPixAlpha_thred = interpolation_f32(
                    paut->dyn[ilow].sigmaEnv.hw_btnrT_statsPixAlpha_thred, paut->dyn[ihigh].sigmaEnv.hw_btnrT_statsPixAlpha_thred, ratio);
        out->dyn.sigmaEnv.sw_btnrT_autoSgmIIR_alpha = interpolation_f32(
                    paut->dyn[ilow].sigmaEnv.sw_btnrT_autoSgmIIR_alpha, paut->dyn[ihigh].sigmaEnv.sw_btnrT_autoSgmIIR_alpha, ratio);
        for (i = 0; i < 16; i++) {
            out->dyn.sigmaEnv.hw_btnrC_iirSpNrSgm_curve.idx[i] = paut->dyn[inear].sigmaEnv.hw_btnrC_iirSpNrSgm_curve.idx[i];
            out->dyn.sigmaEnv.hw_btnrC_curSpNrSgm_curve.idx[i] = paut->dyn[inear].sigmaEnv.hw_btnrC_curSpNrSgm_curve.idx[i];

            out->dyn.sigmaEnv.hw_btnrC_curSpNrSgm_curve.val[i] = interpolation_u16(
                        paut->dyn[ilow].sigmaEnv.hw_btnrC_curSpNrSgm_curve.val[i], paut->dyn[ihigh].sigmaEnv.hw_btnrC_curSpNrSgm_curve.val[i], uratio);
            out->dyn.sigmaEnv.hw_btnrC_iirSpNrSgm_curve.val[i] = interpolation_u16(
                        paut->dyn[ilow].sigmaEnv.hw_btnrC_iirSpNrSgm_curve.val[i], paut->dyn[ihigh].sigmaEnv.hw_btnrC_iirSpNrSgm_curve.val[i], uratio);
        }
        for (i = 0; i < 20; i++) {
            out->dyn.sigmaEnv.hw_btnrC_mdSigma_curve.idx[i] = paut->dyn[inear].sigmaEnv.hw_btnrC_mdSigma_curve.idx[i];
            out->dyn.sigmaEnv.hw_btnrC_mdSigma_curve.val[i] = interpolation_u16(
                        paut->dyn[ilow].sigmaEnv.hw_btnrC_mdSigma_curve.val[i], paut->dyn[ihigh].sigmaEnv.hw_btnrC_mdSigma_curve.val[i], uratio);
        }
        out->dyn.curFrmSpNr.hw_btnrT_spNr_en = paut->dyn[inear].curFrmSpNr.hw_btnrT_spNr_en;
        out->dyn.curFrmSpNr.hw_btnrT_sigmaIdxLpf_en = paut->dyn[inear].curFrmSpNr.hw_btnrT_sigmaIdxLpf_en;
        out->dyn.curFrmSpNr.hw_btnrT_sigmaCurve_mode = paut->dyn[inear].curFrmSpNr.hw_btnrT_sigmaCurve_mode;
        out->dyn.curFrmSpNr.hw_btnrT_sigma_scale = paut->dyn[inear].curFrmSpNr.hw_btnrT_sigma_scale;
        out->dyn.curFrmSpNr.hw_btnrT_sigmaHdrS_scale = paut->dyn[inear].curFrmSpNr.hw_btnrT_sigmaHdrS_scale;
        out->dyn.curFrmSpNr.hw_btnrT_sigma_offset = paut->dyn[inear].curFrmSpNr.hw_btnrT_sigma_offset;
        out->dyn.curFrmSpNr.hw_btnrT_sigmaHdrS_offset = paut->dyn[inear].curFrmSpNr.hw_btnrT_sigmaHdrS_offset;
        out->dyn.curFrmSpNr.sw_btnrT_filtSpatial_strg = interpolation_f32(
                    paut->dyn[ilow].curFrmSpNr.sw_btnrT_filtSpatial_strg, paut->dyn[ihigh].curFrmSpNr.sw_btnrT_filtSpatial_strg, ratio);
        out->dyn.curFrmSpNr.hw_btnrT_pixDiff_maxLimit = paut->dyn[inear].curFrmSpNr.hw_btnrT_pixDiff_maxLimit;
        out->dyn.curFrmSpNr.hw_btnrT_pixDiff_negOff = paut->dyn[inear].curFrmSpNr.hw_btnrT_pixDiff_negOff;
        out->dyn.curFrmSpNr.hw_btnrT_pixDiff_scale = paut->dyn[inear].curFrmSpNr.hw_btnrT_pixDiff_scale;
        out->dyn.curFrmSpNr.hw_btnrT_spNrOut_alpha = interpolation_f32(
                    paut->dyn[ilow].curFrmSpNr.hw_btnrT_spNrOut_alpha, paut->dyn[ihigh].curFrmSpNr.hw_btnrT_spNrOut_alpha, ratio);
        out->dyn.preFrmSpNr.hw_btnrT_spNr_en = paut->dyn[inear].preFrmSpNr.hw_btnrT_spNr_en;
        out->dyn.preFrmSpNr.hw_btnrT_sigmaIdxLpf_en = paut->dyn[inear].preFrmSpNr.hw_btnrT_sigmaIdxLpf_en;
        out->dyn.preFrmSpNr.hw_btnrT_sigmaCurve_mode = paut->dyn[inear].preFrmSpNr.hw_btnrT_sigmaCurve_mode;
        out->dyn.preFrmSpNr.hw_btnrT_sigma_scale = paut->dyn[inear].preFrmSpNr.hw_btnrT_sigma_scale;
        out->dyn.preFrmSpNr.hw_btnrT_sigmaHdrS_scale = paut->dyn[inear].preFrmSpNr.hw_btnrT_sigmaHdrS_scale;
        out->dyn.preFrmSpNr.hw_btnrT_sigma_offset = paut->dyn[inear].preFrmSpNr.hw_btnrT_sigma_offset;
        out->dyn.preFrmSpNr.hw_btnrT_sigmaHdrS_offset = paut->dyn[inear].preFrmSpNr.hw_btnrT_sigmaHdrS_offset;
        out->dyn.preFrmSpNr.hw_btnrT_sigma_mode = paut->dyn[inear].preFrmSpNr.hw_btnrT_sigma_mode;
        out->dyn.preFrmSpNr.sw_btnrT_filtSpatial_strg = interpolation_f32(
                    paut->dyn[ilow].preFrmSpNr.sw_btnrT_filtSpatial_strg, paut->dyn[ihigh].preFrmSpNr.sw_btnrT_filtSpatial_strg, ratio);
        out->dyn.preFrmSpNr.hw_btnrT_pixDiff_maxLimit = paut->dyn[inear].preFrmSpNr.hw_btnrT_pixDiff_maxLimit;
        out->dyn.preFrmSpNr.hw_btnrT_pixDiff_negOff = paut->dyn[inear].preFrmSpNr.hw_btnrT_pixDiff_negOff;
        out->dyn.preFrmSpNr.hw_btnrT_pixDiff_scale = paut->dyn[inear].preFrmSpNr.hw_btnrT_pixDiff_scale;
        out->dyn.preFrmSpNr.hw_btnrT_spNrOut_alpha = interpolation_f32(
                    paut->dyn[ilow].preFrmSpNr.hw_btnrT_spNrOut_alpha, paut->dyn[ihigh].preFrmSpNr.hw_btnrT_spNrOut_alpha, ratio);
        out->dyn.hw_btnrT_noiseBal_mode = paut->dyn[inear].hw_btnrT_noiseBal_mode;
        out->dyn.noiseBal_byHiAlpha.hw_btnrT_curHiOrg_alpha = paut->dyn[inear].noiseBal_byHiAlpha.hw_btnrT_curHiOrg_alpha;
        out->dyn.noiseBal_byHiAlpha.hw_btnrT_iirHiOrg_alpha = paut->dyn[inear].noiseBal_byHiAlpha.hw_btnrT_iirHiOrg_alpha;
        out->dyn.noiseBal_byLoAlpha.hw_btnrT_noiseBal_strg = paut->dyn[inear].noiseBal_byLoAlpha.hw_btnrT_noiseBal_strg;
        out->dyn.locSgmStrg.hw_bnrT_locSgmStrg_maxLimit = paut->dyn[inear].locSgmStrg.hw_bnrT_locSgmStrg_maxLimit;
    }

    {
        out->mdDyn.hw_btnrT_md_en = paut->mdDyn[inear].hw_btnrT_md_en;
        out->mdDyn.mdSigma.hw_btnrT_sigma_scale = paut->mdDyn[inear].mdSigma.hw_btnrT_sigma_scale;
        out->mdDyn.mdSigma.hw_btnrT_sigmaHdrS_scale = paut->mdDyn[inear].mdSigma.hw_btnrT_sigmaHdrS_scale;
        out->mdDyn.mdSigma.hw_btnrT_sigmaCurve_mode = paut->mdDyn[inear].mdSigma.hw_btnrT_sigmaCurve_mode;
        out->mdDyn.subLoMd0_diffCh.hw_btnrT_vFilt_en = paut->mdDyn[inear].subLoMd0_diffCh.hw_btnrT_vFilt_en;
        out->mdDyn.subLoMd0_diffCh.hw_btnrT_hFilt_en = paut->mdDyn[inear].subLoMd0_diffCh.hw_btnrT_hFilt_en;
        out->mdDyn.subLoMd0_diffCh.hw_btnrT_vIIRFstLn_scale = paut->mdDyn[inear].subLoMd0_diffCh.hw_btnrT_vIIRFstLn_scale;
        out->mdDyn.subLoMd0_diffCh.hw_btnrT_vIIRFilt_strg = paut->mdDyn[inear].subLoMd0_diffCh.hw_btnrT_vIIRFilt_strg;
        out->mdDyn.subLoMd0_sgmCh.hw_btnrT_hFilt_en = paut->mdDyn[inear].subLoMd0_sgmCh.hw_btnrT_hFilt_en;
        out->mdDyn.subLoMd0_sgmCh.hw_btnrT_vIIRFstLn_scale = paut->mdDyn[inear].subLoMd0_sgmCh.hw_btnrT_vIIRFstLn_scale;
        out->mdDyn.subLoMd0_sgmCh.hw_btnrT_vIIRFilt_strg = paut->mdDyn[inear].subLoMd0_sgmCh.hw_btnrT_vIIRFilt_strg;
        out->mdDyn.subLoMd0_wgtOpt.hw_btnrT_mdWgt_negOff = paut->mdDyn[inear].subLoMd0_wgtOpt.hw_btnrT_mdWgt_negOff;
        out->mdDyn.subLoMd0_wgtOpt.hw_btnrT_mdWgt_scale = paut->mdDyn[inear].subLoMd0_wgtOpt.hw_btnrT_mdWgt_scale;
        out->mdDyn.subLoMd1.hw_btnrT_lpf_en = paut->mdDyn[inear].subLoMd1.hw_btnrT_lpf_en;
        out->mdDyn.subLoMd1.sw_btnrT_lpfCfg_mode = paut->mdDyn[inear].subLoMd1.sw_btnrT_lpfCfg_mode;
        for (i = 0; i < 9; i++) {
            out->mdDyn.subLoMd1.hw_btnrT_lpfSpatial_wgt[i] = interpolation_f32(
                        paut->mdDyn[ilow].subLoMd1.hw_btnrT_lpfSpatial_wgt[i], paut->mdDyn[ihigh].subLoMd1.hw_btnrT_lpfSpatial_wgt[i], ratio);
        }
        out->mdDyn.subLoMd1.sw_btnrT_lpf_strg = paut->mdDyn[inear].subLoMd1.sw_btnrT_lpf_strg;
        out->mdDyn.subLoMd1.hw_btnrT_mdWgt_maxLimit = paut->mdDyn[inear].subLoMd1.hw_btnrT_mdWgt_maxLimit;
        out->mdDyn.subLoMd1.hw_btnrT_mdWgt_negOff = paut->mdDyn[inear].subLoMd1.hw_btnrT_mdWgt_negOff;
        out->mdDyn.subLoMd1.hw_btnrT_mdWgtFstLnNegOff_en = paut->mdDyn[inear].subLoMd1.hw_btnrT_mdWgtFstLnNegOff_en;
        out->mdDyn.subLoMd1.hw_btnrT_mdWgtFstLn_negOff = paut->mdDyn[inear].subLoMd1.hw_btnrT_mdWgtFstLn_negOff;
        out->mdDyn.subLoMd1.hw_btnrT_mdWgt_scale = paut->mdDyn[inear].subLoMd1.hw_btnrT_mdWgt_scale;
        out->mdDyn.subLoMd1.hw_btnrT_hFilt_en = paut->mdDyn[inear].subLoMd1.hw_btnrT_hFilt_en;
        out->mdDyn.subLoMd1.hw_btnrT_vIIRFilt_strg = paut->mdDyn[inear].subLoMd1.hw_btnrT_vIIRFilt_strg;
        out->mdDyn.loMd.hw_btnrT_loMd_mode = paut->mdDyn[inear].loMd.hw_btnrT_loMd_mode;
        out->mdDyn.loMd.hw_btnrT_preWgtMge_scale = paut->mdDyn[inear].loMd.hw_btnrT_preWgtMge_scale;
        out->mdDyn.loMd.hw_btnrT_preWgtMge_offset = paut->mdDyn[inear].loMd.hw_btnrT_preWgtMge_offset;
        out->mdDyn.hw_btnrT_md_mode = paut->mdDyn[inear].hw_btnrT_md_mode;
        out->mdDyn.loAsRatio_hiByMskMd.hw_btnrT_hfLpf_en = paut->mdDyn[inear].loAsRatio_hiByMskMd.hw_btnrT_hfLpf_en;
        out->mdDyn.loAsRatio_hiByMskMd.sw_btnrT_hfLpfCfg_mode = paut->mdDyn[inear].loAsRatio_hiByMskMd.sw_btnrT_hfLpfCfg_mode;
        for (i = 0; i < 9; i++) {
            out->mdDyn.loAsRatio_hiByMskMd.hw_btnrT_hfLpfSpatial_wgt[i] = interpolation_f32(
                        paut->mdDyn[ilow].loAsRatio_hiByMskMd.hw_btnrT_hfLpfSpatial_wgt[i], paut->mdDyn[ihigh].loAsRatio_hiByMskMd.hw_btnrT_hfLpfSpatial_wgt[i], ratio);
        }
        out->mdDyn.loAsRatio_hiByMskMd.sw_btnrT_hfLpf_strg = paut->mdDyn[inear].loAsRatio_hiByMskMd.sw_btnrT_hfLpf_strg;
        out->mdDyn.loAsRatio_hiByMskMd.hw_btnrT_loWgtStat_scale = paut->mdDyn[inear].loAsRatio_hiByMskMd.hw_btnrT_loWgtStat_scale;
        out->mdDyn.loAsRatio_hiByMskMd.hw_btnrT_hiMdWgt_scale = paut->mdDyn[inear].loAsRatio_hiByMskMd.hw_btnrT_hiMdWgt_scale;
        out->mdDyn.loAsRatio_hiByMskMd.hw_btnrT_loWgtStatHdrS_scale  = paut->mdDyn[inear].loAsRatio_hiByMskMd.hw_btnrT_loWgtStatHdrS_scale;
        out->mdDyn.loAsBias_hiBySgmMd.hw_btnrT_loWgtStat_negOff = paut->mdDyn[inear].loAsBias_hiBySgmMd.hw_btnrT_loWgtStat_negOff;
        out->mdDyn.loAsBias_hiBySgmMd.hw_btnrT_loWgtStatHdrS_negOff = paut->mdDyn[inear].loAsBias_hiBySgmMd.hw_btnrT_loWgtStatHdrS_negOff;
        out->mdDyn.loAsBias_hiBySgmMd.hw_btnrT_loWgtStat_scale = paut->mdDyn[inear].loAsBias_hiBySgmMd.hw_btnrT_loWgtStat_scale;
        out->mdDyn.loAsBias_hiBySgmMd.hw_btnrT_mdWgt_offset = paut->mdDyn[inear].loAsBias_hiBySgmMd.hw_btnrT_mdWgt_offset;
        out->mdDyn.loAsBias_hiBySgmMd.hw_btnrT_mdWgt_minLimit = paut->mdDyn[inear].loAsBias_hiBySgmMd.hw_btnrT_mdWgt_minLimit;
        out->mdDyn.loAsBias_hiBySgmMd.hw_btnrT_mdWgt_scale = paut->mdDyn[inear].loAsBias_hiBySgmMd.hw_btnrT_mdWgt_scale;
        out->mdDyn.loAsBias_hiBySgmMd.hw_btnrT_mdWgtHdrS_scale = paut->mdDyn[inear].loAsBias_hiBySgmMd.hw_btnrT_mdWgtHdrS_scale;
        out->mdDyn.loAsBias_hiBySgmMd.hw_btnrT_loWgtStatHdrS_scale  = paut->mdDyn[inear].loAsBias_hiBySgmMd.hw_btnrT_loWgtStatHdrS_scale;
        out->mdDyn.mdWgtPost.hw_btnrT_lpf_en = paut->mdDyn[inear].mdWgtPost.hw_btnrT_lpf_en;
        for (i = 0; i < 3; i++) {
            out->mdDyn.mdWgtPost.hw_btnr_lpfSpatial_wgt[i] = paut->mdDyn[inear].mdWgtPost.hw_btnr_lpfSpatial_wgt[i];
        }
        out->mdDyn.frmAlpha.hw_btnrT_hiAlpha_minLimit = paut->mdDyn[inear].frmAlpha.hw_btnrT_hiAlpha_minLimit;
        out->mdDyn.frmAlpha.hw_btnrT_hiAlphaHdrS_minLimit = paut->mdDyn[inear].frmAlpha.hw_btnrT_hiAlphaHdrS_minLimit;
        out->mdDyn.frmAlpha.hw_btnrT_hiAlpha_maxLimit = paut->mdDyn[inear].frmAlpha.hw_btnrT_hiAlpha_maxLimit;
        out->mdDyn.frmAlpha.hw_btnrT_hiAlphaHdrS_maxLimit = paut->mdDyn[inear].frmAlpha.hw_btnrT_hiAlphaHdrS_maxLimit;
        out->mdDyn.frmAlpha.hw_btnrT_loAlpha_minLimit = paut->mdDyn[inear].frmAlpha.hw_btnrT_loAlpha_minLimit;
        out->mdDyn.frmAlpha.hw_btnrT_loAlphaHdrS_minLimit = paut->mdDyn[inear].frmAlpha.hw_btnrT_loAlphaHdrS_minLimit;
        out->mdDyn.frmAlpha.hw_btnrT_loAlpha_maxLimit = paut->mdDyn[inear].frmAlpha.hw_btnrT_loAlpha_maxLimit;
        out->mdDyn.frmAlpha.hw_btnrT_loAlphaHdrS_maxLimit = paut->mdDyn[inear].frmAlpha.hw_btnrT_loAlphaHdrS_maxLimit;
    }
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn BtnrApplyStrength
(
    BtnrContext_t *pBtnrCtx,
    btnr_param_t* out)
{
    if(pBtnrCtx == NULL || out == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return XCAM_RETURN_ERROR_PARAM;
    }

    if (pBtnrCtx->strength_en) {
        float fPercent = algo_strength_to_percent(pBtnrCtx->fStrength);

        btnr_md_dyn_t* pmdDyn = &out->mdDyn;
        pmdDyn->mdSigma.hw_btnrT_sigma_scale *= fPercent;
        pmdDyn->mdSigma.hw_btnrT_sigmaHdrS_scale *= fPercent;

        LOGD_ANR("BtnrApplyStrength: fStrength %f, fPercent %f\n", pBtnrCtx->fStrength, fPercent);
    }

    return XCAM_RETURN_NO_ERROR;
}
#endif

#if RKAIQ_HAVE_BAYERTNR_V41
XCamReturn BtnrSelectParam
(
    BtnrContext_t *pBtnrCtx,
    btnr_param_t* out,
    int iso)
{
    LOGD_ANR("%s(%d): enter!\n", __FUNCTION__, __LINE__);

    int i = 0;
    int iso_low = 0, iso_high = 0, ilow = 0, ihigh = 0, inear = 0;
    float ratio = 0.0f;
    uint16_t uratio;
    btnr_param_auto_t *paut = &pBtnrCtx->btnr_attrib->stAuto;

    if(paut == NULL || out == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return XCAM_RETURN_ERROR_PARAM;
    }

    pre_interp(iso, NULL, 0, &ilow, &ihigh, &ratio);
    uratio = ratio * (1 << RATIO_FIXBIT);

    if (ratio > 0.5)
        inear = ihigh;
    else
        inear = ilow;

    out->sta = paut->sta;
    out->mdMeDyn = paut->mdMeDyn[inear];
    out->spNrDyn = paut->spNrDyn[inear];

    return XCAM_RETURN_NO_ERROR;
}
#endif

static XCamReturn
create_context(RkAiqAlgoContext **context, const AlgoCtxInstanceCfg* cfg)
{
    XCamReturn result = XCAM_RETURN_NO_ERROR;
    CamCalibDbV2Context_t *pCalibDbV2 = cfg->calibv2;

    BtnrContext_t* ctx = aiq_mallocz(sizeof(BtnrContext_t));
    if (ctx == NULL) {
        LOGE_ANR("%s create context failed!", __func__);
        return XCAM_RETURN_ERROR_MEM;
    }
    ctx->btnr_attrib =
        (btnr_api_attrib_t*)(CALIBDBV2_GET_MODULE_PTR(pCalibDbV2, bayertnr));

    *context = (RkAiqAlgoContext* )ctx;
    LOGV_ANR("%s: Btnr (exit)\n", __FUNCTION__ );

    *context = (RkAiqAlgoContext*)ctx;
    return result;
}

static XCamReturn
destroy_context(RkAiqAlgoContext *context)
{
    XCamReturn result = XCAM_RETURN_NO_ERROR;
    BtnrContext_t* pBtnrCtx = (BtnrContext_t*)context;
    aiq_free(pBtnrCtx);
    return result;
}

static XCamReturn
prepare(RkAiqAlgoCom* params)
{
    XCamReturn result = XCAM_RETURN_NO_ERROR;
    BtnrContext_t* pBtnrCtx = (BtnrContext_t *)params->ctx;

    if(!!(params->u.prepare.conf_type & RK_AIQ_ALGO_CONFTYPE_UPDATECALIB )) {
        // just update calib ptr
        if (params->u.prepare.conf_type & RK_AIQ_ALGO_CONFTYPE_UPDATECALIB_PTR) {
            pBtnrCtx->btnr_attrib =
                (btnr_api_attrib_t*)(CALIBDBV2_GET_MODULE_PTR(params->u.prepare.calibv2, bayertnr));
            return XCAM_RETURN_NO_ERROR;
        }
    }

    pBtnrCtx->working_mode = params->u.prepare.working_mode;
    pBtnrCtx->btnr_attrib =
        (btnr_api_attrib_t*)(CALIBDBV2_GET_MODULE_PTR(params->u.prepare.calibv2, bayertnr));
    pBtnrCtx->isReCal_ = true;
    return result;
}


XCamReturn Abtnr_processing(const RkAiqAlgoCom* inparams, RkAiqAlgoResCom* outparams, int iso)
{
    BtnrContext_t* pBtnrCtx = (BtnrContext_t *)inparams->ctx;
    btnr_api_attrib_t* btnr_attrib = pBtnrCtx->btnr_attrib;
    btnr_param_t* btnr_res = outparams->algoRes;

    if (btnr_attrib->opMode != RK_AIQ_OP_MODE_AUTO) {
        LOGE_ANR("mode is %d, not auto mode, ignore", btnr_attrib->opMode);
        return XCAM_RETURN_NO_ERROR;
    }
    if (btnr_attrib->en == false || btnr_attrib->bypass == true) {
        if (inparams->u.proc.is_attrib_update) {
            outparams->cfg_update = true;
            outparams->en = btnr_attrib->en;
            outparams->bypass = btnr_attrib->bypass;
        }
        LOGD_ANR("BTNR not enable, skip!");
        return XCAM_RETURN_NO_ERROR;
    }

    bool need_recal = pBtnrCtx->isReCal_;

    bool init = inparams->u.proc.init;
    if (inparams->u.proc.is_attrib_update || inparams->u.proc.init) {
        need_recal = true;
    }

    int delta_iso = abs(iso - pBtnrCtx->pre_iso);
    if(delta_iso > 0.01 || init) {
        pBtnrCtx->pre_iso = iso;
        need_recal = true;
    }

    outparams->cfg_update = true;
    if (need_recal) {
        btnr_res->sta = pBtnrCtx->btnr_attrib->stAuto.sta;
#if RKAIQ_HAVE_BAYERTNR_V30
        BtnrSelectParam(pBtnrCtx, btnr_res, iso);
        BtnrApplyStrength(pBtnrCtx, btnr_res);
#endif
#if RKAIQ_HAVE_BAYERTNR_V41
        BtnrSelectParam(pBtnrCtx, btnr_res, iso);
#endif
        outparams->cfg_update = true;
        outparams->en = btnr_attrib->en;
        outparams->bypass = btnr_attrib->bypass;
        LOGD_ANR("BTNR processing: iso %d, btnr en:%d, bypass:%d", iso, outparams->en, outparams->bypass);

        pBtnrCtx->isReCal_ = false;
	} else {
        outparams->en = btnr_attrib->en;
        outparams->bypass = btnr_attrib->bypass;
	}

    return XCAM_RETURN_NO_ERROR;
}

static XCamReturn
processing(const RkAiqAlgoCom* inparams, RkAiqAlgoResCom* outparams)
{
    XCamReturn result = XCAM_RETURN_NO_ERROR;

    int iso = inparams->u.proc.iso;
    Abtnr_processing(inparams, outparams, iso);

    LOGV_ANR("%s: Btnr (exit)\n", __FUNCTION__ );
    return XCAM_RETURN_NO_ERROR;
}

#if 0
XCamReturn
algo_bayertnr_SetAttrib(RkAiqAlgoContext *ctx,
                        const btnr_api_attrib_t *attr,
                        bool need_sync)
{

    if(ctx == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return XCAM_RETURN_ERROR_PARAM;
    }

    BtnrContext_t* pBtnrCtx = (BtnrContext_t*)ctx;
    pBtnrCtx->isReCal_ = true;

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
algo_bayertnr_GetAttrib(const RkAiqAlgoContext *ctx,
                        btnr_api_attrib_t *attr)
{

    if(ctx == NULL || attr == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return XCAM_RETURN_ERROR_PARAM;
    }

    BtnrContext_t* pBtnrCtx = (BtnrContext_t*)ctx;
    btnr_api_attrib_t* btnr_attrib = pBtnrCtx->btnr_attrib;

    attr->opMode = btnr_attrib->opMode;
    attr->en = btnr_attrib->en;
    attr->bypass = btnr_attrib->bypass;
    memcpy(&attr->stAuto, &btnr_attrib->stAuto, sizeof(btnr_param_auto_t));

    return XCAM_RETURN_NO_ERROR;
}
#endif
XCamReturn
algo_bayertnr_SetStrength(RkAiqAlgoContext *ctx, float strg, bool strg_en)
{

    if(ctx == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return XCAM_RETURN_ERROR_PARAM;
    }

    BtnrContext_t* pBtnrCtx = (BtnrContext_t*)ctx;

    pBtnrCtx->fStrength = strg;
    pBtnrCtx->strength_en = strg_en;
    pBtnrCtx->isReCal_ = true;

    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
algo_bayertnr_GetStrength(RkAiqAlgoContext *ctx, float *strg, bool *strg_en)
{

    if(ctx == NULL) {
        LOGE_ANR("%s(%d): null pointer\n", __FUNCTION__, __LINE__);
        return XCAM_RETURN_ERROR_PARAM;
    }

    BtnrContext_t* pBtnrCtx = (BtnrContext_t*)ctx;

    *strg = pBtnrCtx->fStrength;
    *strg_en = pBtnrCtx->strength_en;

    return XCAM_RETURN_NO_ERROR;
}

#define RKISP_ALGO_ABAYERTNR_VERSION     "v0.0.9"
#define RKISP_ALGO_ABAYERTNR_VENDOR      "Rockchip"
#define RKISP_ALGO_ABAYERTNR_DESCRIPTION "Rockchip btnr algo for ISP32"

RkAiqAlgoDescription g_RkIspAlgoDescBayertnr = {
    .common = {
        .version = RKISP_ALGO_ABAYERTNR_VERSION,
        .vendor  = RKISP_ALGO_ABAYERTNR_VENDOR,
        .description = RKISP_ALGO_ABAYERTNR_DESCRIPTION,
        .type    = RK_AIQ_ALGO_TYPE_AMFNR,
        .id      = 0,
        .create_context  = create_context,
        .destroy_context = destroy_context,
    },
    .prepare = prepare,
    .pre_process = NULL,
    .processing = processing,
    .post_process = NULL,
};
