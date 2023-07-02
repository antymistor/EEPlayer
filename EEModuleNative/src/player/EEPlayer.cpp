//
// Created by antymistor on 2023/6/19.
//

#include "EEPlayer.h"
#include "../reader/EEReader.h"
#include "../output/EEVideoOutput.h"
#include "../output/EEAudioOutput.h"
#include "../utils/EETimeUtils.h"
#include "../thread/EETaskQueue.h"
#include "../utils/EECommonUtils.h"
#include "../gl/EEGLContext.h"
#include <mutex>

#define LOG_TAG "EEPlayer"
#define TCR 1000                //Timestamp Conversion Ratio
#define AVSyncTolerance  50    //unit ms
#define AVSyncThreshold  30    //unit ms
#define SKIP_FRAMES_SYN_AT_SEEK -3
#define SKIP_FRAMES_TOLERANCE_TIME               1000000     //1s
#define SKIP_FRAMES_TOLERANCE_EXEMPTION_DIFFTIME 10000000    //10s
namespace EE {
        extern std::shared_ptr<EEGLContext> getSharedGLContext(const std::shared_ptr<EEShared>& sObj);
        extern std::shared_ptr<EETextureAllocator> getSharedTextureAllocator(const std::shared_ptr<EEShared>& sObj);
        extern std::shared_ptr<EETaskQueue> getSharedTaskQueue(const std::shared_ptr<EEShared>& sObj);

        struct EEPlayer::EEPlayerMember{
             /**
             *  base members
             */
             const char* filepath{};
             EEMediaInfo videoInfo;
             EEMediaInfo audioInfo;
             std::shared_ptr<EETaskQueue> mCallbackTaskQueue = nullptr;
             std::shared_ptr<EEReader> mVideoReader = nullptr;
             std::shared_ptr<EEReader> mAudioReader = nullptr;
             std::shared_ptr<EEOutput> mVideoOutput = nullptr;
             std::shared_ptr<EEOutput> mAudioOutput = nullptr;

             /**
              * User Config
              */

             int fps = -1;
             int64_t startPts = 0;
             EESize maxVideoRenderSize = {0,0};
             EEViewFillMode mVideoDisplayFitMode = IN_CENTER_BY_FIT;
             int64_t mVideoDisplayRotation = 0;
             float mVideoDisplayScale = 1.0f;
             int32_t extra_flag = 0x00;
             std::function<void(const EEPlayerCallBackType& type, const float& value1, const float &value2)> mCallback = nullptr;
             std::shared_ptr<EEShared> sharedObj = nullptr;
             /**
              * surface info
              */
             void* nativewindow = nullptr;
             EESize windowSize = {0,0};

            /**
             * Runtime Information
             */
             int64_t videoPts = 0;
             int64_t audioPts = 0;
             std::shared_ptr<EEFrame> videoFrameStore = nullptr;
             int64_t mVideoFrameCount = SKIP_FRAMES_SYN_AT_SEEK;
             int64_t mAudioFrameCount = SKIP_FRAMES_SYN_AT_SEEK;
             int64_t seekPosition = 0;
             EESize mFrameSize = {0,0};

        };
        EEPlayer::EEPlayer():mMembers(std::make_unique<EEPlayerMember>()){
            mMembers->mCallbackTaskQueue  = std::make_shared<EETaskQueue>(EECommonUtils::createGlobalName("callback"));
        }

        EEPlayer::~EEPlayer(){
            mMembers->mVideoReader->stop();
            mMembers->mAudioReader->stop();
            stop();
            mMembers->mVideoReader = nullptr;
            mMembers->mVideoReader = nullptr;
            mMembers->mVideoOutput = nullptr;
            mMembers->mAudioOutput = nullptr;
            if(mMembers->sharedObj == nullptr){
                EETextureAllocator::getSharedInstance()->clearAll();
            }
        }
        void EEPlayer::updateFilePath(const char* path){
            mMembers->filepath = path;
            //demux videoReader
            mMembers->mVideoReader = std::make_shared<EEReader>();
            mMembers->mVideoReader->update({mMembers->filepath, EE::VideoType});
            mMembers->videoInfo = mMembers->mVideoReader->getMediaInfo();
            //demux audioReader
            mMembers->mAudioReader = std::make_shared<EEReader>();
            mMembers->mAudioReader->update({mMembers->filepath, EE::AudioType});
            mMembers->audioInfo = mMembers->mAudioReader->getMediaInfo();
        }

        EEPlayer::EEPlayerMediaInfo EEPlayer::getMediaInfo(){
            EEPlayerMediaInfo info{
                .videoMineType   = mMembers->videoInfo.baseinfo.mineType,
                .videoDuration   = mMembers->videoInfo.baseinfo.duration / TCR,
                .videoWidth      = mMembers->videoInfo.baseinfo.videoSrcSize.width,
                .videoHeight     = mMembers->videoInfo.baseinfo.videoSrcSize.height,
                .videoRotate     = mMembers->videoInfo.baseinfo.rotate,
                .videofps        = mMembers->videoInfo.baseinfo.fps.num / mMembers->videoInfo.baseinfo.fps.den,
                .audioMineType   = mMembers->audioInfo.baseinfo.mineType,
                .audioDuration   = mMembers->audioInfo.baseinfo.duration / TCR,
                .audioChannels   = mMembers->audioInfo.baseinfo.channels,
                .audioSampleRate = mMembers->audioInfo.baseinfo.samplerate
            };
            return info;
        }


        void EEPlayer::build(const EEPlayerBuildParam& param){
            //copy param
            mMembers->fps = param.videoRenderFps > 0 ? param.videoRenderFps : (mMembers->videoInfo.baseinfo.fps.num / mMembers->videoInfo.baseinfo.fps.den);
            mMembers->mVideoDisplayFitMode = static_cast<EEViewFillMode>(param.videoDisplayFitMode);
            mMembers->startPts = param.startPlayTime;
            mMembers->maxVideoRenderSize = {param.maxRenderWidth , param.maxRenderHeight};
            mMembers->mVideoDisplayRotation = param.videoDisplayRotation;
            mMembers->seekPosition =  mMembers->startPts * TCR;
            mMembers->extra_flag = param.extraFlag;
            mMembers->mCallback = param.callback;
            mMembers->sharedObj = param.sharedObj;
            //build reader
            mMembers->mVideoReader->build({mMembers->maxVideoRenderSize,
                                                 getSharedGLContext(mMembers->sharedObj),
                                                 getSharedTextureAllocator(mMembers->sharedObj)});
            mMembers->mVideoReader->startFrom(mMembers->startPts * TCR, mMembers->startPts == 0 ? -1 : 0);
            int64_t realvideoSeekTime =  mMembers->mVideoReader->getSeekTime();
            mMembers->mAudioReader->build({});
            mMembers->mAudioReader->startFrom(realvideoSeekTime);
            mMembers->mVideoFrameCount = SKIP_FRAMES_SYN_AT_SEEK;
            mMembers->mAudioFrameCount = SKIP_FRAMES_SYN_AT_SEEK;
            mMembers->mFrameSize = mMembers->mVideoReader->getFrameSize();
            //build videoOutput

            mMembers->mVideoOutput = std::make_shared<EEVideoOutput>();

            EE::EEOutput::EEOutputBuildParam param_video = {
                    .fillmode = mMembers->mVideoDisplayFitMode,
                    .srcSize  = mMembers->mFrameSize,
                    .fps      = mMembers->fps ,
                    .videoDisplayRotation = mMembers->mVideoDisplayRotation,
                    .sharedGlContext = getSharedGLContext(mMembers->sharedObj),
                    .extraFlag =  mMembers->extra_flag,
                    .pullFun  =  [&](){
                        if(mMembers->mVideoFrameCount == SKIP_FRAMES_SYN_AT_SEEK + 1){
                            callbackHandle(onFirstVideoFrame,0,0);
                        }else if(mMembers->mVideoFrameCount == 1){
                            callbackHandle(onSeekFinish,0,0);
                        }


                        if(mMembers->videoFrameStore == nullptr){
                            mMembers->videoFrameStore = mMembers->mVideoReader->get(true);
                        }
                        if(mMembers->videoFrameStore == nullptr){
                            return mMembers->videoFrameStore;
                        }

                        if(mMembers->videoFrameStore->frameType == FLAG_END_OF_STREAM){
                            callbackHandle(onPlayToOES,0,0);
                            mMembers->videoFrameStore = nullptr;
                            return mMembers->videoFrameStore;
                        }

                        mMembers->mVideoOutput->getCurrentPts(mMembers->videoPts);
                        mMembers->mAudioOutput->getCurrentPts(mMembers->audioPts);
                        LOGI("videoPts= %ld, audioPts= %ld, timediff= %ld", mMembers->videoPts, mMembers->audioPts,
                             (mMembers->videoPts - mMembers->audioPts))
                        auto ret = mMembers->videoFrameStore;
                        if(mMembers->mAudioFrameCount < 0 || mMembers->mVideoFrameCount < 0){   //前三帧的时间戳有问题，跳过同步
                            mMembers->videoFrameStore = nullptr;
                        } else {
                            if(mMembers->videoPts - mMembers->audioPts > AVSyncTolerance){
                                ret = nullptr;
                            }else if(mMembers->videoPts - mMembers->audioPts < -AVSyncTolerance){
                                auto time = EETimeUtils::getCurrentTimeMillsSecond();
                                while (mMembers->videoFrameStore == nullptr ||
                                mMembers->videoFrameStore->timestamp / TCR < (mMembers->audioPts +  EETimeUtils::getCurrentTimeMillsSecond() - time + AVSyncThreshold)){
                                    mMembers->videoFrameStore = mMembers->mVideoReader->get(true);
                                    if(mMembers->videoFrameStore->frameType == FLAG_END_OF_STREAM){
                                        callbackHandle(onPlayToOES,0,0);
                                        mMembers->videoFrameStore = nullptr;
                                        return mMembers->videoFrameStore;
                                    }
                                }
                                ret = mMembers->videoFrameStore;
                                mMembers->videoFrameStore = nullptr;
                            }else{
                                mMembers->videoFrameStore = nullptr;
                            }
                        }
                        ++mMembers->mVideoFrameCount;
                        return ret;
                    }
            };
            mMembers->mVideoOutput->build(param_video);
            if(mMembers->nativewindow){
                mMembers->mVideoOutput->updateWindow(mMembers->nativewindow, mMembers->windowSize);
            }
            //build audioOutput
            mMembers->mAudioOutput = std::make_shared<EEAudioOutput>();
            EE::EEOutput::EEOutputBuildParam param_audio = {
                    .channelCnt =  mMembers->audioInfo.baseinfo.channels,
                    .sampleRate =  mMembers->audioInfo.baseinfo.samplerate,
                    .extraFlag  =  mMembers->extra_flag,
                    .pullFun    =  [&](){
                        if(mMembers->mAudioFrameCount == SKIP_FRAMES_SYN_AT_SEEK + 1){
                            callbackHandle(onFirstAudioFrame,0,0);
                        }
                        ++mMembers->mAudioFrameCount;
                        return mMembers->mAudioReader->get(true);
                    }
            };
            mMembers->mAudioOutput->build(param_audio);
        }

        void EEPlayer::callbackHandle(const EEPlayerCallBackType& type, const float& value1, const float &value2){
            if(mMembers->mCallback) {
                mMembers->mCallbackTaskQueue->run(EETask([=]() {
                    mMembers->mCallback(type, value1, value2);
                }, true));
            }
        }

        void EEPlayer::updateDisplayWindow(void* win, int width, int height){
            mMembers->nativewindow = win;
            mMembers->windowSize = {width, height};
            //build videoOutput
            if(mMembers->mVideoOutput) {
               mMembers->mVideoOutput->updateWindow(mMembers->nativewindow, mMembers->windowSize);
            }
        }


        void EEPlayer::start(){
             if(mMembers->mAudioReader && mMembers->mAudioOutput){
                 mMembers->mAudioOutput->start();
             }
            if(mMembers->mVideoReader && mMembers->mVideoOutput){
                mMembers->mVideoOutput->start();
            }
        }

        int64_t EEPlayer::calSeekTolerance(int64_t destPos){
            if(mMembers->mAudioFrameCount < 0 || mMembers->mVideoFrameCount < 0){
                return 0;
            }
            if(std::abs(mMembers->videoPts * TCR - destPos) > SKIP_FRAMES_TOLERANCE_EXEMPTION_DIFFTIME){
                return 0;
            }
            return SKIP_FRAMES_TOLERANCE_TIME;
        }

        bool EEPlayer::_seekTo(int64_t videoDestPostion, int64_t audioDestPostion){
            if(mMembers->mAudioFrameCount < 0 || mMembers->mVideoFrameCount < 0){
                return false;
            }
            mMembers->seekPosition = videoDestPostion;
            mMembers->mVideoReader->seekTo(videoDestPostion,  EEPlayer::calSeekTolerance(videoDestPostion));
            int64_t realvideoSeekTime =  mMembers->mVideoReader->getSeekTime();
            int64_t rebackVideoFrame = (videoDestPostion - realvideoSeekTime) *  mMembers->videoInfo.baseinfo.fps.num / mMembers->videoInfo.baseinfo.fps.den / 1000000;
            mMembers->mVideoFrameCount = SKIP_FRAMES_SYN_AT_SEEK;
            mMembers->mAudioReader->seekTo(realvideoSeekTime);
            mMembers->mAudioOutput->flush();
            mMembers->mAudioFrameCount = SKIP_FRAMES_SYN_AT_SEEK;
            mMembers->videoFrameStore = nullptr;
            LOGI("SeekTo videoDestPostion = %ld, audioDestPostion = %ld , realPosition = %ld , ReBackFrames = %ld",
                 videoDestPostion, audioDestPostion, realvideoSeekTime , rebackVideoFrame)
            return true;
        }

        void EEPlayer::resetVideoDisplayFitMode(int videoDisplayFitMode){
            mMembers->mVideoDisplayFitMode = (EEViewFillMode)videoDisplayFitMode;
            EEOutput::EEOutputHotUpdateParam para = {
                    .fillmode             = mMembers->mVideoDisplayFitMode,
                    .fps                  = mMembers->fps,
                    .videoDisplayRotation = mMembers->mVideoDisplayRotation,
                    .videoDisplayScale    = mMembers->mVideoDisplayScale,
            };
            mMembers->mVideoOutput->hotUpdate(para);
        }

        void EEPlayer::resetVideoRenderFps(int videoRenderFps){
            mMembers->fps = videoRenderFps > 0 ? videoRenderFps : (mMembers->videoInfo.baseinfo.fps.num / mMembers->videoInfo.baseinfo.fps.den);
            EEOutput::EEOutputHotUpdateParam para = {
                    .fillmode             = mMembers->mVideoDisplayFitMode,
                    .fps                  = mMembers->fps,
                    .videoDisplayRotation = mMembers->mVideoDisplayRotation,
                    .videoDisplayScale    = mMembers->mVideoDisplayScale,
            };
            mMembers->mVideoOutput->hotUpdate(para);
        }

        void EEPlayer::resetVideoDisplayRotation(int VideoDisplayRotation){
            mMembers->mVideoDisplayRotation = VideoDisplayRotation;
            EEOutput::EEOutputHotUpdateParam para = {
                    .fillmode             = mMembers->mVideoDisplayFitMode,
                    .fps                  = mMembers->fps,
                    .videoDisplayRotation = mMembers->mVideoDisplayRotation,
                    .videoDisplayScale    = mMembers->mVideoDisplayScale,
            };
            mMembers->mVideoOutput->hotUpdate(para);
        }

        void EEPlayer::resetVideoDisplayScale(float scale){
           mMembers->mVideoDisplayScale =  scale;
            EEOutput::EEOutputHotUpdateParam para = {
                    .fillmode             = mMembers->mVideoDisplayFitMode,
                    .fps                  = mMembers->fps,
                    .videoDisplayRotation = mMembers->mVideoDisplayRotation,
                    .videoDisplayScale    = mMembers->mVideoDisplayScale,
            };
            mMembers->mVideoOutput->hotUpdate(para);
        }

        bool EEPlayer::seekTo(float position){
            if(position < 0 || position > 1.f){
                return false;
            }
            int64_t destVideoPos = mMembers->videoInfo.baseinfo.duration * position;
            int64_t destAudioPos = mMembers->audioInfo.baseinfo.duration * position;
            return _seekTo(destVideoPos, destAudioPos);
        }

        bool EEPlayer::seekBy(float distance){
            int64_t destVideoPos = (mMembers->videoPts + distance) * TCR;
            int64_t destAudioPos = (mMembers->audioPts + distance) * TCR;
            destVideoPos = std::min<int64_t>(std::max<int64_t>(0, destVideoPos), mMembers->videoInfo.baseinfo.duration);
            destAudioPos = std::min<int64_t>(std::max<int64_t>(0, destAudioPos), mMembers->audioInfo.baseinfo.duration);
            return _seekTo(destVideoPos, destAudioPos);
        }

        void EEPlayer::stop(){
            if(mMembers->mAudioOutput){
                mMembers->mAudioOutput->stop();
            }
            if(mMembers->mVideoOutput){
                mMembers->mVideoOutput->stop();
            }
            mMembers->mVideoFrameCount = 0;
            mMembers->mAudioFrameCount = 0;
        }
        void EEPlayer::getProgress(float& progress){
            if(mMembers->mAudioFrameCount < 0 || mMembers->mVideoFrameCount < 0){
                progress = (float)mMembers->seekPosition / (float)mMembers->videoInfo.baseinfo.duration;
            }else{
                progress = (float)mMembers->videoPts * TCR / (float)mMembers->videoInfo.baseinfo.duration;
            }
        }
}