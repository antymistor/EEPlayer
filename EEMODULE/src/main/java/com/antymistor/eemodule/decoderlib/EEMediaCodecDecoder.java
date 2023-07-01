package com.antymistor.eemodule.decoderlib;
import android.graphics.SurfaceTexture;
import android.media.MediaCodec;
import android.media.MediaFormat;
import android.os.Handler;
import android.os.HandlerThread;
import android.util.Log;
import android.view.Surface;
import androidx.annotation.Keep;
import com.antymistor.eemodule.base.EEReturnCode;
import java.nio.ByteBuffer;
/**
 * Created by aizhiqiang on 2023/6/2
 *
 * @author aizhiqiang@bytedance.com
 */
@Keep
public class EEMediaCodecDecoder {
    private final String TAG = "EEMediaCodecDecoder";
    private final MediaCodec.BufferInfo m_bufferInfo;
    private final MediaFormat m_format;
    private final String m_minetype;
    private SurfaceTexture m_surfaceTexture;
    private Surface m_surface;
    private MediaCodec m_decoder = null;
    private HandlerThread m_handleThread = new HandlerThread("EE-MediaCodecHandler");
    private Long m_timestampOfLastDecodedFrame = Long.MIN_VALUE;
    private Long m_timestampOfCurTexFrame = Long.MIN_VALUE;
    private static final int m_PendingInputBufferThreshold = 5;
    private Boolean m_firstPlaybackTexFrameUnconsumed = false;
    private long m_pendingInputFrameCount = 0;
    private Boolean m_sawInputEOS = false;
    private Boolean m_sawOutputEOS = false;
    private Object m_frameSyncObject = new Object();
    private boolean m_frameAvailable = false;

    private boolean awaitNewImage() {
        final int TIMEOUT_MS = 3000;
        synchronized (m_frameSyncObject) {
            while (!m_frameAvailable) {
                try {
                    m_frameSyncObject.wait(TIMEOUT_MS);
                    if (!m_frameAvailable) {
                        Log.e(TAG, "Frame wait timed out!");
                        return false;
                    }
                } catch (InterruptedException ie) {
                    Log.e(TAG, "" + ie.getMessage());
                    ie.printStackTrace();
                    return false;
                }
            }
            m_frameAvailable = false;
        }
        return true;
    }

    public EEMediaCodecDecoder(String minetype ,
                               int width, int height, int texId,
                               byte[] sps, byte[] pps){
       m_minetype = minetype;
       m_bufferInfo = new MediaCodec.BufferInfo();

       //build format
       m_format = MediaFormat.createVideoFormat(m_minetype, width, height);
       if(sps != null && sps.length > 0) {
           m_format.setByteBuffer("csd-0", ByteBuffer.wrap(sps));
       }
       if(pps != null && pps.length > 0) {
           m_format.setByteBuffer("csd-1", ByteBuffer.wrap(pps));
       }
       //build surface
       m_handleThread.start();
       m_surfaceTexture = new SurfaceTexture(texId);
        m_surface = new Surface(m_surfaceTexture);
       m_surfaceTexture.setOnFrameAvailableListener(new SurfaceTexture.OnFrameAvailableListener() {
           @Override
           public void onFrameAvailable(SurfaceTexture surfaceTexture) {
               Log.i(TAG, "onFrameAvailable !!");
               synchronized (m_frameSyncObject) {
                   m_frameAvailable = true;
                   m_frameSyncObject.notifyAll();
               }
           }
       }, new Handler(m_handleThread.getLooper()));

       //creatdecoder
       boolean ret = createDecoder();
       Log.i(TAG, "EEMediaCodecDecoder create result" + ret);
    }



    private boolean createDecoder(){
        try {
            m_decoder = MediaCodec.createDecoderByType(m_minetype);
            m_decoder.configure(m_format, m_surface, null, 0);
            m_decoder.start();
        } catch (Exception e) {
            e.printStackTrace();
            releaseDecoder();
            return false;
        }
        return true;
    }

    public boolean flushDecoder(){
        if (m_decoder != null) {
            m_decoder.flush();
        }
        m_timestampOfLastDecodedFrame = Long.MIN_VALUE;
        m_timestampOfCurTexFrame = Long.MIN_VALUE;
        m_firstPlaybackTexFrameUnconsumed = false;
        m_pendingInputFrameCount = 0;
        m_sawInputEOS = false;
        m_sawOutputEOS = false;
        return true;
    }

    public boolean releaseDecoder() {
        if (m_decoder != null) {
            m_decoder.flush();
            m_decoder.release();
            m_decoder = null;
        }
        if (m_surface != null) {
            m_surface.release();
            m_surface = null;
        }
        if (m_surfaceTexture != null) {
            m_surfaceTexture.release();
            m_surfaceTexture = null;
        }

        m_timestampOfLastDecodedFrame = Long.MIN_VALUE;
        m_timestampOfCurTexFrame = Long.MIN_VALUE;
        m_firstPlaybackTexFrameUnconsumed = false;
        m_pendingInputFrameCount = 0;
        m_sawInputEOS = false;
        m_sawOutputEOS = false;
        return true;
    }

    //返回值设计为按位表征的格式
    // returnvalue = m_timestampOfCurTexFrame << 5 | frame.flag << 4 | returnvalue
    public long decodeFrame(byte[] frameData, int inputSize, long timeStamp) {
        int ret = EEReturnCode.EE_FAIL;
        final int TIMEOUT_USEC = 4000;
        //equeue inputbuffer
       do {
           if (!m_sawInputEOS) {
               int inputBufIndex = m_decoder.dequeueInputBuffer(TIMEOUT_USEC);
               int tryCount = 0;
               while(inputBufIndex < 0) {
                   try {
                       Thread.sleep(5, 0);
                   } catch (InterruptedException e) {
                       e.printStackTrace();
                   }
                   inputBufIndex = m_decoder.dequeueInputBuffer(TIMEOUT_USEC);
                   ++tryCount;
                   if (tryCount >= 10) {
                       break;
                   }
               }
               if (inputBufIndex >= 0) {
                   ByteBuffer inputBuf = m_decoder.getInputBuffer(inputBufIndex);
                   if (inputSize == 0) {
                       m_decoder.queueInputBuffer(inputBufIndex, 0, 0, 0L, MediaCodec.BUFFER_FLAG_END_OF_STREAM);
                       m_sawInputEOS = true;
                   } else {
                       inputBuf.clear();
                       inputBuf.put(frameData, 0, inputSize);
                       m_decoder.queueInputBuffer(inputBufIndex, 0, inputSize, timeStamp, 0);
                       ++m_pendingInputFrameCount;
                   }
               } else {
                   ret = EEReturnCode.EE_UNUSUAL;
               }
           }
           //equeue outputbuffer
           int dequeueTimeoutUs = (m_pendingInputFrameCount > m_PendingInputBufferThreshold || m_sawInputEOS) ? TIMEOUT_USEC : 0;

           int decoderStatus;
           do {
               decoderStatus = m_decoder.dequeueOutputBuffer(m_bufferInfo, dequeueTimeoutUs);
               if (decoderStatus == MediaCodec.INFO_OUTPUT_BUFFERS_CHANGED || decoderStatus == MediaCodec.INFO_OUTPUT_FORMAT_CHANGED) {
                   Log.e(TAG, "Unexpected result from decoder.dequeueOutputBuffer: " + decoderStatus);
               } else { // decoderStatus >= 0
                   break;
               }
           } while (true);

           if (decoderStatus == MediaCodec.INFO_TRY_AGAIN_LATER) {
               ret = EEReturnCode.EE_TRY_AGAIN;
           } else if (decoderStatus < 0) {
               ret = EEReturnCode.EE_FAIL;
           } else if ((m_bufferInfo.flags & MediaCodec.BUFFER_FLAG_END_OF_STREAM) != 0) {
               m_decoder.releaseOutputBuffer(decoderStatus, false);
               m_sawOutputEOS = true;
               ret = EEReturnCode.EE_EOF;
           } else {
               boolean isNewFrame = m_timestampOfLastDecodedFrame != m_bufferInfo.presentationTimeUs;
               m_timestampOfLastDecodedFrame = m_bufferInfo.presentationTimeUs;
               --m_pendingInputFrameCount;
               m_decoder.releaseOutputBuffer(decoderStatus, true);
               if (awaitNewImage()) {
                   m_timestampOfCurTexFrame = m_bufferInfo.presentationTimeUs;
                   //need check from
                   m_surfaceTexture.updateTexImage();
                   ret = isNewFrame ? EEReturnCode.EE_OK : EEReturnCode.EE_OLDFRAME;
               } else {
                   ret = EEReturnCode.EE_WAIT_LONG;
               }
           }
       }while(ret == EEReturnCode.EE_TRY_AGAIN);
        Log.d(TAG, "Return Code = " + ret);
        return m_timestampOfLastDecodedFrame << 8 | (long) m_bufferInfo.flags << 4 | ret;
    }
}
