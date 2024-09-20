/*
 * Copyright 2020 Rockchip Electronics Co. LTD
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

#include "c_rk_info_service_control.h"

#include "dumpcam_server/include/rk_aiq_registry.h"
#include "rk_cmd_service_control.h"
#include "uAPI2_c/rk_aiq_api_private_c.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "c_AIQInfoServiceControl"

void AIQInfoServiceControl_Constructor(AIQInfoServiceControl* const me,
                                       fptr_AIQInfoServiceControl_onReceived onReceived_function) {
    me->parser = aiq_create_json_parser_ctx();
    me->onReceived = onReceived_function;
}

void AIQInfoServiceControl_Destructor(AIQInfoServiceControl *const me) {
    aiq_release_json_parser_ctx(me->parser);
}

AIQInfoServiceControl* AIQInfoServiceControlCreate() {
    AIQInfoServiceControl *me = (AIQInfoServiceControl*)malloc(sizeof(AIQInfoServiceControl));

    if (me) {
        AIQInfoServiceControl_Constructor(me, AIQInfoServiceControl_onReceived);
    }

    return me;
}

void AIQInfoServiceControlDestroy(AIQInfoServiceControl *const me) {
    if (!me)
        return;
    AIQInfoServiceControl_Destructor(me);
    free(me);
    return;
}

enum dump_mod_type {
    DUMP_AIQ_HWI,
    DUMP_AIQ_CORE,
    DUMP_AIQ_CAMGROUP,
    DUMP_AIQ_MAX,
};

struct dump_mod_descr {
    enum dump_mod_type type;
    const char *descr;
    struct {
        int (*dump_fn_t)(void *dumper, st_string *dump_info, int argc, void *argv[]);
        void *dumper;
    } dump;
};

static struct dump_mod_descr aiq_modules[] = {
    { DUMP_AIQ_HWI,       "HWI",          { NULL, NULL } },
    { DUMP_AIQ_CORE,      "CORE",         { NULL, NULL } },
    { DUMP_AIQ_CAMGROUP,  "CAM_GROUP",    { NULL, NULL } },
    { DUMP_AIQ_MAX,       "Unknown",      { NULL, NULL } }
};
#ifndef ARRAY_SIZE
#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))
#endif
#define AIQ_MODS ARRAY_SIZE(aiq_modules)

static void __dumpAllModule(rk_aiq_sys_ctx_t* ctx, st_string* dumpInfo, int argc, void* argv[]) {
    if (!ctx) return;

    for (int i = 0; i < (int)AIQ_MODS; i ++) {
        if (aiq_modules[i].type == DUMP_AIQ_HWI) {
            AiqCamHwBase_t* camHw           = ctx->_camHw;
            aiq_modules[i].dump.dumper      = camHw;
            aiq_modules[i].dump.dump_fn_t   = camHw->dump;
        }

        if (!aiq_modules[i].dump.dumper || !aiq_modules[i].dump.dump_fn_t)
            continue;

        aiq_modules[i].dump.dump_fn_t(aiq_modules[i].dump.dumper, dumpInfo, argc, argv);
        string_printf(dumpInfo, "\n");
    }
}

void AIQInfoServiceControl_dumpAllModule(AIQInfoServiceControl* const me, st_string* dumpInfo,
                                         int argc, void* argv[]) {
    if (!me) return;

    RKAIQRegistry* registery = RKAIQRegistry_get();
    if (!registery) return;

    rk_aiq_sys_ctx_t* ctx = registery->mArray[0];
    if (!ctx) return;

    if (ctx->cam_type == RK_AIQ_CAM_TYPE_GROUP) {
        const rk_aiq_camgroup_ctx_t* grpCtx = (rk_aiq_camgroup_ctx_t*)ctx;
        for (int i = 0; i < RK_AIQ_CAM_GROUP_MAX_CAMS; i++)
            __dumpAllModule(grpCtx->cam_ctxs_array[i], dumpInfo, argc, argv);
    } else if (ctx->cam_type == RK_AIQ_CAM_TYPE_SINGLE) {
        for (int i = 0; i < registery->mCnt; i++)
            __dumpAllModule(registery->mArray[i], dumpInfo, argc, argv);
    }

    return;
}

static void __dumpMultiModule(rk_aiq_sys_ctx_t* ctx, st_string* dumpInfo, int argc, void* argv[],
                              char* str) {
    if (!ctx) return;

    for (int i = 0; i < (int)AIQ_MODS; i++) {
        if (!strcasecmp(aiq_modules[i].descr, str)) {
            if (aiq_modules[i].type == DUMP_AIQ_HWI) {
                AiqCamHwBase_t* camHw         = ctx->_camHw;
                aiq_modules[i].dump.dumper    = camHw;
                aiq_modules[i].dump.dump_fn_t = camHw->dump;
            }

            if (!aiq_modules[i].dump.dumper || !aiq_modules[i].dump.dump_fn_t) continue;

            if (i > 1) {
                string_printf(dumpInfo, "[");
                string_printf(dumpInfo, aiq_modules[i].descr);
                string_printf(dumpInfo, "]\n\n");
            }
            aiq_modules[i].dump.dump_fn_t(aiq_modules[i].dump.dumper, dumpInfo, argc, argv);
            string_printf(dumpInfo, "\n");
        }
    }
}

void AIQInfoServiceControl_dumpMultiModule(AIQInfoServiceControl* const me, st_string* dumpInfo,
                                           char* moduleName, int argc, void* argv[]) {
    if (!me) return;

    int modulesCount = 0;
    string_list    *n = NULL;
    string_list    *pos = NULL;
    string_list    *modules = AIQInfoServiceControl_getModules(me, moduleName, '-');

    list_for_each_entry(pos, &modules->list, list) {
        modulesCount++;
    }
    pos = NULL;

    list_for_each_entry(pos, &modules->list, list) {
        RKAIQRegistry* registery = RKAIQRegistry_get();
        if (!registery) return;

        rk_aiq_sys_ctx_t* ctx = registery->mArray[0];
        if (!ctx) return;

        if (ctx->cam_type == RK_AIQ_CAM_TYPE_GROUP) {
            const rk_aiq_camgroup_ctx_t* grpCtx = (rk_aiq_camgroup_ctx_t*)ctx;
            for (int i = 0; i < RK_AIQ_CAM_GROUP_MAX_CAMS; i++)
                __dumpMultiModule(grpCtx->cam_ctxs_array[i], dumpInfo, argc, argv, pos->str);
        } else if (ctx->cam_type == RK_AIQ_CAM_TYPE_SINGLE) {
            for (int i = 0; i < registery->mCnt; i++)
                __dumpMultiModule(registery->mArray[i], dumpInfo, argc, argv, pos->str);
        }
    }
    pos = NULL;

    list_for_each_entry_safe(pos, n, &modules->list, list) {
        list_del(&pos->list);
        if (pos) {
            if (pos->str)
                free(pos->str);
            free(pos);
        }
    }
    if (modules) {
        free(modules);
        modules = NULL;
    }

    return;
}

void AIQInfoServiceControl_onReceived(AIQInfoServiceControl *const me,\
                                     AIQIPCRequest *request, AIQIPCResponse *response) {
    c_AIQJsonObject json = RK_NULL;
    char  *moudleName = RK_NULL;
    char **moduleArgsValue = RK_NULL;
    int    moduleArgsNum = 0;
    st_string *stResult = NULL;
    string_new(stResult);

    aiq_parseText(me->parser, request->getContent(request));
    aiq_json_get_string(me->parser, "module", &moudleName);
    aiq_json_get_string_array(me->parser, "moduleArgs", &moduleArgsValue, &moduleArgsNum);

    if (!moudleName)
        goto out;

    if (!strcmp(moudleName, "all")) {
        AIQInfoServiceControl_dumpAllModule(me, stResult, moduleArgsNum, (void **)moduleArgsValue);
        response->setContent(response, string_body(stResult), string_len(stResult));
    } else if (!strcmp(moudleName, "record")) {
    } else if (!strcmp(moudleName, "cat")) {
        char argvArray[1024];
        memcpy(argvArray, "cat ", 5);
        for (int i = 0; i < moduleArgsNum; i++) {
            snprintf(argvArray + strlen(argvArray), 1024 - strlen(argvArray), "%s ", moduleArgsValue[i]);
        }
        aiq_cmd_service_process(argvArray, stResult);
        response->setContent(response, string_body(stResult), string_len(stResult));
    } else{
        AIQInfoServiceControl_dumpMultiModule(me, stResult, moudleName,  moduleArgsNum, (void **)moduleArgsValue);
        response->setContent(response, string_body(stResult), string_len(stResult));
    }

out:
    if (moduleArgsValue) {
        for (int i = 0; i < moduleArgsNum; i++) {
            if (moduleArgsValue[i])
                free(moduleArgsValue[i]);
        }
        free(moduleArgsValue);
    }

    if (moudleName)
        free(moudleName);

    string_free(stResult);  // important! ut string destroy at here.
}

string_list *AIQInfoServiceControl_getModules(AIQInfoServiceControl *const me,\
                                            const char *moduleName, const char delim) {
    if (!me)
        return NULL;

    string_list *head = NULL;
    char *token;
    char *tmpModuleName = (char *)malloc(strlen(moduleName) + 1);
    if (!tmpModuleName) {
        return head;
    }

    // Initialize the head of the list
    head = (string_list *)malloc(sizeof(string_list));
    if (!head) {
        free(tmpModuleName);
        goto __FREE_TMP_MOD_NAME;
    }
    INIT_LIST_HEAD(&head->list);

    strcpy(tmpModuleName, moduleName);

    token = strtok(tmpModuleName, &delim);

    while (NULL != token) {
        string_list *add = (string_list *)malloc(sizeof(string_list));
        if (add) {
            add->str = (char *)malloc(strlen(token) + 1);
            if (add->str) {
                strcpy(add->str, token);
                list_add_tail(&add->list, &head->list);
            } else {
                free(add);
            }
        }
        token = strtok(NULL, &delim);
    }

__FREE_TMP_MOD_NAME:
    free(tmpModuleName);

    return head;
}
