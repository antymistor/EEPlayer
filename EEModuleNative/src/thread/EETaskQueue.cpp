//
// Created by ByteDance on 2023/6/16.
//

#include "EETaskQueue.h"
#include "../basedefine/EEList.h"
#include <list>
#include <thread>
namespace EE{
    struct EETaskQueue::EETaskQueueMembers{
        std::string mThreadName;
        std::thread* mThread = nullptr;
        std::atomic<bool> mQuit = { false };
        EEList<std::shared_ptr<EETask>> mTaskQueue;
    };

    EETaskQueue::EETaskQueue(const std::string& name) : mMembers(std::make_unique<EETaskQueueMembers>()){
        mMembers->mThreadName = name;
        mMembers->mThread = new std::thread(&EETaskQueue::runInLoop, this);
        pthread_setname_np(mMembers->mThread->native_handle(), mMembers->mThreadName.substr(0, 15).c_str());
    }

    EEReturnCode EETaskQueue::run(const EETask &task){
        if(mMembers->mQuit){
            return EE_FAIL;
        }
        auto etask = std::make_shared<EETask>(task);
        mMembers->mTaskQueue.put(etask, true);
        EEReturnCode ret = EE_OK;
        if(!etask->mIsAsync && !mMembers->mQuit){
            ret = etask->aWaitFinish();
        }
        return ret;
    }

    void EETaskQueue::runInLoop(){
        while(!mMembers->mQuit) {
            std::shared_ptr<EETask> task  = mMembers->mTaskQueue.get(true);
            if (nullptr != task && !mMembers->mQuit) {
                task->run();
                task->notify();
            }
        }
        mMembers->mTaskQueue.clear();
    }

    EETaskQueue::~EETaskQueue(){
        mMembers->mQuit = true;
        mMembers->mTaskQueue.stop();
        if(mMembers->mThread){
            mMembers->mThread->join();
        }
        delete mMembers->mThread;
    }
}