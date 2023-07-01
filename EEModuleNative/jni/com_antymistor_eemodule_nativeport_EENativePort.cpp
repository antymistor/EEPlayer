//
// Created by ByteDance on 2023/5/30.
//
#include "com_antymistor_eemodule_nativeport_EENativePort.h"
#include <string>
#include <android/native_window.h>
#include <android/native_window_jni.h>
#include <jni.h>
#include "../src/empty.h"
#include "../src/JNIEnv/EEJNIEnv.h"
#include "../src/basedefine/EECodeDefine.h"
#include "../src/player/EEPlayer.h"

#define LOG_TAG "JNI_INTERFACE"
using POINTER = jlong;

class JavaObjectHolder{
public:
    JavaObjectHolder(JNIEnv *env, jobject obj){
        javaObject = (jobject)(env->NewGlobalRef(obj));
    }
    ~JavaObjectHolder(){
        JNIEnv* env_;
        EE::EEJNIEnv jnienv(&env_);
        env_->DeleteGlobalRef(javaObject);
    }
    jobject getObj(){
        return javaObject;
    }
private:
    jobject javaObject = nullptr;
};

JNIEXPORT jlong JNICALL Java_com_antymistor_eemodule_nativeport_EENativePort_nativeCreateJavaObjectHolder
        (JNIEnv *env, jclass clazz, jobject obj){
    auto* p = new JavaObjectHolder(env, obj);
    return (POINTER)p;
}

JNIEXPORT void JNICALL Java_com_antymistor_eemodule_nativeport_EENativePort_nativeReleaseJavaObjectHolder
        (JNIEnv *env, jclass clazz, jlong holder_pointer){
    if( 0 != holder_pointer){
        auto * p = (JavaObjectHolder *)holder_pointer;
        delete p;
    }
}

JNIEXPORT void JNICALL Java_com_antymistor_eemodule_nativeport_EENativePort_nativeLoadJNI(JNIEnv * env, jclass){
    EE::EEJNIEnv::setJVM(env);
    LOGI("nativeLoadJNI!")
}

JNIEXPORT jlong JNICALL Java_com_antymistor_eemodule_nativeport_EENativePort_nativeCreateEEPlayer
        (JNIEnv *env, jclass clazz){
    auto * p = new EE::EEPlayer();
    return (POINTER) p;
}

JNIEXPORT void JNICALL Java_com_antymistor_eemodule_nativeport_EENativePort_nativeEEPlayerUpdateFilePath
        (JNIEnv *env, jclass clazz, jlong pointer, jstring path){
    if( 0 != pointer){
        auto * p = (EE::EEPlayer *)pointer;
        const char *mSrcPath = env->GetStringUTFChars(path, nullptr);
        p->updateFilePath(mSrcPath);
        env->ReleaseStringUTFChars(path, mSrcPath);
    }
}

JNIEXPORT void JNICALL Java_com_antymistor_eemodule_nativeport_EENativePort_nativeEEPlayerUpdateDisplayWindow
        (JNIEnv *env,jclass clazz, jlong pointer, jobject surface) {
    if( 0 != pointer){
        auto * p = (EE::EEPlayer *)pointer;
        if (surface) {
            ANativeWindow *window;
            int width;
            int height;
            window = ANativeWindow_fromSurface(env, surface);
            height = ANativeWindow_getHeight(window);
            width = ANativeWindow_getWidth(window);
            p->updateDisplayWindow(window, width, height);
        }
    }
}

JNIEXPORT void JNICALL Java_com_antymistor_eemodule_nativeport_EENativePort_nativeEEPlayerStart
        (JNIEnv *env, jclass clazz, jlong pointer) {
    if( 0 != pointer){
        auto * p = (EE::EEPlayer *)pointer;
         p->start();
    }

}

JNIEXPORT jboolean JNICALL Java_com_antymistor_eemodule_nativeport_EENativePort_nativeEEPlayerSeekTo
        (JNIEnv *env, jclass clazz, jlong pointer, jfloat position){
    if( 0 != pointer){
        auto * p = (EE::EEPlayer *)pointer;
        return p->seekTo(position);
    }
    return false;
}

JNIEXPORT jboolean JNICALL Java_com_antymistor_eemodule_nativeport_EENativePort_nativeEEPlayerSeekBy
        (JNIEnv *env, jclass clazz, jlong pointer, jfloat distance) {
    if( 0 != pointer){
        auto * p = (EE::EEPlayer *)pointer;
        return p->seekBy(distance);
    }
    return false;
}

JNIEXPORT void JNICALL Java_com_antymistor_eemodule_nativeport_EENativePort_nativeEEPlayerStop
        (JNIEnv *env, jclass clazz, jlong pointer){
    if( 0 != pointer){
        auto * p = (EE::EEPlayer *)pointer;
        p->stop();
    }
}

JNIEXPORT jfloat JNICALL Java_com_antymistor_eemodule_nativeport_EENativePort_nativeEEPlayerGetProgress
        (JNIEnv *env, jclass clazz, jlong pointer){
    float progress = -1;
    if( 0 != pointer){
        auto * p = (EE::EEPlayer *)pointer;
        p->getProgress(progress);
    }
    return progress;
}

JNIEXPORT jobject JNICALL Java_com_antymistor_eemodule_nativeport_EENativePort_nativeEEPlayergetMediaInfo
        (JNIEnv *env,jclass clazz,jlong pointer) {
    if( 0 != pointer) {
        auto *p = (EE::EEPlayer *) pointer;
        auto nativeMediaInfo      = p->getMediaInfo();
        jclass EEPlayerMediaInfo  = env->FindClass("com/antymistor/eemodule/eeplayer/EEMediaInfo");
        jmethodID initMethod      = env->GetMethodID(EEPlayerMediaInfo, "<init>", "()V");
        jfieldID  videoMineType_p = env->GetFieldID(EEPlayerMediaInfo, "videoMineType","Ljava/lang/String;");
        jfieldID  videoDuration_p = env->GetFieldID(EEPlayerMediaInfo, "videoDuration", "J");
        jfieldID  videoWidth_p    = env->GetFieldID(EEPlayerMediaInfo, "videoWidth", "I");
        jfieldID  videoHeight_p    = env->GetFieldID(EEPlayerMediaInfo, "videoHeight", "I");
        jfieldID  videoRotate_p   = env->GetFieldID(EEPlayerMediaInfo, "videoRotate", "I");
        jfieldID  videofps_p      = env->GetFieldID(EEPlayerMediaInfo, "videofps", "I");
        jfieldID  audioMineType_p   = env->GetFieldID(EEPlayerMediaInfo, "audioMineType","Ljava/lang/String;");
        jfieldID  audioDuration_p   = env->GetFieldID(EEPlayerMediaInfo, "audioDuration", "J");
        jfieldID  audioChannels_p   = env->GetFieldID(EEPlayerMediaInfo, "audioChannels", "I");
        jfieldID  audioSampleRate_p = env->GetFieldID(EEPlayerMediaInfo, "audioSampleRate", "I");

        jobject   MediaInfoObj    = env->NewObject(EEPlayerMediaInfo, initMethod);
        env->SetObjectField(MediaInfoObj, videoMineType_p, env->NewStringUTF(nativeMediaInfo.videoMineType));
        env->SetLongField(MediaInfoObj, videoDuration_p, nativeMediaInfo.videoDuration);
        env->SetIntField(MediaInfoObj, videoWidth_p, nativeMediaInfo.videoWidth);
        env->SetIntField(MediaInfoObj, videoHeight_p, nativeMediaInfo.videoHeight);
        env->SetIntField(MediaInfoObj, videoRotate_p, nativeMediaInfo.videoRotate);
        env->SetIntField(MediaInfoObj, videofps_p, nativeMediaInfo.videofps);
        env->SetObjectField(MediaInfoObj, audioMineType_p, env->NewStringUTF(nativeMediaInfo.audioMineType));
        env->SetLongField(MediaInfoObj, audioDuration_p, nativeMediaInfo.audioDuration);
        env->SetIntField(MediaInfoObj, audioChannels_p, nativeMediaInfo.audioChannels);
        env->SetIntField(MediaInfoObj, audioSampleRate_p, nativeMediaInfo.audioSampleRate);
        return MediaInfoObj;
    }
    return nullptr;
}
JNIEXPORT void JNICALL Java_com_antymistor_eemodule_nativeport_EENativePort_nativeEEPlayerBuild
        (JNIEnv *env, jclass clazz, jlong pointer, jobject param, jlong playerholder){
    if( 0 != pointer){
        auto * p = (EE::EEPlayer *)pointer;
        jclass EEPlayerBuildParam = env->FindClass("com/antymistor/eemodule/eeplayer/EEPlayerBuildParam");
        jfieldID maxRenderWidth_p = env->GetFieldID(EEPlayerBuildParam, "maxRenderWidth", "I");
        jfieldID maxRenderHeight_P = env->GetFieldID(EEPlayerBuildParam, "maxRenderHeight", "I");
        jfieldID videoRenderFps_p = env->GetFieldID(EEPlayerBuildParam, "videoRenderFps", "I");
        jfieldID videoDisplayFitMode_p = env->GetFieldID(EEPlayerBuildParam, "videoDisplayFitMode", "I");
        jfieldID videoDisplayRotation_p = env->GetFieldID(EEPlayerBuildParam, "videoDisplayRotation","I");
        jfieldID startPlayTime_p = env->GetFieldID(EEPlayerBuildParam, "startPlayTime", "J");
        jfieldID extraFlag_p = env->GetFieldID(EEPlayerBuildParam, "extraFlag", "I");


        jclass     jclazz_player    = nullptr;
        jmethodID  jmethod_callback = nullptr;
        JavaObjectHolder * holderp  = nullptr;
        if( 0 != playerholder){
            holderp = (JavaObjectHolder *)playerholder;
            jclazz_player    = env->GetObjectClass(holderp->getObj());
            jmethod_callback = env->GetMethodID(jclazz_player, "onInfo", "(IFF)V");
        }

        EE::EEPlayer::EEPlayerBuildParam nativeparam = {
                .maxRenderWidth = env->GetIntField(param, maxRenderWidth_p),
                .maxRenderHeight = env->GetIntField(param, maxRenderHeight_P),
                .videoRenderFps = env->GetIntField(param, videoRenderFps_p),
                .videoDisplayFitMode = env->GetIntField(param, videoDisplayFitMode_p),
                .videoDisplayRotation = env->GetIntField(param, videoDisplayRotation_p),
                .startPlayTime = env->GetLongField(param, startPlayTime_p),
                .extraFlag = env->GetIntField(param, extraFlag_p),
                .callback =  [=](const EE::EEPlayer::EEPlayerCallBackType & type,const float & value1,const float & value2){
                                if(holderp == nullptr){
                                    return ;
                                }
                                JNIEnv* env_;
                                EE::EEJNIEnv jnienv(&env_);
                                env_->CallVoidMethod(holderp->getObj(), jmethod_callback, (int)type, (float)value1, (float)value2);
                            },
        };
        p->build(nativeparam);
    }
}

JNIEXPORT void JNICALL Java_com_antymistor_eemodule_nativeport_EENativePort_nativeEEPlayerResetVideoDisplayFitMode
        (JNIEnv *env, jclass clazz,  jlong pointer, jint video_display_fit_mode) {
    if( 0 != pointer){
        auto * p = (EE::EEPlayer *)pointer;
        p->resetVideoDisplayFitMode(video_display_fit_mode);
    }
}
JNIEXPORT void JNICALL Java_com_antymistor_eemodule_nativeport_EENativePort_nativeEEPlayerResetVideoRenderFps
        (JNIEnv *env,jclass clazz,  jlong pointer, jint video_render_fps) {
    if( 0 != pointer){
        auto * p = (EE::EEPlayer *)pointer;
        p->resetVideoRenderFps(video_render_fps);
    }
}

JNIEXPORT void JNICALL Java_com_antymistor_eemodule_nativeport_EENativePort_nativeEEPlayerResetVideoDisplayRotation
        (JNIEnv *env, jclass clazz,  jlong pointer, jint video_display_rotation){
    if( 0 != pointer){
        auto * p = (EE::EEPlayer *)pointer;
        p->resetVideoDisplayRotation(video_display_rotation);
    }
}

JNIEXPORT void JNICALL Java_com_antymistor_eemodule_nativeport_EENativePort_nativeEEPlayerResetVideoDisplayScale(
        JNIEnv *env, jclass clazz, jlong pointer, jfloat scale){
    if( 0 != pointer){
        auto * p = (EE::EEPlayer *)pointer;
        p->resetVideoDisplayScale(scale);
    }
}

JNIEXPORT void JNICALL Java_com_antymistor_eemodule_nativeport_EENativePort_nativeDestroyEEPlayer
        (JNIEnv *env,jclass clazz, jlong pointer){
    if( 0 != pointer){
        auto * p = (EE::EEPlayer *)pointer;
        delete p;
    }
}