package com.antymistor.eemodule.nativeport;
import android.view.Surface;
import androidx.annotation.Keep;
import com.antymistor.eemodule.eeplayer.EEMediaInfo;
import com.antymistor.eemodule.eeplayer.EEPlayerBuildParam;
import com.antymistor.eemodule.eeplayer.EEPlayer;

/**
 * Created by antymistor on 2023/5/26
 *
 * @author azq2018@zju.edu.cn.com
 */
@Keep
public class EENativePort {
    public static void LoadJNI(){
       // System.loadLibrary("x264");
        System.loadLibrary("eemodule-lib");
        nativeLoadJNI();
    }

    public static native long nativeCreateJavaObjectHolder(Object obj);
    public static native void nativeReleaseJavaObjectHolder(long holderPointer);

    public static native void  nativeLoadJNI();
    public static native long  nativeCreateEEPlayer();
    public static native void  nativeDestroyEEPlayer(long pointer);
    public static native void  nativeEEPlayerUpdateFilePath(long pointer, String path);
    public static native void  nativeEEPlayerUpdateDisplayWindow(long pointer, Surface surface);
    public static native EEMediaInfo  nativeEEPlayergetMediaInfo(long pointer);
    public static native void  nativeEEPlayerBuild(long pointer, EEPlayerBuildParam param, long javaholder);
    public static native void  nativeEEPlayerStart(long pointer);
    public static native void  nativeEEPlayerResetVideoDisplayFitMode(long pointer, int videoDisplayFitMode);
    public static native void  nativeEEPlayerResetVideoRenderFps(long pointer, int videoRenderFps);
    public static native void  nativeEEPlayerResetVideoDisplayRotation(long pointer, int VideoDisplayRotation);
    public static native void  nativeEEPlayerResetVideoDisplayScale(long pointer, float Scale);
    public static native boolean  nativeEEPlayerSeekTo(long pointer, float position);
    public static native boolean  nativeEEPlayerSeekBy(long pointer, float distance);
    public static native void  nativeEEPlayerStop(long pointer);
    public static native float nativeEEPlayerGetProgress(long pointer);

}
