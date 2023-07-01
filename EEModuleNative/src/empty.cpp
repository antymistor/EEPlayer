//
// Created by antymistor on 2023/5/28.
//
//#include "libavcodec/avcodec.h"
#include <thread>
#include "empty.h"
#include "utils/EEMmapUtils.h"
#include "utils/EETimeUtils.h"
#include "gl/EEGLContext.h"
#include "gl/EETexture.h"
#include "thread/EETask.h"
#include "thread/EETaskQueue.h"
#include "output/EEAudioOutput.h"
extern "C" {
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libavutil/imgutils.h"
#include "libswscale/swscale.h"
#include "FDK_audio.h"
#include "aacdecoder_lib.h"
}
#include "reader/decoder/EEVideoDecoder.h"
#include "utils/EETimeUtils.h"
#include "reader/EEReader.h"
#define LOG_TAG "empty"

void test4(){
    EE::EEReader reader;
    reader.update({"/sdcard/DCIM/test.mp4", EE::VideoType});
    auto info = reader.getMediaInfo();
    reader.build({{1920,1920}});
    reader.startFrom(0);

//    EE::EEAudioOutput output;
//    EE::EEOutput::EEOutputBuildParam param = {info.baseinfo.channels, info.baseinfo.samplerate};
//    output.setPullFrameFun([&](){ return reader.get(true); });
//    output.build(param);
//    output.start();
//    int timecnt = 4;
//    long pts;
//    while (timecnt){
//        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
//        output.getCurrentPts(pts);
//        LOGI("audio timestamp = %ld ms", pts)
//        --timecnt;
//    }

    long time = EE::EETimeUtils::getCurrentTimeMillsSecond();
    std::shared_ptr<EE::EEFrame> frame = nullptr;
    while(frame == nullptr ||  frame->frameType != EE::FLAG_END_OF_STREAM){
        frame = reader.get(true);
        long nowtime = EE::EETimeUtils::getCurrentTimeMillsSecond();
        LOGI("decode one stamp time cost = %ld ,pts = %ld, duration = %ld", (nowtime - time), frame->timestamp, frame->duration)
        time = nowtime;
    }
}

void test3(){
    EE::EETaskQueue queue("test thread");
    EE::EETask task([](){
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        LOGI("DO THREAD TEST")
    }, false);
    auto time = EE::EETimeUtils::getCurrentTimeMillsSecond();
    queue.run(task);
    LOGI("TEST TIME Cost = %ld", (EE::EETimeUtils::getCurrentTimeMillsSecond() - time));
}

void test2(){
    EE::EETaskQueue queue("test gl");
    EE::EETask task([]() {
        auto context = std::make_shared<EE::EEGLContext>();
        context->initContext(nullptr, true);
        context->initSurface(nullptr, EE::PIXEL_BUFFER_MODE);
        context->makeCurrent();
        auto f = context->getEEFrameBuffer();
        auto ea = std::make_shared<EE::EETextureAllocator>();
        auto tex1 = ea->allocateEETexture({{30, 40}});
        auto tex2 = ea->allocateEETexture({{30, 40}});
        auto tex3 = ea->allocateEETexture({{30, 40}});
        f->bindEETexture(tex1);
        tex1->unRegisterFromAllocator();
        auto tex4 = ea->allocateEETexture({{30, 40}});
        auto tex5 = ea->allocateEETexture({{300, 400}});
        tex5 = nullptr;
        tex4 = nullptr;
        auto tex5new = ea->allocateEETexture({{300, 400}});
        auto tex4new = ea->allocateEETexture({{30, 40}});
    });
    queue.run(task);
}

void test1(){
    av_register_all();
    const char* path = "/sdcard/DCIM/test.mp4";
    AVFormatContext* fmtctx = nullptr;
    int maxtrytime = 10;
    while(maxtrytime > 0 && fmtctx == nullptr){
        fmtctx = avformat_alloc_context();
        auto errorcode = avformat_open_input(&fmtctx, path, nullptr, nullptr);
        LOGI("avformat_open_input error code =%s", av_err2str(errorcode))
        --maxtrytime;
    }
    if(nullptr == fmtctx){
        return;
    }
    avformat_find_stream_info(fmtctx, nullptr);
    int audioindex = -1;
    for(int i=0; i!=fmtctx->nb_streams; ++i){
        if(AVMEDIA_TYPE_AUDIO == fmtctx->streams[i]->codecpar->codec_type){
            audioindex = i;
            LOGI("Audio stream id = %d ", i);
        }
    }
    //build audio decoder
    AVCodec* codecaudio = avcodec_find_decoder(fmtctx->streams[audioindex]->codecpar->codec_id);
    AVCodecContext* contextaudio = nullptr;
   // contextaudio = fmtctx->streams[audioindex]->codec;
    contextaudio = avcodec_alloc_context3(codecaudio);
    AVCodecParameters* par = new AVCodecParameters();
    *par = *(fmtctx->streams[audioindex]->codecpar);
    par->extradata_size = fmtctx->streams[audioindex]->codecpar->extradata_size;
    par->extradata = new unsigned char[par->extradata_size];
    memcpy(par->extradata, fmtctx->streams[audioindex]->codecpar->extradata, par->extradata_size);
    avcodec_parameters_to_context(contextaudio, par);
    avcodec_open2(contextaudio, codecaudio, nullptr);

  //  auto audioframe = av_frame_alloc();

    //demux and decoder
    AVPacket packet;
    AVPacket packettest;
    AVFrame* frame = new AVFrame();
    //AVFrame* frame = av_frame_alloc();
    while(true){
        if(av_read_frame(fmtctx, &packet)){
            break;
        }
       // packettest = packet;
        packettest.data = packet.data;
        packettest.size = packet.size;
        packettest.pts = packet.pts;
        packettest.buf = nullptr;
        if(packet.stream_index == audioindex){
            avcodec_send_packet(contextaudio, &packettest);
            int gotframe = avcodec_receive_frame(contextaudio,frame);
            int h  = av_samples_get_buffer_size(nullptr, 2,
                                      1000,
                                       AV_SAMPLE_FMT_S16, 1);
            if(gotframe == 0){
                LOGI("decode audio success !!!error other ");
            }
            av_packet_unref(&packet);
        }
    }

}

void empty(){
//    test code
    {
      //test1();
    }

    {
        //test2();
    }

    {
       // test3();
    }
    {
        test4();
    }
     LOGI("hello here is empty");
     int a = 1;
}
