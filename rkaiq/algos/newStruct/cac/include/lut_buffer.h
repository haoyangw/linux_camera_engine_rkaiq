/*
 *  Copyright (c) 2022 Rockchip Electronics Co.,Ltd
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
 * Author: Cody Xie <cody.xie@rock-chips.com>
 */
#ifndef ALGOS_CAC_LUT_BUFFER_H
#define ALGOS_CAC_LUT_BUFFER_H

#include "common/rk_aiq_types_priv_c.h"

typedef enum LutBufferState_e {
    kInitial   = 0,
    kWait2Chip = 1,
    kChipInUse = 2,
} LutBufferState;

typedef struct LutBufferConfig_s {
    bool IsBigMode;
    uint32_t Width;
    uint32_t Height;
    uint32_t LutHCount;
    uint32_t LutVCount;
    uint16_t ScaleFactor;
    uint16_t PsfCfgCount;
} LutBufferConfig;

typedef struct LutBuffer_s {
    LutBufferState State;
    LutBufferConfig Config;
    int Fd;
    int Size;
    void* Addr;
} LutBuffer;

typedef struct LutBufferManager_s {
    const isp_drv_share_mem_ops_t* mem_ops_;
    void* mem_ctx_;
    LutBufferConfig config_;
} LutBufferManager;

#endif  // ALGOS_CAC_LUT_BUFFER_H
