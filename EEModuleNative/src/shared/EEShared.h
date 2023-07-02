//
// Created by antymistor on 2023/6/6.
//

#ifndef CVTEXTREADER_EEShared_H
#define CVTEXTREADER_EEShared_H
//#include "../basedefine/EECodeDefine.h"
//#include "../thread/EETaskQueue.h"
//#include "../gl/EEGLContext.h"
//#include "../gl/EETexture.h"
//#include <mutex>
#include <memory>


namespace EE {
    class EEShared {
    public:
        EEShared(const char* name);
        ~EEShared();
//        std::shared_ptr<EEGLContext> getSharedGLContext();
//        std::shared_ptr<EETextureAllocator> getSharedTextureAllocator();
//        std::shared_ptr<EETaskQueue> getSharedTaskQueue();
        struct EESharedMembers;
        std::unique_ptr<EESharedMembers> mMembers;
    };
}


#endif //CVTEXTREADER_EEShared_H