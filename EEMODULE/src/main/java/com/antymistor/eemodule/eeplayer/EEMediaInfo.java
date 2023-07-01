package com.antymistor.eemodule.eeplayer;

import androidx.annotation.Keep;

/**
 * Created by aizhiqiang on 2023/6/22
 *
 * @author aizhiqiang@bytedance.com
 */
@Keep
public class EEMediaInfo {
    //video related
    public String videoMineType;
    public long   videoDuration = 0;
    public int    videoWidth = 0;
    public int    videoHeight = 0;
    public int    videoRotate = 0;
    public int    videofps    = 0;

    //audio related
    public String audioMineType;
    public long   audioDuration = 0;
    public int    audioChannels = 0;
    public int    audioSampleRate = 0;
}
