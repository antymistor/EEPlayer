//
// Created by ByteDance on 2023/5/27.
//

#include "EEJNIEnv.h"
#include "../basedefine/EECodeDefine.h"
#include "../reader/decoder/EEMediaCodecDecoder.h"
#include <sys/prctl.h>
#include <mutex>
#define LOG_TAG "EEJNIEnv"
namespace EE{
    JavaVM *EEJNIEnv::g_jvm = nullptr;
    pthread_key_t EEJNIEnv::mThreadKey = 0;
    EEJNIEnv::EEJNIEnv(JNIEnv **env) {
        int status = g_jvm->GetEnv((void **) (env), JNI_VERSION_1_6);
        if (status < 0) {
            char thread_name[16] = { 0 };
            prctl(PR_GET_NAME, (char *)(thread_name));
            JavaVMAttachArgs args;
            args.version = JNI_VERSION_1_6;
            args.name = thread_name;
            args.group = nullptr;
            if (g_jvm->AttachCurrentThread(env, (void *)(&args)) != JNI_OK) {
                LOGE("JNI AttachCurrentThread() failed");
                *env = nullptr;
                return;
            }
            pthread_setspecific(mThreadKey, (void *) (*env));
        }
    }


    void EEJNIEnv::JNI_ThreadDestroyed(void *value) {
        auto *env = (JNIEnv *) value;
        if (env != nullptr) {
            g_jvm->DetachCurrentThread();
            pthread_setspecific(mThreadKey, nullptr);
        }
    }

    void EEJNIEnv::initJNIClass(JNIEnv *env_){
        EEMediaCodecDecoder::initJniClass(env_);
    }

    void EEJNIEnv::setJVM(JNIEnv *env_) {
        static std::once_flag flag;
       // std::call_once(flag, [&](){
            LOGI("LoadJNI Finished");
            env_->GetJavaVM(&g_jvm);
            initJNIClass(env_);
            pthread_key_create(&mThreadKey, JNI_ThreadDestroyed);
       // });
    }
}