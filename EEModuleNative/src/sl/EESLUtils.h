//
// Created by antymistor on 2023/6/18.
//

#ifndef CVTEXTREADER_EESLUTILS_H
#define CVTEXTREADER_EESLUTILS_H
#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>
namespace EE{
    class EESLUtils{
    public:
        static void destroyObj(SLObjectItf& object){
            if (nullptr != object) {
                (*object)->Destroy(object);
            }
        }
        static SLresult realize(SLObjectItf& object){
            return (*object)->Realize(object, SL_BOOLEAN_FALSE);
        }

        static int getSampleRate(int sampleRate) {
            int samplesPerSec = SL_SAMPLINGRATE_44_1;
            switch (sampleRate) {
                case 8000:
                    samplesPerSec = SL_SAMPLINGRATE_8;
                    break;
                case 11025:
                    samplesPerSec = SL_SAMPLINGRATE_11_025;
                    break;
                case 12000:
                    samplesPerSec = SL_SAMPLINGRATE_12;
                    break;
                case 16000:
                    samplesPerSec = SL_SAMPLINGRATE_16;
                    break;
                case 22050:
                    samplesPerSec = SL_SAMPLINGRATE_22_05;
                    break;
                case 24000:
                    samplesPerSec = SL_SAMPLINGRATE_24;
                    break;
                case 32000:
                    samplesPerSec = SL_SAMPLINGRATE_32;
                    break;
                case 44100:
                    samplesPerSec = SL_SAMPLINGRATE_44_1;
                    break;
                case 48000:
                    samplesPerSec = SL_SAMPLINGRATE_48;
                    break;
                case 64000:
                    samplesPerSec = SL_SAMPLINGRATE_64;
                    break;
                case 88200:
                    samplesPerSec = SL_SAMPLINGRATE_88_2;
                    break;
                case 96000:
                    samplesPerSec = SL_SAMPLINGRATE_96;
                    break;
                case 192000:
                    samplesPerSec = SL_SAMPLINGRATE_192;
                    break;
                default:
                    samplesPerSec = SL_SAMPLINGRATE_44_1;
            }
            return samplesPerSec;
        }
        static int getChannelMask(int channels) {
            int channelMask = SL_SPEAKER_FRONT_CENTER;
            switch (channels) {
                case 1:
                    channelMask = SL_SPEAKER_FRONT_CENTER;
                    break;
                case 2:
                    channelMask = SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT;
                    break;
            };
            return channelMask;
        }


    };
}
#endif //CVTEXTREADER_EESLUTILS_H
