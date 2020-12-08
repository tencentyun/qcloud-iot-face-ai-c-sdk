# 项目信息
project (QCloud_IoT_SDK C CXX)

# CMake 最低版本号要求
cmake_minimum_required (VERSION 3.5)

# 设置CMAKE使用编译工具及编译选项
set(CMAKE_SYSTEM_NAME  Linux)
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH ${TOOL_CHAIN_PATH})
set(CMAKE_C_COMPILER  "/opt/toolchains/Hi3516DV300/arm-himix200-linux/arm-himix200-linux/bin/arm-himix200-linux-gcc")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu99 -Wfatal-errors -Os -mfloat-abi=softfp -mfpu=neon -lstdc++")

set(PLATFORM "Hi3516DV300" CACHE STRING "")

# 设置编译相关变量
if (${PLATFORM} STREQUAL "Hi3516DV300")
    link_directories(
        ${PROJECT_SOURCE_DIR}/libs/${PLATFORM}
        ${PROJECT_SOURCE_DIR}/libs/${PLATFORM}/opencv/lib/
        ${PROJECT_SOURCE_DIR}/libs/${PLATFORM}/hisi
    )
    include_directories(
        ${PROJECT_SOURCE_DIR}/include
        ${PROJECT_SOURCE_DIR}/libs/${PLATFORM}/opencv/include
        ${PROJECT_SOURCE_DIR}/iot-explorer-c-sdk/sdk/include
        ${PROJECT_SOURCE_DIR}/iot-explorer-c-sdk/sdk/include/exports
    )
else()
    message(FATAL_ERROR "UNSUPPORTED PLATFORM ${PLATFORM}")
endif()



set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/output/bin)

# 编译explorer sdk
if (${SAMPLE_ONLY})
    file(GLOB iot_c_sdk ${CMAKE_SOURCE_DIR}/output/lib/*.a)
else()
    add_subdirectory(${PROJECT_SOURCE_DIR}/iot-explorer-c-sdk)
endif()


# 设置依赖库
set(iot_open_cv
    m dl rt pthread
    opencv_highgui
    IlmImf libjasper libjpeg libpng libtiff zlib
    opencv_ml opencv_calib3d opencv_contrib opencv_core opencv_features2d opencv_flann
    opencv_imgproc opencv_legacy opencv_objdetect opencv_ts opencv_video opencv_nonfree
    opencv_photo opencv_stitching opencv_superres opencv_videostab
)
set(iot_hisi mpi ive nnie securec VoiceEngine dnvqe upvqe)

# 编译生成可执行程序
file(GLOB src_facekit_sample ${CMAKE_CURRENT_SOURCE_DIR}/sample/*.c)
add_executable(facekit_sample ${src_facekit_sample})
target_link_libraries(facekit_sample  -Wl,--start-group ${iot_open_cv} iotfacekit ${iot_hisi} ${iot_c_sdk} -Wl,--end-group)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/device_info.json DESTINATION ${EXECUTABLE_OUTPUT_PATH})
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/facekit_config.json DESTINATION ${EXECUTABLE_OUTPUT_PATH})
file(COPY models DESTINATION ${EXECUTABLE_OUTPUT_PATH})
