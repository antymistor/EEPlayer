//
// Created by ByteDance on 2023/6/13.
//

#include "EEAudioDecoder.h"
extern "C" {
    #include "libavcodec/avcodec.h"
    #include "libavformat/avformat.h"
    #include "libswresample/swresample.h"
}
namespace EE {
        struct  EEAudioDecoder::EEAudioDecoderMembers{
            EEMediaInfoBase audioinfo;
            AVCodec* mCodec = nullptr;
            AVCodecContext* mCodecContext = nullptr;
            SwrContext* mSwrContext = nullptr;
            AVPacket* mPacket = nullptr;
            AVFrame*  mFrame  = nullptr;
            uint8_t * reSamplebuffer     = nullptr;
            long   reSamplebufferSize = 0;
        };
        EEAudioDecoder::EEAudioDecoder() : mMembers(std::make_unique<EEAudioDecoderMembers>()){}
        EEAudioDecoder::~EEAudioDecoder(){
            EEAudioDecoder::release();
        }
        EESize EEAudioDecoder::getFrameSize() {
            return EESize();
        }
        EEReturnCode  EEAudioDecoder::build(const EEDecoder::EEDecoderBuildParam& param){
            EEAudioDecoder::release();
            mMembers->audioinfo = param.mediainfo.baseinfo;
            //build CodecContext
            mMembers->mCodec = avcodec_find_decoder(static_cast<AVCodecID>(mMembers->audioinfo.codecID));
            mMembers->mCodecContext = avcodec_alloc_context3(mMembers->mCodec);
            ((AVCodecParameters*)param.mediainfo.baseinfo.extraObj.get())->extradata = (uint8_t*)param.mediainfo.extradata_0->data;
            ((AVCodecParameters*)param.mediainfo.baseinfo.extraObj.get())->extradata_size = (int)param.mediainfo.extradata_0->size;
            avcodec_parameters_to_context(mMembers->mCodecContext, ((AVCodecParameters*)param.mediainfo.baseinfo.extraObj.get()));
            avcodec_open2(mMembers->mCodecContext, mMembers->mCodec, nullptr);
            //build SwrContext
           if(mMembers->mCodecContext->sample_fmt != AV_SAMPLE_FMT_S16){
               mMembers->mSwrContext = swr_alloc_set_opts(nullptr, av_get_default_channel_layout(mMembers->mCodecContext->channels),
                                                          AV_SAMPLE_FMT_S16, mMembers->mCodecContext->sample_rate,
                                                            av_get_default_channel_layout(mMembers->mCodecContext->channels),
                                                                       mMembers->mCodecContext->sample_fmt, mMembers->mCodecContext->sample_rate, 0, NULL);
               if (!mMembers->mSwrContext || swr_init(mMembers->mSwrContext)) {
                   if (mMembers->mSwrContext) {
                       swr_free(&mMembers->mSwrContext);
                   }
                   avcodec_close(mMembers->mCodecContext);
                   return EE_FAIL;
               }
           }
           //build packet & fram
           mMembers->mPacket = av_packet_alloc();
           mMembers->mPacket->buf = nullptr;
           mMembers->mFrame = av_frame_alloc();
           return EE_OK;
        }
        std::shared_ptr<EEFrame>  EEAudioDecoder::decode(std::shared_ptr<EEPacket> packt, bool skipRender){
            std::shared_ptr<EEFrame> retFrame = nullptr;
            //decode by ffmpeg
            mMembers->mPacket->data = (uint8_t*)packt->buffer->data;
            mMembers->mPacket->size = (int)packt->buffer->size;
            mMembers->mPacket->pts = packt->timestamp;
            mMembers->mPacket->duration = packt->duration;
            int error = AVERROR(EAGAIN);
            while (error == AVERROR(EAGAIN)){
                error = avcodec_send_packet(mMembers->mCodecContext, mMembers->mPacket);
            }
            if(error != 0){
                return retFrame;
            }
            error = AVERROR(EAGAIN);
            while (error == AVERROR(EAGAIN)){
                error = avcodec_receive_frame(mMembers->mCodecContext, mMembers->mFrame);
            }


            if(error == 0 &&  mMembers->mFrame->data[0] != nullptr && mMembers->mFrame->linesize[0] > 0){
                uint8_t * pixel = nullptr;
                long  pixelsize = 0;
                if(nullptr != mMembers->mSwrContext){
                      //TODO nb_samples*2 是一个可疑的调用
                      long bufSize = av_samples_get_buffer_size(nullptr, mMembers->mCodecContext->channels,
                                                                                mMembers->mFrame->nb_samples,
                                                                      AV_SAMPLE_FMT_S16, 1);
                      if(mMembers->reSamplebufferSize < bufSize){
                          mMembers->reSamplebufferSize = bufSize;
                          mMembers->reSamplebuffer = static_cast<uint8_t *>(realloc(mMembers->reSamplebuffer, mMembers->reSamplebufferSize));
                      }
                      int numFrames = swr_convert(mMembers->mSwrContext, &mMembers->reSamplebuffer,
                                                  mMembers->mFrame->nb_samples,(const uint8_t **)(mMembers->mFrame->data), mMembers->mFrame->nb_samples);
                      if (numFrames < 0) {
                        return retFrame;
                      }
                      pixel = mMembers->reSamplebuffer;
                      pixelsize = mMembers->reSamplebufferSize;
                }else{
                    pixel = mMembers->mFrame->data[0];
                    pixelsize =  mMembers->mFrame->linesize[0];
                }
                retFrame = std::make_shared<EEFrame>(AudioType, FLAG_NORMAL_FRAME);
                retFrame->buffer = std::make_shared<EEBuffer>((char* )pixel, pixelsize);
                retFrame->timestamp = mMembers->mFrame->pts;
                retFrame->type = AudioType;
                retFrame->duration = packt->duration;
                retFrame->nb_samples = mMembers->mFrame->nb_samples;
            }
            return retFrame;
        }
        EEReturnCode  EEAudioDecoder::flush(){
            return EE_OK;
        }
        EEReturnCode  EEAudioDecoder::release(){
            if(mMembers->mSwrContext){
                swr_free(&mMembers->mSwrContext);
                mMembers->mSwrContext = nullptr;
            }
            if(mMembers->mCodecContext){
                avcodec_close(mMembers->mCodecContext);
                mMembers->mCodecContext = nullptr;
            }
            if(mMembers->mPacket){
                av_packet_free(&mMembers->mPacket);
                mMembers->mPacket = nullptr;
            }
            if(mMembers->mFrame){
                av_frame_free(&mMembers->mFrame);
                mMembers->mFrame = nullptr;
            }
            if(mMembers->reSamplebuffer){
                free(mMembers->reSamplebuffer);
                mMembers->reSamplebuffer = nullptr;
                mMembers->reSamplebufferSize = 0;
            }
            return EE_OK;
        }
}