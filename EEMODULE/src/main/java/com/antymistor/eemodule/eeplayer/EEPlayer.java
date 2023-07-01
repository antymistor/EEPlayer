package com.antymistor.eemodule.eeplayer;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import androidx.annotation.NonNull;
import com.antymistor.eemodule.nativeport.EENativePort;
/**
 * Created by antymistor on 2023/6/19
 *
 * @author azq2018@zju.edu.cn.com
 */
public class EEPlayer {
    public enum EEPlayerCallBackType{
        onNothing,
        onFirstVideoFrame,
        onFirstAudioFrame,
        onSeekFinish,
        onPlayToOES
    }
    public interface EEPlayerListener{
        void onInfo(EEPlayerCallBackType type, float value1, float value2);
    }



    private long EEPlayerNativeHandle = 0;
    private long EEPlayerJavaHandle   = 0;

    public void create(){
        EEPlayerNativeHandle = EENativePort.nativeCreateEEPlayer();
        EEPlayerJavaHandle   = EENativePort.nativeCreateJavaObjectHolder(this);
    }

    private EEPlayerListener listener = null;
    public void setListener(EEPlayerListener listener_){
        listener = listener_;
    }

    public void onInfo(int type, float value1, float value2){
        if(listener!= null){
            listener.onInfo( EEPlayerCallBackType.values()[type],value1,value2);
        }
    }

    public void destroy(){
         EENativePort.nativeDestroyEEPlayer(EEPlayerNativeHandle);
         EEPlayerNativeHandle = 0;
         EENativePort.nativeReleaseJavaObjectHolder(EEPlayerJavaHandle);
         EEPlayerJavaHandle = 0;
    }

    public void updateFilePath(String path){
        EENativePort.nativeEEPlayerUpdateFilePath(EEPlayerNativeHandle, path);
    }

    public EEMediaInfo getMediaInfo(){
        return EENativePort.nativeEEPlayergetMediaInfo(EEPlayerNativeHandle);
    }

    public void setSurfaceView(SurfaceView view){
        view.getHolder().addCallback(new SurfaceHolder.Callback() {
            @Override
            public void surfaceCreated(@NonNull SurfaceHolder holder) {}
            @Override
            public void surfaceChanged(@NonNull SurfaceHolder holder, int format, int width, int height) {
                EENativePort.nativeEEPlayerUpdateDisplayWindow(EEPlayerNativeHandle, holder.getSurface());
            }
            @Override
            public void surfaceDestroyed(@NonNull SurfaceHolder holder) {}
        });
    }

    public void build(EEPlayerBuildParam param){
        EENativePort.nativeEEPlayerBuild(EEPlayerNativeHandle, param, EEPlayerJavaHandle);
    }

    public void start(){
        EENativePort.nativeEEPlayerStart(EEPlayerNativeHandle);
    }

    public void resetVideoDisplayFitMode(int videoDisplayFitMode){
        EENativePort.nativeEEPlayerResetVideoDisplayFitMode(EEPlayerNativeHandle, videoDisplayFitMode);
    }
    public void resetVideoRenderFps(int videoRenderFps){
        EENativePort.nativeEEPlayerResetVideoRenderFps (EEPlayerNativeHandle, videoRenderFps);
    }
    public void resetVideoDisplayRotation(int VideoDisplayRotation){
        EENativePort.nativeEEPlayerResetVideoDisplayRotation(EEPlayerNativeHandle, VideoDisplayRotation);
    }
    public void resetVideoDisplayScale(float scale){
        EENativePort.nativeEEPlayerResetVideoDisplayScale(EEPlayerNativeHandle, scale);
    }


    public boolean seekTo(float position){
        return EENativePort.nativeEEPlayerSeekTo(EEPlayerNativeHandle, position);
    }

    public boolean seekBy(float distance){
        return EENativePort.nativeEEPlayerSeekBy(EEPlayerNativeHandle, distance);
    }

    public void stop(){
        EENativePort.nativeEEPlayerStop(EEPlayerNativeHandle);
    }

    public Float getProgress(){
        return  EENativePort.nativeEEPlayerGetProgress(EEPlayerNativeHandle);
    }
}
