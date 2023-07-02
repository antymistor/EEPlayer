//
// Created by antymistor on 2023/6/18.
//

#include "EEVideoOutput.h"
#include "../render/EETextureCopyRender.h"
#include "../thread/EETaskQueue.h"
#include "../utils/EECommonUtils.h"
namespace EE{
    struct EEVideoOutput::EEVideoOutputMembers{
        std::unique_ptr<EETextureCopyRender> mRender = nullptr;
        std::shared_ptr<EETextureCopyRender::EETextureCopyRenderParam> mRenderParam = nullptr;
        std::unique_ptr<EETextureCopyRender>  mRender_blurr = nullptr;
        std::shared_ptr<EETextureCopyRender::EETextureCopyRenderParam>   mRenderParam_blurr = nullptr;
        std::shared_ptr<EEGLContext> mGLContext = nullptr;
        std::unique_ptr<EETaskQueue> mTaskQueue = nullptr;
        std::function<std::shared_ptr<EEFrame>()> mPullFrameFun = nullptr;
        int fps = 30;
        EEViewFillMode fillmode = IN_CENTER_BY_FIT;
        int64_t displayRotation = 0;
        float videoDisplayScale = 1.0f;
        int32_t extraFlag = 0x00;
        bool enableHQDisplay = false;
        bool enableBlurrBackGround = false;
        EESize srcFrameSize;

        bool isUpdateStarted = false;
        bool isUpdateView    = false;
        EEPlayState state = Stoped;
        int64_t pts = 0;

        std::mutex mMutex;
        std::condition_variable mCondition;
    };

    EEVideoOutput::EEVideoOutput() : mMembers(std::make_unique<EEVideoOutputMembers>()){}
    EEVideoOutput::~EEVideoOutput(){
        EEVideoOutput::release();
    }

    EEReturnCode EEVideoOutput::updateWindow(void* pANativeWindow, const EESize& windowsize ){
        if(mMembers->isUpdateView){
            return EE_OK;
        }else{
            mMembers->isUpdateView = true;
        }
        mMembers->mTaskQueue->run(EETask([&, _pANativeWindow = pANativeWindow, _windowsize = windowsize](){
            // init surface
            mMembers->mGLContext->initSurface( (EGLNativeWindowType)_pANativeWindow, WINDOW_MODE, _windowsize);
            mMembers->mGLContext->makeCurrent();
            mMembers->mGLContext->makeCurrent();
            //  build RenderParam
            mMembers->mRenderParam = std::make_shared<EETextureCopyRender::EETextureCopyRenderParam>();
            mMembers->mRenderParam->fillmode = mMembers->fillmode;
            mMembers->mRenderParam->isFlipByY = true;
            if(mMembers->enableHQDisplay){
                mMembers->mRenderParam->customFragshader = sharp2DFragmentCode;
                mMembers->mRenderParam->paramList = {(float)mMembers->srcFrameSize.width, (float)mMembers->srcFrameSize.height, -0.2, 1};
            }else{
                mMembers->mRenderParam->customFragshader = base2DFragmentCode;
            }
            mMembers->mRenderParam->destFramebuffer = nullptr;
            mMembers->mRenderParam->rotation = (int)mMembers->displayRotation;
            mMembers->mRenderParam->destRenderArea = {_windowsize.width, _windowsize.height};
            //build Render
            mMembers->mRender = std::make_unique<EETextureCopyRender>();
            mMembers->mRender->build(mMembers->mRenderParam);

            if(mMembers->enableBlurrBackGround){
                //build mRenderParam_blurr
                mMembers->mRenderParam_blurr = std::make_shared<EETextureCopyRender::EETextureCopyRenderParam>();
                mMembers->mRenderParam_blurr->fillmode = IN_CENTER_BY_STRETCH;
                mMembers->mRenderParam_blurr->paramList = {(float)mMembers->srcFrameSize.width, (float)mMembers->srcFrameSize.height, 0.08, 40};
                mMembers->mRenderParam_blurr->isFlipByY = true;
                mMembers->mRenderParam_blurr->customFragshader = gauss2DFragmentCode;
                mMembers->mRenderParam_blurr->destFramebuffer = nullptr;
                mMembers->mRenderParam_blurr->rotation = (int)mMembers->displayRotation;
                mMembers->mRenderParam_blurr->destRenderArea = {_windowsize.width, _windowsize.height};
                //build blurr
                mMembers->mRender_blurr = std::make_unique<EETextureCopyRender>();
                mMembers->mRender_blurr->build(mMembers->mRenderParam_blurr);
            }
        }, true));
        _start();
        return EE_OK;
    }

    EEReturnCode EEVideoOutput::build(const EEOutputBuildParam& param){
        EEVideoOutput::release();
        mMembers->fillmode = param.fillmode;
        mMembers->mPullFrameFun = param.pullFun;
        mMembers->fps = param.fps;
        mMembers->extraFlag = param.extraFlag;
        mMembers->srcFrameSize = param.srcSize;
        mMembers->enableBlurrBackGround = (mMembers->extraFlag & ENABLE_BLURR_BACKGROUND_DISPLAY) == ENABLE_BLURR_BACKGROUND_DISPLAY;
        mMembers->enableHQDisplay = (mMembers->extraFlag & ENABLE_HIGH_Q_DISPLAY) == ENABLE_HIGH_Q_DISPLAY;
        mMembers->displayRotation = param.videoDisplayRotation;
        mMembers->mTaskQueue = std::make_unique<EETaskQueue>(EECommonUtils::createGlobalName("VidDisplay"));
        mMembers->mTaskQueue->run(EETask([&, sharedContext = param.sharedGlContext](){
            //build glcontext
            mMembers->mGLContext = std::make_shared<EEGLContext>();
            mMembers->mGLContext->initContext(sharedContext == nullptr? nullptr : sharedContext->getEGLContext() ,
                                              true);
        }, true));
        return EE_OK;
    }

    EEReturnCode EEVideoOutput::hotUpdate(const EEOutputHotUpdateParam& param){
        mMembers->mRenderParam->rotation = (int)param.videoDisplayRotation;
        mMembers->mRenderParam->fillmode = param.fillmode;
        mMembers->mRenderParam->scale = param.videoDisplayScale;
        mMembers->fps = param.fps;
        return EE_OK;
    }

    EEReturnCode EEVideoOutput::start(){
        if( mMembers->state == Playing){
            return EE_FAIL;
        }
        mMembers->isUpdateStarted = true;
        _start();
        return EE_OK;
    }

    EEReturnCode EEVideoOutput::_start(){
        if(mMembers->isUpdateStarted && mMembers->isUpdateView) {
            mMembers->mTaskQueue->run(EETask([&]() {
                mMembers->state = Playing;
                while (mMembers->state == Playing) {
                    auto frame = mMembers->mPullFrameFun();
                    if(frame != nullptr && frame->frame != nullptr){
                        glClearColor(0, 0, 0, 0);
                        glClear(GL_COLOR_BUFFER_BIT);
                        if(mMembers->mRenderParam_blurr &&  mMembers->mRender_blurr){
                            mMembers->mRenderParam_blurr->srcTexture = frame->frame;
                            mMembers->mRender_blurr->render();
                        }
                        mMembers->mRenderParam->srcTexture = frame->frame;
                        mMembers->mRender->render();
                        mMembers->mGLContext->swapBuffers();
                        mMembers->pts = frame->timestamp;
                    }
                    std::unique_lock<std::mutex> lock(mMembers->mMutex);
                    mMembers->mCondition.wait_for(lock, std::chrono::milliseconds(1000 / mMembers->fps));
                }
                mMembers->mCondition.notify_all();
                mMembers->state = Stoped;
            }, true));
        }
        return EE_OK;
    }
    EEReturnCode EEVideoOutput::stop(){
        if(mMembers->state == Playing) {
            mMembers->state = Stoped;
            mMembers->isUpdateStarted = false;
            mMembers->mCondition.notify_all();
            std::unique_lock<std::mutex> lock(mMembers->mMutex);
            mMembers->mCondition.wait(lock);
            mMembers->mCondition.notify_all();
        }
        return EE_OK;
    }
    EEPlayState  EEVideoOutput::getPlayState(){
        return mMembers->state;
    }

    EEReturnCode  EEVideoOutput::getCurrentPts(int64_t& pts) {
        pts = mMembers->pts / 1000; //convert to ms unit
        return EE_OK;
    }

    EEReturnCode EEVideoOutput::release(){
        EEVideoOutput::stop();
        if(mMembers->mTaskQueue){
            mMembers->mTaskQueue->run(EETask([&](){
                mMembers->mRender = nullptr;
                mMembers->mRenderParam = nullptr;
                mMembers->mGLContext = nullptr;
            }));
        }
        return EE_OK;
    }
}