//
// Created by ByteDance on 2023/6/19.
//

#ifndef CVTEXTREADER_EEPLAYER_H
#define CVTEXTREADER_EEPLAYER_H
#include <memory>
#include <functional>

namespace EE {
    class EEPlayer {
        public:
              EEPlayer();
             ~EEPlayer();

             // 1. UpdateFilePath & updateDisplayWindow
             // 2. getVideoInfo
             // 3. build
             // 4. updateDisplayWindow
             // 5. start



              enum EEPlayerCallBackType{
                    onNothing         = 0,
                    onFirstVideoFrame = 1,
                    onFirstAudioFrame = 2,
                    onSeekFinish      = 3,
                    onPlayToOES       = 4,
              };
              struct EEPlayerBuildParam {
                  //video about
                  int maxRenderWidth      = 0;
                  int maxRenderHeight     = 0;
                  int videoRenderFps      = 0;
                  int videoDisplayFitMode = 0;
                  int videoDisplayRotation= 0;
                  // both
                  int64_t startPlayTime       = 0;
                  int32_t extraFlag           = 0;
                  std::function<void(const EEPlayerCallBackType& type, const float& value1, const float &value2)> callback= nullptr;
              };

              struct EEPlayerMediaInfo {
                  //video related
                  const char* videoMineType;
                  long videoDuration = 0;
                  int videoWidth = 0;
                  int videoHeight = 0;
                  int videoRotate = 0;
                  int videofps    = 0;

                  //audio related
                  const char* audioMineType;
                  long audioDuration = 0;
                  int audioChannels = 0;
                  int audioSampleRate = 0;
              };


              void updateFilePath(const char* path);
              void updateDisplayWindow(void* win, int width, int height);
              EEPlayerMediaInfo getMediaInfo();
              void build(const EEPlayerBuildParam& param);
              void start();

              //hot update
              void resetVideoDisplayFitMode(int videoDisplayFitMode);
              void resetVideoRenderFps(int videoRenderFps);
              void resetVideoDisplayRotation(int VideoDisplayRotation);
              void resetVideoDisplayScale(float scale);
              bool seekTo(float position);
              bool seekBy(float distance);


              void stop();
              void getProgress(float& progress);
        private:
              void callbackHandle(const EEPlayerCallBackType& type, const float& value1, const float &value2);
              bool _seekTo(int64_t videoDestPostion, int64_t audioDestPostion);
              int64_t calSeekTolerance(int64_t destPos);
              struct EEPlayerMember;
              std::unique_ptr<EEPlayerMember> mMembers;
    };
}
#endif //CVTEXTREADER_EEPLAYER_H
