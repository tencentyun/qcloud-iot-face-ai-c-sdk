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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>

#include "qcloud_iot_import.h"
#include "qcloud_iot_export.h"
#include "qcloud_iot_facekit.h"
#include "opencv2/highgui/highgui_c.h"

extern int  facekit_cb_save_feature(const char* face_lib, const char* feature_id, float* feature, int length);
extern int  facekit_cb_del_feature(const char* face_lib, const char* feature_id);
extern int  facekit_cb_del_face_lib(void* handle, const char* face_lib, FeatureUnRegisterFunc func);
extern int  facekit_cb_check_feature_exit(const char* face_lib, const char* feature_id);
extern int  facekit_cb_register_features(void* handle, FeatureRegisterFunc func);
extern int  facekit_cb_ext_save_events(const char* event_json, int len);
extern void facekit_cb_get_feature_fail(const char* face_lib, const char* feature_id, int error);
extern void facekit_cb_download_fail(const char* face_lib, const char* feature_id, const char* url);

static pthread_t sg_data_template_thread;
static int       sg_data_template_task_exit = 0;

static pthread_t sg_face_retrieval_thread;
static int       sg_face_retrieval_task_exit = 0;

static void* sg_template_client = NULL;
static void* sg_facekit_handle  = NULL;

static DeviceInfo           sg_devInfo;
static IOT_FaceKit_Config   sg_config;
static IOT_FaceKit_CallBack sg_callback;

static char   sg_data_report_buffer[2048];
static size_t sg_data_report_buffersize = sizeof(sg_data_report_buffer) / sizeof(sg_data_report_buffer[0]);

static int sg_main_exit = 0;

static int _check_and_create_dir(const char* dir)
{
    int rc;
    if (!opendir(dir)) {
        rc = mkdir(dir, S_IRWXU);
        if (rc) {
            Log_e("mkdir %s failed", dir);
            return rc;
        }
    }
    return 0;
}

// Setup MQTT event handler
static void _event_handler(void* pClient, void* handle_context, MQTTEventMsg* msg)
{
    uintptr_t packet_id = (uintptr_t)msg->msg;

    switch (msg->event_type) {
        case MQTT_EVENT_UNDEF:
            Log_i("undefined event occur.");
            break;

        case MQTT_EVENT_DISCONNECT:
            Log_i("MQTT disconnect.");
            break;

        case MQTT_EVENT_RECONNECT:
            Log_i("MQTT reconnect.");
            break;

        case MQTT_EVENT_SUBCRIBE_SUCCESS:
            Log_i("subscribe success, packet-id=%u", packet_id);
            break;

        case MQTT_EVENT_SUBCRIBE_TIMEOUT:
            Log_i("subscribe wait ack timeout, packet-id=%u", packet_id);
            break;

        case MQTT_EVENT_SUBCRIBE_NACK:
            Log_i("subscribe nack, packet-id=%u", packet_id);
            break;

        case MQTT_EVENT_PUBLISH_SUCCESS:
            Log_i("publish success, packet-id=%u", (unsigned int)packet_id);
            break;

        case MQTT_EVENT_PUBLISH_TIMEOUT:
            Log_i("publish timeout, packet-id=%u", (unsigned int)packet_id);
            break;

        case MQTT_EVENT_PUBLISH_NACK:
            Log_i("publish nack, packet-id=%u", (unsigned int)packet_id);
            break;
        default:
            Log_i("Should NOT arrive here.");
            break;
    }
}

// Setup MQTT construct parameters
static int _setup_connect_init_params(TemplateInitParams* initParams)
{
    int ret;

    ret = HAL_GetDevInfo((void*)&sg_devInfo);
    if (QCLOUD_RET_SUCCESS != ret) {
        return ret;
    }

    initParams->device_name = sg_devInfo.device_name;
    initParams->product_id  = sg_devInfo.product_id;

#ifdef AUTH_MODE_CERT
    /* TLS with certs*/
    char  certs_dir[PATH_MAX + 1] = "certs";
    char  current_path[PATH_MAX + 1];
    char* cwd = getcwd(current_path, sizeof(current_path));
    if (cwd == NULL) {
        Log_e("getcwd return NULL");
        return QCLOUD_ERR_FAILURE;
    }
    sprintf(sg_cert_file, "%s/%s/%s", current_path, certs_dir, sg_devInfo.dev_cert_file_name);
    sprintf(sg_key_file, "%s/%s/%s", current_path, certs_dir, sg_devInfo.dev_key_file_name);

    initParams->cert_file = sg_cert_file;
    initParams->key_file  = sg_key_file;
#else
    initParams->device_secret = sg_devInfo.device_secret;
#endif

    initParams->command_timeout        = QCLOUD_IOT_MQTT_COMMAND_TIMEOUT;
    initParams->keep_alive_interval_ms = QCLOUD_IOT_MQTT_KEEP_ALIVE_INTERNAL;
    initParams->auto_connect_enable    = 1;
    initParams->event_handle.h_fp      = _event_handler;
    return QCLOUD_RET_SUCCESS;
}

/*You should get the real info for your device, here just for example*/
static int _get_sys_info(void* handle, char* pJsonDoc, size_t sizeOfBuffer)
{
    /*platform info has at least one of module_hardinfo/module_softinfo/fw_ver property*/
    DeviceProperty plat_info[] = {
        {.key = "module_hardinfo", .type = TYPE_TEMPLATE_STRING, .data = "ESP8266"},
        {.key = "module_softinfo", .type = TYPE_TEMPLATE_STRING, .data = "V1.0"},
        {.key = "fw_ver", .type = TYPE_TEMPLATE_STRING, .data = QCLOUD_IOT_DEVICE_SDK_VERSION},
        {.key = "imei", .type = TYPE_TEMPLATE_STRING, .data = "11-22-33-44"},
        {.key = "lat", .type = TYPE_TEMPLATE_STRING, .data = "22.546015"},
        {.key = "lon", .type = TYPE_TEMPLATE_STRING, .data = "113.941125"},
        {NULL, NULL, 0}  // end
    };

    /*self define info*/
    DeviceProperty self_info[] = {{.key  = "append_info",
                                   .type = TYPE_TEMPLATE_STRING,
                                   .data = "{\\\"device_id\\\":\\\"xxxx\\\",\\\"machine_type\\\":\\\"linux\\\"}"},
                                  {NULL, NULL, 0}};

    // not support json in append_info
    return IOT_Template_JSON_ConstructSysInfo(handle, pJsonDoc, sizeOfBuffer, plat_info, self_info);
}

static int _handle_offline_events(void* client)
{
    char event_json[256];
    int  flag;
    if (access("./log/event.log", 0) != 0) {
        return 0;
    }

    FILE* fp = fopen("./log/event.log", "rb+");
    if (!fp) {
        Log_e("file open failed");
        return -1;
    }

    // when 0 means publish failed
    int offset = ftell(fp);
    while (EOF != fscanf(fp, "%d %[^\n]\n", &flag, event_json)) {
        // Log_d("event json %s, flag %d", event_json, flag);
        if (!flag) {
            IOT_Facekit_RetrievalEventPost(client, event_json);
            fseek(fp, offset, SEEK_SET);
            fprintf(fp, "%d %s\n", 1, event_json);
        }
        offset = ftell(fp);
    }
    fclose(fp);
    return 0;
}

static void* _data_template_task_entry(void* args)
{
    int rc = 0;
    // report device info, must be done once when device online
    rc = _get_sys_info(sg_template_client, sg_data_report_buffer, sg_data_report_buffersize);
    if (QCLOUD_RET_SUCCESS == rc) {
        rc = IOT_Template_Report_SysInfo_Sync(sg_template_client, sg_data_report_buffer, sg_data_report_buffersize,
                                              QCLOUD_IOT_MQTT_COMMAND_TIMEOUT);
        if (rc != QCLOUD_RET_SUCCESS) {
            Log_e("Report system info fail, err: %d", rc);
        }
    } else {
        Log_e("Get system info fail, err: %d", rc);
    }

    while (!sg_data_template_task_exit) {
        if (!IOT_Template_Get_Yield_Status(sg_template_client, &rc)) {
            // if yield thread failed, then should do someting to restart
            IOT_Facekit_SetClient(sg_facekit_handle, NULL);  // set null before destory
            break;
        }
        HAL_SleepMs(2000);
    }
    return NULL;
}

static void* _face_retrieval_task_entry(void* args)
{
    int  i = 0;
    char test_img_path[128];

    // init facekit lib
    sg_facekit_handle =
        IOT_Facekit_Init(&sg_config, sg_callback, sg_template_client, sg_devInfo.product_id, sg_devInfo.device_name);
    if (!sg_facekit_handle) {
        Log_e("Face kit init failed!");
        return NULL;
    }

    IOT_Facekit_PrintVersion();

    // start feature update task, automatically get picture from cloud and update to local
    int task_state = IOT_Facekit_FeatureUpdateTaskStart(sg_facekit_handle);

    while (!sg_face_retrieval_task_exit) {
        // a sample to retrieval feature
        strcpy(test_img_path, (char*)args);
        // HAL_Snprintf(test_img_path, 128, "./test_img/%d.jpg", ((i++) % 7) + 1);

        IplImage*         img;
        IOT_FaceKit_Frame frame;
        char              feature_id[128];
        float             feature[512];

        img          = cvLoadImage(test_img_path, -1);
        frame.data   = img->image_data;
        frame.width  = img->width;
        frame.height = img->height;

        // Log_d("img_width: %d, img_height: %d", img->width, img->height);

        // retrieval
        int rc = IOT_Facekit_RetrievalFeature(sg_facekit_handle, frame, 80, feature_id);
        if (rc) {
            Log_e("retrieval failed!");
        } else {
            Log_d("retrieval success!");
        }
        cvReleaseImage(&img);
        HAL_SleepMs(10000);
    }

    if (!task_state) {
        IOT_Facekit_FeatureUpdateTaskStop(sg_facekit_handle);
    }
    IOT_Facekit_Deinit(sg_facekit_handle);
    return NULL;
}

static void _main_exit(int sig)
{
    Log_e("Exit by signal:%d\n", sig);
    sg_main_exit = 1;
}

static void _main_reboot(int sig)
{
    pid_t     pid;
    pthread_t tid;

    pid = getpid();
    tid = pthread_self();

    Log_e("vDumpStack %d  pid %u tid %u", sig, pid, tid);
    sg_main_exit = 1;
    // system("reboot"); // define what you need to reboot
}

int main(int argc, char** argv)
{
    // catch signal
    signal(SIGTERM, _main_exit);
    signal(SIGKILL, _main_exit);
    signal(SIGHUP, _main_exit);
    signal(SIGQUIT, _main_exit);
    signal(SIGINT, _main_exit);
    signal(SIGSEGV, _main_reboot);
    signal(SIGABRT, _main_reboot);
    signal(SIGFPE, _main_reboot);

    IOT_Log_Set_Level(eLOG_DEBUG);

    if (argc != 2 && access(argv[1], 0) != 0) {
        Log_e("Usage: ./facekit_sample <images, eg: my.jpg>");
        return -1;
    }

    int rc = 0;

    // 1. init config
    rc = IOT_Facekit_ConfigInit("./facekit_config.json");
    if (rc) {
        Log_e("json file invalid!");
        return -1;
    }
    IOT_Facekit_GetConfig(&sg_config);

    rc = _check_and_create_dir(sg_config.license_dir);
    rc |= _check_and_create_dir(sg_config.img_dir);
    rc |= _check_and_create_dir(sg_config.resource_dir);
    if (rc) {
        Log_e("check the dir in the facekit_coinfig.json!");
        IOT_Facekit_ConfigDeinit();
        goto exit;
    }

    // 2. init callback
    sg_callback.facekit_feature_save                  = facekit_cb_save_feature;
    sg_callback.facekit_feature_delete                = facekit_cb_del_feature;
    sg_callback.facekit_feature_check_exit            = facekit_cb_check_feature_exit;
    sg_callback.facekit_face_lib_delete               = facekit_cb_del_face_lib;
    sg_callback.facekit_features_register             = facekit_cb_register_features;
    sg_callback.facekit_offline_events_save           = facekit_cb_ext_save_events;
    sg_callback.facekit_error_handle_get_feature_fail = facekit_cb_get_feature_fail;
    sg_callback.facekit_error_handle_download_fail    = facekit_cb_download_fail;

    // 2. init mqtt connection
    TemplateInitParams init_params = DEFAULT_TEMPLATE_INIT_PARAMS;
    rc                             = _setup_connect_init_params(&init_params);
    if (QCLOUD_RET_SUCCESS != rc) {
        Log_e("init params err,rc=%d", rc);
        goto exit;
    }

    sg_template_client = IOT_Template_Construct(&init_params, NULL);
    if (sg_template_client) {
        Log_i("Cloud Device Construct Success");
        // 2.1 start yield thread
        if (QCLOUD_RET_SUCCESS != IOT_Template_Start_Yield_Thread(sg_template_client)) {
            Log_e("start template yield thread fail");
            goto exit;
        }
        // 2.2 handle offline events
        _handle_offline_events(sg_template_client);

        // 2.3 set up a task to do some mqtt staff
        rc = pthread_create((pthread_t*)&sg_data_template_thread, NULL, _data_template_task_entry, NULL);
        if (rc) {
            Log_e("Create thread failed: %d", rc);
            goto exit;
        }
    } else {
        Log_e("Cloud Device Construct Failed");
    }

    // 3. start facekit task
    rc = pthread_create((pthread_t*)&sg_face_retrieval_thread, NULL, _face_retrieval_task_entry, argv[1]);
    if (QCLOUD_RET_SUCCESS != rc) {
        Log_e("Create thread failed: %d", rc);
        goto exit;
    }

    // 4. idle loop
    while (!sg_main_exit) {
        HAL_SleepMs(1000);
    }

    // 5. if error happen, exit
    // 5.1 exit facekit task
    sg_face_retrieval_task_exit = 1;
    pthread_join(*((pthread_t*)&sg_face_retrieval_thread), NULL);

    if (sg_template_client) {
        // 5.2 exit data template task
        sg_data_template_task_exit = 1;
        pthread_join(*((pthread_t*)&sg_data_template_thread), NULL);

        // 5.3 detroy template client
        IOT_Template_Stop_Yield_Thread(sg_template_client);
        IOT_Template_Destroy(sg_template_client);
    }
exit:
    // 5.4 deinit config
    IOT_Facekit_ConfigDeinit();
    return 0;
}
