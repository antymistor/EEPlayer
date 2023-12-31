/* DO NOT EDIT THIS FILE - it is machine generated */
#include <jni.h>
/* Header for class com_antymistor_eemodule_nativeport_EENativePort */

#ifndef _Included_com_antymistor_eemodule_nativeport_EENativePort
#define _Included_com_antymistor_eemodule_nativeport_EENativePort
#ifdef __cplusplus
extern "C" {
#endif
JNIEXPORT void JNICALL Java_com_antymistor_eemodule_nativeport_EENativePort_nativeLoadJNI
  (JNIEnv *, jclass);
JNIEXPORT jlong JNICALL Java_com_antymistor_eemodule_nativeport_EENativePort_nativeCreateEEPlayer
        (JNIEnv *env, jclass clazz);
JNIEXPORT void JNICALL Java_com_antymistor_eemodule_nativeport_EENativePort_nativeEEPlayerUpdateFilePath
        (JNIEnv *env, jclass clazz, jlong pointer, jstring path);
JNIEXPORT void JNICALL Java_com_antymistor_eemodule_nativeport_EENativePort_nativeEEPlayerUpdateDisplayWindow
        (JNIEnv *env,jclass clazz, jlong pointer, jobject surface) ;
JNIEXPORT void JNICALL Java_com_antymistor_eemodule_nativeport_EENativePort_nativeEEPlayerStart
        (JNIEnv *env, jclass clazz, jlong pointer) ;
JNIEXPORT jboolean JNICALL Java_com_antymistor_eemodule_nativeport_EENativePort_nativeEEPlayerSeekTo
        (JNIEnv *env, jclass clazz, jlong pointer, jfloat position);
JNIEXPORT jboolean JNICALL Java_com_antymistor_eemodule_nativeport_EENativePort_nativeEEPlayerSeekBy
        (JNIEnv *env, jclass clazz, jlong pointer, jfloat distance) ;
JNIEXPORT void JNICALL Java_com_antymistor_eemodule_nativeport_EENativePort_nativeEEPlayerStop
        (JNIEnv *env, jclass clazz, jlong pointer);
JNIEXPORT jfloat JNICALL Java_com_antymistor_eemodule_nativeport_EENativePort_nativeEEPlayerGetProgress
        (JNIEnv *env, jclass clazz, jlong pointer);
JNIEXPORT jobject JNICALL Java_com_antymistor_eemodule_nativeport_EENativePort_nativeEEPlayergetMediaInfo
        (JNIEnv *env,jclass clazz,jlong pointer) ;
JNIEXPORT void JNICALL Java_com_antymistor_eemodule_nativeport_EENativePort_nativeEEPlayerBuild
        (JNIEnv *env, jclass clazz, jlong pointer, jobject param, jlong playerholder);
JNIEXPORT void JNICALL Java_com_antymistor_eemodule_nativeport_EENativePort_nativeEEPlayerResetVideoDisplayFitMode
        (JNIEnv *env, jclass clazz,  jlong pointer, jint video_display_fit_mode) ;
JNIEXPORT void JNICALL Java_com_antymistor_eemodule_nativeport_EENativePort_nativeEEPlayerResetVideoRenderFps
        (JNIEnv *env,jclass clazz,  jlong pointer, jint video_render_fps) ;
JNIEXPORT void JNICALL Java_com_antymistor_eemodule_nativeport_EENativePort_nativeEEPlayerResetVideoDisplayRotation
        (JNIEnv *env, jclass clazz,  jlong pointer, jint video_display_rotation);
JNIEXPORT void JNICALL Java_com_antymistor_eemodule_nativeport_EENativePort_nativeEEPlayerResetVideoDisplayScale(
        JNIEnv *env, jclass clazz, jlong pointer, jfloat scale);
JNIEXPORT void JNICALL Java_com_antymistor_eemodule_nativeport_EENativePort_nativeDestroyEEPlayer
        (JNIEnv *env,jclass clazz, jlong pointer);
JNIEXPORT jlong JNICALL Java_com_antymistor_eemodule_nativeport_EENativePort_nativeCreateJavaObjectHolder
        (JNIEnv *env, jclass clazz, jobject obj);

JNIEXPORT void JNICALL Java_com_antymistor_eemodule_nativeport_EENativePort_nativeReleaseJavaObjectHolder
        (JNIEnv *env, jclass clazz, jlong holder_pointer);

#ifdef __cplusplus
}
#endif
#endif