package com.antymistor.eeplayer.utils;

import android.graphics.Bitmap;
import android.graphics.Matrix;

/**
 * Created by aizhiqiang on 2023/6/23
 *
 * @author aizhiqiang@bytedance.com
 */
public class BitmapResize {
    public static Bitmap BitmapCut(Bitmap src , int destW , int destH){
        int srcW = src.getWidth();
        int srcH = src.getHeight();
        float ScaleRatio = 1.0f;
        int cutLen;
        Matrix matrix = new Matrix();
        if(srcW * destH <  srcH * destW){   // 按照宽对齐
            ScaleRatio = 1.0f * destW / srcW;
            cutLen = srcW * destH / destW;
            matrix.postScale(ScaleRatio, ScaleRatio);
            return Bitmap.createBitmap(src, 0, (srcH - cutLen) / 2 , srcW, cutLen, matrix, true);
        }else{
            ScaleRatio = 1.0f * destH / srcH;  // 按照高对齐
            cutLen = srcH * destW / destH;
            matrix.postScale(ScaleRatio, ScaleRatio);
            return Bitmap.createBitmap(src, (srcW - cutLen) / 2 ,0, cutLen, srcH, matrix, true);
        }
    }
}
