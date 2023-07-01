//
// Created by antymistor on 2023/6/18.
//

#ifndef CVTEXTREADER_EEAUDIOOUTPUT_H
#define CVTEXTREADER_EEAUDIOOUTPUT_H

#include <SLES/OpenSLES_Android.h>
#include "EEOutput.h"
#include "EEOutputDefine.h"
namespace EE {
    class EEAudioOutput : public EEOutput{
        public:
            EEAudioOutput();
            ~EEAudioOutput();
            EEReturnCode getCurrentPts(int64_t& pts) override;
            EEReturnCode build(const EEOutputBuildParam& param) override;
            EEReturnCode start() override;
            EEReturnCode mute(bool ismute) override;
            EEReturnCode stop() override;
            EEReturnCode release() override;
            EEReturnCode flush() override;
            EEPlayState  getPlayState() override;

        private:
            static void bufferQueueCallback(SLAndroidSimpleBufferQueueItf caller,void *pContext);
            struct EEAudioOutputMembers;
            std::unique_ptr<EEAudioOutputMembers> mMembers;

    };
}
#endif //CVTEXTREADER_EEAUDIOOUTPUT_H
