//
// Created by ByteDance on 2023/6/1.
//

#ifndef CVTEXTREADER_EECODEDEFINE_H
#define CVTEXTREADER_EECODEDEFINE_H
#include <android/log.h>
#include <string>
#define LOGI(...)  __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__);
#define LOGE(...)  __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__);
#define ARRAY_LEN(a) (sizeof(a) / sizeof(a[0]))

//extraFlag define
#define ENABLE_BLURR_BACKGROUND_DISPLAY 0x01
#define ENABLE_HIGH_Q_DISPLAY           0x02

namespace EE{
   enum EEReturnCode {
       EE_OK         = 0,
       EE_FAIL       = 1,
       EE_BADFILE    = 2,
       EE_BADDISPLAY = 3,
       EE_BADCONFIG  = 4,
       EE_BADCONTEXT = 5,
       EE_BADSURFACE = 6,
       EE_NULLINPUT  = 7,
       EE_NOTINIT    = 8,
       EE_EOF        = 10,
       EE_WAIT_LONG  = 11,
       EE_UNUSUAL    = 12,
       EE_OLDFRAME   = 13,
   };

   enum EEFrameType{
       FLAG_NORMAL_FRAME    = 0,
       FLAG_KEY_FRAME     = 1,
       FLAG_CODEC_CONFIG  = 2,
       FLAG_END_OF_STREAM = 4,
       FLAG_PARTIAL_FRAME = 8,
   };

    enum EEEGLConfigMode {
        PIXEL_BUFFER_MODE,
        WINDOW_MODE,
        PIXMAP_MODE
    };

    enum EEViewFillMode{
        IN_CENTER_BY_STRETCH = 0,
        IN_CENTER_BY_FIT     = 1,
        IN_CENTER_BY_CROP    = 2
    };

    struct EERatio{
        int num = 0;
        int den = 1;
        EERatio(int num_ = 0, int den_ = 1){
            num = num_;
            den = den_;
        }
    };

   struct EESize {
       int width = 0;
       int height = 0;
       EESize(int w = 0, int h = 0){
           width = w;
           height = h;
       }
       bool operator==(const EESize& size){
           return width == size.width && height == size.height;
       }
       bool operator!=(const EESize& size){
           return !(*this == size);
       }
   };
   struct EERect {
       int pos_x  = 0;
       int pos_y  = 0;
       int width  = 0;
       int height = 0;
       EERect(int w = 0, int h = 0){
          width = w;
          height = h;
       }
       EERect(EESize size){
           width = size.width;
           height =  size.height;
       }
   };

   struct EEBuffer {
       char* data = nullptr;
       long size = 0;
       EEBuffer(const char* data_ = nullptr, const long size_ = 0){
           if(size_ > 0 && data_ != nullptr){
               size = size_;
               data = new char[size];
               memcpy(data, data_, size);
           }
       }
       ~EEBuffer(){
           if(data){
               delete []data;
           }
       }
       EEBuffer& operator=(const EEBuffer& buffer){
           if(data){
               delete []data;
           }
           if(buffer.size > 0 && buffer.data != nullptr){
               size = buffer.size;
               data = new char[size];
               memcpy(data, buffer.data, size);
           }
           return *this;
       }
   };
}
#endif //CVTEXTREADER_EECODEDEFINE_H