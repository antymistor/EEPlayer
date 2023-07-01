//
// Created by ByteDance on 2023/6/13.
//

#include "EEReader.h"
#include "decoder/EEVideoDecoder.h"
#include "decoder/EEAudioDecoder.h"
#include "../basedefine/EEList.h"
#include "../utils/EECommonUtils.h"
#include "demuxer/EEDemuxer.h"
#include <thread>
namespace EE {
    struct EEReader::EEVideoReaderMembers{
        EEReaderUpdateParam             mUpdateParam;
        EEReaderBuildParam              mBuildParam;
        EEMediaInfo                     mMediainfo;
        std::shared_ptr<EEDemuxer>      mDemuxer = nullptr;
        std::shared_ptr<EEDecoder>      mDecoder = nullptr;
        EEList<std::shared_ptr<EEFrame>> mFrameList;
        std::thread* worker = nullptr;
        bool mIsStop = false;
        int64_t seekStartStamp = 0;
    };

    EEReader::EEReader() : mMembers(std::make_unique<EEVideoReaderMembers>()){
        mMembers->mDemuxer = std::make_shared<EEDemuxer>();
    }

    EEReader::~EEReader() {
        stop();
    }

    EEReturnCode EEReader::update(const EEReaderUpdateParam& param){
        mMembers->mUpdateParam = param;
        mMembers->mMediainfo = mMembers->mDemuxer->build(mMembers->mUpdateParam.srcFilePath, mMembers->mUpdateParam.type, mMembers->mUpdateParam.maxCacheSize / 2);
        return EE_OK;
    }

    EEMediaInfo EEReader::getMediaInfo(){
        return  mMembers->mMediainfo;
    }

    EEReturnCode EEReader::build(const EEReaderBuildParam& param){
        mMembers->mBuildParam = param;
        mMembers->mDecoder = nullptr;
        if(strcmp(mMembers->mMediainfo.baseinfo.mineType, "") != 0){
            if(mMembers->mUpdateParam.type == VideoType){
                mMembers->mDecoder = std::make_shared<EEVideoDecoder>();
            }else if(mMembers->mUpdateParam.type == AudioType){
                mMembers->mDecoder = std::make_shared<EEAudioDecoder>();
            }
        }
        if(mMembers->mDecoder == nullptr){
            return EE_FAIL;
        }
        mMembers->mDecoder->build({mMembers->mMediainfo , mMembers->mBuildParam.maxVideoFrameSize});
        return EE_OK;
    }
    EESize  EEReader::getFrameSize(){
        EESize size;
       if(mMembers->mDecoder){
           size = mMembers->mDecoder->getFrameSize();
       }
        return size;
    }

    int64_t EEReader::getSeekTime(){
        return mMembers->seekStartStamp;
    }

    EEReturnCode EEReader::startFrom(long timestamp ,int64_t tolerance){
        if(mMembers->mDemuxer == nullptr || mMembers->mDecoder == nullptr){
            return EE_FAIL;
        }
        stop();
        mMembers->mIsStop = false;
        mMembers->mFrameList.updateMaxSize(mMembers->mUpdateParam.maxCacheSize);
        mMembers->mFrameList.start();
        mMembers->mDemuxer->startFrom(timestamp);
        if(tolerance < 0){
            mMembers->seekStartStamp = timestamp;
        }else{
            mMembers->seekStartStamp = std::min<int64_t>(mMembers->mDemuxer->getSeekTime() + tolerance, timestamp);
        }
        mMembers->worker = new std::thread([&](){
            while (!mMembers->mIsStop){
                 std::shared_ptr<EEPacket> packet = mMembers->mDemuxer->get();
                 if(packet != nullptr && packet->frameType == FLAG_END_OF_STREAM){
                    mMembers->mFrameList.put(std::make_shared<EEFrame>(mMembers->mMediainfo.baseinfo.type,FLAG_END_OF_STREAM), true);
                    mMembers->mIsStop = true;
                    break;
                 }
                 if(mMembers->mUpdateParam.type == AudioType && packet->timestamp < mMembers->seekStartStamp){
                     continue;
                 }
                 auto frame = mMembers->mDecoder->decode(packet, packet->timestamp < mMembers->seekStartStamp);
                 if(frame != nullptr && (frame->frame != nullptr || frame->buffer != nullptr)){
                     mMembers->mFrameList.put(frame, true);
                 }
            }
        });
        pthread_setname_np(mMembers->worker->native_handle(), EECommonUtils::createGlobalName("EEReader").c_str());
        return EE_OK;
    }
    EEReturnCode EEReader::stop(){
        mMembers->mIsStop = true;
        mMembers->mFrameList.stop();
        if( mMembers->worker) {
            mMembers->worker->join();
            mMembers->worker = nullptr;
        }
        return EE_OK;
    }

    EEReturnCode EEReader::seekTo(int64_t pts, int64_t tolerance){

        /**
            PLAN-A Do Not Need to restart read thread
                   But Not Work, Need to  Know why
            mMembers->mFrameList.stop(true);
            flush();
            mMembers->mDemuxer->stop();
            mMembers->mDemuxer->flush();
            mMembers->mDecoder->flush();
            mMembers->mDemuxer->startFrom(pts);
            mMembers->seekStartStamp = pts;
            mMembers->mFrameList.start();
         */

        /**
         * PLAN-B RestartThread
         */
            stop();
            flush();
            mMembers->mDemuxer->stop();
            mMembers->mDemuxer->flush();
            mMembers->mDecoder->flush();
            startFrom(pts, tolerance);
            return EE_OK;
    }

    EEReturnCode EEReader::flush(){
        mMembers->mFrameList.clear();
        return EE_OK;
    }


    std::shared_ptr<EEFrame> EEReader::get(bool wait){
        return mMembers->mFrameList.get(wait);
    }
}