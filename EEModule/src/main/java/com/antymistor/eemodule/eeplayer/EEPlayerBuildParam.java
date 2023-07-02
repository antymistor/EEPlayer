package com.antymistor.eemodule.eeplayer;
import androidx.annotation.Keep;
import com.antymistor.eemodule.sharedObj.EESharedObj;

/**
 * Created by antymistor on 2023/6/22
 *
 * @author azq2018@zju.edu.cn.com
 */
@Keep

public class EEPlayerBuildParam {
    static public final int Fill_in_Strech = 0;
    static public final int Fill_in_Fit    = 1;
    static public final int Fill_in_Crop   = 2;
    //video about
    public int maxRenderWidth      = 0;
    public int maxRenderHeight     = 0;
    public int videoRenderFps      = 0;
    public int videoDisplayFitMode = Fill_in_Fit;
    public int videoDisplayRotation= 0;
    public EESharedObj sharedObj = null;
    // both
    public long startPlayTime      = 0;
    public int  extraFlag         = 0;
}
