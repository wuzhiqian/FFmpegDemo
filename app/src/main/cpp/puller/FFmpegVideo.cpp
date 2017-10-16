//
// Created by windows on 2017/10/12.
//

#include "../include/puller/FFmpegVideo.h"

FFmpegVideo::FFmpegVideo() {
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&cond, NULL);
}


int FFmpegVideo::get(AVPacket *packet) {
    pthread_mutex_lock(&mutex);
    while (isPlay) {
        if (!queue.empty()) {
            if (av_packet_ref(packet, queue.front()))
                break;
            AVPacket *pkt = queue.front();
            queue.pop();
            av_free(pkt);
            break;
        } else
            pthread_cond_wait(&cond, &mutex);
    }
    pthread_mutex_unlock(&mutex);
    return 0;
}




int FFmpegVideo::put(AVPacket *packet) {
    AVPacket *pkt = (AVPacket *) av_mallocz(sizeof(AVPacket));
    if (av_copy_packet(pkt, packet))
        return 0;
    pthread_mutex_lock(&mutex);
    queue.push(pkt);
    pthread_cond_signal(&cond);
    pthread_mutex_unlock(&mutex);
    return 1;

}

void FFmpegVideo::flush() {
    pthread_mutex_lock(&mutex);
    while (1){
        if(queue.size() <=0)
            break;
        AVPacket *pkt = queue.front();
        queue.pop();
        av_free(pkt);
    }
    pthread_mutex_unlock(&mutex);
}

void FFmpegVideo::setPause(int data) {
    pause = data;
    if(!pause){
        pthread_mutex_lock(&mutex);
        pthread_cond_signal(&cond);
        pthread_mutex_unlock(&mutex);
    }
}
static void (*video_call)(AVFrame *frame);


void *playVideo(void *args) {
    FFmpegVideo *video = (FFmpegVideo *) args;
    AVFrame *frame = av_frame_alloc();
    SwsContext *swsCxt = sws_getContext(video->codecContext->width, video->codecContext->height,
                                        video->codecContext->pix_fmt, video->codecContext->width,
                                        video->codecContext->height, AV_PIX_FMT_RGBA, SWS_BILINEAR,
                                        0, 0, 0);
    AVFrame *rgbFrame = av_frame_alloc();
    uint8_t *outBuff = (uint8_t *) av_mallocz(
            avpicture_get_size(AV_PIX_FMT_RGBA, video->codecContext->width,
                               video->codecContext->height));
    int re = avpicture_fill((AVPicture *) rgbFrame, outBuff, AV_PIX_FMT_RGBA,
                            video->codecContext->width, video->codecContext->height);
    SwsContext *swsContext = sws_getContext(video->codecContext->width, video->codecContext->height,
                                            video->codecContext->pix_fmt,
                                            video->codecContext->width, video->codecContext->height,
                                            AV_PIX_FMT_RGBA, SWS_BICUBIC, NULL, NULL, NULL);
    int length;
    int getFrame;
    AVPacket *packet = (AVPacket *) av_mallocz(sizeof(AVPacket));
    double lastPlay, play, lastDelay, delay, audioClock, diff, syncThreshold, startTime, pts, actualDelay;
    startTime = av_gettime() / 1000000.0;
    while (video->isPlay) {
        if (video->pause) {
            pthread_mutex_lock(&video->mutex);
            pthread_cond_wait(&video->cond, &video->mutex);
            pthread_mutex_unlock(&video->mutex);
        }
        video->get(packet);
        length = avcodec_decode_video2(video->codecContext, frame, &getFrame, packet);
        if (!getFrame)
            continue;
        int code = sws_scale(swsCxt, (const uint8_t *const *) frame->data, frame->linesize, 0,
                             video->codecContext->height, rgbFrame->data, rgbFrame->linesize);
        if ((pts = av_frame_get_best_effort_timestamp(frame)) == AV_NOPTS_VALUE)
            pts = 0;
        play = pts * av_q2d(video->time_base);
        play = video->synchronize(frame, play);
        delay = play - lastPlay;
        if (delay <= 0 || delay > 1)
            delay = lastDelay;
        audioClock = video->audio->clock;
        lastDelay = delay;
        lastPlay = play;

        diff = video->clock - audioClock;
        syncThreshold = (delay > 0.01 ? 0.01 : delay);
        if (fabs(diff) < 10) {
            if (diff <= -syncThreshold)
                delay = 0;
            else
                delay = 2 * delay;
        }
        startTime += delay;
        actualDelay = startTime - av_gettime() / 1000000.0;
        if (actualDelay < 0.01)
            actualDelay = 0.01;
        av_usleep(actualDelay * 1000000.0 + 6000);
        video_call(rgbFrame);
    }
    av_free(packet);
    av_frame_free(&frame);
    av_frame_free(&rgbFrame);
    sws_freeContext(swsCxt);

    size_t size = video->queue.size();
    for (int i = 0; i < size; i++) {
        AVPacket *pkt = video->queue.front();
        av_free(pkt);
        video->queue.pop();
    }
    pthread_exit(0);
}

void FFmpegVideo::play() {
    isPlay = 1;
    pthread_create(&p_playId, 0, playVideo, this);
}


void FFmpegVideo::stop() {
    pthread_mutex_lock(&mutex);
    isPlay = 0;
    pthread_cond_signal(&cond);
    pthread_mutex_unlock(&mutex);
    pthread_join(p_playId, 0);
    if (this->codecContext) {
        if (avcodec_is_open(this->codecContext))
            avcodec_close(this->codecContext);
        avcodec_free_context(&this->codecContext);
        this->codecContext = 0;
    }
}


void FFmpegVideo::setAvCodecContext(AVCodecContext *codec) {
    codecContext = codec;
}

void FFmpegVideo::setPlayCall(void (*call)(AVFrame *)) {
    video_call = call;
}

double FFmpegVideo::synchronize(AVFrame *frame, double play) {
    if (play != 0)
        clock = play;
    else
        play = clock;
    double repeatPict = frame->repeat_pict;
    double frameDelay = av_q2d(codecContext->time_base);
    double fps = 1 / frameDelay;
    double extraDelay = repeatPict / (2 * fps);
    double delay = extraDelay + frameDelay;
    clock += delay;
    return play;
}

void FFmpegVideo::setAudio(FFmpegAudio *audio) {
    this->audio = audio;
}


FFmpegVideo::~FFmpegVideo() {
    for (int i = 0; i < queue.size(); i++) {
        AVPacket *packet = queue.front();
        queue.pop();
        av_free(packet);
    }
    pthread_cond_destroy(&cond);
    pthread_mutex_destroy(&mutex);
}
