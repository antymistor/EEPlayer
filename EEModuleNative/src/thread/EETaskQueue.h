//
// Created by antymistor on 2023/6/16.
//

#ifndef CVTEXTREADER_EETASKQUEUE_H
#define CVTEXTREADER_EETASKQUEUE_H
#include "EETask.h"
namespace EE {
    class EETaskQueue{
    public:
        EETaskQueue(const std::string& name);
        EEReturnCode run(const EETask &task);
        ~EETaskQueue();
    private:
        void runInLoop();
        struct EETaskQueueMembers;
        std::unique_ptr<EETaskQueueMembers> mMembers;
    };
}


#endif //CVTEXTREADER_EETASKQUEUE_H
