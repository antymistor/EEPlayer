//
// Created by ByteDance on 2023/6/16.
//

#ifndef CVTEXTREADER_EELIST_H
#define CVTEXTREADER_EELIST_H

#include "EECodeDefine.h"
#include <list>
#include <mutex>
namespace EE {
    template<typename T> class EEList {
        public:
            EEList(int maxsize = INT_MAX) : maxSize(maxsize){}
            EEReturnCode clear(){
                std::unique_lock<std::mutex> lock(mMutex);
                mList.clear();
                mCondition.notify_all();
                return EE_OK;
            }
            EEReturnCode put(const T& ele, bool wait = true){
                std::unique_lock<std::mutex> lock(mMutex);
                EEReturnCode ret = EE_FAIL;
                while (mState == stopped_withBlock || ( mState != stopped_withNOBlock && mList.size() >= maxSize) ){
                    if(wait){
                        mCondition.wait(lock);
                    }else{
                        return ret;
                    }
                }
                if(mState == started) {
                    mList.emplace_back(ele);
                    ret = EE_OK;
                }
                mCondition.notify_all();
                return ret;
            }
            T get(bool wait = true){
                std::unique_lock<std::mutex> lock(mMutex);
                T ret{};
                while (mState == stopped_withBlock || ( mState != stopped_withNOBlock && mList.empty()) ){
                    if(wait){
                        mCondition.wait(lock);
                    }else{
                        return ret;
                    }
                }
                if(mState == started) {
                    ret = mList.front();
                    mList.pop_front();
                }
                mCondition.notify_all();
                return ret;
            }
            void stop(bool isWithBlock = false){
                std::unique_lock<std::mutex> lock(mMutex);
                mState = isWithBlock ? stopped_withBlock : stopped_withNOBlock;
                mCondition.notify_all();
            }
            void start(){
                std::unique_lock<std::mutex> lock(mMutex);
                mState = started;
                mCondition.notify_all();
            }
            void updateMaxSize(int maxsize){
                std::unique_lock<std::mutex> lock(mMutex);
                maxSize = maxsize;
                mCondition.notify_all();
            }
        private:
            enum EEListState{
                started = 0,
                stopped_withNOBlock = 1,
                stopped_withBlock =2
            };
            std::list<T> mList;
            std::mutex mMutex;
            std::condition_variable mCondition;
            std::atomic<int64_t> maxSize;
            std::atomic<EEListState> mState = started;
    };
}
#endif //CVTEXTREADER_EELIST_H
