cmake_minimum_required(VERSION 3.6)
project(native-src LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 17)


#set path
set(PATH_TO_FFMEAG ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/ffmpeg)
set(PATH_TO_FDKAAC ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/fdk_aac)
set(PATH_TO_GL3   ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/gl3)
#set(PATH_TO_X264 ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/x264)
set(PATH_TO_JNIENV ${CMAKE_CURRENT_SOURCE_DIR}/JNIEnv)
set(PATH_TO_BASEDEFINE ${CMAKE_CURRENT_SOURCE_DIR}/basedefine)
set(PATH_TO_UTILS ${CMAKE_CURRENT_SOURCE_DIR}/utils)
set(PATH_TO_READER ${CMAKE_CURRENT_SOURCE_DIR}/reader)
set(PATH_TO_GL ${CMAKE_CURRENT_SOURCE_DIR}/gl)
set(PATH_TO_SL ${CMAKE_CURRENT_SOURCE_DIR}/sl)
set(PATH_TO_THREAD ${CMAKE_CURRENT_SOURCE_DIR}/thread)
set(PATH_TO_RENDER ${CMAKE_CURRENT_SOURCE_DIR}/render)
set(PATH_TO_OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/output)
set(PATH_TO_PLAYER ${CMAKE_CURRENT_SOURCE_DIR}/player)

#set include
include_directories(./empty.h)
include_directories(${PATH_TO_FFMEAG}/include)
include_directories(${PATH_TO_FDKAAC}/include)
include_directories(${PATH_TO_GL3}/*.h)
#include_directories(${PATH_TO_X264}/include)
include_directories(${PATH_TO_JNIENV}/*.h)
include_directories(${PATH_TO_BASEDEFINE}/*.h)
include_directories(${PATH_TO_UTILS}/*.h)
include_directories(${PATH_TO_READER}/*/*.h)
include_directories(${PATH_TO_READER}/*.h)
include_directories(${PATH_TO_GL}/*.h)
include_directories(${PATH_TO_SL}/*.h)
include_directories(${PATH_TO_THREAD}/*.h)
include_directories(${PATH_TO_RENDER}/*.h)
include_directories(${PATH_TO_OUTPUT}/*.h)
include_directories(${PATH_TO_PLAYER}/*.h)

#add dynmic library
#add_library(libx264 SHARED IMPORTED)
#set_target_properties(libx264 PROPERTIES IMPORTED_LOCATION ${PATH_TO_X264}/libs/${ANDROID_ABI}/libx264.so)
add_library(libfdk-aac SHARED IMPORTED)
set_target_properties(libfdk-aac PROPERTIES IMPORTED_LOCATION ${PATH_TO_FDKAAC}/libs/${ANDROID_ABI}/libfdk-aac.so)
add_library(libavcodec SHARED IMPORTED)
set_target_properties(libavcodec PROPERTIES IMPORTED_LOCATION ${PATH_TO_FFMEAG}/libs/${ANDROID_ABI}/libavcodec.so)
add_library(libavdevice SHARED IMPORTED)
set_target_properties(libavdevice PROPERTIES IMPORTED_LOCATION ${PATH_TO_FFMEAG}/libs/${ANDROID_ABI}/libavdevice.so)
add_library(libavfilter SHARED IMPORTED)
set_target_properties(libavfilter PROPERTIES IMPORTED_LOCATION ${PATH_TO_FFMEAG}/libs/${ANDROID_ABI}/libavfilter.so)
add_library(libavformat SHARED IMPORTED)
set_target_properties(libavformat PROPERTIES IMPORTED_LOCATION ${PATH_TO_FFMEAG}/libs/${ANDROID_ABI}/libavformat.so)
add_library(libavutil SHARED IMPORTED)
set_target_properties(libavutil PROPERTIES IMPORTED_LOCATION ${PATH_TO_FFMEAG}/libs/${ANDROID_ABI}/libavutil.so)
add_library(libswresample SHARED IMPORTED)
set_target_properties(libswresample PROPERTIES IMPORTED_LOCATION ${PATH_TO_FFMEAG}/libs/${ANDROID_ABI}/libswresample.so)
add_library(libswscale SHARED IMPORTED)
set_target_properties(libswscale PROPERTIES IMPORTED_LOCATION ${PATH_TO_FFMEAG}/libs/${ANDROID_ABI}/libswscale.so)

#add src cpp
file(GLOB FILES_EMPTY ./empty.cpp)
file(GLOB FILES_JNI_ENV ${PATH_TO_JNIENV}/*.cpp)
file(GLOB FILES_UTILS ${PATH_TO_UTILS}/*.cpp)
file(GLOB FILES_DECODER ${PATH_TO_READER}/*/*.cpp
                        ${PATH_TO_READER}/*.cpp)
file(GLOB FILES_GL ${PATH_TO_GL}/*.cpp)
file(GLOB FILES_SL ${PATH_TO_SL}/*.cpp)
file(GLOB FILES_3RDGL3 ${PATH_TO_GL3}/*.c)
file(GLOB FILES_THREAD ${PATH_TO_THREAD}/*.cpp)
file(GLOB FILES_RENDER ${PATH_TO_RENDER}/*.cpp)
file(GLOB FILES_BASE   ${PATH_TO_BASEDEFINE}/*.cpp)
file(GLOB FILES_OUTPUT ${PATH_TO_OUTPUT}/*.cpp)
file(GLOB FILES_PLAYER ${PATH_TO_PLAYER}/*.cpp)

add_library(native-src
            SHARED
           ${FILES_EMPTY}
           ${FILES_JNI_ENV}
           ${FILES_UTILS}
           ${FILES_DECODER}
           ${FILES_CACHE}
           ${FILES_GL}
           ${FILES_SL}
           ${FILES_3RDGL3}
           ${FILES_THREAD}
           ${FILES_RENDER}
           ${FILES_BASE}
           ${FILES_OUTPUT}
           ${FILES_PLAYER})

target_link_libraries(
        native-src
        log
        android
        GLESv2
        EGL
        z
        OpenSLES
#        libx264
        libfdk-aac
        libavcodec
        libavdevice
        libavfilter
        libavformat
        libavutil
        libswresample
        libswscale
)