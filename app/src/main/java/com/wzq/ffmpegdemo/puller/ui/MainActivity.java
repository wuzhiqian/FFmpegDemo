package com.wzq.ffmpegdemo.puller.ui;


import android.content.res.Configuration;
import android.os.Handler;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.view.MotionEvent;
import android.view.SurfaceView;
import android.view.View;
import android.view.WindowManager;
import android.widget.ImageView;
import android.widget.RelativeLayout;
import android.widget.TextView;

import com.wzq.ffmpegdemo.R;
import com.wzq.ffmpegdemo.constants.constant;
import com.wzq.ffmpegdemo.puller.utils.Puller;
import com.wzq.ffmpegdemo.widget.ControlLayout;

public class MainActivity extends AppCompatActivity implements View.OnClickListener {

    private SurfaceView surfaceView;
    private Puller puller;
    private ImageView mediaPlayImage;
    private TextView startText;

    private RelativeLayout content;

    private double time;
    private TextView loading;

    Handler handler = new Handler();
    Handler loadingHandler = new Handler();
    private ControlLayout controlLayout;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        getWindow().setFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON, WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);


        surfaceView = (SurfaceView) findViewById(R.id.surfaceView);
        content = (RelativeLayout) findViewById(R.id.rContent);
        mediaPlayImage = (ImageView) findViewById(R.id.playImg);
        startText = (TextView) findViewById(R.id.startText);
        loading = (TextView) findViewById(R.id.loading);
        controlLayout = (ControlLayout) findViewById(R.id.controlBar);
        mediaPlayImage.setOnClickListener(this);

        puller = new Puller();
        puller.setSurfaceView(surfaceView);

        setWH(getResources().getConfiguration().orientation);

        startRunTime();
        controlLayout.setVisibility(View.VISIBLE);
        surfaceView.setOnTouchListener(new View.OnTouchListener() {
            @Override
            public boolean onTouch(View view, MotionEvent motionEvent) {
                switch (motionEvent.getAction()) {
                    case MotionEvent.ACTION_DOWN:
                        controlLayout.setVisibility(View.VISIBLE);
                        if(puller.isPlay() == 1)
                            controlLayout.setVisibleBar();
                        break;
                    case MotionEvent.ACTION_CANCEL:
                        break;
                }
                return false;
            }
        });

    }

    private void startRunTime() {
        handler.postDelayed(new Runnable() {
            @Override
            public void run() {
                if (puller.isPlay() == 1) {
                    startText.setText(String.format("%02d:%02d", ((long) (time + puller.getTime())) / 60, ((long) (time + puller.getTime())) % 60));
                }
                handler.postDelayed(this, 500);
            }
        }, 500);
    }


    private void loading() {
        loading.setVisibility(View.VISIBLE);
        loading.setText("loading...");
        if (mediaPlayImage.isEnabled())
            mediaPlayImage.setEnabled(false);
        controlLayout.setVisibleBar();
        loadingHandler.postDelayed(new Runnable() {
            @Override
            public void run() {
                if (puller.isPlay() == 1) {
                    loading.setVisibility(View.GONE);
                    loadingHandler.removeCallbacksAndMessages(null);
                    mediaPlayImage.setImageResource(R.mipmap.mediacontroller_pause);
                    if (!mediaPlayImage.isEnabled())
                        mediaPlayImage.setEnabled(true);
                } else {
                    if (!mediaPlayImage.isEnabled())
                        mediaPlayImage.setEnabled(true);
                    loadingHandler.postDelayed(this, 1000);
                }
            }
        }, 1000);
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
        loading.setText("stop...");
        loading.setVisibility(View.VISIBLE);
        loadingHandler.removeCallbacksAndMessages(null);
        if (!mediaPlayImage.isEnabled())
            mediaPlayImage.setEnabled(true);
        controlLayout.setVisibility(View.VISIBLE);
//        controlLayout.setVisibleBar();
    }

    private void rePlay() {
        loading();
        puller.play(constant.BASE_URL);

    }


    @Override
    protected void onStop() {
        super.onStop();
        if (puller.isPlay() == 1) {
            doPause();
        }
    }

    @Override
    public void onConfigurationChanged(Configuration newConfig) {
        super.onConfigurationChanged(newConfig);
        setWH(newConfig.orientation);
    }

    @Override
    protected void onDestroy() {
        handler.removeCallbacksAndMessages(null);
        loadingHandler.removeCallbacksAndMessages(null);
        super.onDestroy();
    }


    @Override
    public void onClick(View view) {
        switch (view.getId()) {
            case R.id.playImg:
                play();
                break;
        }
    }

}
