package com.wzq.ffmpegdemo.puller.ui;

import android.content.Context;
import android.content.res.Configuration;
import android.hardware.SensorManager;
import android.os.Handler;
import android.os.PowerManager;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.DisplayMetrics;
import android.util.Log;
import android.view.OrientationEventListener;
import android.view.SurfaceView;
import android.view.View;
import android.view.ViewGroup;
import android.view.WindowManager;
import android.widget.ImageView;
import android.widget.RelativeLayout;
import android.widget.TextView;

import com.wzq.ffmpegdemo.R;
import com.wzq.ffmpegdemo.constants.constant;
import com.wzq.ffmpegdemo.puller.utils.Puller;

public class MainActivity extends AppCompatActivity implements View.OnClickListener {

    private SurfaceView surfaceView;
    private Puller puller;
    private ImageView mediaPlayImage;
    private TextView startText;

    private RelativeLayout content;

    private double time;


    Handler handler = new Handler();

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        getWindow().setFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON, WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);


        surfaceView = (SurfaceView) findViewById(R.id.surfaceView);
        content = (RelativeLayout) findViewById(R.id.rContent);
        mediaPlayImage = (ImageView) findViewById(R.id.playImg);
        startText = (TextView) findViewById(R.id.startText);
        mediaPlayImage.setOnClickListener(this);

        puller = new Puller();
        puller.setSurfaceView(surfaceView);

        setWH(getResources().getConfiguration().orientation);

        startRunTime();

    }

    private void startRunTime(){
        handler.postDelayed(new Runnable() {
            @Override
            public void run() {
                if (puller.isPlay() == 1) {
                    startText.setText(String.format("%02d:%02d", ((long)(time + puller.getTime())) / 60, ((long)(time + puller.getTime())) % 60));
                }
                handler.postDelayed(this, 500);
            }
        }, 500);
    }


    private void setWH(int oritention) {
        float screenWidth = getWindowManager().getDefaultDisplay().getWidth();
        RelativeLayout.LayoutParams layoutParams = (RelativeLayout.LayoutParams) content.getLayoutParams();
        layoutParams.height = (int) (screenWidth * 9 / 16.0);
        layoutParams.width = (int) screenWidth;
        content.setLayoutParams(layoutParams);

    }

    public void play() {
        if (puller.isPlay() == 1) {
            doPause();
        } else {
            rePlay();
        }

    }

    private void doPause() {
        mediaPlayImage.setImageResource(R.mipmap.mediacontroller_play);
        time += puller.getTime();
        puller.release();

    }

    private void rePlay() {
        puller.play(constant.BASE_URL);
        mediaPlayImage.setImageResource(R.mipmap.mediacontroller_pause);
    }



    @Override
    public void onConfigurationChanged(Configuration newConfig) {
        super.onConfigurationChanged(newConfig);
        setWH(newConfig.orientation);
    }

    @Override
    protected void onDestroy() {
        handler.removeCallbacksAndMessages(null);
        super.onDestroy();
    }
//
//    @Override
//    protected void onStop() {
//        doPause();
//        super.onStop();
//    }

    @Override
    public void onClick(View view) {
        switch (view.getId()) {
            case R.id.playImg:
                play();
                break;
        }
    }

}
