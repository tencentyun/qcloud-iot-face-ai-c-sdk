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
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>

#include "qcloud_iot_export.h"
#include "qcloud_iot_import.h"
#include "qcloud_iot_facekit.h"

#define FEATURE_RECORD "./feat/record.txt"

static int _remove_dir(const char *dir)
{
    char           cur_dir[] = ".";
    char           up_dir[]  = "..";
    char           dir_name[128];
    DIR *          dirp;
    struct dirent *dp;
    struct stat    dir_stat;

    if (0 != access(dir, F_OK)) {
        return 0;
    }

    if (0 > stat(dir, &dir_stat)) {
        return -1;
    }

    if (S_ISREG(dir_stat.st_mode)) {
        remove(dir);
    } else if (S_ISDIR(dir_stat.st_mode)) {
        dirp = opendir(dir);
        while ((dp = readdir(dirp)) != NULL) {
            if ((0 == strcmp(cur_dir, dp->d_name)) || (0 == strcmp(up_dir, dp->d_name))) {
                continue;
            }
            sprintf(dir_name, "%s/%s", dir, dp->d_name);
            _remove_dir(dir_name);
        }
        closedir(dirp);
        rmdir(dir);
    } else {
        Log_e("unknow file type!");
    }
    return 0;
}

static void _update_record(const char *face_lib, const char *feature_id, const char *file_path, int flag)
{
    char tmp_face_lib[128];
    char tmp_feature_id[128];
    char tmp_file_path[128];
    int  tmp_flag;

    FILE *fp = fopen(FEATURE_RECORD, "rb+");
    if (fp) {
        int offset = ftell(fp);
        while (EOF != fscanf(fp, "%[^,],%[^,],%[^,],%d\n", tmp_face_lib, tmp_feature_id, tmp_file_path, &tmp_flag)) {
            if (!strcmp(face_lib, tmp_face_lib) && !strcmp(feature_id, tmp_feature_id)) {
                fseek(fp, offset, SEEK_SET);
                fprintf(fp, "%s,%s,%s,%d\n", face_lib, feature_id, tmp_file_path, flag);
                goto exit;
            }
            offset = ftell(fp);
        }
        // if a new one just add in the end
        if (!flag) {
            fprintf(fp, "%s,%s,%s,%d\n", face_lib, feature_id, file_path, flag);
        }
    exit:
        fclose(fp);
    }
}

int facekit_cb_save_feature(const char *face_lib, const char *feature_id, float *feature, int length)
{
    char file_path[128];
    char file_dir[128];

    HAL_Snprintf(file_dir, 128, "./feat/%s", face_lib);

    DIR *dir = opendir(file_dir);
    if (!dir) {
        int rc = mkdir(file_dir, S_IRWXU);
        if (rc) {
            Log_e("mkdir %s failed", file_dir);
            return -1;
        }
    } else {
        closedir(dir);
    }

    HAL_Snprintf(file_path, 128, "./feat/%s/%s.feat", face_lib, feature_id);
    FILE *fp = fopen(file_path, "wb+");
    if (!fp) {
        Log_e("file open failed");
        return -1;
    }
    fwrite(feature, length * sizeof(float), 1, fp);
    fclose(fp);
    _update_record(face_lib, feature_id, file_path, 0);
    return 0;
}

int facekit_cb_del_feature(const char *face_lib, const char *feature_id)
{
    char file_path[128];
    HAL_Snprintf(file_path, 128, "./feat/%s/%s.feat", face_lib, feature_id);
    remove(file_path);
    _update_record(face_lib, feature_id, NULL, 1);
    return 0;
}

int facekit_cb_del_face_lib(void *handle, const char *face_lib, FeatureUnRegisterFunc func)
{
    char  file_dir[128];
    char  tmp_face_lib[128];
    char  file_path[128];
    int   flag;
    char *feature_id = HAL_Malloc(128);

    FILE *fp = fopen(FEATURE_RECORD, "rb+");
    if (!fp) {
        Log_w("file open failed, should set up facekit update task!");
        goto exit;
    }

    int offset = ftell(fp);
    while (EOF != fscanf(fp, "%[^,],%[^,],%[^,],%d\n", tmp_face_lib, feature_id, file_path, &flag)) {
        if (!strcmp(face_lib, tmp_face_lib) && !flag) {
            Log_d("Unregister feature %s %s %d", tmp_face_lib, feature_id, flag);
            fseek(fp, offset, SEEK_SET);
            fprintf(fp, "%s,%s,%s,1\n", face_lib, feature_id, file_path);
            func(handle, &feature_id, 1);
        }
        offset = ftell(fp);
    }
    fclose(fp);
exit:
    HAL_Free(feature_id);
    HAL_Snprintf(file_dir, 128, "./feat/%s", face_lib);
    DIR *dir = opendir(file_dir);
    if (dir) {
        closedir(dir);
        return _remove_dir(file_dir);
    }
    return 0;
}

int facekit_cb_check_feature_exit(const char *face_lib, const char *feature_id)
{
    char file_path[128];
    HAL_Snprintf(file_path, 128, "./feat/%s/%s.feat", face_lib, feature_id);
    return (access(file_path, 0) == 0);
}

int facekit_cb_register_features(void *handle, FeatureRegisterFunc func)
{
    int    rc         = 0;
    char * face_lib   = HAL_Malloc(128);
    float *feature    = HAL_Malloc(sizeof(float) * 512);
    char * feature_id = HAL_Malloc(128);
    int    flag;

    char file_path[128];

    DIR *dir = opendir("./feat");
    if (!dir) {
        rc = mkdir("./feat", S_IRWXU);
        if (rc) {
            Log_e("mkdir %s failed", "./feat");
            return rc;
        }
    } else {
        closedir(dir);
    }

    FILE *fp = fopen(FEATURE_RECORD, "a+");
    if (!fp) {
        Log_w("file open failed, should set up facekit update task!");
        goto exit;
    }

    while (EOF != fscanf(fp, "%[^,],%[^,],%[^,],%d\n", face_lib, feature_id, file_path, &flag)) {
        // Log_d("register feature %s file path %s", feature_id, file_path);
        FILE *feat_file = fopen(file_path, "rb");
        if (feat_file) {
            fread(feature, 1, 2048, feat_file);
            Log_d("register feature %s", feature_id);
            func(handle, &feature, &feature_id, 1);
            fclose(feat_file);
        }
    }
    fclose(fp);
exit:
    HAL_Free(face_lib);
    HAL_Free(feature);
    HAL_Free(feature_id);
    return 0;
}

int facekit_cb_ext_save_events(const char *event_json, int len)
{
    DIR *dir = opendir("./log");
    if (!dir) {
        int rc = mkdir("./log", S_IRWXU);
        if (rc) {
            Log_e("mkdir %s failed", "./log");
            return rc;
        }
    } else {
        closedir(dir);
    }

    FILE *fp = fopen("./log/event.log", "ab+");
    if (!fp) {
        Log_e("file open failed");
        return -1;
    }
    fseek(fp, 0, SEEK_END);
    // when 0 means publish failed
    fprintf(fp, "%d %s\n", 0, event_json);
    fclose(fp);
    return 0;
}

void facekit_cb_get_feature_fail(const char *face_lib, const char *feature_id, int error)
{
    // if get feature failed do what you need
}

void facekit_cb_download_fail(const char *face_lib, const char *feature_id, const char *url)
{
    // if download failed do what you need
}

void facekit_cb_download_success(const char *face_lib, const char *feature_id, const char *img_path)
{
    // if download failed do what you need
}