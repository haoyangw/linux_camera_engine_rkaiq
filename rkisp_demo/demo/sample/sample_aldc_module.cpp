/*
 *  Copyright (c) 2021 Rockchip Corporation
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

#include "sample_comm.h"

#include "uAPI2/rk_aiq_user_api2_aldc.h"

static void sample_aldc_usage()
{
    printf("Usage : \n");
    printf("\t 0) ALDC:         test the correction level of ALDC iteratively in sync.\n");
    printf("\t 1) ALDC:         test the correction level of ALDC iteratively in async.\n");
    printf("\t 2) ALDC:         enter the correction level manually.\n");
    printf("\n");

    printf("\t h) ALDC: help.\n");
    printf("\t q/Q) ALDC:       return to main sample screen.\n");
    printf("\n");
    printf("\t please press the key: \n\n");

    return;
}

void sample_print_aldc_info(const void *arg)
{
    printf ("enter ALDC test!\n");
}

XCamReturn sample_aldc_en(const rk_aiq_sys_ctx_t* ctx, bool en)
{
    XCamReturn ret = XCAM_RETURN_BYPASS;
    if (ctx == NULL) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_SAMPLE_CHECK_RET(ret, "param error!");
    }
    return ret;
}

XCamReturn sample_aldc_setCorrectLevel(const rk_aiq_sys_ctx_t* ctx, int correctLevel,  rk_aiq_uapi_mode_sync_e sync)
{
    XCamReturn ret = XCAM_RETURN_NO_ERROR;
    if (ctx == NULL) {
        ret = XCAM_RETURN_ERROR_PARAM;
        RKAIQ_SAMPLE_CHECK_RET(ret, "param error!");
    }
    rk_aiq_ldc_attrib_t ldcAttr;
    memset(&ldcAttr, 0, sizeof(ldcAttr));
    ret = rk_aiq_user_api2_aldc_GetAttrib(ctx, &ldcAttr);
    RKAIQ_SAMPLE_CHECK_RET(ret, "get ldc attrib failed!");
    ldcAttr.sync.sync_mode = sync;
    ldcAttr.ldch.correct_level = correctLevel;
    ldcAttr.ldcv.correct_level = correctLevel;
    ret = rk_aiq_user_api2_aldc_SetAttrib(ctx, &ldcAttr);
    ret = rk_aiq_user_api2_aldc_GetAttrib(ctx, &ldcAttr);
    RKAIQ_SAMPLE_CHECK_RET(ret, "get ldch attrib failed!");

    printf ("sync_mode: %d, level: %d, done: %d\n", ldcAttr.sync.sync_mode, correctLevel, ldcAttr.sync.done);

    return ret;
}

XCamReturn sample_aldc_module (const void *arg)
{
    int key = -1;
    CLEAR();

    const demo_context_t *demo_ctx = (demo_context_t *)arg;
    const rk_aiq_sys_ctx_t* ctx;
    if (demo_ctx->camGroup) {
        ctx = (rk_aiq_sys_ctx_t*)(demo_ctx->camgroup_ctx);
    } else {
        ctx = (rk_aiq_sys_ctx_t*)(demo_ctx->aiq_ctx);
    }

    if (ctx == nullptr) {
        ERR ("%s, ctx is nullptr\n", __FUNCTION__);
        return XCAM_RETURN_ERROR_PARAM;
    }

    sample_aldc_usage ();
    do {
        key = getchar ();
        while (key == '\n' || key == '\r')
            key = getchar ();
        printf ("\n");

        switch (key)
        {
        case 'h':
            CLEAR();
            sample_aldc_usage ();
            break;
        case '0':
            printf("test the correction level of ALDCH iteratively in sync mode...\n");
            for (int level = 1; level <= 255; level++) {
                usleep(100*1000);
                sample_aldc_setCorrectLevel(ctx, level, RK_AIQ_UAPI_MODE_DEFAULT);
            }
            printf("end of the test\n\n");
            break;
        case '1':
            printf("test the correction level of ALDCH iteratively in async mode...\n");
            for (int level = 1; level <= 255; level++) {
                usleep(100*1000);
                sample_aldc_setCorrectLevel(ctx, level, RK_AIQ_UAPI_MODE_ASYNC);
            }
            printf("end of the test\n\n");
            break;
        case '2':
            static int32_t level = 0;
            if (level == 0)
                level = 200;
            else
                level = 0;

            printf("test the correction level : %d\n", level);
            sample_aldc_setCorrectLevel(ctx, level, RK_AIQ_UAPI_MODE_DEFAULT);
        default:
            break;
        }
    } while (key != 'q' && key != 'Q');

    return XCAM_RETURN_NO_ERROR;
}
