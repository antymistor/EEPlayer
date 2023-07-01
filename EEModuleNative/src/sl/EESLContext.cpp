//
// Created by antymistor on 2023/6/18.
//

#include "EESLContext.h"
#include "../basedefine/EECodeDefine.h"
#include <mutex>
namespace EE {

        SLEngineItf EESLContext::GetInstanceEngineItf(){
            static EESLContext gContext;
            static std::once_flag flag;
            std::call_once(flag, [&](){
                gContext.build();
            });
            if(gContext.isbuildSuccess){
                return gContext.engineItf;
            }
            return nullptr;
        }
        void EESLContext::build(){
            //create global engine
            SLEngineOption engineOptions[] = { { (SLuint32) SL_ENGINEOPTION_THREADSAFE, (SLuint32) SL_BOOLEAN_TRUE } };
            auto ret = slCreateEngine(&engineObject, ARRAY_LEN(engineOptions),  engineOptions, 0, nullptr, nullptr);
            if(ret != SL_RESULT_SUCCESS){
                return ;
            }
            //Realize the engine object
            ret = (*engineObject)->Realize(engineObject, SL_BOOLEAN_FALSE);
            if(ret != SL_RESULT_SUCCESS){
                return ;
            }
            //get Itf
            ret = (*engineObject)->GetInterface(engineObject, SL_IID_ENGINE, &engineItf);
            isbuildSuccess = (ret == SL_RESULT_SUCCESS);
        }
}