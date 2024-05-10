/*
* ldch_types_prvt.h

* for rockchip v2.0.0
*
*  Copyright (c) 2019 Rockchip Corporation
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
/* for rockchip v2.0.0*/

#ifndef __RKAIQ_TYPES_Ldch_ALGO_PRVT_H__
#define __RKAIQ_TYPES_Ldch_ALGO_PRVT_H__

/**
 * @file ldch_types_prvt.h
 *
 * @brief
 *
 *****************************************************************************/
/**
 * @page module_name_page Module Name
 * Describe here what this module does.
 *
 * For a detailed list of functions and implementation detail refer to:
 * - @ref module_name
 *
 * @defgroup Ldch Auto debayer
 * @{
 *
 */
#include "include/ldch_algo_api.h"
#include "RkAiqCalibDbTypes.h"
#include "RkAiqCalibDbTypesV2.h"
#include "RkAiqCalibDbV2Helper.h"
#include <xcam_mutex.h>
#include "xcam_thread.h"
#include "smartptr.h"
#include "safe_list.h"
#include "xcam_log.h"
#include "gen_mesh/genMesh.h"
#include "rk_aiq_types_priv.h"

#define DEFAULT_RECALCULATE_DELTA_ISO (0.01)

#define MAP_TO_255LEVEL(level, level_max) \
    ((float)(level) / 255 * (level_max));

typedef enum {
    LDCH_CORRECT_LEVEL0,        // 100%
    LDCH_CORRECT_LEVEL1,        // 75%
    LDCH_CORRECT_LEVEL2,        // 100%
    LDCH_CORRECT_LEVEL3,        // 75%
    LDCH_BYPASS
} LDCHCorrectLevel;

typedef enum LDCHState_e {
    LDCH_STATE_INVALID           = 0,                   /**< initialization value */
    LDCH_STATE_INITIALIZED       = 1,                   /**< instance is created, but not initialized */
    LDCH_STATE_STOPPED           = 2,                   /**< instance is confiured (ready to start) or stopped */
    LDCH_STATE_RUNNING           = 3,                   /**< instance is running (processes frames) */
    LDCH_STATE_MAX                                      /**< max */
} LDCHState_t;

class LutCache {
 public:
    LutCache() = delete;
    explicit LutCache(size_t size)
        : _addr(nullptr), _size(size) {}
    LutCache(const LutCache&) = delete;
    LutCache& operator=(const LutCache&) = delete;
    ~LutCache() {
        ReleaseBuffer();
    }

    void ReleaseBuffer() {
        if (_addr) {
            LOGV_ALDCH("release lut cache size: %d", _size);
            xcam_free(_addr);
            _addr = nullptr;
        }
    }

    void* GetBuffer() {
        if (!_addr) {
            LOGV_ALDCH("malloc lut cache size: %d", _size);
            _addr = xcam_malloc(_size);
        }

        return _addr;
    }

    size_t GetSize() {
        LOGV_ALDCH("get lut cache size: %d", _size);
        return _size;
    }

 private:
    void*   _addr;
    size_t  _size;
};

class RKAiqLdchThread;

typedef struct LdchContext_s {
    const RkAiqAlgoCom_prepare_t* prepare_params;
    ldch_api_attrib_t* ldch_attrib;
    int iso;
    bool isReCal_;
    bool ldch_en;
    uint32_t src_width;
    uint32_t src_height;
    uint32_t dst_width;
    uint32_t dst_height;
    uint32_t lut_h_size;
    uint32_t lut_v_size;
    uint32_t lut_mapxy_size;
    uint16_t* lut_mapxy;
    char meshfile[256];
    int correct_level;
    int correct_level_max;
    const char* resource_path;
    std::atomic<bool> genLdchMeshInit;

    struct CameraCoeff camCoeff;
    LdchParams ldchParams;
    LDCHState_t eState;
    std::atomic<bool> isLutUpdated;

    SmartPtr<RKAiqLdchThread> ldchReadMeshThread;
    isp_drv_share_mem_ops_t *share_mem_ops;
    void* share_mem_ctx;
    rk_aiq_ldch_share_mem_info_t *ldch_mem_info[2];
    int32_t update_lut_mem_fd[2];
    int32_t ready_lut_mem_fd[2];

    uint8_t frm_end_dis;
    uint8_t zero_interp_en;
    uint8_t sample_avr_en;
    uint8_t bic_mode_en;
    uint8_t force_map_en;
    uint8_t map13p3_en;
    uint8_t bicubic[ISP32_LDCH_BIC_NUM];

    std::atomic<bool> hasAllocShareMem;
    SmartPtr<LutCache> _lutCache;

    bool is_multi_isp {false};
    uint8_t multi_isp_extended_pixel {0};
    uint8_t multi_isp_number {1};
    struct CameraCoeff camCoeff_left;
    struct CameraCoeff camCoeff_right;
    LdchParams ldchParams_left;
    LdchParams ldchParams_right;
} LdchContext_t;

class RKAiqLdchThread
    : public Thread {
public:
    RKAiqLdchThread(LdchContext_t* ldchHandle)
        : Thread("ldchThread")
          , hLDCH(ldchHandle) {};
    ~RKAiqLdchThread() {
        mAttrQueue.clear ();
    };

    void triger_stop() {
        mAttrQueue.pause_pop ();
    };

    void triger_start() {
        mAttrQueue.resume_pop ();
    };

    bool push_attr (const SmartPtr<ldch_api_attrib_t> buffer) {
        mAttrQueue.push (buffer);
        return true;
    };

    bool is_empty () {
        return mAttrQueue.is_empty();
    };

    void clear_attr () {
        mAttrQueue.clear ();
    };

protected:
    //virtual bool started ();
    virtual void stopped () {
        mAttrQueue.clear ();
    };
    virtual bool loop ();
private:
    LdchContext_t* hLDCH;
    SafeList<ldch_api_attrib_t> mAttrQueue;
};

#endif//__RKAIQ_TYPES_Ldch_ALGO_PRVT_H__
