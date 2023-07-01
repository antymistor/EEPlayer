package com.antymistor.eeplayer;
import android.graphics.Color;
import android.graphics.PorterDuff;
import android.os.Bundle;
import android.util.Log;
import android.view.GestureDetector;
import android.view.MotionEvent;
import android.view.ScaleGestureDetector;
import android.view.SurfaceView;
import android.view.View;
import android.view.WindowManager;
import android.widget.SeekBar;
import android.widget.TextView;
import android.widget.Toast;

import androidx.appcompat.app.AppCompatActivity;
import androidx.core.view.GestureDetectorCompat;
import androidx.core.view.MotionEventCompat;
import com.antymistor.eemodule.eeplayer.EEMediaInfo;
import com.antymistor.eemodule.eeplayer.EEPlayer;
import com.antymistor.eemodule.eeplayer.EEPlayerBuildParam;
import com.antymistor.eemodule.eeplayer.EEPlayerSpConfig;
import com.antymistor.eemodule.nativeport.EENativePort;
import com.antymistor.eeplayer.utils.SeekbarAdvance;
import com.google.gson.Gson;
import com.google.gson.JsonElement;
import com.google.gson.JsonParser;

import java.io.BufferedWriter;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.OutputStreamWriter;
import java.nio.charset.StandardCharsets;
import java.util.Objects;
import java.util.Timer;
import java.util.TimerTask;
import java.util.concurrent.ConcurrentHashMap;

/**
 * Created by aizhiqiang on 2023/6/23
 *
 * @author aizhiqiang@bytedance.com
 */
public class VideoPlayActivity extends AppCompatActivity {
    static {
        EENativePort.LoadJNI();
    }

    private static final int prgresssbarPrecision = 250;  //250ms
    private static final int flingerTheshold = 100;
    private static final int seekBySpeedMax = 20;
    private static final int timerPeriod = 1000;
    private static final float maxScale = 5.0f;
    private static final float minScale = 0.1f;

    private SurfaceView displayView;
    private EEPlayer mPlayer;
    private TextView time_horizontal;
    private TextView time_vertical;
    private SeekbarAdvance mProgressbar = null;
    private EEPlayerBuildParam param;
    private EEMediaInfo mVideoinfo;
    private Timer mtimer;
    private String displayTimeStr = "";
    private long   displayTime = 0;    //ms
    private Boolean isVisable = false;
    private Boolean isinSeeking = false;
    private long mLastTouchDownTime = 0;
    private float  mLastTouchDownPosX = 0;
    private float  mLastTouchDownPosY = 0;
    private int    seekBySpeed = 20;
    private float scale = 1.0f;
    private GestureDetectorCompat mDetector;
    private ScaleGestureDetector mDetectorScale;
    private File progressfile;
    private statusinfo statusinfo_     = null;

    public static class statusinfo{
        ConcurrentHashMap<String, Long> fileprogresslist = new ConcurrentHashMap<>();
        String currentFilePathDy = "";
        long progress = 0;
    }

    private static boolean isJson(String content) {
        JsonElement jsonElement;
        try {
            jsonElement = new JsonParser().parse(content);
        } catch (Exception e) {
            return false;
        }
        if (jsonElement == null) {
            return false;
        }
        return jsonElement.isJsonObject();
    }

    private Boolean checkFileExits(String Path){
        File file = new File(Path);
        return file.exists();
    }

    private void GetAndSetStatusFromDisk(){
        try {
            progressfile = new File(getFilesDir().getPath() + "/progress.json");
            if (!progressfile.exists() && !progressfile.createNewFile()) {
                progressfile = null;
            } else {
                FileInputStream fin = new FileInputStream(progressfile);
                int len = fin.available();
                if (len > 0) {
                    byte[] buffer = new byte[len];
                    fin.read(buffer);
                    Gson gsonin = new Gson();
                    String jstr = new String(buffer);
                    if(isJson(jstr)) {
                        statusinfo_ = gsonin.fromJson(jstr, statusinfo.class);
                        statusinfo_.currentFilePathDy = getIntent().getStringExtra("filePath").isEmpty() ? statusinfo_.currentFilePathDy : getIntent().getStringExtra("filePath");
                        if(statusinfo_.currentFilePathDy == null || !checkFileExits(statusinfo_.currentFilePathDy)){
                            statusinfo_.currentFilePathDy = null;
                            return;
                        }
                        if(statusinfo_.fileprogresslist != null && statusinfo_.fileprogresslist.containsKey(statusinfo_.currentFilePathDy) ){
                            Long pf =  statusinfo_.fileprogresslist.get(statusinfo_.currentFilePathDy);
                            if(pf != null){
                                statusinfo_.progress = pf;
                            }else{
                                statusinfo_.progress = 0;
                            }
                        }
                        Log.e("aizhiqiang","read json success" + String.valueOf(statusinfo_.progress) );
                    }else{
                        Log.e("aizhiqiang","read json fail");
                    }
                }
            }
            if (statusinfo_ == null) {
                Log.e("aizhiqiang","read json fail");
                statusinfo_ = new statusinfo();
                statusinfo_.progress = 0;
            }
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    private void loadProgressbar(){
        if(mVideoinfo != null) {
            mProgressbar.setMax((int)mVideoinfo.videoDuration/prgresssbarPrecision);
            mProgressbar.getThumb().setColorFilter(Color.parseColor("#ff00ff00"), PorterDuff.Mode.SRC_IN);
            mProgressbar.setOnClickThumbListener(new SeekbarAdvance.OnClickThumbListener() {
                @Override
                public void onClickThumb() { }
            });
            mProgressbar.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
                @Override
                public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                    if (isVisable && fromUser && mPlayer != null && mVideoinfo != null && !isinSeeking) {
                        isinSeeking = true;
                        float destRatio = 1.0f * prgresssbarPrecision * progress / mVideoinfo.videoDuration;
                        if(mPlayer.seekTo(destRatio) && isStop){
                            mPlayer.start();
                        }
                    }
                }
                @Override
                public void onStartTrackingTouch(SeekBar seekBar) { }
                @Override
                public void onStopTrackingTouch(SeekBar seekBar) { }
            });
            mProgressbar.setProgress(0);
            mProgressbar.bringToFront();
            mProgressbar.setVisibility(View.INVISIBLE);
            mProgressbar.setEnabled(false);
        }
    }



    private void flipVisable(){
        if(isVisable){
            isVisable = false;
            time_horizontal.setText("");
            time_vertical.setText("");
            mProgressbar.setVisibility(View.GONE);
            mProgressbar.setEnabled(false);
        }else{
            isVisable = true;
            mProgressbar.setVisibility(View.VISIBLE);
            mProgressbar.setEnabled(true);
        }
    }

    private void UpdateDisplayTime(){
        if(mPlayer != null && mVideoinfo != null){
            displayTime = (long)(mPlayer.getProgress() * mVideoinfo.videoDuration);  // unit = s
            statusinfo_.progress = displayTime / 1000;
            long hour_played = statusinfo_.progress / 3600;
            long min_played  = (statusinfo_.progress % 3600 ) / 60;
            long sec_played  = (statusinfo_.progress % 60);
            displayTimeStr = String.format("%02d:%02d:%02d", hour_played, min_played, sec_played);
        }
        if(isVisable){
            runOnUiThread(() -> {
                mProgressbar.setProgress((int) displayTime / prgresssbarPrecision);
                if(param.videoDisplayRotation == 90){
                    time_horizontal.setText("");
                    time_vertical.setText(displayTimeStr);
                    time_vertical.setVisibility(View.VISIBLE);
                }else{
                    time_vertical.setText("");
                    time_horizontal.setText(displayTimeStr);
                }
            });
        }
    }

    void saveStatusToDisk(){
        //write current progress to disk
        if(progressfile.exists() && statusinfo_ != null){
            try {
                if(statusinfo_.currentFilePathDy != null && !statusinfo_.currentFilePathDy.isEmpty()){
                    statusinfo_.fileprogresslist.put(statusinfo_.currentFilePathDy, statusinfo_.progress);
                }
                BufferedWriter writer = new BufferedWriter(new OutputStreamWriter(new FileOutputStream(progressfile,false), StandardCharsets.UTF_8));
                Gson gson = new Gson();
                String content = gson.toJson(statusinfo_);
                writer.write(content);
                writer.close();
            }catch ( IOException e) {
                e.printStackTrace();
            }
        }
    }

    private void buildTimer(){
        mtimer = new Timer();
        mtimer.schedule(new TimerTask() {
            @Override
            public void run() {
                UpdateDisplayTime();
                saveStatusToDisk();
            }
        }, 100, timerPeriod);
    }


    private void releaseALl(){
        if(mtimer != null){
            mtimer.cancel();
            mtimer = null;
        }
        if(mPlayer != null){
            mPlayer.destroy();
            mPlayer = null;
        }
    }


    @Override
    public void onBackPressed() {
        super.onBackPressed();
        releaseALl();
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        Objects.requireNonNull(getSupportActionBar()).hide();
        WindowManager.LayoutParams attributes = getWindow().getAttributes();
        attributes.flags |= WindowManager.LayoutParams.FLAG_TRANSLUCENT_NAVIGATION;
        getWindow().setAttributes(attributes);
        getWindow().setStatusBarColor(Color.TRANSPARENT);
        setContentView(R.layout.videoplay_activity);

        //Load Disk
        GetAndSetStatusFromDisk();
        if(statusinfo_.currentFilePathDy == null || statusinfo_.currentFilePathDy.isEmpty()){
            return;
        }
        //build palyer
        displayView =  findViewById(R.id.videoview);
        mPlayer = new EEPlayer();
        mPlayer.create();
        mPlayer.updateFilePath(statusinfo_.currentFilePathDy);
        mPlayer.setSurfaceView(displayView);
        mVideoinfo = mPlayer.getMediaInfo();
        param = new EEPlayerBuildParam();
        if( (mVideoinfo.videoRotate % 180 == 0  && mVideoinfo.videoHeight < mVideoinfo.videoWidth ) ||
            (mVideoinfo.videoRotate % 180 == 90  && mVideoinfo.videoWidth < mVideoinfo.videoHeight ) ) {
            param.videoDisplayRotation = 90;
        }
        param.extraFlag = EEPlayerSpConfig.ENABLE_BLURR_BACKGROUND_DISPLAY
                         | EEPlayerSpConfig.ENABLE_HIGH_Q_DISPLAY
                          ;
        param.startPlayTime = Math.min(statusinfo_.progress * 1000,  mVideoinfo.videoDuration - 500);
        mPlayer.setListener(new EEPlayer.EEPlayerListener() {
            @Override
            public void onInfo(EEPlayer.EEPlayerCallBackType type, float value1, float value2) {
                switch (type){
                    case onFirstVideoFrame: break;
                    case onSeekFinish:{
                            isinSeeking = false;
                            if(isStop){
                                mPlayer.stop();
                            }
                       }break;
                    case onPlayToOES:{
                        Log.i("onInfo", "PlayToEnd");
                        }break;
                    default: break;
                }
            }
        });
        mPlayer.build(param);
        mPlayer.start();
        //find textview
        time_vertical = findViewById(R.id.time_vertical);
        time_horizontal = findViewById(R.id.time_horizontal);
        //find progressbar
        mProgressbar = findViewById(R.id.processbar);
        //create detect
        loadGesture();
        //set Other
        seekBySpeed = (int) Math.min(seekBySpeedMax, mVideoinfo.videoDuration / 10000);
    }

    @Override
    protected void onStart() {
        super.onStart();
        loadProgressbar();
        buildTimer();
    }

    private boolean isStop = false;
    private void flipPlayState(){
        Log.i("flipPlayState", "src stop is" + isStop);
        if(isStop){
            isStop = false;
            if(mPlayer!= null){
                mPlayer.start();
            }
        }else{
            isStop = true;
            if(mPlayer!= null){
                mPlayer.stop();
            }
        }
    }

    @Override
    public boolean onTouchEvent(MotionEvent event) {
        mDetector.onTouchEvent(event);
        mDetectorScale.onTouchEvent(event);
        final int action = MotionEventCompat.getActionMasked(event);
        if (action == MotionEvent.ACTION_DOWN) {
            if( (System.currentTimeMillis() - mLastTouchDownTime < 200)){
                flipPlayState();
            }
            mLastTouchDownTime = System.currentTimeMillis();
            mLastTouchDownPosX = event.getX();
            mLastTouchDownPosY = event.getY();
        }else if(action == MotionEvent.ACTION_UP){
           if((System.currentTimeMillis() - mLastTouchDownTime) < 200 &&
                   Math.abs(event.getX() - mLastTouchDownPosX) < 10 &&
                   Math.abs(event.getY() - mLastTouchDownPosY) < 10){
               flipVisable();
           }
        }
        return true;
    }
    int modifyNumber(int in){
        if(in > 0){
            return Math.max(in, 200);
        }else{
            return Math.min(in, -200);
        }
    }

    private void loadGesture(){
        mDetector = new GestureDetectorCompat(this, new GestureDetector.SimpleOnGestureListener(){
            @Override
            public boolean onFling(MotionEvent event1, MotionEvent event2,
                                   float velocityX, float velocityY) {
                float diffY =  event2.getY() - event1.getY();
                float diffX =  event2.getX() - event1.getX();
                int dest = 0;
                if(Math.abs(diffX) > flingerTheshold && Math.abs(diffY) < flingerTheshold  &&
                        param.videoDisplayRotation == 0 && mPlayer != null && !isinSeeking){
                    dest = modifyNumber((int)diffX * seekBySpeed);
                }else if(Math.abs(diffX) < flingerTheshold && Math.abs(diffY) > flingerTheshold &&
                        param.videoDisplayRotation == 90 && mPlayer != null && !isinSeeking){
                    dest = modifyNumber((int)diffY * seekBySpeed);
                }
                if(dest != 0){
                    isinSeeking = true;
                    Log.i("onFling", "seek to=" + dest);
                    if(mPlayer.seekBy(dest) && isStop){
                        mPlayer.start();
                    }
                }
                return true;
            }
        });


        mDetectorScale = new ScaleGestureDetector(this, new ScaleGestureDetector.OnScaleGestureListener() {
            @Override
            public boolean onScale(ScaleGestureDetector detector) {
                scale +=  (detector.getCurrentSpan() - detector.getPreviousSpan()) / 1000;
                scale = Math.min( Math.max(scale, minScale ), maxScale);
                if(mPlayer != null){
                    mPlayer.resetVideoDisplayScale(scale);
                }
                return true;
            }

            @Override
            public boolean onScaleBegin(ScaleGestureDetector detector) {
                return true;
            }

            @Override
            public void onScaleEnd(ScaleGestureDetector detector) {

            }
        });
    }
}
