# IOT Explorer 人脸识别 C SDK API 说明

## 一、SDK配置API

### IOT_Facekit_ConfigInit

* **简介：** 从文件中读取并初始化配置
* **声明：** `int IOT_Facekit_ConfigInit(const char *json_path);`
* **参数：**
  * json_path：配置文件路径，示例中为`device_info.json`
* **返回值：**

|返回值|描述|
|---|---|
|QCLOUD_IOT_FACEKIT_RET_SUCCESS|成功|
|QCLOUD_IOT_FACEKIT_ERR_PARSE_JSON_FAILED|失败，文件解析错误|

---

### IOT_Facekit_ConfigDeinit

* **简介：** 清除配置
* **声明：** `void IOT_Facekit_ConfigDeinit(void);`
* **参数：** 无
* **返回值：** 无

---

### IOT_Facekit_GetConfig

* **简介：** 获取当前从文件中解析获取的配置
* **声明：** `void IOT_Facekit_GetConfig(IOT_FaceKit_Config *config);`
* **参数：**
  * config：配置，见`IOT_FaceKit_Config`
* **返回值：** 无

## 二、人脸识别API

### IOT_Facekit_Init

* **简介：** 初始化人脸识别功能
* **声明：** `void *IOT_Facekit_Init(IOT_FaceKit_Config *config, IOT_FaceKit_CallBack callback, void *template_client,
                       const char *product_id, const char *device_name);`
* **参数：**
  * config：配置，通过`IOT_Facekit_GetConfig`获得
  * callback：用户回调，用户可参考示例`facekit_callback.c`实现，并初始化传入
  * template_client：数据模板句柄，通过物联网开发平台 C SDK中`IOT_Template_Construct`获得，当为`NULL`时，使用离线模式
  * product_id：物联网开发平台设备产品ID
  * device_name：物联网开发平台设备名
* **返回值：** 句柄，用作传入其他API，出错返回`NULL`

---

### IOT_Facekit_Deinit

* **简介：** 清除人脸识别功能
* **声明：** `void IOT_Facekit_Deinit(void *handle);`
* **参数：**
  * handle：句柄，通过`IOT_Facekit_Init`获得
* **返回值：** 无

---

### IOT_Facekit_PrintVersion

* **简介：** 打印人脸识别所使用AI库的相应版本
* **声明：** `void IOT_Facekit_PrintVersion(void);`
* **参数：** 无
* **返回值：** 无

---

### IOT_Facekit_RetrievalFeature

* **简介：** 检索人脸
* **声明：** `int IOT_Facekit_RetrievalFeature(void *handle, IOT_FaceKit_Frame frame, float threshold, char *feature_id);`
* **参数：**
  * handle：句柄，通过`IOT_Facekit_Init`获得
  * frame：待检索帧，见`IOT_FaceKit_Frame`
  * threshold：人脸检索的阈值，推荐设置为`80`
  * feature_id：特征id，对应控制台的人员ID。当检索失败时将无意义。
* **返回值：**

|返回值|描述|
|---|---|
|QCLOUD_IOT_FACEKIT_RET_SUCCESS|检索成功|
|QCLOUD_IOT_FACEKIT_ERR_GET_FEATURE_FAILED|提取特征失败|
|QCLOUD_IOT_FACEKIT_ERR_RETRIEVAL_FAILED|检索失败|

---

### IOT_Facekit_RetrievalEventPost

* **简介：** 用作上报离线时发生的检索事件
* **声明：** `int IOT_Facekit_RetrievalEventPost(void *template_client, const char *event_json);`
* **参数：**
  * template_client：数据模板句柄，通过物联网开发平台 C SDK中`IOT_Template_Construct`
  * event_json：待上报事件，与用户回调`facekit_offline_events_save`中相对应
* **返回值：**

|返回值|描述|
|---|---|
|QCLOUD_IOT_FACEKIT_RET_SUCCESS|发送成功|
|QCLOUD_IOT_FACEKIT_ERR_REPORT_EVENT_FAILED|发送失败|

---

### IOT_Facekit_FeatureUpdateTaskStart

* **简介：** 启动人脸特征更新任务，可以同步云端下发的人脸更新请求
* **声明：** `int IOT_Facekit_FeatureUpdateTaskStart(void *handle);`
* **参数：**
  * handle：句柄，通过`IOT_Facekit_Init`获得
* **返回值：**

|返回值|描述|
|---|---|
|QCLOUD_IOT_FACEKIT_RET_SUCCESS|启动成功|
|QCLOUD_IOT_FACEKIT_ERR_FAILURE|启动失败|

---

### IOT_Facekit_FeatureUpdateTaskStop

* **简介：** 停止人脸特征更新任务
* **声明：** `void IOT_Facekit_FeatureUpdateTaskStop(void *handle);`
* **参数：**
  * handle：句柄，通过`IOT_Facekit_Init`获得
* **返回值：** 无

---

### IOT_Facekit_SetClient

* **简介：** 启动人脸特征更新任务，可以同步云端下发的人脸更新请求
* **声明：** `void IOT_Facekit_SetClient(void *handle, void *template_client);`
* **参数：**
  * handle：句柄，通过`IOT_Facekit_Init`获得
  * template_client：数据模板句柄，通过物联网开发平台 C SDK中`IOT_Template_Construct`获得，当为`NULL`时，使用离线模式
* **返回值：** 无

## 三、数据结构

- IOT_FaceKit_Config

|成员|说明|
|---|---|
|license_dir|license存放目录|
|resource_dir|资源下载目录|
|img_dir|人脸图片下载目录|
|tracker_lib_dir|人脸追踪模型库路径|
|tracker_config|人脸追踪模型配置文件|
|tracker_threshold|人脸置信度阈值，0.8~1.0；默认0.93|
|tracker_detect_interval|检测线程帧间隔，[1~30]，越小检测延迟越短；默认6|
|tracker_image_type|图像方向，0:竖屏 1:横屏；默认0|
|tracker_image_padding|检测选项，0:关闭补边 1:开启补边；默认0，注意：当[长边：短边]不等于[16:9]时，可以开启补边，以提高检测效果|
|feature_lib_dir|人脸特征模型路径|
|feature_config|人脸特征模型配置文件|
|alignment_lib_dir|人脸精准匹配模型路径|
|alignment_config|人脸精准匹配模型配置文件|
|quality_pro_lib_dir|人脸质量归因模型路径|
|quality_pro_config|人脸质量归因模型配置文件|
|quality_pro_score_facing_threshold|脸部正面角度评分阈值：分数越低，角度越大|
|quality_pro_score_visibility_threshold|脸部可见度评分阈值：分数越低，遮挡程度越严重|
|quality_pro_score_sharp_threshold|清晰度评分阈值：分数越低，模糊程度越严重|
|quality_pro_score_bright_env_threshold|针对亮光场景的亮度评分阈值：分数越低，光线越暗，分数越高，光线越亮。|
|quality_pro_score_dark_env_threshold|针对暗光场景的亮度评分阈值：分数越低，光线越暗，分数越高，光线越亮。|
|quality_pro_is_bright_environment_mode|场景模式，0采用暗光阈值，1采用亮光的阈值|
|retrieval_cvt_table_path|人脸检索转换表对应的路径|

- IOT_FaceKit_CallBack

|成员|说明|
|---|---|
|facekit_feature_save|保存特征回调|
|facekit_feature_delete|删除特征回调|
|facekit_feature_check_exit|检查特征是否存在回调|
|facekit_face_lib_delete|人脸库删除回调|
|facekit_features_register|特征注册回调|
|facekit_offline_events_save|离线事件保存回调|
|facekit_error_handle_get_feature_fail|特征生成失败处理回调|
|facekit_error_handle_download_fail|人脸图片下载失败回调|

- IOT_FaceKit_Frame

|成员|说明|
|---|---|
|data|输入RGB图片数据|
|width|图片宽|
|height|图片高|

- IoT_Facekit_ErrCode

|成员|说明|
|---|---|
|QCLOUD_IOT_FACEKIT_RET_SUCCESS|执行成功|
|QCLOUD_IOT_FACEKIT_ERR_FAILURE|执行失败|
|QCLOUD_IOT_FACEKIT_ERR_PARSE_JSON_FAILED|json文件解析错误|
|QCLOUD_IOT_FACEKIT_ERR_GET_FEATURE_FAILED|获取特征失败|
|QCLOUD_IOT_FACEKIT_ERR_RETRIEVAL_FAILED|检索失败|
|QCLOUD_IOT_FACEKIT_ERR_REPORT_EVENT_FAILED|上报检索事件失败|
