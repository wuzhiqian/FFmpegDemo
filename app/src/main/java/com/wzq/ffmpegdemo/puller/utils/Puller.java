package com.wzq.ffmpegdemo.puller.utils;

import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.SurfaceView;

/**
 * Created by wzq on 2017/10/12.
 */

public class Puller implements SurfaceHolder.Callback {
    static {
        System.loadLibrary("faac");
//        System.loadLibrary("rtmp");
        System.loadLibrary("ffmpeg");
        System.loadLibrary("x264");
        System.loadLibrary("native-lib");
    }

    private SurfaceView surfaceView;

    public native void display(Surface surface);

    public native void playNative(String path);


    public native void release();

    public native void pause();


    public native void seeking(double pos);

    public native int isPlay();

    public native double getTime();

    public native double getTotleTime();

    public void play(String path) {
        if (surfaceView == null)
            return;
        playNative(path);
    }

    public void setSurfaceView(SurfaceView surfaceView) {
        this.surfaceView = surfaceView;
        display(surfaceView.getHolder().getSurface());
        surfaceView.getHolder().addCallback(this);
    }


    @Override
    public void surfaceCreated(SurfaceHolder surfaceHolder) {

    }

    @Override
    public void surfaceChanged(SurfaceHolder surfaceHolder, int i, int i1, int i2) {
        display(surfaceHolder.getSurface());
    }

    @Override
    public void surfaceDestroyed(SurfaceHolder surfaceHolder) {

    }
}
