package com.wzq.ffmpegdemo.puller.ui;


import android.Manifest;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.content.res.Configuration;
import android.net.Uri;
import android.os.Build;
import android.os.Handler;
import android.support.annotation.NonNull;
import android.support.v4.app.ActivityCompat;
import android.support.v4.content.ContextCompat;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.text.TextUtils;
import android.util.Log;
import android.view.MotionEvent;
import android.view.SurfaceView;
import android.view.View;
import android.view.WindowManager;
import android.widget.ImageView;
import android.widget.RelativeLayout;
import android.widget.SeekBar;
import android.widget.TextView;
import android.widget.Toast;

import com.wzq.ffmpegdemo.R;
import com.wzq.ffmpegdemo.constants.constant;
import com.wzq.ffmpegdemo.puller.utils.FileUtil;
import com.wzq.ffmpegdemo.puller.utils.Puller;
import com.wzq.ffmpegdemo.widget.ControlLayout;

public class MainActivity extends AppCompatActivity implements View.OnClickListener {

    private SurfaceView surfaceView;
    private Puller puller;
    private ImageView mediaPlayImage;
    private TextView startText;
    private TextView endText;

    private RelativeLayout content;

    private TextView loading;

    Handler handler = new Handler();
    Handler loadingHandler = new Handler();
    private ControlLayout controlLayout;
    private final int SELECT_FILE_FLAG = 1;
    private final int READ_STORAGE = 100;
    private String path;
    private FileUtil fileUtil;
    private boolean isPlay = false;
    private SeekBar seekBar;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        getWindow().setFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON, WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);


        surfaceView = (SurfaceView) findViewById(R.id.surfaceView);
        content = (RelativeLayout) findViewById(R.id.rContent);
        mediaPlayImage = (ImageView) findViewById(R.id.playImg);
        startText = (TextView) findViewById(R.id.startText);
        endText = (TextView) findViewById(R.id.endText);
        loading = (TextView) findViewById(R.id.loading);
        controlLayout = (ControlLayout) findViewById(R.id.controlBar);
        seekBar = (SeekBar) findViewById(R.id.seekBar);
        mediaPlayImage.setOnClickListener(this);
        fileUtil = new FileUtil();
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
                        if (puller.isPlay() == 1)
                            controlLayout.setVisibleBar();
                        break;
                    case MotionEvent.ACTION_CANCEL:
                        break;
                }
                return false;
            }
        });

        seekBar.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            @Override
            public void onProgressChanged(SeekBar seekBar, int i, boolean b) {
                Log.e("onProgressChanged", "" + i);
                if (b)
                    puller.seeking(puller.getTotleTime() / (seekBar.getMax() * 1.0) * i);
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {

            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {
            }
        });

        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {
            if (ContextCompat.checkSelfPermission(this, Manifest.permission.READ_EXTERNAL_STORAGE) != PackageManager.PERMISSION_GRANTED) {
                ActivityCompat.requestPermissions(this, new String[]{Manifest.permission.READ_EXTERNAL_STORAGE}, READ_STORAGE);
            }
        }
    }

    @Override
    public void onRequestPermissionsResult(int requestCode, @NonNull String[] permissions, @NonNull int[] grantResults) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults);
        if (requestCode == READ_STORAGE) {
            if (grantResults[0] != PackageManager.PERMISSION_GRANTED) {
                if (ContextCompat.checkSelfPermission(this, Manifest.permission.READ_EXTERNAL_STORAGE) != PackageManager.PERMISSION_GRANTED) {
                    ActivityCompat.requestPermissions(this, new String[]{Manifest.permission.READ_EXTERNAL_STORAGE}, READ_STORAGE);
                }
            }
        }
    }

    private void startRunTime() {

        handler.postDelayed(new Runnable() {
            @Override
            public void run() {
                if (puller.isPlay() == 1) {
                    startText.setText(String.format("%02d:%02d", ((long) (puller.getTime())) / 60, ((long) (puller.getTime())) % 60));
                    endText.setText(String.format("%02d:%02d", ((long) (puller.getTotleTime() - puller.getTime())) / 60, ((long) (puller.getTotleTime() - puller.getTime())) % 60));
                    seekBar.setProgress((int)(puller.getTime() * seekBar.getMax() / puller.getTotleTime()));
                }
                handler.postDelayed(this, 500);
            }
        }, 500);
    }

    public void selectFile(View view) {
        Intent intent = new Intent(Intent.ACTION_GET_CONTENT);
        intent.setType("*/*");
        intent.addCategory(Intent.CATEGORY_OPENABLE);
        startActivityForResult(intent, SELECT_FILE_FLAG);
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
                    loadingHandler.postDelayed(this, 500);
                }
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
        puller.pause();
        loading.setText("stop...");
        loading.setVisibility(View.VISIBLE);
        loadingHandler.removeCallbacksAndMessages(null);
        if (!mediaPlayImage.isEnabled())
            mediaPlayImage.setEnabled(true);
        controlLayout.setVisibility(View.VISIBLE);
    }

    private void rePlay() {
        if (TextUtils.isEmpty(path)) {
            Toast.makeText(this, "请选择文件~", Toast.LENGTH_SHORT).show();
            return;
        }
        loading();
        puller.play(path);

    }


    @Override
    public void onWindowFocusChanged(boolean hasFocus) {
        super.onWindowFocusChanged(hasFocus);
        if (hasFocus) {
            if (isPlay) {
                if (puller.isPlay() != 1)
                    rePlay();
            }
        } else {
            if (puller.isPlay() == 1) {
                doPause();
                isPlay = true;
            } else
                isPlay = false;
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
        puller.release();
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

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        super.onActivityResult(requestCode, resultCode, data);
        if (requestCode == SELECT_FILE_FLAG)
            if (resultCode == RESULT_OK) {
                Uri uri = data.getData();
                if ("file".equalsIgnoreCase(uri.getScheme())) {
                    path = uri.getPath();
                } else {
                    if (Build.VERSION.SDK_INT > Build.VERSION_CODES.KITKAT) {
                        path = fileUtil.getPath(this, uri);
                    } else {
                        path = fileUtil.getRealPathFromURI(uri, this);
                    }
                }
            }
    }
}
