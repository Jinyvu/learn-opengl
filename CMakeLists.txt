cmake_minimum_required(VERSION 3.18.1)

project(HelloGL VERSION 0.0.1)

# 使用 C++ 17 标准
set(CMAKE_CXX_STANDARD 17)

# 设置代码文件目录
set(SRC_DIR ${PROJECT_SOURCE_DIR}/src)

# 添加头文件
set(HEADER_DIR ${PROJECT_SOURCE_DIR}/include)
set(LIB_DIR ${PROJECT_SOURCE_DIR}/lib)
set(THIRD_PARTY_DIR ${PROJECT_SOURCE_DIR}/third_party)
set(FFMPEG_DIR /opt/homebrew/Cellar/ffmpeg/7.0.1)
include_directories(${HEADER_DIR}/ ${LIB_DIR}/)
include_directories(${FFMPEG_DIR}/include)

# 三方库头文件
set(ASSIMP_HEADER_DIR ${THIRD_PARTY_DIR}/assimp/include)
include_directories(${ASSIMP_HEADER_DIR}/)

# 三方库动态链接
set(GLFW_LINK ${LIB_DIR}/libglfw.3.dylib)
set(ASSIMP_LINK ${THIRD_PARTY_DIR}/assimp/bin/libassimp.5.4.1.dylib)

set(FFMPEG_AVFORMAT_LINK ${FFMPEG_DIR}/lib/libavformat.dylib)
set(FFMPEG_AVCODEC_LINK ${FFMPEG_DIR}/lib/libavcodec.dylib)
set(FFMPEG_SWSCALE_LINK ${FFMPEG_DIR}/lib/libswscale.dylib)
set(FFMPEG_SWSCALE_LINK ${FFMPEG_DIR}/lib/libswscale.dylib)
set(FFMPEG_AVUTIL_LINK ${FFMPEG_DIR}/lib/libavutil.dylib)

set(SOURCES ${SRC_DIR}/glad.c ${SRC_DIR}/test/displayTest/main.cpp)

# 添加可执行文件
add_executable(HelloGL ${SOURCES})

# 链接三方库库
target_link_libraries(HelloGL ${GLFW_LINK} ${ASSIMP_LINK})
target_link_libraries(HelloGL ${FFMPEG_AVFORMAT_LINK} ${FFMPEG_AVCODEC_LINK} ${FFMPEG_SWSCALE_LINK} ${FFMPEG_AVUTIL_LINK})

# 链接系统的 OpenGL 框架和其他必要的库
if (APPLE)
    target_link_libraries(HelloGL "-framework OpenGL")
    target_link_libraries(HelloGL "-framework Cocoa")
endif()