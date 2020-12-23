# IOT Explorer 人脸识别 C SDK 开发指南

IOT Explorer 人脸识别 C SDK 是基于物联网开发平台设备端C SDK之上构建的，依赖 C SDK的部分接口。

整个开发流程可以分为以下三个部分：

## SDK配置

SDK 配置主要包括设备信息配置（`device_info.json`）和人脸识别配置（`facekit_config.json`），其中设备信息配置请参见[设备信息存储](https://cloud.tencent.com/document/product/1081/48376)。

人脸识别配置包括以下部分：

- 下载相关（download_dir）

  |配置项|说明|
  |---|---|
  |license_dir|license存放目录|
  |resource_dir|资源下载目录|
  |img_dir|人脸图片下载目录|

- 人脸库相关（facekit_options）
  - 人脸追踪（tracker）

  |配置项|说明|
  |---|---|
  |lib_dir|人脸追踪模型库路径|
  |config|人脸追踪模型配置文件|
  |threshold|人脸置信度阈值，0.8~1.0；默认0.93|
  |detect_interval|检测线程帧间隔，[1~30]，越小检测延迟越短；默认6|
  |image_type|图像方向，0:竖屏 1:横屏；默认0|
  |image_padding|检测选项，0:关闭补边 1:开启补边；默认0，注意：当[长边：短边]不等于[16:9]时，可以开启补边，以提高检测效果|

  - 人脸特征（feature）

  |配置项|说明|
  |---|---|
  |lib_dir|人脸特征模型路径|
  |config|人脸特征模型配置文件|

  - 人脸精准匹配（alignment）

  |配置项|说明|
  |---|---|
  |lib_dir|人脸精准匹配模型路径|
  |config|人脸精准匹配模型配置文件|

  - 人脸质量归因（quality_pro）

  |配置项|说明|
  |---|---|
  |lib_dir|人脸质量归因模型路径|
  |config|人脸质量归因模型配置文件|
  |score_facing_threshold|脸部正面角度评分阈值：分数越低，角度越大|
  |score_visibility_threshold|脸部可见度评分阈值：分数越低，遮挡程度越严重|
  |score_sharp_threshold|清晰度评分阈值：分数越低，模糊程度越严重|
  |score_bright_env_threshold|针对亮光场景的亮度评分阈值：分数越低，光线越暗，分数越高，光线越亮。|
  |score_dark_env_threshold|针对暗光场景的亮度评分阈值：分数越低，光线越暗，分数越高，光线越亮。|
  |is_bright_environment_mode|场景模式，0采用暗光阈值，1采用亮光的阈值|

  - 人脸检索（retrieval）

  |配置项|说明|
  |---|---|
  |cvt_table_path|人脸检索转换表对应的路径|

## 实现回调

用户回调实现可参考`sample/facekit_callback.c`。

**1. facekit_feature_save**
* **说明：** 保存特征回调，当接收到云端下发更新人脸库的请求时，下载相应的人脸图片后生成特征调用
* **声明：** `int (*facekit_feature_save)(const char *face_lib, const char *feature_id, float *feature, int length);`
* **参数：**
  * face_lib：人脸库文件名，命名方式为`ai_face_list_<人员库ID>.csv`，其中人员库ID为控制台输入
  * feature_id：特征ID，与人员ID保持一致
  * feature：特征数组
  * length：特征数组长度，通常为512
* **返回值：** 0表示保存成功

**2. facekit_feature_delete**
* **说明：** 删除特征回调，当接收到云端下发删除人脸图片的请求时，调用以删除相应的特征
* **声明：** `int (*facekit_feature_delete)(const char *face_lib, const char *feature_id);`
* **参数：**
  * face_lib：人脸库文件名，命名方式为`ai_face_list_<人员库ID>.csv`，其中人员库ID为控制台输入
  * feature_id：特征ID，与人员ID保持一致
* **返回值：** 0表示删除成功

**3. facekit_feature_check_exit**
* **说明：** 检查特征是否存在回调，检查人脸特征是否存在，以防止图片下载完未生成特征时出现异常
* **声明：** `int (*facekit_feature_check_exit)(const char *face_lib, const char *feature_id);`
* **参数：**
  * face_lib：人脸库文件名，命名方式为`ai_face_list_<人员库ID>.csv`，其中人员库ID为控制台输入
  * feature_id：特征ID，与人员ID保持一致
* **返回值：** 1表示存在，0表示不存在

**4. facekit_face_lib_delete**
* **说明：** 人脸库删除回调，当接收到云端下发删除人脸库，调用以删除相应的特征库
* **声明：** `int (*facekit_face_lib_delete)(void *handle, const char *face_lib, FeatureUnRegisterFunc func);`
* **参数：**
  * handle：提供用户调用用作`func`的参数，用户无需关注
  * face_lib： C SDK中`IOT_Template_Construct`获得，当为`NULL`时，使用离线模式
  * func：提供用户调用取消当前已经登记的特征的接口
* **返回值：** 0表示删除成功

**5. facekit_features_register**
* **简介：** 特征注册回调，人脸库功能初始化时调用以注册已生成的特征
* **声明：** `int (*facekit_features_register)(void *handle, FeatureRegisterFunc func);`
* **参数：**
  * handle：提供用户调用用作`func`的参数，用户无需关注
  * func：提供用户调用登记特征的接口
* **返回值：** 无

**6. facekit_offline_events_save**
* **简介：** 离线事件保存回调，当事件发送失败时调用，用户根据需求是否处理
* **声明：** `int (*facekit_offline_events_save)(const char *event_json, int len);`
* **参数：**
  * event_json：未发送成功的事件
  * len：长度
* **返回值：** 0表示成功

**7. facekit_error_handle_get_feature_fail**
* **简介：** 特征生成失败处理回调，当云端下发的图片不符合登记标准时调用
* **声明：** `void (*facekit_error_handle_get_feature_fail)(const char *face_lib, const char *feature_id, int error);`
* **参数：**
  * face_lib：人脸库文件名，命名方式为`ai_face_list_<人员库ID>.csv`，其中人员库ID为控制台输入
  * feature_id：特征ID，与人员ID保持一致
  * error：错误码，暂无定义
* **返回值：** 无

**8. facekit_error_handle_download_fail**
* **简介：** 人脸图片下载失败回调，当云端下发的图片下载失败时触发
* **声明：** `void (*facekit_error_handle_download_fail)(const char *face_lib, const char *file_name, const char *url);`
* **参数：**
  * face_lib：人脸库文件名，命名方式为`ai_face_list_<人员库ID>.csv`，其中人员库ID为控制台输入
  * feature_id：特征ID，与人员ID保持一致
  * url：图片对应的url
* **返回值：** 无

**9. facekit_handle_download_success**
* **简介：** 人脸图片下载成功回调，当云端下发的图片下载成功时触发
* **声明：** `void (*facekit_handle_download_success)(const char *face_lib, const char *feature_id, const char *img_path);`
* **参数：**
  * face_lib：人脸库文件名，命名方式为`ai_face_list_<人员库ID>.csv`，其中人员库ID为控制台输入
  * feature_id：特征ID，与人员ID保持一致
  * img_path：图片下载到本地的路径
* **返回值：** 无

## 业务逻辑开发

根据业务需求，人脸识别服务开发请参见[IOT-Explorer人脸识别C-SDK-API说明](./IOT-Explorer人脸识别C-SDK-API说明.md)调用相应API开发业务逻辑，IoT Explorer的基础服务开发请参见[物联网开发平台设备端 C SDK使用参考](https://cloud.tencent.com/document/product/1081/48377)。
