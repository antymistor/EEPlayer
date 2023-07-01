//
// Created by ByteDance on 2023/6/6.
//

#include "EETask.h"
#include <list>
#include <thread>
#include "../basedefine/EECodeDefine.h"
#define LOG_TAG "EETASK"
namespace EE{
    EETask::EETask(const EETask& task) : mTask(task.mTask), mIsAsync((bool)task.mIsAsync){}
    EETask::EETask(std::function<void()> function, bool isAsync) : mTask(std::move(function)), mIsAsync(isAsync) {}
    EEReturnCode EETask::aWaitFinish(){
       std::unique_lock<std::mutex> lock(mMutex);
       while(!mIsFinished){
           mCondition.wait(lock);
       }
       return EE_OK;
    }
    void EETask::run(){
        if(mTask){
            mTask();
        }
        mIsFinished = true;
    }
    void EETask::notify(){
        mCondition.notify_all();
    }
}