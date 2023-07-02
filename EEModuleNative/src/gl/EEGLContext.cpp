//
// Created by antymistor on 2023/6/5.
//

#include "EEGLContext.h"
#include <egl/eglext.h>
#include "../thirdparty/gl3/GL3Stub.h"
#include <mutex>
namespace EE {
    struct EEGLContext::GLContextBuildParam{
        EGLContext m_context = EGL_NO_CONTEXT;
        EGLContext m_sharedContext = EGL_NO_CONTEXT ;
        EGLDisplay m_display = EGL_NO_DISPLAY;
        EGLSurface m_surface = EGL_NO_SURFACE;
        EGLConfig  m_config = nullptr;
        std::unique_ptr<EEFrameBufferAllocator> mFrameBufferAllocator = nullptr;
    };

    std::shared_ptr<EEGLContext> EEGLContext::globalContext = nullptr;

    void EEGLContext::setStaticParam(GLContextStaticParam param){
        staticparam.glVersion = param.glVersion > 0 ? param.glVersion : staticparam.glVersion;
        staticparam.s_bitA    = param.s_bitA > 0    ? param.s_bitA    : staticparam.s_bitA;
        staticparam.s_bitR    = param.s_bitR > 0    ? param.s_bitR    : staticparam.s_bitR;
        staticparam.s_bitG    = param.s_bitG > 0    ? param.s_bitG    : staticparam.s_bitG;
        staticparam.s_bitB    = param.s_bitB > 0    ? param.s_bitB    : staticparam.s_bitB;
    }
    EEGLContext::GLContextStaticParam EEGLContext::staticparam = {3,8,8,8,8};


    EEGLContext::EEGLContext():upContext(std::make_unique<GLContextBuildParam>()){}
    EEReturnCode EEGLContext::initContext(EGLContext sharedContext, bool enableSharedFromGlobalContext){
        upContext->m_sharedContext = sharedContext;
        //Todo becareful of multithread build case
        if (nullptr == upContext->m_sharedContext && enableSharedFromGlobalContext) {
            if (globalContext != nullptr) {
                upContext->m_sharedContext = globalContext->getEGLContext();
            } else {
                globalContext = shared_from_this();
            }
        }
        EGLint major, minor;
        if ((upContext->m_display = eglGetDisplay(EGL_DEFAULT_DISPLAY)) == EGL_NO_DISPLAY ||
            !eglInitialize(upContext->m_display, &major, &minor)                                   ) {
            return EE_BADDISPLAY;
        }
        static std::once_flag onceflag;
        std::call_once(onceflag, [&](){
            staticparam.glVersion = (minor < 4 || staticparam.glVersion !=3 || !gl3stubInit() ) ? 2 : 3;
        });
        EGLint configSpec[] = { EGL_RED_SIZE, staticparam.s_bitR,
                                EGL_GREEN_SIZE, staticparam.s_bitG,
                                EGL_BLUE_SIZE, staticparam.s_bitB,
                                EGL_ALPHA_SIZE, staticparam.s_bitA,
                                EGL_RENDERABLE_TYPE, staticparam.glVersion == 3 ? EGL_OPENGL_ES3_BIT_KHR : EGL_OPENGL_ES2_BIT,
                                EGL_SURFACE_TYPE, EGL_WINDOW_BIT, EGL_NONE };
        EGLint numConfigs;
        if (!eglChooseConfig(upContext->m_display, configSpec, &upContext->m_config, 1, &numConfigs)) {
            release();
            return EE_BADCONFIG;
        }
        EGLint attribList[] { EGL_CONTEXT_CLIENT_VERSION, staticparam.glVersion, EGL_NONE };
        if( (upContext->m_context = eglCreateContext(upContext->m_display, upContext->m_config, upContext->m_sharedContext, attribList)) == EGL_NO_CONTEXT){
            return EE_BADCONTEXT;
        }
        return EE_OK;
    }

    EEReturnCode EEGLContext::initSurface(EGLNativeWindowType window, EEEGLConfigMode configmode, EESize size){
        EGLint pBufferAttrib[] = { EGL_WIDTH, size.width, EGL_HEIGHT, size.height, EGL_LARGEST_PBUFFER, EGL_TRUE, EGL_NONE };
        switch (configmode) {
            case PIXEL_BUFFER_MODE:
                upContext->m_surface = eglCreatePbufferSurface(upContext->m_display, upContext->m_config, pBufferAttrib);
                break;
            case WINDOW_MODE:
                pBufferAttrib[4] = EGL_NONE;
                pBufferAttrib[5] = EGL_NONE;
                upContext->m_surface = eglCreateWindowSurface(upContext->m_display, upContext->m_config, window, nullptr);
                break;
            case PIXMAP_MODE:
                upContext->m_surface = eglCreatePixmapSurface(upContext->m_display, upContext->m_config, (EGLNativePixmapType)window, pBufferAttrib);
                break;
            default: return EE_FAIL;
        }
        if(upContext->m_surface == EGL_NO_SURFACE){
            return EE_BADSURFACE;
        }
        return EE_OK;
    }

    EEReturnCode EEGLContext::makeCurrent(EGLSurface surface){
        surface = surface == EGL_NO_SURFACE ? upContext->m_surface : surface;
        if(surface == EGL_NO_SURFACE || !eglMakeCurrent(upContext->m_display, surface, surface, upContext->m_context)){
            return EE_FAIL;
        }
        return EE_OK;
    }

    EEReturnCode EEGLContext::doneCurrent(){
        if (upContext->m_display != EGL_NO_DISPLAY) {
            eglMakeCurrent(upContext->m_display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
            return EE_OK;
        }
        return EE_FAIL;
    }

    EEReturnCode EEGLContext::swapBuffers(){
        return eglSwapBuffers(upContext->m_display, upContext->m_surface) ? EE_OK : EE_FAIL;
    }

    EGLContext EEGLContext::getEGLContext(){
        return upContext->m_context;
    }

    std::shared_ptr<EEFrameBuffer> EEGLContext::getEEFrameBuffer(){
        if(upContext->mFrameBufferAllocator == nullptr){
            upContext->mFrameBufferAllocator = std::make_unique<EEFrameBufferAllocator>();
        }
        return  upContext->mFrameBufferAllocator->allocateEEFrameBuffer();
    }

    void EEGLContext::release(){
        if(upContext->m_context != EGL_NO_CONTEXT && upContext->m_display != EGL_NO_DISPLAY) {
            glFinish();
            eglMakeCurrent(upContext->m_display, EGL_NO_SURFACE, EGL_NO_SURFACE,EGL_NO_CONTEXT);
            if (upContext->m_surface != EGL_NO_SURFACE) {
               eglDestroySurface(upContext->m_display, upContext->m_surface);
            }
            eglDestroyContext(upContext->m_display, upContext->m_context);
            eglTerminate(upContext->m_display);
            upContext->m_display = EGL_NO_DISPLAY;
            upContext->m_surface = EGL_NO_SURFACE;
            upContext->m_context = EGL_NO_CONTEXT;
        }
    }
    EEGLContext::~EEGLContext(){
        upContext->mFrameBufferAllocator = nullptr;
        release();
    }

}