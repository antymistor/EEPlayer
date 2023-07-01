cmake_minimum_required(VERSION 3.6)
project(eemodule-lib LANGUAGES C CXX)
message("start build")
set(CMAKE_CXX_STANDARD 17)


set(PATH_TO_JNI_LAYER ${CMAKE_CURRENT_SOURCE_DIR}/../EEModuleNative/jni)
set(PATH_TO_EEMODULE ${CMAKE_CURRENT_SOURCE_DIR}/../EEModuleNative/src)
set(PATH_TO_EEMODULE_JNIENV ${PATH_TO_EEMODULE}/JNIEnv)

include_directories(${PATH_TO_EEMODULE_JNIENV})
include_directories(${PATH_TO_JNI_LAYER})

file(GLOB FILES_JNI_LAYER ${PATH_TO_JNI_LAYER}/*.cpp)

add_subdirectory(${PATH_TO_EEMODULE} native-src.out)
add_library(eemodule-lib SHARED
          ${FILES_JNI_LAYER})
target_link_libraries(
                        eemodule-lib
                        native-src
                        log
                        android
                        GLESv2
                        EGL
                        z
                        OpenSLES
                        )

