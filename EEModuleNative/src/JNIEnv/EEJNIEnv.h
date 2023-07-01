//
// Created by ByteDance on 2023/5/27.
//

#ifndef CVTEXTREADER_EEJNIENV_H
#define CVTEXTREADER_EEJNIENV_H
#include <jni.h>
#include <pthread.h>
namespace EE {
    class EEJNIEnv {
        public:
            EEJNIEnv(JNIEnv **env);
            static void initJNIClass(JNIEnv *env_);
            static void setJVM(JNIEnv *env_);
            static void JNI_ThreadDestroyed(void *value);
        private:
            static JavaVM *g_jvm;
            static pthread_key_t mThreadKey;
    };
}


#endif //CVTEXTREADER_EEJNIENV_H
