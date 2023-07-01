//
// Created by ByteDance on 2023/6/18.
//

#ifndef CVTEXTREADER_EEVIDEOOUTPUT_H
#define CVTEXTREADER_EEVIDEOOUTPUT_H

#include "EEOutput.h"
namespace EE {
    class EEVideoOutput : public EEOutput{
    public:
        EEVideoOutput();
        ~EEVideoOutput();
        EEReturnCode updateWindow(void* pANativeWindow, const EESize& windowsize ) override;
        EEReturnCode build(const EEOutputBuildParam& param) override;
        EEReturnCode start() override;
        EEReturnCode stop() override;
        EEReturnCode release() override;
        EEPlayState  getPlayState() override;
        EEReturnCode getCurrentPts(int64_t& pts) override;
        EEReturnCode hotUpdate(const EEOutputHotUpdateParam& param) override;

    private:
        EEReturnCode _start();
        struct EEVideoOutputMembers;
        std::unique_ptr<EEVideoOutputMembers> mMembers;
    };
}


#endif //CVTEXTREADER_EEVIDEOOUTPUT_H
