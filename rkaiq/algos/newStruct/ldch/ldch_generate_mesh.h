/*
 * rk_aiq_ldch_generate_mesh.h
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
#ifndef LDCH_GENERATE_MESH_H_
#define LDCH_GENERATE_MESH_H_

#include "ldch_types_prvt.h"

RKAIQ_BEGIN_DECLARE

XCamReturn get_ldch_buf(LdchContext_t* pldchCtx, uint8_t isp_id);
XCamReturn release_ldch_buf(LdchContext_t* pldchCtx, uint8_t isp_id);
bool read_mesh_from_file(LdchContext_t* pldchCtx, const char* fileName);
XCamReturn aiqGenLdchMeshInit(LdchContext_t* pldchCtx);
XCamReturn aiqGenLdchMeshDeInit(LdchContext_t* pldchCtx);
bool aiqGenMesh(LdchContext_t* pldchCtx, int32_t level, uint8_t isp_id);
void put_ldch_buf(LdchContext_t* pldchCtx, uint8_t isp_id);

RKAIQ_END_DECLARE

#endif
