/*
 * Tencent is pleased to support the open source community by making IoT Hub
 available.
 * Copyright (C) 2016 THL A29 Limited, a Tencent company. All rights reserved.

 * Licensed under the MIT License (the "License"); you may not use this file
 except in
 * compliance with the License. You may obtain a copy of the License at
 * http://opensource.org/licenses/MIT

 * Unless required by applicable law or agreed to in writing, software
 distributed under the License is
 * distributed on an "AS IS" basis, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 KIND,
 * either express or implied. See the License for the specific language
 governing permissions and
 * limitations under the License.
 *
 */

#ifndef QCLOUD_IOT_FACE_AI_C_SDK_INCLUDE_QCLOUD_IOT_FACEKIT_H_
#define QCLOUD_IOT_FACE_AI_C_SDK_INCLUDE_QCLOUD_IOT_FACEKIT_H_

#include <stdint.h>
#include <stdbool.h>

#define MAX_SIZE_OF_FEATURE_ID 128

// ----------------------------------------------------------------------------
//   struct define
// ----------------------------------------------------------------------------
typedef struct {
    char *license_dir;
    char *resource_dir;
    char *img_dir;

    char *tracker_lib_dir;
    char *tracker_config;
    float tracker_threshold;
    int   tracker_detect_interval;
    int   tracker_image_type;
    int   tracker_image_padding;
    char *feature_lib_dir;
    char *feature_config;
    char *alignment_lib_dir;
    char *alignment_config;
    char *quality_pro_lib_dir;
    char *quality_pro_config;
    float quality_pro_score_facing_threshold;
    float quality_pro_score_visibility_threshold;
    float quality_pro_score_sharp_threshold;
    float quality_pro_score_bright_env_threshold;
    float quality_pro_score_dark_env_threshold;
    int   quality_pro_is_bright_environment_mode;
    char *retrieval_cvt_table_path;
} IOT_FaceKit_Config;

typedef int (*FeatureRegisterFunc)(void *handle, float **features, char **feature_ids,
                                   const int num);

typedef int (*FeatureUnRegisterFunc)(void *handle, char **feature_ids, const int num);

// callback should be implentmented by user
typedef struct {
    int (*facekit_feature_save)(const char *face_lib, const char *feature_id, float *feature,
                                int length);
    int (*facekit_feature_delete)(const char *face_lib, const char *feature_id);
    int (*facekit_feature_check_exit)(const char *face_lib, const char *feature_id);
    int (*facekit_face_lib_delete)(void *handle, const char *face_lib, FeatureUnRegisterFunc func);
    int (*facekit_features_register)(void *handle, FeatureRegisterFunc func);
    int (*facekit_offline_events_save)(const char *event_json, int len);
    void (*facekit_error_handle_get_feature_fail)(const char *face_lib, const char *feature_id,
                                                  int error);
    void (*facekit_error_handle_download_fail)(const char *face_lib, const char *feature_id,
                                               const char *url);
    void (*facekit_handle_download_success)(const char *face_lib, const char *feature_id,
                                            const char *img_path);
} IOT_FaceKit_CallBack;

typedef struct {
    uint8_t *data;
    int      width;
    int      height;
} IOT_FaceKit_Frame;

// ----------------------------------------------------------------------------
//  error code defines
// ----------------------------------------------------------------------------

typedef enum {
    QCLOUD_IOT_FACEKIT_RET_SUCCESS             = 0,
    QCLOUD_IOT_FACEKIT_ERR_FAILURE             = -1001,
    QCLOUD_IOT_FACEKIT_ERR_PARSE_JSON_FAILED   = -1002,
    QCLOUD_IOT_FACEKIT_ERR_GET_LICENCE_FAILED  = -1003,
    QCLOUD_IOT_FACEKIT_ERR_GET_FEATURE_FAILED  = -1004,
    QCLOUD_IOT_FACEKIT_ERR_RETRIEVAL_FAILED    = -1005,
    QCLOUD_IOT_FACEKIT_ERR_REPORT_EVENT_FAILED = -1006
} IoT_Facekit_ErrCode;

// ----------------------------------------------------------------------------
// Facekit common API
// ----------------------------------------------------------------------------

/**
 * @brief 初始化配置
 *
 * @param[in] json_path 配置文件路径
 * @return 0 for success
 */
int IOT_Facekit_ConfigInit(const char *json_path);

/**
 * @brief 反初始化
 *
 */
void IOT_Facekit_ConfigDeinit(void);

/**
 * @brief 获取配置
 *
 */
void IOT_Facekit_GetConfig(IOT_FaceKit_Config *config);

/**
 * @brief 初始化
 *
 * @param[in] config 配置项
 * @param[in] mqtt_client mqtt handle
 * @return handle
 */
void *IOT_Facekit_Init(IOT_FaceKit_Config *config, IOT_FaceKit_CallBack callback,
                       void *template_client, const char *product_id, const char *device_name);

/**
 * @brief 反初始化
 *
 * @param[in] handle
 */
void IOT_Facekit_Deinit(void *handle);

/**
 * @brief 打印版本
 *
 */
void IOT_Facekit_PrintVersion(void);

/**
 * @brief 根据阈值，检索特征
 *
 * @return 0 for success
 */
int IOT_Facekit_RetrievalFeature(void *handle, IOT_FaceKit_Frame frame, float threshold,
                                 char *feature_id);

/**
 * @brief 特征库事件上报
 *
 * @return 0 for found
 */
int IOT_Facekit_RetrievalEventPost(void *template_client, const char *event_json);

/**
 * @brief 启动特征库更新任务，需要依赖外部进行yield
 *
 * @return 0 for found
 */
int IOT_Facekit_FeatureUpdateTaskStart(void *handle);

/**
 * @brief 停止特征库更新任务
 *
 * @return 0 for found
 */
void IOT_Facekit_FeatureUpdateTaskStop(void *handle);

/**
 * @brief 设置client
 *
 */
void IOT_Facekit_SetClient(void *handle, void *template_client);

#endif  // QCLOUD_IOT_FACE_AI_C_SDK_INCLUDE_QCLOUD_IOT_FACEKIT_H_
