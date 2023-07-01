//
// Created by antymistor on 2023/6/4.
//

#include "EEMediaCodecDecoder.h"
#include <jni.h>
#include "../../JNIEnv/EEJNIEnv.h"
#define LOG_TAG "EEMediaCodecDecoder"
namespace EE {
    struct EEMediaCodecDecoder::EEMediaCodecDecoderMembers{
        jobject                             jdecoder        = nullptr;
        jmethodID                           jdecoderinit    = nullptr;
        jmethodID                           jdecoderdecode  = nullptr;
        jmethodID                           jdecoderflush   = nullptr;
        jmethodID                           jdecoderrelease = nullptr;
        jbyteArray                          jPacketByteArray= nullptr;
        long long                           jPacketSize     = 0;
        EESize                              inSize;
        const char*                         minetype;
        std::shared_ptr<EETexture>          mOESTexture;
    };

    jclass EEMediaCodecDecoder::decoderClass = nullptr;
    EEReturnCode EEMediaCodecDecoder::initJniClass(JNIEnv *env_){
        if(decoderClass == nullptr) {
            jclass jclasst = env_->FindClass("com/antymistor/eemodule/decoderlib/EEMediaCodecDecoder");
            if (env_->ExceptionCheck()) {
                env_->ExceptionClear();
            }
            decoderClass = (jclass)(env_->NewGlobalRef(jclasst));
            env_->DeleteLocalRef(jclasst);
        }
        return EE_OK;
    }

    EEMediaCodecDecoder::EEMediaCodecDecoder():mMembers(std::make_unique<EEMediaCodecDecoderMembers>()){}

    EEReturnCode EEMediaCodecDecoder::initJniMethod(JNIEnv *env_) {
        if(decoderClass == nullptr) {
            return EE_FAIL;
        }
        mMembers->jdecoderinit    = env_->GetMethodID(decoderClass, "<init>", "(Ljava/lang/String;III[B[B)V");
        mMembers->jdecoderdecode  = env_->GetMethodID(decoderClass, "decodeFrame", "([BIJ)J");
        mMembers->jdecoderflush   = env_->GetMethodID(decoderClass, "flushDecoder", "()Z");
        mMembers->jdecoderrelease = env_->GetMethodID(decoderClass, "releaseDecoder", "()Z");
        return EE_OK;
    }

    EEReturnCode EEMediaCodecDecoder::init(const DecoderinitParam& param){
       //base sth
        mMembers->inSize = param.inSize;
        mMembers->minetype = param.minetype;
        mMembers->mOESTexture =  param.externalOESTexture;
        if(nullptr == mMembers->mOESTexture || 0 == mMembers->mOESTexture->getTexID()){
            return EE_BADCONTEXT;
        }
        //create jni obj
        JNIEnv* env_ = nullptr;
        EE::EEJNIEnv jnienv(&env_);
        initJniMethod(env_);
        jstring jminetype = env_->NewStringUTF(mMembers->minetype);
        jbyteArray specConfig_0_j = nullptr;
        jbyteArray specConfig_1_j = nullptr;
        if(param.specConfig_0_len > 0 && param.specConfig_0){
            specConfig_0_j = env_->NewByteArray((jsize)param.specConfig_0_len);
            env_->SetByteArrayRegion(specConfig_0_j, 0, (jsize)param.specConfig_0_len, (jbyte*) param.specConfig_0);
        }
        if(param.specConfig_1_len > 0 && param.specConfig_1){
            specConfig_1_j = env_->NewByteArray((jsize)param.specConfig_1_len);
            env_->SetByteArrayRegion(specConfig_1_j, 0, (jsize)param.specConfig_1_len, (jbyte*) param.specConfig_1);
        }
        jobject jObjecttemp = env_->NewObject(decoderClass, mMembers->jdecoderinit, jminetype,
                                              mMembers->inSize.width, mMembers->inSize.height,
                                              (int)mMembers->mOESTexture->getTexID(),
                                              specConfig_0_j,
                                              specConfig_1_j);
        mMembers->jdecoder = env_->NewGlobalRef(jObjecttemp);
        //release jtempobj
        env_->DeleteLocalRef(jObjecttemp);
        env_->DeleteLocalRef(jminetype);
        if(specConfig_0_j) {
            env_->DeleteLocalRef(specConfig_0_j);
        }
        if(specConfig_1_j) {
            env_->DeleteLocalRef(specConfig_1_j);
        }
       return EE_OK;
    }

    EEReturnCode EEMediaCodecDecoder::decode(char* buffer, int buffersize, long timestamp, DecodeReturnParam* param){
        JNIEnv* env_;
        EE::EEJNIEnv jnienv(&env_);
        //allcoate jPacketByteArray
        if(mMembers->jPacketSize < buffersize){
            if(mMembers->jPacketByteArray){
                env_->DeleteGlobalRef(mMembers->jPacketByteArray);
            }
            jbyteArray bytebuffer = env_->NewByteArray((jsize)buffersize);
            mMembers->jPacketByteArray = (jbyteArray)(env_->NewGlobalRef(bytebuffer));
            mMembers->jPacketSize = buffersize;
            env_->DeleteLocalRef(bytebuffer);
            LOGI("allocate new buffer newsize = %d", buffersize)
        }
        env_->SetByteArrayRegion(mMembers->jPacketByteArray, 0, (jsize)buffersize, (jbyte*) buffer);
        auto ret_ = env_->CallLongMethod(mMembers->jdecoder, mMembers->jdecoderdecode, mMembers->jPacketByteArray, buffersize, (jlong)timestamp);
        if(param){
            param->returnCode = static_cast<EEReturnCode>(ret_ & 0xf);
            param->keyType    = static_cast<EEFrameType>((ret_ & 0xf0) >> 4);
            param->decodedTimeStamp = ret_ >> 8;
        }
        return  EE_OK;
    }

    EEReturnCode EEMediaCodecDecoder::flush(){
        JNIEnv* env_;
        EE::EEJNIEnv jnienv(&env_);
        auto ret_ = env_->CallBooleanMethod(mMembers->jdecoder, mMembers->jdecoderflush);
        return ret_ == JNI_TRUE ? EE_OK : EE_FAIL;
    }

    EEMediaCodecDecoder::~EEMediaCodecDecoder(){
        JNIEnv* env_;
        EE::EEJNIEnv jnienv(&env_);
        env_->CallBooleanMethod(mMembers->jdecoder, mMembers->jdecoderrelease);
        env_->DeleteGlobalRef(mMembers->jdecoder);
        env_->DeleteGlobalRef(mMembers->jPacketByteArray);
    }
}



