//
// Created by yang on 2020/2/21.
//

#ifndef TENNIS_API_API_H
#define TENNIS_API_API_H

#include "common.h"
#include "device.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ts_op_creator_map;
typedef struct ts_op_creator_map ts_op_creator_map;

struct ts_device_context;
typedef struct ts_device_context ts_device_context;

TENNIS_C_API ts_op_creator_map* ts_get_creator_map();

TENNIS_C_API void ts_flush_creator(ts_op_creator_map* creator_map);

TENNIS_C_API void ts_free_creator_map(ts_op_creator_map* creator_map);

TENNIS_C_API ts_device_context* ts_initial_device_context(const ts_Device *device);

TENNIS_C_API void ts_free_device_context(ts_device_context* device);

#ifdef __cplusplus
}
#endif

#endif //TENNIS_API_API_H