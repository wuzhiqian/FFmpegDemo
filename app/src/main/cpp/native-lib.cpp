#include <jni.h>
#include <string>
#include "include/puller/FFmpegAudio.h"
#include <android/native_window_jni.h>
#include "include/puller/FFmpegVideo.h"



extern "C" {


#include <libavformat/avformat.h>
#include <libavutil/time.h>

FFmpegAudio *audio;
FFmpegVideo *video;
pthread_t p_tid;
int isPlay = 0;
const char *path;
ANativeWindow *window = 0;

pthread_mutex_t mutex;

void call_video_play(AVFrame *frame) {
    if (!window)
        return;
    ANativeWindow_Buffer window_buffer;
    if (ANativeWindow_lock(window, &window_buffer, 0))
        return;
    uint8_t *dst = (uint8_t *) window_buffer.bits;
    int dstStride = window_buffer.stride * 4;
    uint8_t *src = frame->data[0];
    int srcStride = frame->linesize[0];
    for (int i = 0; i < window_buffer.height; i++) {
        memcpy(dst + i * dstStride, src + i * srcStride, srcStride);
    }
    ANativeWindow_unlockAndPost(window);
}
void *process(void *args) {
    av_register_all();
    avformat_network_init();
    AVFormatContext *formatContext = avformat_alloc_context();
    if (avformat_open_input(&formatContext, path, NULL, NULL) != 0) {
        LOGE("%s", "打开视频失败");
    }
    if (avformat_find_stream_info(formatContext, NULL) < 0) {
        LOGE("%s", "获取视频失败");
    }

    int i = 0;

    for (; i < formatContext->nb_streams; i++) {
        AVCodecContext *pCodeCtx = formatContext->streams[i]->codec;
        AVCodec *pCodec = avcodec_find_decoder(pCodeCtx->codec_id);
        AVCodecContext *codec = avcodec_alloc_context3(pCodec);
        avcodec_copy_context(codec, pCodeCtx);
        if (avcodec_open2(codec, pCodec, NULL) < 0) {
            LOGE("%s", "解码器打不开");
            continue;
        }


        if (formatContext->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO) {
            audio->setAvCodecContext(codec);
            audio->index = i;
            audio->time_base = formatContext->streams[i]->time_base;
        } else if (formatContext->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO) {
            video->setAvCodecContext(codec);
            video->index = i;
            video->time_base = formatContext->streams[i]->time_base;
            if (window)
                ANativeWindow_setBuffersGeometry(window, video->codecContext->width,
                                                 video->codecContext->height,
                                                 WINDOW_FORMAT_RGBA_8888);

        }
    }
    video->setAudio(audio);
    video->play();
    audio->play();
    isPlay = 1;
    AVPacket *packet = (AVPacket *) av_mallocz(sizeof(AVPacket));
    int ret;
    while (isPlay) {
        ret = av_read_frame(formatContext, packet);
        if (ret == 0) {
            if (audio && audio->isPlay && packet->stream_index == audio->index) {
                audio->put(packet);
            } else if (video && video->isPlay && packet->stream_index == video->index)
                video->put(packet);
            av_packet_unref(packet);
        } else if (ret == AVERROR_EOF) {
            while (isPlay) {
                if (audio->queue.empty() && video->queue.empty())
                    break;
                av_usleep(10000);
            }
        }
    }
    isPlay = 0;
    if (video && video->isPlay)
        video->stop();
    if (audio && audio->isPlay)
        audio->stop();

    av_free_packet(packet);
    avformat_free_context(formatContext);
    pthread_exit(0);

}


extern "C"
JNIEXPORT void JNICALL
Java_com_wzq_ffmpegdemo_puller_utils_Puller_release(JNIEnv *env, jobject instance) {

    if(!isPlay)
    {
        return;
    }
    pthread_mutex_lock(&mutex);
    if (isPlay) {
        isPlay = 0;
        pthread_join(p_tid, NULL);
    }
    if (video) {
        if (video->isPlay)
            video->stop();
        delete (video);
        video = 0;
    }

    if (audio) {
        if (audio->isPlay) {
            audio->stop();
        }
        delete (audio);
        audio = 0;
    }


    pthread_mutex_unlock(&mutex);
    pthread_mutex_destroy(&mutex);
//    pthread_exit(0);

}

extern "C"
JNIEXPORT void JNICALL
Java_com_wzq_ffmpegdemo_puller_utils_Puller_playNative(JNIEnv *env, jobject instance,
                                                       jstring path_) {
    if(isPlay)
        return;
    pthread_mutex_init(&mutex, NULL);
    pthread_mutex_lock(&mutex);
    path = env->GetStringUTFChars(path_, 0);
    audio = new FFmpegAudio();
    video = new FFmpegVideo();
    video->setPlayCall(call_video_play);
    pthread_create(&p_tid, NULL, process, NULL);
    pthread_mutex_unlock(&mutex);
}


extern "C"
JNIEXPORT void JNICALL
Java_com_wzq_ffmpegdemo_puller_utils_Puller_display(JNIEnv *env, jobject instance,
                                                    jobject surface) {
    if (window) {
        ANativeWindow_release(window);
        window = 0;
    }
    window = ANativeWindow_fromSurface(env, surface);
    if (video && video->codecContext)
        ANativeWindow_setBuffersGeometry(window, video->codecContext->width,
                                         video->codecContext->height, WINDOW_FORMAT_RGBA_8888);

}
JNIEXPORT jint JNICALL
Java_com_wzq_ffmpegdemo_puller_utils_Puller_isPlay(JNIEnv *env, jobject instance) {
    return isPlay;
}

JNIEXPORT jdouble JNICALL
Java_com_wzq_ffmpegdemo_puller_utils_Puller_getTime(JNIEnv *env, jobject instance) {
    return audio->clock;
}

}