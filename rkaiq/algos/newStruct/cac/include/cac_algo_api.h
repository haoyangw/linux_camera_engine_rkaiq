#ifndef _CAC_ALOG_API_H_
#define _CAC_ALOG_API_H_

#include "rk_aiq_algo_des.h"

#if RKAIQ_HAVE_CAC_V11
#include "isp/rk_aiq_isp_cac21.h"
#elif RKAIQ_HAVE_CAC_V30
#include "isp/rk_aiq_isp_cac30.h"
#else
#error "wrong cac hw version !"
#endif

#include "algos/rk_aiq_api_types_cac.h"

#define DIV_ROUND_UP(n, d) (((n) + (d)-1) / (d))
#define CAC_PSF_BUF_NUM 2
#define BITS_PER_BYTE   8
#define BYTES_PER_WORD  4
#define BITS_PER_WORD   (BITS_PER_BYTE * BYTES_PER_WORD)

static const uint32_t IspBigModeWidthLimit = 1536;
static const uint32_t IspBigModeSizeLimit  = IspBigModeWidthLimit * 864;
static const uint32_t CacPsfCountLimit     = 336;
static const uint8_t CacChannelCount       = 2;
static const uint8_t CacScaleFactorDefault = 64;
static const uint8_t CacScaleFactorBigMode = 128;
static const uint16_t CacScaleFactor256Mode = 256;
static const uint8_t CacStrengthDistance   = 128;
static const uint8_t CacPsfKernelSize      = 7 * 5;
static const uint8_t CacPsfKernelWordSizeInMemory =
    DIV_ROUND_UP((CacPsfKernelSize - 1) * BITS_PER_BYTE, BITS_PER_WORD);
static const uint8_t CacPsfBufferCount = CAC_PSF_BUF_NUM;

typedef enum CacHwVersion_e
{
    CAC_HARDWARE_V1 = 0,
    CAC_HARDWARE_MAX,
} CacHwVersion_t;

XCAM_BEGIN_DECLARE

#if 0
XCamReturn
algo_cac_SetAttrib
(
    RkAiqAlgoContext* ctx,
    cac_api_attrib_t *attr
);

XCamReturn
algo_cac_GetAttrib
(
    RkAiqAlgoContext*  ctx,
    cac_api_attrib_t *attr
);
#endif
extern RkAiqAlgoDescription g_RkIspAlgoDescCac;
XCAM_END_DECLARE

#endif
