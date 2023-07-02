//
// Created by antymistor on 2023/6/13.
//

#ifndef CVTEXTREADER_EEREADER_H
#define CVTEXTREADER_EEREADER_H

#include <string>
#include "../gl/EEGLContext.h"
#include "../basedefine/EECodeDefine.h"
#include "../gl/EETexture.h"
#include "decoder/EEVideoDecoder.h"
#include "decoder/EEAudioDecoder.h"
namespace EE {
    class EEReader {
    public:
        struct EEReaderUpdateParam{
            const char* srcFilePath;
            EEStreamType type = NoType;
            int maxCacheSize = 5;
            EEReaderUpdateParam (const char* path = nullptr, EEStreamType type_ = VideoType) :
                    srcFilePath(path), type(type_){}
        };
        struct EEReaderBuildParam{
            EESize maxVideoFrameSize = {0,0};
            std::shared_ptr<EEGLContext> sharedGLContext = nullptr;
            std::shared_ptr<EETextureAllocator> sharedTextureAllocator = nullptr;
            EEReaderBuildParam ( const EESize& size = {0, 0},
                                 const std::shared_ptr<EEGLContext>& glContext = nullptr,
                                 const std::shared_ptr<EETextureAllocator>& textureAllocator = nullptr) :
                    maxVideoFrameSize(size), sharedGLContext(glContext),sharedTextureAllocator(textureAllocator) {}
        };
        EEReader();
        ~EEReader();
        EEReturnCode update(const EEReaderUpdateParam& param);
        EEMediaInfo  getMediaInfo();
        EEReturnCode build(const EEReaderBuildParam& param);
        EEReturnCode startFrom(long timestamp = 0, int64_t tolerance = -1);
        EEReturnCode flush();
        EEReturnCode stop();
        EEReturnCode seekTo(int64_t pts, int64_t tolerance = -1);
        std::shared_ptr<EEFrame> get(bool wait = false);
        int64_t getSeekTime();
        EESize  getFrameSize();
    private:
       struct EEVideoReaderMembers;
       std::unique_ptr<EEVideoReaderMembers> mMembers;
    };
}


#endif //CVTEXTREADER_EEREADER_H
