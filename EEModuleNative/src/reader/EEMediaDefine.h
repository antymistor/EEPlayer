//
// Created by antymistor on 2023/6/14.
//

#ifndef CVTEXTREADER_EEMEDIADEFINE_H
#define CVTEXTREADER_EEMEDIADEFINE_H
#include "../gl/EETexture.h"

namespace EE {
#pragma mark - baseType
    enum EEStreamType{
        NoType,
        VideoType,
        AudioType,
        subtitleType
    };


#pragma mark - MediaInfo
    struct EEMediaInfoBase{
        //both
        EEStreamType type;
        int64_t codecID = 0;
        const char* mineType;
        const char* bsfName;
        long starttime = 0;
        long duration = 0;
        std::shared_ptr<void> extraObj = nullptr;

        //video related
        EESize videoSrcSize = {0,0};
        int rotate = 0;
        bool isFlipX = false;
        bool isFlipY = false;
        EERatio fps;

        //audio related
        int channels = 0;
        int samplerate = 0;
    };

    struct EEMediaInfo{
        EEMediaInfoBase baseinfo;
        std::shared_ptr<EEBuffer> extradata_0 = nullptr;
        std::shared_ptr<EEBuffer> extradata_1 = nullptr;
    };

#pragma mark - Packet

    struct EEPacket {
        EEStreamType type = NoType;
        std::shared_ptr<EEBuffer> buffer = nullptr;
        long timestamp = 0;
        long duration = 0;
        EEFrameType frameType = FLAG_NORMAL_FRAME;
        EEPacket(const char *buffer_,
                 long buffersize_,
                 long timestamp_ = 0,
                 long duration_ = 0,
                 EEStreamType type_ = NoType,
                 EEFrameType frameType_ = FLAG_NORMAL_FRAME) {
            buffer = std::make_shared<EEBuffer>(buffer_, buffersize_);
            timestamp = timestamp_;
            duration = duration_;
            type = type_;
            frameType = frameType_;
        }
    };

#pragma mark - Frame
    struct EEFrame {
        EEStreamType type = NoType;
        std::shared_ptr<EETexture> frame = nullptr;   // for videoframe
        std::shared_ptr<EEBuffer>  buffer = nullptr;  // for audioframe
        EESize size ;
        long timestamp = 0;
        long duration = 0;
        long nb_samples = 0;
        EEFrameType frameType = FLAG_NORMAL_FRAME;
        EEFrame(EEStreamType type_ = NoType, EEFrameType frameType_ = FLAG_NORMAL_FRAME) {
           type = type_;
           frameType = frameType_;
        }
    };

}



#endif //CVTEXTREADER_EEMEDIADEFINE_H
