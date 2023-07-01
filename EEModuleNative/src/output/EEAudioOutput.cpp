//
// Created by antymistor on 2023/6/18.
//

#include "EEAudioOutput.h"
#include "../sl/EESLContext.h"
#include "../sl/EESLUtils.h"
#define LOG_TAG "EEAudioOutput"
#define CHECK(result) if(SL_RESULT_SUCCESS != result){ \
                               release();          \
                               return EE_FAIL;     \
                       }
namespace EE{
    struct EEAudioOutput::EEAudioOutputMembers{
        SLEngineItf                   mEngine = nullptr;
        SLObjectItf                   mOutputMixObject = nullptr;
        SLObjectItf                   mAudioPlayerObject = nullptr;
        SLAndroidSimpleBufferQueueItf mAudioPlayerBufferQueue = nullptr;
        SLPlayItf                     mAudioPlayerPlay = nullptr;
        SLVolumeItf                   mAudioPlayerVolume = nullptr;
        std::function<std::shared_ptr<EEFrame>()> mPullFrameFun = nullptr;

        EEPlayState mPlayState = Stoped;
        int mChannelCnt = 0;
        int mSampleRate = 0;
        std::atomic<bool> isFlushed = true;
       // int64_t pts_offset = 0;
        //debug
        int64_t pts_abs = -1;
        bool isMute = false;

    };

    void EEAudioOutput::bufferQueueCallback(SLAndroidSimpleBufferQueueItf caller,void *pContext){
        auto* audioOutput = (EEAudioOutput* )pContext;
        if(audioOutput->getPlayState() == EEPlayState::Playing){
            auto frame = audioOutput->mMembers->mPullFrameFun();
            if( frame!= nullptr && frame->buffer != nullptr){
                audioOutput->mMembers->pts_abs = frame->timestamp / 1000;
//                if(audioOutput->mMembers->isFlushed){
//                    audioOutput->mMembers->pts_offset += frame->timestamp / 1000;
//                    audioOutput->mMembers->isFlushed = false;
//                }
                (*audioOutput->mMembers->mAudioPlayerBufferQueue)->Enqueue(audioOutput->mMembers->mAudioPlayerBufferQueue,
                                                                           frame->buffer->data, frame->buffer->size);
            }
        }
    }


    EEAudioOutput::EEAudioOutput() : mMembers(std::make_unique<EEAudioOutputMembers>()){}
    EEAudioOutput::~EEAudioOutput(){
        EEAudioOutput::release();
    }
    EEReturnCode EEAudioOutput::getCurrentPts(int64_t& pts){
        if (nullptr != mMembers->mAudioPlayerObject && nullptr != (*mMembers->mAudioPlayerPlay)) {
          //  SLmillisecond position = 0;
          //  (*mMembers->mAudioPlayerPlay)->GetPosition(mMembers->mAudioPlayerPlay, &position);
          //  pts = position + mMembers->pts_offset;
            pts = mMembers->pts_abs;
           // LOGI("getCurrentPts abspts= %ld, slpts= %ld diff= %ld", mMembers->pts_abs, position + mMembers->pts_offset, (mMembers->pts_abs - position - mMembers->pts_offset))
            return EE_OK;
        }
        return EE_FAIL;
    }
    EEReturnCode EEAudioOutput::build(const EEOutputBuildParam& param){
        mMembers->mChannelCnt = param.channelCnt;
        mMembers->mSampleRate = param.sampleRate;
        mMembers->mPullFrameFun = param.pullFun;
        mMembers->mEngine = EESLContext::GetInstanceEngineItf();
        if(nullptr == mMembers->mEngine){
            return EE_FAIL;
        }
        SLresult result = SL_RESULT_UNKNOWN_ERROR;
        // Create output mix object
        result = (*mMembers->mEngine)->CreateOutputMix(mMembers->mEngine, &mMembers->mOutputMixObject, 0, nullptr, nullptr);
        CHECK(result)
        result = EESLUtils::realize(mMembers->mOutputMixObject);
        CHECK(result)
        // Create AudioPlayerObject
        SLDataLocator_AndroidSimpleBufferQueue dataSourceLocator = { SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE,1};
        int samplesPerSec = EESLUtils::getSampleRate(mMembers->mSampleRate);
        int channelMask =  EESLUtils::getChannelMask(mMembers->mChannelCnt);
        SLDataFormat_PCM dataSourceFormat = { SL_DATAFORMAT_PCM,(SLuint32)mMembers->mChannelCnt, (SLuint32)samplesPerSec,
                                              SL_PCMSAMPLEFORMAT_FIXED_16, SL_PCMSAMPLEFORMAT_FIXED_16,(SLuint32)channelMask, SL_BYTEORDER_LITTLEENDIAN};
        SLDataSource dataSource = { &dataSourceLocator,&dataSourceFormat};
        SLDataLocator_OutputMix dataSinkLocator = { SL_DATALOCATOR_OUTPUTMIX,mMembers->mOutputMixObject};
        SLDataSink dataSink = { &dataSinkLocator,  nullptr };
        SLInterfaceID interfaceIds[] = { SL_IID_BUFFERQUEUE, SL_IID_VOLUME};
        SLboolean requiredInterfaces[] = { SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE};
        result = (*mMembers->mEngine)->CreateAudioPlayer(mMembers->mEngine, &mMembers->mAudioPlayerObject,
                                                         &dataSource, &dataSink, ARRAY_LEN(interfaceIds), interfaceIds, requiredInterfaces);
        CHECK(result)
        result = EESLUtils::realize(mMembers->mAudioPlayerObject);
        CHECK(result)
        //get bufferqueue
        result = (*mMembers->mAudioPlayerObject)->GetInterface(mMembers->mAudioPlayerObject, SL_IID_BUFFERQUEUE, &mMembers->mAudioPlayerBufferQueue);
        CHECK(result)
        // Registers the player callback
        result = (*mMembers->mAudioPlayerBufferQueue)->RegisterCallback(mMembers->mAudioPlayerBufferQueue, bufferQueueCallback, this);
        CHECK(result)
        // Get audio player interface
        result = (*mMembers->mAudioPlayerObject)->GetInterface(mMembers->mAudioPlayerObject, SL_IID_PLAY, &mMembers->mAudioPlayerPlay);
        CHECK(result)
        result = (*mMembers->mAudioPlayerObject)->GetInterface(mMembers->mAudioPlayerObject, SL_IID_VOLUME, &mMembers->mAudioPlayerVolume);
        CHECK(result)
        return EE_OK;
    }
    EEReturnCode EEAudioOutput::start(){
        SLresult result = (*mMembers->mAudioPlayerPlay)->SetPlayState(mMembers->mAudioPlayerPlay, SL_PLAYSTATE_PLAYING);
        CHECK(result)
        mMembers->mPlayState = EEPlayState::Playing;
        bufferQueueCallback(nullptr, this);
        return EE_OK;
    }

    EEReturnCode EEAudioOutput::stop(){
        SLresult result = (*mMembers->mAudioPlayerPlay)->SetPlayState(mMembers->mAudioPlayerPlay, SL_PLAYSTATE_PAUSED);
        CHECK(result)
        mMembers->mPlayState = EEPlayState::Paused;
        return EE_OK;
    }

    EEReturnCode EEAudioOutput::mute(bool ismute) {
        SLresult result = SL_RESULT_SUCCESS;
        if(mMembers->isMute && !ismute){
            result = (*mMembers->mAudioPlayerVolume)->SetMute(mMembers->mAudioPlayerVolume, false);
        }else if ( !mMembers->isMute && ismute){
            result = (*mMembers->mAudioPlayerVolume)->SetMute(mMembers->mAudioPlayerVolume, true);
        }
        mMembers->isMute = ismute;
        CHECK(result)
        return EE_OK;
    }

    EEReturnCode EEAudioOutput::flush() {
//        SLmillisecond position = 0;
//        (*mMembers->mAudioPlayerPlay)->GetPosition(mMembers->mAudioPlayerPlay, &position);
//        mMembers->pts_offset = -(int64_t)position;
//        mMembers->isFlushed = true;
        SLresult result = (*mMembers->mAudioPlayerBufferQueue)->Clear(mMembers->mAudioPlayerBufferQueue);
        CHECK(result)
//        result = (*mMembers->mAudioPlayerVolume)->SetMute(mMembers->mAudioPlayerVolume, true);
//        CHECK(result)
       // mMembers->isFlushed = true;
        bufferQueueCallback(nullptr, this);
        return EE_OK;
    }

    EEReturnCode EEAudioOutput::release(){
        stop();
        EESLUtils::destroyObj(mMembers->mAudioPlayerObject);
        EESLUtils::destroyObj(mMembers->mOutputMixObject);
        return EE_OK;
    }

    EEPlayState EEAudioOutput::getPlayState(){
        return mMembers->mPlayState;
    }
}
