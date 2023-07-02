//
// Created by antymistor on 2023/6/16.
//

#ifndef CVTEXTREADER_EEDECODER_H
#define CVTEXTREADER_EEDECODER_H
#include "../EEMediaDefine.h"
#include "../gl/EEGLContext.h"
namespace EE{
    class EEDecoder {
        public:
            struct EEDecoderBuildParam {
                EEMediaInfo mediainfo;
                EESize maxsize = {0, 0};
                std::shared_ptr<EEGLContext> sharedGLContext = nullptr;
                std::shared_ptr<EETextureAllocator> sharedTextureAllocator = nullptr;
            };
            virtual EEReturnCode build(const EEDecoder::EEDecoderBuildParam& param)  = 0;
            virtual std::shared_ptr<EEFrame> decode(std::shared_ptr<EEPacket> packt, bool skipRender) = 0;
            virtual EEReturnCode flush() =  0;
            virtual EEReturnCode release() = 0;
            virtual ~EEDecoder() = default;
            virtual EESize getFrameSize() = 0;
    };
}

#endif //CVTEXTREADER_EEDECODER_H
