package com.antymistor.eemodule.sharedObj;

import com.antymistor.eemodule.nativeport.EENativePort;

public class EESharedObj {
    private long EESharedObjNativeHandle = 0;
    private String name;
    public void create(String name_){
        name = name_;
        EESharedObjNativeHandle = EENativePort.nativeCreateSharedObject(name);
    }
    public void destroy(){
        EENativePort.nativeReleaseSharedObject(EESharedObjNativeHandle);
        EESharedObjNativeHandle = 0;
    }
    public long getNativeHandle(){
        return EESharedObjNativeHandle;
    }
}
