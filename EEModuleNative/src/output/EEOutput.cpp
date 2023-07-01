//
// Created by antymistor on 2023/6/18.
//
#include "EEOutput.h"
namespace EE{
    EEReturnCode EEOutput::updateWindow(void* pANativeWindow, const EESize& windowsize ){
        return EE_FAIL;
    }
    EEReturnCode EEOutput::getCurrentPts(int64_t& pts) {
        return EE_FAIL;
    }
    EEReturnCode EEOutput::build(const EEOutputBuildParam& param) {
        return EE_FAIL;
    }

    EEReturnCode EEOutput::hotUpdate(const EEOutputHotUpdateParam& param){
        return EE_FAIL;
    }
    EEReturnCode EEOutput::start() {
        return EE_FAIL;
    }
    EEReturnCode EEOutput::mute(bool ismute){
        return EE_FAIL;
    }
    EEReturnCode EEOutput::stop(){
        return EE_FAIL;
    }
    EEReturnCode EEOutput::release() {
        return EE_FAIL;
    }
    EEReturnCode EEOutput::flush() {
        return EE_FAIL;
    }
    EEPlayState EEOutput::getPlayState(){
        return Stoped;
    }
}

