/*
 *   Copyright (c) 2024 Rockchip Corporation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include "common/aiq_notifier.h"

#include "dumpcam_server/info/include/rk_info_utils.h"
#include "dumpcam_server/info/include/st_string.h"

static int aiq_notifier_subscriber_valid(struct aiq_notifier_subscriber* sub) {
    switch (sub->match_type) {
        case AIQ_NOTIFIER_MATCH_HWI_BASE:
        case AIQ_NOTIFIER_MATCH_HWI_STREAM_CAP:
        case AIQ_NOTIFIER_MATCH_HWI_STREAM_PROC:
        case AIQ_NOTIFIER_MATCH_HWI_SENSOR:
        case AIQ_NOTIFIER_MATCH_HWI_ISP_PARAMS:
            break;
        default:
            printf("Invalid match type %u on %s\n", sub->match_type, sub->name);
            return -1;
    }

    return 0;
}

void aiq_notifier_init(struct aiq_notifier* notifier) { INIT_LIST_HEAD(&notifier->sub_list); }

int aiq_notifier_add_subscriber(struct aiq_notifier* notifier,
                                struct aiq_notifier_subscriber* sub) {
    int ret;

    // mutex_lock(&list_lock);

    ret = aiq_notifier_subscriber_valid(sub);
    if (ret) goto unlock;

    list_add_tail(&sub->list, &notifier->sub_list);

unlock:
    // mutex_unlock(&list_lock);
    return ret;
}

int aiq_notifier_notify_dumpinfo(struct aiq_notifier* notifier, int type, st_string* dump_info,
                                 int argc, void* argv[]) {
    struct aiq_notifier_subscriber *sub, *tmp;
    char buffer[MAX_LINE_LENGTH] = {0};

    list_for_each_entry_safe(sub, tmp, &notifier->sub_list, list) {
        if (type != AIQ_NOTIFIER_MATCH_ALL && (enum aiq_notifier_match_type)type != sub->match_type)
            continue;

        if (sub->match_type != AIQ_NOTIFIER_MATCH_HWI_STREAM_CAP &&
            sub->match_type != AIQ_NOTIFIER_MATCH_HWI_STREAM_PROC) {
            snprintf(buffer, MAX_LINE_LENGTH, "[%s]:", sub->name);
            string_printf(dump_info, buffer);
            string_printf(dump_info, "\n");
            sub->dump.dump_fn_t(sub->dump.dumper, dump_info, argc, argv);
            string_printf(dump_info, "\n");
        } else {
            sub->dump.dump_fn_t(sub->dump.dumper, dump_info, argc, argv);
        }
    }

    return 0;
}
