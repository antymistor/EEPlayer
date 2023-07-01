//
// Created by antymistor on 2023/6/14.
//

#ifndef CVTEXTREADER_EEDEMUXER_H
#define CVTEXTREADER_EEDEMUXER_H

#include "../EEMediaDefine.h"
namespace EE {
    class EEDemuxer {
        public:
            EEDemuxer();
            ~EEDemuxer();
            EEMediaInfo    build(const char* filepath, EEStreamType type ,int maxCacheSize = 5);
            EEReturnCode   startFrom(long timestamp = 0);
            EEReturnCode   stop();
            EEReturnCode   flush();  //stop --> flush --> startFrom
            EEReturnCode   release();
            std::shared_ptr<EEPacket> get();
            int64_t getSeekTime();

        private:
            struct EEDemuxerMembers;
            std::unique_ptr<EEDemuxerMembers> mMembers;
    };
}

#endif //CVTEXTREADER_EEDEMUXER_H
