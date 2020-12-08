# 腾讯云物联网开发平台人脸识别 C SDK

腾讯云物联网开发平台人脸识别 c sdk 在[腾讯云物联网开发平台设备端 C-SDK](https://github.com/tencentyun/qcloud-iot-explorer-sdk-embedded-c)的基础上，融合物联网和人脸识别能力，实现远程更新人脸库、检索人脸、统计检索结果等功能，丰富了物联网设备在人脸识别领域的应用场景。

在设备已经开启`人脸识别`增值服务的前提下，开发者基于物联网开发平台人脸识别 C SDK，调用相应API并实现相应回调，即可在已经支持的芯片架构中，实现人脸识别功能。

## SDK 目录结构简介

| 名称               | 说明|
| ------------------ | ------------------------------------------------------------ |
| CMakeLists.txt     | cmake编译描述文件|
| cmake_build.sh     | Linux下使用cmake的编译脚本|
| device_info.json   | 运行配置文件，包括设备信息，人脸识别相关参数|
| docs               | 文档目录，SDK使用说明文档|
| include            | 提供给用户使用的外部头文件|
| iot-explorer-c-sdk | 腾讯云物联网开发平台设备端 C SDK 以及配套的CMakeLists.txt|
| libs               | SDK 依赖的相关库|
| sample             | 应用示例|
| models             | AI 模型运行所需文件|

## SDK 快速体验

IOT Explorer 人脸识别 C SDK采用CMAKE进行工程编译，用户可以通过以下三个步骤快速体验SDK功能:

### 创建设备

参考[控制台操作说明]()，进行设备创建，并开启`人脸识别`增值服务。

### SDK编译

1. 打开`CMakeLists.txt`，编译以下部分，设置用户编译工具路径，当前只支持`Hi3516DV300`。
```cmake
set(CMAKE_C_COMPILER  "/opt/toolchains/Hi3516DV300/arm-himix200-linux/arm-himix200-linux/bin/arm-himix200-linux-gcc")
```
2. 根据设备情况，设置`device_info.json`和`facekit_config.json`
3. 运行`./cmake_bulid.sh all`进行编译
4. 将`output/bin`目录拷贝到相应的硬件平台下

### 运行示例

进入到拷贝目录下，运行`./facekit_sample <images, eg: my.jpg>`，即可实现对某个包含人脸的图片的循环检索，从控制台下发该图片对应的人脸注册图片，即可实现对该人的检索，并可在`运营分析->人脸识别统计`中看到上报的结果。

## SDK 开发

目前SDK已支持适配的芯片架构：

| 芯片              | 状态|
| ------------------ | ------------------------------------------------------------ |
|  Hi3516DV300    | 已支持|

SDK 开发请参见`docs`目录文档 [IOT-Explorer人脸识别C-SDK开发指南](./docs/IOT-Explorer人脸识别C-SDK开发指南.md) 和 [IOT-Explorer人脸识别C-SDK-API说明](./docs/IOT-Explorer人脸识别C-SDK-API说明.md)。
