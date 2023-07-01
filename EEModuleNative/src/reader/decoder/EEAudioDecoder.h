//
// Created by antymistor on 2023/6/13.
//

#ifndef CVTEXTREADER_EEAUDIODECODER_H
#define CVTEXTREADER_EEAUDIODECODER_H
#include "../EEMediaDefine.h"
#include "EEDecoder.h"
namespace EE {
    //It's seemed that using mediacodec on audio decode is not ordinary, so use ffmpeg finally
    class EEAudioDecoder : public EEDecoder {
    public:
        EEAudioDecoder();
        ~EEAudioDecoder() override;
        EEReturnCode build(const EEDecoder::EEDecoderBuildParam& param)  override;
        std::shared_ptr<EEFrame> decode(std::shared_ptr<EEPacket> packt, bool skipRender)  override;
        EEReturnCode flush()  override;
        EEReturnCode release() override;
        EESize getFrameSize() override;
    private:
        struct EEAudioDecoderMembers;
        std::unique_ptr<EEAudioDecoderMembers> mMembers;
    };
}

#endif //CVTEXTREADER_EEAUDIODECODER_H
