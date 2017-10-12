package com.wzq.ffmpegdemo.puller.ui;

import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.view.SurfaceView;
import android.view.View;
import android.widget.ImageView;

import com.wzq.ffmpegdemo.R;
import com.wzq.ffmpegdemo.constants.constant;
import com.wzq.ffmpegdemo.puller.utils.Puller;

public class MainActivity extends AppCompatActivity implements View.OnClickListener {

    private SurfaceView surfaceView;
    private Puller puller;
    private ImageView mediaPlayImage;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        surfaceView = (SurfaceView) findViewById(R.id.surfaceView);
        mediaPlayImage = (ImageView) findViewById(R.id.playImg);
        mediaPlayImage.setOnClickListener(this);


        puller = new Puller();
        puller.setSurfaceView(surfaceView);
    }

    public void play() {
        if (puller.isPlay() == 1) {
            puller.release();
            mediaPlayImage.setImageResource(R.mipmap.mediacontroller_play);
        } else {
            puller.play(constant.BASE_URL);
            mediaPlayImage.setImageResource(R.mipmap.mediacontroller_pause);
        }

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
