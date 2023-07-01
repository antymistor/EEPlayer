//
// Created by antymistor on 2023/6/12.
//

#include "EECommonUtils.h"
#include <random>
namespace EE {
   void EECommonUtils::resize(const EESize& maxsize , EESize& destsize){
       if(destsize.width <=0 || destsize.height <= 0){
           return;
       }
       if(maxsize.width > 0 && maxsize.height > 0){
           if(maxsize.width * destsize.height > maxsize.height * destsize.width){
               destsize.width = maxsize.height * destsize.width / destsize.height;
               destsize.height = maxsize.height;
           }else{
               destsize.height = maxsize.width * destsize.height / destsize.width;
               destsize.width = maxsize.width;
           }
       }else if( maxsize.width > 0){
           destsize.height = maxsize.width * destsize.height / destsize.width;
           destsize.width = maxsize.width;
       }else if( maxsize.height > 0){
           destsize.width = maxsize.height * destsize.width / destsize.height;
           destsize.height = maxsize.height;
       }
   }
  int64_t EECommonUtils::createRandomNumber(int64_t maxNum){
      std::random_device rd;
      auto gen = std::default_random_engine(rd());
      std::uniform_int_distribution<int64_t> dis(0, maxNum);
      return dis(gen);
   }
   std::string EECommonUtils::createRandomName(const std::string& base){
       std::string  out = std::to_string(createRandomNumber(9999));
       if(!base.empty()){
           out = base+"-"+out;
       }
       return out;
   }
   int64_t EECommonUtils::createtGlobalCount(){
       static int64_t count = -1;
       static std::mutex mMutex;
       int64_t ret;
       mMutex.lock();
       ++count;
       ret = count;
       mMutex.unlock();
       return ret;
   }
    std::string EECommonUtils::createGlobalName(const std::string& base){
        std::string  out = std::to_string(createtGlobalCount());
        if(!base.empty()){
            out = base+"-"+out;
        }
        return out;
   }
}