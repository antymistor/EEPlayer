//
// Created by ByteDance on 2023/6/4.
//

#ifndef CVTEXTREADER_EEMEDIACODECDECODER_H
#define CVTEXTREADER_EEMEDIACODECDECODER_H

#include <memory>
#include <string>
#include <jni.h>
#include "../../gl/EETexture.h"
#include "../../basedefine/EECodeDefine.h"
namespace EE {
    class EEMediaCodecDecoder {
    public:
        struct DecoderinitParam{
            const char* minetype                                          = "";
            EESize     inSize                                             = {1080,1920};
            std::shared_ptr<EETexture> externalOESTexture                 = nullptr;
            char* specConfig_0                                            = nullptr;
            uint64_t specConfig_0_len                                     = 0;
            char* specConfig_1                                            = nullptr;
            uint64_t specConfig_1_len                                     = 0;
        };
        struct DecodeReturnParam{
            long decodedTimeStamp    = 0;
            EEReturnCode returnCode  = EE_OK;
            EEFrameType keyType = FLAG_NORMAL_FRAME;
        };
        EEMediaCodecDecoder();
        ~EEMediaCodecDecoder();
        static EEReturnCode initJniClass(JNIEnv *env_); //call from main thread
        EEReturnCode init(const DecoderinitParam& param);
        EEReturnCode decode(char* buffer, int buffersize, long timestamp, DecodeReturnParam* param = nullptr);
        EEReturnCode flush();

    private:
        static jclass decoderClass;
        EEReturnCode initJniMethod(JNIEnv* env_);
        struct EEMediaCodecDecoderMembers;
        std::unique_ptr<EEMediaCodecDecoderMembers> mMembers;
    };
}


#endif //CVTEXTREADER_EEMEDIACODECDECODER_H
