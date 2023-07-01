//
// Created by ByteDance on 2023/6/18.
//

#ifndef CVTEXTREADER_EESLCONTEXT_H
#define CVTEXTREADER_EESLCONTEXT_H
#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>
namespace EE {
    class EESLContext {
    public:
        static SLEngineItf GetInstanceEngineItf();
    private:
        SLObjectItf engineObject;
        SLEngineItf engineItf;
        bool isbuildSuccess = false;
        EESLContext() = default;
        virtual ~EESLContext() = default;
        void build();
    };
}


#endif //CVTEXTREADER_EESLCONTEXT_H
