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

#ifndef _AIQ_NOTIFIER_H_
#define _AIQ_NOTIFIER_H_

#include "common/list.h"
#include "dumpcam_server/info/include/st_string.h"

/**
 * enum v4l2_async_match_type - type of asynchronous subdevice logic to be used
 *  in order to identify a match
 *
 * @V4L2_ASYNC_MATCH_CUSTOM: Match will use the logic provided by &struct
 *  v4l2_async_subdev.match ops
 * @V4L2_ASYNC_MATCH_DEVNAME: Match will use the device name
 * @V4L2_ASYNC_MATCH_I2C: Match will check for I2C adapter ID and address
 * @V4L2_ASYNC_MATCH_FWNODE: Match will use firmware node
 *
 * This enum is used by the asyncrhronous sub-device logic to define the
 * algorithm that will be used to match an asynchronous device.
 */
enum aiq_notifier_match_type {
    AIQ_NOTIFIER_MATCH_HWI_BASE = 0,
    AIQ_NOTIFIER_MATCH_HWI_STREAM_CAP,
    AIQ_NOTIFIER_MATCH_HWI_STREAM_PROC,
    AIQ_NOTIFIER_MATCH_HWI_SENSOR,
    AIQ_NOTIFIER_MATCH_HWI_ISP_PARAMS,
    AIQ_NOTIFIER_MATCH_ALL,
    AIQ_NOTIFIER_MATCH_MAX,
};

/**
 * @struct aiq_notifier_subscriber - aiq notifier subscriber
 *
 * @owner:
 * @dump:
 * @sub_list:
 */
struct aiq_notifier_subscriber {
    enum aiq_notifier_match_type match_type;
    const char* name;
    struct {
        int (*dump_fn_t)(void* dumper, st_string* dump_info, int argc, void* argv[]);
        void* dumper;
    } dump;

    struct list_head list;
};

#define AIQ_NOTIFIER_SUBSCRIBER_INIT(type, dump_fn, dumper) \
    { .match_type = type, .dump.dump_fn_t = dump_fn, .dump.dumper = dumper }

/**
 * @struct aiq_notifier - aiq notifier data
 *
 * @sub_list:
 */
struct aiq_notifier {
    struct list_head sub_list;
};

/**
 * aiq_notifier_init - Initialize a notifier.
 *
 * @notifier: pointer to &struct v4l2_async_notifier
 *
 * This function initializes the notifier @list. It must be called
 * before the first call to @v4l2_async_notifier_add_subdev.
 */
void aiq_notifier_init(struct aiq_notifier* notifier);

/**
 * aiq_notifier_add_subscriber - Add an subdev to the
 *              notifier's master asd list.
 *
 * @notifier: pointer to &struct v4l2_async_notifier
 * @asd: pointer to &struct v4l2_async_subdev
 *
 * Call this function before registering a notifier to link the provided @asd to
 * the notifiers master @asd_list. The @asd must be allocated with k*alloc() as
 * it will be freed by the framework when the notifier is destroyed.
 */
int aiq_notifier_add_subscriber(struct aiq_notifier* notifier, struct aiq_notifier_subscriber* sub);

/**
 *  aiq_notifier_notify_dumpinfo - Add an subdev to the
 *              notifier's master asd list.
 *
 * @notifier: pointer to &struct v4l2_async_notifier
 * @asd: pointer to &struct v4l2_async_subdev
 *
 * Call this function before registering a notifier to link the provided @asd to
 * the notifiers master @asd_list. The @asd must be allocated with k*alloc() as
 * it will be freed by the framework when the notifier is destroyed.
 */

int aiq_notifier_notify_dumpinfo(struct aiq_notifier* notifier, int type, st_string* dump_info,
                                 int argc, void* argv[]);

#endif  // _AIQ_NOTIFIER_H_
