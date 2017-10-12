//
// Created by wzq on 2017/10/12.
//

#ifndef FFMPEGDEMO_FFMPEGAUDIO_H
#define FFMPEGDEMO_FFMPEGAUDIO_H


#include<SLES/OpenSLES_Android.h>
#include <queue>

extern "C" {
#include "../Log.h"
#include <libavcodec/avcodec.h>
#include <pthread.h>
#include <libswresample/swresample.h>

class FFmpegAudio {
public:
    FFmpegAudio();

    ~FFmpegAudio();

    void setAvCodecContext(AVCodecContext *codecContext);

    int get(AVPacket *packet);

    int put(AVPacket *packet);

    void play();

    void stop();

    int createPlayer();

    int createFFmpeg(FFmpegAudio *audio);

public:
    int isPlay;
    int index;
    std::queue<AVPacket *> queue;
    pthread_t p_playId;
    AVCodecContext *codecContext;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    SwrContext *swrContext;
    uint8_t *out_buffer;
    int out_channel_nb;

    double clock;

    AVRational time_base;


    SLObjectItf engineObject;
    SLEngineItf engineItf;
    SLEnvironmentalReverbItf slEnvironmentalReverbItf;
    SLObjectItf outputMixObject;
    SLObjectItf bqPlayerObject;
    SLEffectSendItf playerEffectSend;
    SLVolumeItf volumeItf;
    SLPlayItf playItf;
    SLAndroidSimpleBufferQueueItf bufferQueueItf;
};


};


#endif //FFMPEGDEMO_FFMPEGAUDIO_H
