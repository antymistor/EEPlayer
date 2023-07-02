//
// Created by antymistor on 2023/6/12.
//

#include "EEVideoDecoder.h"
#include "EEMediaCodecDecoder.h"
#include "../../utils/EECommonUtils.h"
#include "../../thread/EETaskQueue.h"
#include "../../render/EETextureCopyRender.h"
#define LOG_TAG "EEVideoDecoder"
namespace EE{
    struct EEVideoDecoder::EEVideoDecoderMembers{
        EESize destSize= {0,0};
        EEMediaInfoBase videoinfo;
        std::unique_ptr<EEMediaCodecDecoder> mDecoder = nullptr;
        std::unique_ptr<EETextureCopyRender> mRender = nullptr;
        std::shared_ptr<EETextureCopyRender::EETextureCopyRenderParam> mRenderParam = nullptr;
        std::shared_ptr<EEGLContext> mGLContext = nullptr;
        std::unique_ptr<EETaskQueue> mTaskQueue = nullptr;
        std::shared_ptr<EETexture>   mOESTexture = nullptr;
        std::shared_ptr<EETextureAllocator> mSharedTextureAllocator = nullptr;
    };
    EEVideoDecoder::EEVideoDecoder() : mMembers(std::make_unique<EEVideoDecoderMembers>()){}
    EEVideoDecoder::~EEVideoDecoder(){
        EEVideoDecoder::release();
    }
    EESize  EEVideoDecoder::getFrameSize() {
        return mMembers->destSize;
    }
    EEReturnCode EEVideoDecoder::build(const EEDecoder::EEDecoderBuildParam& param){
        EEVideoDecoder::release();
        //copy info
        mMembers->videoinfo = param.mediainfo.baseinfo;
        mMembers->mSharedTextureAllocator = param.sharedTextureAllocator;
        //handle size
        mMembers->destSize = ( mMembers->videoinfo.rotate == 90 ||  mMembers->videoinfo.rotate == 270) ?
                             EESize(mMembers->videoinfo.videoSrcSize.height, mMembers->videoinfo.videoSrcSize.width) :
                             mMembers->videoinfo.videoSrcSize;
        EECommonUtils::resize(param.maxsize, mMembers->destSize);
        //build thread
        mMembers->mTaskQueue = std::make_unique<EETaskQueue>(EECommonUtils::createGlobalName("VidDecode"));
        mMembers->mTaskQueue->run(EETask([&, buffer = param.mediainfo.extradata_0, sharedContext = param.sharedGLContext](){
            //build glcontext
            mMembers->mGLContext = std::make_shared<EEGLContext>();
            mMembers->mGLContext->initContext(sharedContext == nullptr? nullptr : sharedContext->getEGLContext() ,
                                              true);
            mMembers->mGLContext->initSurface(nullptr, PIXEL_BUFFER_MODE);
            mMembers->mGLContext->makeCurrent();

            //build OES Texture
            EE::EEGLUtils::TextureBuildParam texbuildparam;
            texbuildparam.textype = GL_TEXTURE_EXTERNAL_OES;
            texbuildparam.size = mMembers->videoinfo.videoSrcSize;
            if( mMembers->mSharedTextureAllocator != nullptr){
                mMembers->mOESTexture = mMembers->mSharedTextureAllocator->allocateEETexture(texbuildparam);
            }else{
                mMembers->mOESTexture = EETextureAllocator::getSharedInstance()->allocateEETexture(texbuildparam);
            }
            mMembers->mOESTexture ->unRegisterFromAllocator();

            // build Decoder
            EEMediaCodecDecoder::DecoderinitParam param_;
            param_.minetype = mMembers->videoinfo.mineType;
            param_.inSize = mMembers->videoinfo.videoSrcSize;
            param_.specConfig_0 = buffer->data;
            param_.specConfig_0_len = buffer->size;
            param_.externalOESTexture = mMembers->mOESTexture;
            mMembers->mDecoder = std::make_unique<EEMediaCodecDecoder>();
            mMembers->mDecoder->init(param_);

            //build RenderParam
            mMembers->mRenderParam = std::make_shared<EETextureCopyRender::EETextureCopyRenderParam>();
            mMembers->mRenderParam->destRenderArea = {mMembers->destSize.width, mMembers->destSize.height};
            mMembers->mRenderParam->rotation = -mMembers->videoinfo.rotate;
            mMembers->mRenderParam->customFragshader = baseOESFragmentCode;

            //build Render
            mMembers->mRender = std::make_unique<EETextureCopyRender>();
            mMembers->mRender->build(mMembers->mRenderParam);
        }));
        return EE_OK;
    }

    std::shared_ptr<EEFrame> EEVideoDecoder::decode(std::shared_ptr<EEPacket> packt, bool skipRender){
        //decode
        EEMediaCodecDecoder::DecodeReturnParam param;
        std::shared_ptr<EEFrame> retFrame = nullptr;
        if(!mMembers->mTaskQueue || !packt || packt->buffer->size <= 0){
            return nullptr;
        }
        retFrame = std::make_shared<EEFrame>();
        //decode and render
        static int count = 0;
        mMembers->mTaskQueue->run(EETask([&]() {
            mMembers->mDecoder->decode(packt->buffer->data, (int)packt->buffer->size, packt->timestamp, &param);
            retFrame->frameType = param.keyType;
            retFrame->timestamp = param.decodedTimeStamp;
            retFrame->type = VideoType;
            if(param.returnCode != EE_OK || skipRender){
                return ;
            }
            std::shared_ptr<EETexture> destTexture = nullptr;
            if( mMembers->mSharedTextureAllocator != nullptr){
                destTexture = mMembers->mSharedTextureAllocator->allocateEETexture( mMembers->destSize);
            }else{
                destTexture = EETextureAllocator::getSharedInstance()->allocateEETexture( mMembers->destSize);
            }
            auto fbo = mMembers->mGLContext->getEEFrameBuffer();
            destTexture->excuteBind(true);
            fbo->bindEETexture(destTexture);
            destTexture->excuteBind(false);
            mMembers->mRenderParam->srcTexture = mMembers->mOESTexture;
            mMembers->mRenderParam->destFramebuffer = fbo;
            mMembers->mRender->render();
            glFinish();
            retFrame->frame = destTexture;
            mMembers->mRenderParam->destFramebuffer = nullptr;
        }));
        return retFrame;
    }

    EEReturnCode EEVideoDecoder::flush(){
        if(!mMembers->mTaskQueue){
            return EE_FAIL;
        }
        EEReturnCode ret;
        mMembers->mTaskQueue->run(EETask([&](){
            ret = mMembers->mDecoder->flush();
        }));
        return ret;
    }
    EEReturnCode EEVideoDecoder::release(){
        if(mMembers->mTaskQueue){
            mMembers->mTaskQueue->run(EETask([&](){
                mMembers->mRender = nullptr;
                mMembers->mRenderParam = nullptr;
                mMembers->mDecoder = nullptr;
                mMembers->mOESTexture = nullptr;
                mMembers->mGLContext = nullptr;
            }));
        }
        return EE_OK;
    }
}