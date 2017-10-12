#include <jni.h>
#include <string>
#include "include/puller/FFmpegAudio.h"
#include <android/native_window_jni.h>

extern "C" {
FFmpegAudio *audio;

pthread_t p_tid;

int isPlay = 0;
const char *path;

#include <libavformat/avformat.h>
#include <libavutil/time.h>


void *process(void *args) {
    av_register_all();
    avformat_network_init();
    AVFormatContext *formatContext = avformat_alloc_context();
    LOGE("%s", path);
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
        }
    }
    audio->play();
    isPlay = 1;
    AVPacket *packet = (AVPacket *) av_mallocz(sizeof(AVPacket));
    int ret;
    while (isPlay) {
        ret = av_read_frame(formatContext, packet);
        if (ret == 0) {
            if (audio && audio->isPlay && packet->stream_index == audio->index) {
                audio->put(packet);
            }
            av_packet_unref(packet);
        } else if (ret == AVERROR_EOF) {
            while (isPlay) {
                if (audio->queue.empty())
                    break;
                av_usleep(10000);
            }
        }
    }
    isPlay = 0;
    if (audio && audio->isPlay)
        audio->stop();

    av_free_packet(packet);
    avformat_free_context(formatContext);
    pthread_exit(0);

}


extern "C"
JNIEXPORT void JNICALL
Java_com_wzq_ffmpegdemo_puller_utils_Puller_release(JNIEnv *env, jobject instance) {
    if (isPlay) {
        isPlay = 0;
        pthread_join(p_tid, 0);
    }

    if (audio) {
        if (audio->isPlay) {
            audio->stop();
        }
        delete (audio);
        audio = 0;
    }

}

extern "C"
JNIEXPORT void JNICALL
Java_com_wzq_ffmpegdemo_puller_utils_Puller_playNative(JNIEnv *env, jobject instance,
                                                       jstring path_) {
    path = env->GetStringUTFChars(path_, 0);
    audio = new FFmpegAudio;
    pthread_create(&p_tid, NULL, process, NULL);
    //  env->ReleaseStringUTFChars(path_, path);
}


extern "C"
JNIEXPORT void JNICALL
Java_com_wzq_ffmpegdemo_puller_utils_Puller_display(JNIEnv *env, jobject instance,
                                                    jobject surface) {

}


}