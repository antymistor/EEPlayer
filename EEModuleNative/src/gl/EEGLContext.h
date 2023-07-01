//
// Created by ByteDance on 2023/6/5.
//

#ifndef CVTEXTREADER_EEGLCONTEXT_H
#define CVTEXTREADER_EEGLCONTEXT_H
#include <memory>
#include <EGL/egl.h>
#include "EEFrameBuffer.h"
#include "../basedefine/EECodeDefine.h"
#include <mutex>
namespace EE {
class EEGLContext : public std::enable_shared_from_this<EEGLContext>{
    public:
        struct GLContextStaticParam{
            int glVersion = 0;
            int s_bitR    = 0;
            int s_bitG    = 0;
            int s_bitB    = 0;
            int s_bitA    = 0;
        };

        static void setStaticParam(GLContextStaticParam param);
        EEGLContext();
        ~EEGLContext();
        EEReturnCode makeCurrent(EGLSurface surface = nullptr);
        EEReturnCode doneCurrent();
        EEReturnCode swapBuffers();

        EEReturnCode initContext(EGLContext sharedContext = nullptr, bool enableSharedFromGlobalContext = false);
        EEReturnCode initSurface(EGLNativeWindowType window , EEEGLConfigMode configmode, EESize size = {1920, 1920});
        EGLContext getEGLContext();
        std::shared_ptr<EEFrameBuffer> getEEFrameBuffer();

    private:
        void release();
        static GLContextStaticParam staticparam;
        struct GLContextBuildParam;
        std::unique_ptr <GLContextBuildParam> upContext;
        static std::shared_ptr<EEGLContext> globalContext;
    };
}


#endif //CVTEXTREADER_EEGLCONTEXT_H
