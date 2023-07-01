//
// Created by antymistor on 2023/6/6.
//

#ifndef CVTEXTREADER_EETASK_H
#define CVTEXTREADER_EETASK_H
#include "../basedefine/EECodeDefine.h"
#include <memory>
#include <mutex>

namespace EE {
    class EETask {
        friend class EETaskQueue;
        public:
            EETask(std::function<void()>  function, bool isAsync = false);
            EETask(const EETask& task);
            EETask() = delete;
            void run();
        private:
            EEReturnCode aWaitFinish();
            void notify();
            std::function<void()> mTask = nullptr;
            std::atomic<bool> mIsFinished = { false };
            std::atomic<bool> mIsAsync = { false };

            std::mutex mMutex;
            std::condition_variable mCondition;
    };
}


#endif //CVTEXTREADER_EETASK_H
