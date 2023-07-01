//
// Created by ByteDance on 2023/6/18.
//

#ifndef CVTEXTREADER_EEOUTPUT_H
#define CVTEXTREADER_EEOUTPUT_H

#include "../basedefine/EECodeDefine.h"
#include "../reader/EEMediaDefine.h"
#include "EEOutputDefine.h"
namespace EE {
    class EEOutput {
    public:
        struct EEOutputBuildParam{
            //For Audio
            int channelCnt = 0;
            int sampleRate = 0;
            //For Video
            EEViewFillMode fillmode = IN_CENTER_BY_FIT;
            EESize srcSize = {0,0};
            int fps;
            int64_t videoDisplayRotation = 0;
            //ForBoth
            int32_t extraFlag = 0x00;
            std::function<std::shared_ptr<EEFrame>()> pullFun;
        };

        struct EEOutputHotUpdateParam{
            EEViewFillMode fillmode = IN_CENTER_BY_FIT;
            int fps = 30;
            int64_t videoDisplayRotation = 0;
            float   videoDisplayScale    = 0;
        };

        virtual EEReturnCode updateWindow(void* pANativeWindow, const EESize& windowsize );
        virtual EEReturnCode getCurrentPts(int64_t& pts);
        virtual EEReturnCode build(const EEOutputBuildParam& param);
        virtual EEReturnCode hotUpdate(const EEOutputHotUpdateParam& param);
        virtual EEReturnCode start();
        virtual EEReturnCode mute(bool ismute);
        virtual EEReturnCode stop();
        virtual EEReturnCode release() ;
        virtual EEReturnCode flush() ;
        virtual EEPlayState getPlayState();
        virtual ~EEOutput() = default;
    };
}


#endif //CVTEXTREADER_EEOUTPUT_H
