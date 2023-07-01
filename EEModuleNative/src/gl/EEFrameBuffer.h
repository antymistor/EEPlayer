//
// Created by antymistor on 2023/6/5.
//

#ifndef CVTEXTREADER_EEFRAMEBUFFER_H
#define CVTEXTREADER_EEFRAMEBUFFER_H

#include "EETexture.h"
namespace EE {
    class EEFrameBuffer;
    class EEFrameBufferAllocator {
        public:
            EEFrameBufferAllocator(int initMaxCapcity = 3);
            std::shared_ptr<EEFrameBuffer> allocateEEFrameBuffer();
            ~EEFrameBufferAllocator();

        private:
            void cleanIdleFrameBuffer();
            struct EEFrameBufferAllocatorMembers;
            std::unique_ptr<EEFrameBufferAllocatorMembers> mMembers;
    };

    class EEFrameBuffer{
        friend class EEFrameBufferAllocator;
    public:
        GLuint getFrameBufferID() const;
        ~EEFrameBuffer();
        EEReturnCode bindEETexture(const std::shared_ptr<EETexture>& sptex) const;
        void excuteBind(bool isbind) const;
    private:
        GLuint mFramebufferID = 0;
        EEFrameBuffer();
    };
}


#endif //CVTEXTREADER_EEFRAMEBUFFER_H
