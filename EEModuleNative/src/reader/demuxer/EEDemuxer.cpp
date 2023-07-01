//
// Created by antymistor on 2023/6/14.
//

#include "EEDemuxer.h"
#include "../../basedefine/EECodeDefine.h"
#include "../../basedefine/EEList.h"
#include "../../utils/EECommonUtils.h"
#include <thread>
extern "C" {
    #include "libavcodec/avcodec.h"
    #include "libavformat/avformat.h"
}
#define MAX_TRY_CNT 5
#define LOG_TAG "EEDemuxer"
namespace EE {
    const char* getMineTypeName(const AVCodecID& id){
        switch (id) {
            case AV_CODEC_ID_H264: return "video/avc";
            case AV_CODEC_ID_HEVC: return "video/hevc";
            case AV_CODEC_ID_MPEG2VIDEO: return "video/mpeg2";
            case AV_CODEC_ID_MPEG4: return "video/mp4v-es";
            case AV_CODEC_ID_VP8: return "video/x-vnd.on2.vp8";
            case AV_CODEC_ID_VP9: return"video/x-vnd.on2.vp9";
            case AV_CODEC_ID_AAC: return "audio/aac";
            case AV_CODEC_ID_MP3: return "audio/mp3";
            default:break;
        }
        return "";
    }

    const char* getBSFName(AVCodecID id){
        switch (id) {
            case AV_CODEC_ID_H264: {
                return "h264_mp4toannexb";
            }
            case AV_CODEC_ID_HEVC: {
                return "hevc_mp4toannexb";
            }
            default:break;
        }
        return "";
    }

    struct EEDemuxer::EEDemuxerMembers{
        EEStreamType type;
        AVFormatContext* mFormatContext = nullptr;
        AVBSFContext*    mBSFContext = nullptr;
        int mStreamID = 0;
        AVCodecID mCodecID;

        EEMediaInfo mMediaInfo;
        int maxCacheSize   = 5;
        AVPacket mPacket;
        bool mIsStop = false;
        EEList<std::shared_ptr<EEPacket>> mPacketList;
        std::thread* worker = nullptr;

        int64_t seekTime = -1;

        std::mutex mMutex;
        std::condition_variable mCondition;

    };

    EEDemuxer::EEDemuxer() : mMembers(std::make_unique<EEDemuxerMembers>()){}

    EEMediaInfo  EEDemuxer::build(const char* filepath, EEStreamType type ,int maxCacheSize ){
        release();
        mMembers->maxCacheSize = maxCacheSize;
        mMembers->mPacketList.updateMaxSize(mMembers->maxCacheSize);
        mMembers->type         = type;
        int maxtry = MAX_TRY_CNT;
        //create FormatContext
        while(nullptr == mMembers->mFormatContext && maxtry > 0){
            mMembers->mFormatContext = avformat_alloc_context();
            avformat_open_input(&mMembers->mFormatContext, filepath, nullptr, nullptr);
            --maxtry;
        }
        //find stream
        mMembers->mStreamID =  av_find_best_stream(mMembers->mFormatContext,
                                                   mMembers->type == VideoType ? AVMEDIA_TYPE_VIDEO : AVMEDIA_TYPE_AUDIO,
                                                   -1, -1, nullptr, 0);
        //get CodecID
        mMembers->mCodecID = mMembers->mFormatContext->streams[mMembers->mStreamID]->codecpar->codec_id;
        mMembers->mMediaInfo.baseinfo.codecID = mMembers->mCodecID;
        //get StreamType
        mMembers->mMediaInfo.baseinfo.type = mMembers->type;
        //get mineType
        mMembers->mMediaInfo.baseinfo.mineType = getMineTypeName(mMembers->mCodecID);
        //get bsf
        mMembers->mMediaInfo.baseinfo.bsfName  = getBSFName(mMembers->mCodecID);
        if(strcmp(mMembers->mMediaInfo.baseinfo.bsfName , "") !=0 ){
            const AVBitStreamFilter *bsf = av_bsf_get_by_name(mMembers->mMediaInfo.baseinfo.bsfName);
            av_bsf_alloc(bsf, &mMembers->mBSFContext);
            avcodec_parameters_from_context(mMembers->mBSFContext->par_in, mMembers->mFormatContext->streams[mMembers->mStreamID]->codec);
            av_bsf_init(mMembers->mBSFContext);
            mMembers->mMediaInfo.extradata_0 = std::make_shared<EEBuffer>((char *)mMembers->mBSFContext->par_out->extradata,
                                                                          mMembers->mBSFContext->par_out->extradata_size);
        }else{
            mMembers->mMediaInfo.extradata_0 = std::make_shared<EEBuffer>(
                    (char *)mMembers->mFormatContext->streams[mMembers->mStreamID]->codecpar->extradata,
                                   mMembers->mFormatContext->streams[mMembers->mStreamID]->codecpar->extradata_size);
        }

        //get duration
        mMembers->mMediaInfo.baseinfo.duration = av_rescale_q(mMembers->mFormatContext->streams[mMembers->mStreamID]->duration
                , mMembers->mFormatContext->streams[mMembers->mStreamID]->time_base, AV_TIME_BASE_Q);


        if(mMembers->type == VideoType){
            //get srcSize
            mMembers->mMediaInfo.baseinfo.videoSrcSize = {mMembers->mFormatContext->streams[mMembers->mStreamID]->codec->width,
                                                          mMembers->mFormatContext->streams[mMembers->mStreamID]->codec->height};
            //get rotate
            AVDictionaryEntry* entry = nullptr;
            while ((entry = av_dict_get( mMembers->mFormatContext->streams[mMembers->mStreamID]->metadata, "", entry, AV_DICT_IGNORE_SUFFIX))){
                LOGI("entry: key is %s value is %s\n", entry->key, entry->value)
                if (0 == strcmp(entry->key, "rotate")) {
                    mMembers->mMediaInfo.baseinfo.rotate = (int)strtol(entry->value, nullptr, 10);
                }
            }
            //get fps
            mMembers->mMediaInfo.baseinfo.fps = {
                    mMembers->mFormatContext->streams[mMembers->mStreamID]->avg_frame_rate.num,
                    mMembers->mFormatContext->streams[mMembers->mStreamID]->avg_frame_rate.den,
            };
        }else if(mMembers->type == AudioType){
            auto *par = new AVCodecParameters();
            *par = *(mMembers->mFormatContext->streams[mMembers->mStreamID]->codecpar);  //extradata imple by extradata_0
            mMembers->mMediaInfo.baseinfo.extraObj = std::shared_ptr<void>(par, [](void* ptr){
                 delete (AVCodecParameters*) ptr;
            });
            mMembers->mMediaInfo.baseinfo.channels = mMembers->mFormatContext->streams[mMembers->mStreamID]->codecpar->channels;
            mMembers->mMediaInfo.baseinfo.samplerate = mMembers->mFormatContext->streams[mMembers->mStreamID]->codecpar->sample_rate;
        }

        //Todo Need to impl more mediainfo on audio
        LOGI("\n"
             "MediaInfo of %s \n"
             "MineType=%s, StreamID = %d, SrcSize=%dx%d ,Rotation=%d ,ExtraDataSize=%ld ,Duration=%ld us , fps = %.2f\n",
             filepath,
             mMembers->mMediaInfo.baseinfo.mineType,
             mMembers->mStreamID,
             mMembers->mMediaInfo.baseinfo.videoSrcSize.width, mMembers->mMediaInfo.baseinfo.videoSrcSize.height,
             mMembers->mMediaInfo.baseinfo.rotate,
             mMembers->mMediaInfo.extradata_0->size,
             mMembers->mMediaInfo.baseinfo.duration,
             1.0f * mMembers->mMediaInfo.baseinfo.fps.num / mMembers->mMediaInfo.baseinfo.fps.den
             )
        return mMembers->mMediaInfo;
    }
    EEDemuxer::~EEDemuxer(){
        release();
    }
    EEReturnCode EEDemuxer::startFrom(long timestamp){
        stop();
        mMembers->mIsStop = false;
        mMembers->mPacketList.start();
        mMembers->seekTime = -1;
        if(timestamp >= 0){  // TODO 此处可以优化，部分情况下可以不用重新seek, 以优化性能
            int64_t pos = av_rescale_q(timestamp, AV_TIME_BASE_Q, mMembers->mFormatContext->streams[mMembers->mStreamID]->time_base);
            avformat_seek_file(mMembers->mFormatContext, mMembers->mStreamID, INT_MIN, pos, INT_MAX,
                              mMembers->type == VideoType ? AVSEEK_FLAG_BACKWARD : AVSEEK_FLAG_ANY );
            av_packet_unref(&mMembers->mPacket);
        }
        mMembers->worker = new std::thread([&](){
             while (!mMembers->mIsStop){
                 if(av_read_frame(mMembers->mFormatContext, &mMembers->mPacket)) {
                     mMembers->mIsStop = true;
                     mMembers->mPacketList.put(std::make_shared<EEPacket>(nullptr,0,0, 0, mMembers->type,
                                                                          FLAG_END_OF_STREAM)  ,true);
                     break;
                 }
                 if(mMembers->mPacket.stream_index != mMembers->mStreamID){
                     av_packet_unref(&mMembers->mPacket);
                     continue;
                 }
                 long pos = av_rescale_q(mMembers->mPacket.pts,mMembers->mFormatContext->streams[mMembers->mStreamID]->time_base, AV_TIME_BASE_Q);
                 long duration = av_rescale_q(mMembers->mPacket.duration,mMembers->mFormatContext->streams[mMembers->mStreamID]->time_base, AV_TIME_BASE_Q);
                 if(mMembers->seekTime == -1){
                     std::unique_lock<std::mutex> lock(mMembers->mMutex);
                     mMembers->seekTime = pos;
                     mMembers->mCondition.notify_all();
                 }
                 if(mMembers->type == VideoType) {
                     av_bsf_send_packet(mMembers->mBSFContext, &mMembers->mPacket);
                     av_bsf_receive_packet(mMembers->mBSFContext, &mMembers->mPacket);
                 }
                 mMembers->mPacketList.put(std::make_shared<EEPacket>((char *)mMembers->mPacket.data,mMembers->mPacket.size,pos, duration, mMembers->type,
                                                                                 (mMembers->mPacket.flags & AV_PKT_FLAG_KEY) == AV_PKT_FLAG_KEY ? FLAG_KEY_FRAME : FLAG_NORMAL_FRAME)  ,true);

                 av_packet_unref(&mMembers->mPacket);

             }
        });
        pthread_setname_np(mMembers->worker->native_handle(), EECommonUtils::createGlobalName("EEDemuxer").c_str());
        return EE_OK;
    }
    EEReturnCode EEDemuxer::stop(){
        mMembers->mIsStop = true;
        mMembers->mPacketList.stop();
        if( mMembers->worker) {
            mMembers->worker->join();
            mMembers->worker = nullptr;
        }
        return EE_OK;
    }

    EEReturnCode EEDemuxer::release(){
        stop();
        if(mMembers->mBSFContext){
            av_bsf_free(&mMembers->mBSFContext);
            mMembers->mBSFContext = nullptr;
        }
        if(mMembers->mFormatContext){
            avformat_close_input(&mMembers->mFormatContext);
            avformat_free_context(mMembers->mFormatContext);
            mMembers->mFormatContext = nullptr;
        }
        return EE_OK;
    }

    EEReturnCode EEDemuxer::flush(){
        mMembers->mPacketList.clear();
        return EE_OK;
    }
    std::shared_ptr<EEPacket> EEDemuxer::get(){
        return mMembers->mPacketList.get();
    }

    int64_t EEDemuxer::getSeekTime(){
        while(mMembers->seekTime == -1){
            std::unique_lock<std::mutex> lock(mMembers->mMutex);
            mMembers->mCondition.wait(lock);
        }
        return mMembers->seekTime;
    }
}