//
// Created by ByteDance on 2023/6/12.
//

#ifndef CVTEXTREADER_EEVIDEODECODER_H
#define CVTEXTREADER_EEVIDEODECODER_H
#include "../EEMediaDefine.h"
#include "EEDecoder.h"
namespace EE {
    class EEVideoDecoder : public EEDecoder {
        public:
            EEVideoDecoder();
            ~EEVideoDecoder() override;
            EEReturnCode build(const EEDecoder::EEDecoderBuildParam& param)  override;
            std::shared_ptr<EEFrame> decode(std::shared_ptr<EEPacket> packt, bool skipRender)  override;
            EEReturnCode flush() override;
            EEReturnCode release() override;
            EESize getFrameSize() override;
        private:
            struct EEVideoDecoderMembers;
            std::unique_ptr<EEVideoDecoderMembers> mMembers;
    };
}


#endif //CVTEXTREADER_EEVIDEODECODER_H
