#include "rk_aiq_uapi_accm_int.h"
#include "accm/rk_aiq_types_accm_algo_prvt.h"

#if RKAIQ_HAVE_CCM_V1
XCamReturn
rk_aiq_uapi_accm_SetAttrib(RkAiqAlgoContext *ctx,
                           const rk_aiq_ccm_attrib_t* attr,
                           bool need_sync)
{

    accm_context_t* ccm_contex = (accm_context_t*)ctx->accm_para;
    ccm_contex->invarMode = ccm_contex->mCurAtt.mode & attr->mode;
    ccm_contex->mCurAtt = *attr;
    ccm_contex->updateAtt = true;
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
rk_aiq_uapi_accm_GetAttrib(const RkAiqAlgoContext *ctx,
                           rk_aiq_ccm_attrib_t *attr)
{

    accm_context_t* ccm_contex = (accm_context_t*)ctx->accm_para;
    memcpy(ccm_contex->mCurAtt.stManual.ccMatrix, ccm_contex->ccmHwConf.matrix, sizeof(ccm_contex->ccmHwConf.matrix));
    memcpy(ccm_contex->mCurAtt.stManual.ccOffsets, ccm_contex->ccmHwConf.offs, sizeof(ccm_contex->ccmHwConf.offs));
    memcpy(ccm_contex->mCurAtt.stManual.y_alpha_curve, ccm_contex->ccmHwConf.alp_y, sizeof(ccm_contex->ccmHwConf.alp_y));
    ccm_contex->mCurAtt.stManual.low_bound_pos_bit = ccm_contex->ccmHwConf.bound_bit;

    memcpy(attr, &ccm_contex->mCurAtt, sizeof(rk_aiq_ccm_attrib_t));

    return XCAM_RETURN_NO_ERROR;
}
#else
XCamReturn
rk_aiq_uapi_accm_SetAttrib(RkAiqAlgoContext *ctx,
                           const rk_aiq_ccm_attrib_t* attr,
                           bool need_sync)
{
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
rk_aiq_uapi_accm_GetAttrib(const RkAiqAlgoContext *ctx,
                           rk_aiq_ccm_attrib_t *attr)
{
    return XCAM_RETURN_NO_ERROR;
}
#endif

#if RKAIQ_HAVE_CCM_V2
XCamReturn
rk_aiq_uapi_accm_v2_SetAttrib(RkAiqAlgoContext *ctx,
                           const rk_aiq_ccm_v2_attrib_t* attr,
                           bool need_sync)
{

    accm_context_t* ccm_contex = (accm_context_t*)ctx->accm_para;
    ccm_contex->invarMode = ccm_contex->mCurAttV2.mode & attr->mode;
    ccm_contex->mCurAttV2 = *attr;
    ccm_contex->updateAtt = true;
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
rk_aiq_uapi_accm_v2_GetAttrib(const RkAiqAlgoContext *ctx,
                           rk_aiq_ccm_v2_attrib_t *attr)
{

    accm_context_t* ccm_contex = (accm_context_t*)ctx->accm_para;

    memcpy(ccm_contex->mCurAttV2.stManual.ccMatrix, ccm_contex->ccmHwConf_v2.matrix,
            sizeof(float)*9);
    memcpy(ccm_contex->mCurAttV2.stManual.ccOffsets, ccm_contex->ccmHwConf_v2.offs,
            sizeof(float)*3);
    memcpy(ccm_contex->mCurAttV2.stManual.y_alpha_curve, ccm_contex->ccmHwConf_v2.alp_y,
            sizeof(float)*CCM_CURVE_DOT_NUM_V2);
    memcpy(ccm_contex->mCurAttV2.stManual.enh_rgb2y_para, ccm_contex->ccmHwConf_v2.enh_rgb2y_para,
            sizeof(unsigned char)*3);
    ccm_contex->mCurAttV2.stManual.enh_adj_en  = ccm_contex->ccmHwConf_v2.enh_adj_en;
    ccm_contex->mCurAttV2.stManual.enh_rat_max = ccm_contex->ccmHwConf_v2.enh_rat_max;
    ccm_contex->mCurAttV2.stManual.highy_adj_en  = ccm_contex->ccmHwConf_v2.highy_adj_en;
    ccm_contex->mCurAttV2.stManual.asym_enable  = ccm_contex->ccmHwConf_v2.asym_adj_en;
    ccm_contex->mCurAttV2.stManual.bound_pos_bit  = ccm_contex->ccmHwConf_v2.bound_bit;
    ccm_contex->mCurAttV2.stManual.right_pos_bit  = ccm_contex->ccmHwConf_v2.right_bit;

    memcpy(attr, &ccm_contex->mCurAttV2, sizeof(rk_aiq_ccm_v2_attrib_t));

    return XCAM_RETURN_NO_ERROR;
}


#else
XCamReturn
rk_aiq_uapi_accm_v2_SetAttrib(RkAiqAlgoContext *ctx,
                           const rk_aiq_ccm_v2_attrib_t* attr,
                           bool need_sync)
{
    return XCAM_RETURN_NO_ERROR;
}

XCamReturn
rk_aiq_uapi_accm_v2_GetAttrib(const RkAiqAlgoContext *ctx,
                           rk_aiq_ccm_v2_attrib_t *attr)
{
    return XCAM_RETURN_NO_ERROR;
}
#endif

XCamReturn
rk_aiq_uapi_accm_QueryCcmInfo(const RkAiqAlgoContext *ctx,
                              rk_aiq_ccm_querry_info_t *ccm_querry_info )
{

    accm_context_t* ccm_contex = (accm_context_t*)ctx->accm_para;
    ccm_querry_info->finalSat = 0;
    memset(ccm_querry_info->ccmname1, 0x0, sizeof(ccm_querry_info->ccmname1));
    memset(ccm_querry_info->ccmname2, 0x0, sizeof(ccm_querry_info->ccmname2));

#if RKAIQ_HAVE_CCM_V1
    if (ccm_contex->ccmHwConf.ccmEnable && ccm_contex->mCurAtt.mode == RK_AIQ_CCM_MODE_AUTO){
        ccm_querry_info->finalSat = ccm_contex->accmRest.fSaturation;
        if (ccm_contex->accmRest.pCcmProfile1) {
            if (ccm_contex->accmRest.pCcmProfile1->name) {
                strcpy(ccm_querry_info->ccmname1, ccm_contex->accmRest.pCcmProfile1->name);
            }
        }
        if (ccm_contex->accmRest.pCcmProfile2) {
            if (ccm_contex->accmRest.pCcmProfile2->name) {
                strcpy(ccm_querry_info->ccmname2, ccm_contex->accmRest.pCcmProfile2->name);
            }
        }
        else
            strcpy(ccm_querry_info->ccmname2, ccm_querry_info->ccmname1);
    }
    memcpy(ccm_querry_info->y_alpha_curve, ccm_contex->ccmHwConf.alp_y, sizeof(ccm_contex->ccmHwConf.alp_y));
    memcpy(ccm_querry_info->ccMatrix, ccm_contex->ccmHwConf.matrix, sizeof(ccm_contex->ccmHwConf.matrix));
    memcpy(ccm_querry_info->ccOffsets, ccm_contex->ccmHwConf.offs, sizeof(ccm_contex->ccmHwConf.offs));
    ccm_querry_info->ccm_en = ccm_contex->ccmHwConf.ccmEnable;
    ccm_querry_info->low_bound_pos_bit = ccm_contex->ccmHwConf.bound_bit;
    ccm_querry_info->right_pos_bit = ccm_contex->ccmHwConf.bound_bit;
    ccm_querry_info->highy_adj_en = true;
    ccm_querry_info->asym_enable = false;
#endif

#if RKAIQ_HAVE_CCM_V2
    if (ccm_contex->ccmHwConf_v2.ccmEnable && ccm_contex->mCurAttV2.mode == RK_AIQ_CCM_MODE_AUTO){
        ccm_querry_info->finalSat = ccm_contex->accmRest.fSaturation;
        if (ccm_contex->accmRest.pCcmProfile1) {
            if (ccm_contex->accmRest.pCcmProfile1->name) {
                strcpy(ccm_querry_info->ccmname1, ccm_contex->accmRest.pCcmProfile1->name);
            }
        }
        if (ccm_contex->accmRest.pCcmProfile2) {
            if (ccm_contex->accmRest.pCcmProfile2->name) {
                strcpy(ccm_querry_info->ccmname2, ccm_contex->accmRest.pCcmProfile2->name);
            }
        }
        else
            strcpy(ccm_querry_info->ccmname2, ccm_querry_info->ccmname1);
    }
    ccm_querry_info->highy_adj_en = ccm_contex->ccmHwConf_v2.highy_adj_en;
    ccm_querry_info->asym_enable  = ccm_contex->ccmHwConf_v2.asym_adj_en;
    memcpy(ccm_querry_info->y_alpha_curve, ccm_contex->ccmHwConf_v2.alp_y,
           sizeof(ccm_contex->ccmHwConf_v2.alp_y));
    memcpy(ccm_querry_info->ccMatrix, ccm_contex->ccmHwConf_v2.matrix,
           sizeof(ccm_contex->ccmHwConf_v2.matrix));
    memcpy(ccm_querry_info->ccOffsets, ccm_contex->ccmHwConf_v2.offs,
           sizeof(ccm_contex->ccmHwConf_v2.offs));
    ccm_querry_info->ccm_en            = ccm_contex->ccmHwConf_v2.ccmEnable;
    ccm_querry_info->low_bound_pos_bit = ccm_contex->ccmHwConf_v2.bound_bit;
    ccm_querry_info->right_pos_bit     = ccm_contex->ccmHwConf_v2.right_bit;
#endif

    ccm_querry_info->color_inhibition_level = ccm_contex->accmRest.color_inhibition_level;
    ccm_querry_info->color_saturation_level = ccm_contex->accmRest.color_saturation_level;

    return XCAM_RETURN_NO_ERROR;
}
#if RKAIQ_HAVE_CCM_V1
static XCamReturn AccmApiCalibFree(accm_context_t* ctx,
                            const rk_aiq_ccm_calib_attrib_t* attr) {
#elif RKAIQ_HAVE_CCM_V2
static XCamReturn AccmApiCalibFree(accm_context_t* ctx,
                            const rk_aiq_ccm_v2_calib_attrib_t* attr) {
#endif
    LOGI_ACCM("%s: (enter)\n", __FUNCTION__);
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
#if RKAIQ_HAVE_CCM_V1
    CalibDbV2_Ccm_Para_V2_t* ApiCalib = &ctx->ApiCalib_v1;
#elif RKAIQ_HAVE_CCM_V2
    CalibDbV2_Ccm_Para_V32_t* ApiCalib = &ctx->ApiCalib_v2;
#endif
    if (ApiCalib->TuningPara.aCcmCof &&
            (attr->aCcmCof_len != ApiCalib->TuningPara.aCcmCof_len)) {
        for (int i = 0; i < ApiCalib->TuningPara.aCcmCof_len; i++) {
            for (int j = 0; j < ApiCalib->TuningPara.aCcmCof[i].matrixUsed_len; j++)
                free(ApiCalib->TuningPara.aCcmCof[i].matrixUsed[j]);
            free(ApiCalib->TuningPara.aCcmCof[i].matrixUsed);
            free(ApiCalib->TuningPara.aCcmCof[i].name);
        }
        free(ApiCalib->TuningPara.aCcmCof);
        ApiCalib->TuningPara.aCcmCof = NULL;
    }

    for (int i = 0; i < ApiCalib->TuningPara.aCcmCof_len; i++) {
        if (ApiCalib->TuningPara.aCcmCof[i].matrixUsed &&
            (attr->aCcmCof[i].matrixUsed_len !=
            ApiCalib->TuningPara.aCcmCof[i].matrixUsed_len)) {
            for (int j = 0; j < ApiCalib->TuningPara.aCcmCof[i].matrixUsed_len; j++)
                free(ApiCalib->TuningPara.aCcmCof[i].matrixUsed[j]);
            free(ApiCalib->TuningPara.aCcmCof[i].matrixUsed);
            ApiCalib->TuningPara.aCcmCof[i].matrixUsed = NULL;
        }
    }


    if (ApiCalib->TuningPara.matrixAll &&
            (attr->matrixAll_len != ApiCalib->TuningPara.matrixAll_len)) {
        for (int i = 0; i < ApiCalib->TuningPara.matrixAll_len; i++) {
            free(ApiCalib->TuningPara.matrixAll[i].name);
            free(ApiCalib->TuningPara.matrixAll[i].illumination);
        }
        free(ApiCalib->TuningPara.matrixAll);
        ApiCalib->TuningPara.matrixAll = NULL;
    }

    LOGI_ACCM("%s: (exit)\n", __FUNCTION__);
    return ret;
}

#if RKAIQ_HAVE_CCM_V1
XCamReturn
rk_aiq_uapi_accm_SetIqParam(RkAiqAlgoContext *ctx,
                           const rk_aiq_ccm_calib_attrib_t* attr,
                           bool need_sync) {
#elif RKAIQ_HAVE_CCM_V2
XCamReturn
rk_aiq_uapi_accm_SetIqParam(RkAiqAlgoContext *ctx,
                           const rk_aiq_ccm_v2_calib_attrib_t* attr,
                           bool need_sync) {
#endif
    accm_context_t* ccm_contex = (accm_context_t*)ctx->accm_para;
    AccmApiCalibFree(ccm_contex, attr);
    #if RKAIQ_HAVE_CCM_V1
        CalibDbV2_Ccm_Para_V2_t* ApiCalib = &ccm_contex->ApiCalib_v1;
        memcpy(&ApiCalib->lumaCCM, &attr->lumaCCM, sizeof(CalibDbV2_Ccm_Luma_Ccm_t));
    #elif RKAIQ_HAVE_CCM_V2
        CalibDbV2_Ccm_Para_V32_t* ApiCalib = &ccm_contex->ApiCalib_v2;
        memcpy(&ApiCalib->lumaCCM, &attr->lumaCCM, sizeof(CalibDbV2_Ccm_Luma_Ccm_V2_t));
    #endif
    memcpy(&ApiCalib->control, &attr->control, sizeof(CalibDbV2_Ccm_Control_Para_t));
    ApiCalib->TuningPara.damp_enable = attr->damp_enable;
    CalibDbV2_Ccm_Tuning_Para_t* TuningPara = &ApiCalib->TuningPara;
    TuningPara->aCcmCof_len = attr->aCcmCof_len;
    TuningPara->matrixAll_len = attr->matrixAll_len;
    if (!TuningPara->aCcmCof_len || !TuningPara->matrixAll_len) {
        return XCAM_RETURN_ERROR_PARAM;
    }
    if (TuningPara->aCcmCof == NULL) {
        TuningPara->aCcmCof = (CalibDbV2_Ccm_Accm_Cof_Para_t*)malloc(
            sizeof(CalibDbV2_Ccm_Accm_Cof_Para_t) * TuningPara->aCcmCof_len);
        for (int i = 0; i < TuningPara->aCcmCof_len; i++)
        {
            TuningPara->aCcmCof[i].name = (char*)malloc(sizeof(char)*CCM_ILLUMINATION_NAME);
            TuningPara->aCcmCof[i].matrixUsed_len = attr->aCcmCof[i].matrixUsed_len;
            TuningPara->aCcmCof[i].matrixUsed = (char**)malloc(sizeof(char*)*CCM_PROFILES_NUM_MAX);
            for (int j = 0; j < TuningPara->aCcmCof[i].matrixUsed_len; j++)
                TuningPara->aCcmCof[i].matrixUsed[j] = (char*)malloc(sizeof(char)*CCM_PROFILE_NAME);
        }
    }
    for (int i = 0; i < TuningPara->aCcmCof_len; i++)
    {
        if (TuningPara->aCcmCof[i].matrixUsed == NULL) {
            TuningPara->aCcmCof[i].matrixUsed =
                (char**)malloc(sizeof(char*)*CCM_PROFILES_NUM_MAX);
            for (int j = 0; j < TuningPara->aCcmCof[i].matrixUsed_len; j++)
                TuningPara->aCcmCof[i].matrixUsed[j] =
                    (char*)malloc(sizeof(char)*CCM_PROFILE_NAME);
        }
    }
    for (int i = 0; i < TuningPara->aCcmCof_len; i++) {
        strcpy(TuningPara->aCcmCof[i].name, attr->aCcmCof[i].name);

        memcpy(TuningPara->aCcmCof[i].awbGain,
               attr->aCcmCof[i].awbGain,
               sizeof(TuningPara->aCcmCof[i].awbGain));

        TuningPara->aCcmCof[i].minDist = attr->aCcmCof[i].minDist;

        for (int j = 0; j < TuningPara->aCcmCof[i].matrixUsed_len; j ++) {
            sprintf(TuningPara->aCcmCof[i].matrixUsed[j], "%s",
                   attr->aCcmCof[i].matrixUsed[j]);
        }
        TuningPara->aCcmCof[i].gain_sat_curve = attr->aCcmCof[i].gain_sat_curve;
    }
    if (TuningPara->matrixAll == NULL) {
        TuningPara->matrixAll = (CalibDbV2_Ccm_Matrix_Para_t*)malloc(
            sizeof(CalibDbV2_Ccm_Matrix_Para_t) * TuningPara->matrixAll_len);
        for ( int i = 0; i < TuningPara->matrixAll_len; i++){
            TuningPara->matrixAll[i].name =  (char*)malloc(sizeof(char)*CCM_PROFILE_NAME);
            TuningPara->matrixAll[i].illumination = (char*)malloc(sizeof(char)*CCM_ILLUMINATION_NAME);
        }
    }

    for (int i = 0; i < TuningPara->matrixAll_len; i++) {
        strcpy(TuningPara->matrixAll[i].name, attr->matrixAll[i].name);
        strcpy(TuningPara->matrixAll[i].illumination,
               attr->matrixAll[i].illumination);

        memcpy(TuningPara->matrixAll[i].ccMatrix,
               attr->matrixAll[i].ccMatrix,
               sizeof(TuningPara->matrixAll[i].ccMatrix));

        memcpy(TuningPara->matrixAll[i].ccOffsets,
               attr->matrixAll[i].ccOffsets,
               sizeof(TuningPara->matrixAll[i].ccOffsets));

        TuningPara->matrixAll[i].saturation = attr->matrixAll[i].saturation;
    }
    ccm_contex->isApiUpdateCalib = true;

    return XCAM_RETURN_NO_ERROR;
}

#if RKAIQ_HAVE_CCM_V1
XCamReturn
rk_aiq_uapi_accm_GetIqParam(const RkAiqAlgoContext *ctx,
                           rk_aiq_ccm_calib_attrib_t* attr) {
#elif RKAIQ_HAVE_CCM_V2
XCamReturn
rk_aiq_uapi_accm_GetIqParam(const RkAiqAlgoContext *ctx,
                           rk_aiq_ccm_v2_calib_attrib_t* attr) {
#endif

    accm_context_t* ccm_contex = (accm_context_t*)ctx->accm_para;
    #if RKAIQ_HAVE_CCM_V1
        const CalibDbV2_Ccm_Para_V2_t* calib = ccm_contex->ccm_v1;
        XCAM_ASSERT(calib != nullptr);
        memcpy(&attr->lumaCCM, &calib->lumaCCM, sizeof(CalibDbV2_Ccm_Luma_Ccm_t));
    #elif RKAIQ_HAVE_CCM_V2
        const CalibDbV2_Ccm_Para_V32_t* calib = ccm_contex->ccm_v2;
        XCAM_ASSERT(calib != nullptr);
        memcpy(&attr->lumaCCM, &calib->lumaCCM, sizeof(CalibDbV2_Ccm_Luma_Ccm_V2_t));
    #endif
    memcpy(&attr->control, &calib->control, sizeof(CalibDbV2_Ccm_Control_Para_t));
    attr->damp_enable = calib->TuningPara.damp_enable;
    const CalibDbV2_Ccm_Tuning_Para_t* TuningPara = &calib->TuningPara;
    attr->aCcmCof_len = TuningPara->aCcmCof_len;
    attr->matrixAll_len = TuningPara->matrixAll_len;
    if (!TuningPara->aCcmCof_len || !TuningPara->matrixAll_len ||
        TuningPara->aCcmCof == NULL || TuningPara->matrixAll == NULL) {
        return XCAM_RETURN_ERROR_PARAM;
    }

    for (int i = 0; i < TuningPara->aCcmCof_len; i++) {
        strcpy(attr->aCcmCof[i].name, TuningPara->aCcmCof[i].name);

        memcpy(attr->aCcmCof[i].awbGain,
               TuningPara->aCcmCof[i].awbGain,
               sizeof(TuningPara->aCcmCof[i].awbGain));

        attr->aCcmCof[i].minDist = TuningPara->aCcmCof[i].minDist;
        attr->aCcmCof[i].matrixUsed_len = TuningPara->aCcmCof[i].matrixUsed_len;

        if (!attr->aCcmCof[i].matrixUsed_len)
            return XCAM_RETURN_ERROR_PARAM;
        for (int j = 0; j < TuningPara->aCcmCof[i].matrixUsed_len; j ++) {
            sprintf(attr->aCcmCof[i].matrixUsed[j], "%s",
                   TuningPara->aCcmCof[i].matrixUsed[j]);
        }
        attr->aCcmCof[i].gain_sat_curve = TuningPara->aCcmCof[i].gain_sat_curve;
    }

    for (int i = 0; i < TuningPara->matrixAll_len; i++) {
        strcpy(attr->matrixAll[i].name, TuningPara->matrixAll[i].name);
        strcpy(attr->matrixAll[i].illumination,
               TuningPara->matrixAll[i].illumination);

        memcpy(attr->matrixAll[i].ccMatrix,
               TuningPara->matrixAll[i].ccMatrix,
               sizeof(TuningPara->matrixAll[i].ccMatrix));

        memcpy(attr->matrixAll[i].ccOffsets,
               TuningPara->matrixAll[i].ccOffsets,
               sizeof(TuningPara->matrixAll[i].ccOffsets));

        attr->matrixAll[i].saturation = TuningPara->matrixAll[i].saturation;
    }

    return XCAM_RETURN_NO_ERROR;
}

