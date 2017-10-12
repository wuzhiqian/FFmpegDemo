//
// Created by windows on 2017/10/12.
//

#include "../include/puller/FFmpegAudio.h"

FFmpegAudio::FFmpegAudio() {
    clock = 0;
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&cond, NULL);
}


int FFmpegAudio::get(AVPacket *packet) {
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


int FFmpegAudio::put(AVPacket *packet) {
    AVPacket *pkt = (AVPacket *) av_mallocz(sizeof(AVPacket));
    if (av_packet_ref(pkt, packet))
        return 0;
    pthread_mutex_lock(&mutex);
    queue.push(pkt);
    pthread_cond_signal(&cond);
    pthread_mutex_unlock(&mutex);
    return 1;
}


void FFmpegAudio::setAvCodecContext(AVCodecContext *context) {
    codecContext = context;
    createFFmpeg(this);
}


int FFmpegAudio::createFFmpeg(FFmpegAudio *audio) {
    audio->swrContext = swr_alloc();
    audio->out_buffer = (uint8_t *) av_mallocz(44100 * 2);
    uint64_t out_ch = AV_CH_LAYOUT_STEREO;
    enum AVSampleFormat outFormat = AV_SAMPLE_FMT_S16;
    int outSampleRate = audio->codecContext->sample_rate;
    swr_alloc_set_opts(audio->swrContext, out_ch, outFormat, outSampleRate,
                       audio->codecContext->channel_layout, audio->codecContext->sample_fmt,
                       audio->codecContext->sample_rate, 0, NULL);
    swr_init(audio->swrContext);
    audio->out_channel_nb = av_get_channel_layout_nb_channels(AV_CH_LAYOUT_STEREO);
    return 0;
}


void *playAudio(void *args) {
    FFmpegAudio *audio = (FFmpegAudio *) args;
    audio->createPlayer();
    pthread_exit(0);
}

int getPcm(FFmpegAudio *audio) {
    int getFrame;
    int size;
    AVPacket *packet = (AVPacket *) av_mallocz(sizeof(AVPacket));
    AVFrame *frame = av_frame_alloc();
    while (audio->isPlay) {
        size = 0;
        audio->get(packet);
        if (packet->pts != AV_NOPTS_VALUE)
            audio->clock = av_q2d(audio->time_base) * packet->pts;
        avcodec_decode_audio4(audio->codecContext, frame, &getFrame, packet);
        if (getFrame) {
            swr_convert(audio->swrContext, &audio->out_buffer, 44100 * 2,
                        (const uint8_t **) frame->data, frame->nb_samples);
            size = av_samples_get_buffer_size(NULL, audio->out_channel_nb, frame->nb_samples,
                                              AV_SAMPLE_FMT_S16, 1);
            break;
        }
    }
    av_free(packet);
    av_frame_free(&frame);
    return size;
}

void bqPlayCallback(SLAndroidSimpleBufferQueueItf pItf_, void *args) {
    FFmpegAudio *audio = (FFmpegAudio *) args;
    int dataLength = getPcm(audio);
    if (dataLength > 0) {
        double time = dataLength / (44100 * 2 * 2.0);
        audio->clock = audio->clock + time;
        (*pItf_)->Enqueue(pItf_, audio->out_buffer, dataLength);
    }
}


int FFmpegAudio::createPlayer() {
    if (slCreateEngine(&engineObject, 0, NULL, 0, NULL, NULL) != SL_RESULT_SUCCESS)
        return 0;
    if ((*engineObject)->Realize(engineObject, SL_BOOLEAN_FALSE) != SL_RESULT_SUCCESS)
        return 0;
    if ((*engineObject)->GetInterface(engineObject, SL_IID_ENGINE, &engineItf) != SL_RESULT_SUCCESS)
        return 0;
    if ((*engineItf)->CreateOutputMix(engineItf, &outputMixObject, 0, 0, 0) != SL_RESULT_SUCCESS)
        return 0;
    if ((*outputMixObject)->Realize(outputMixObject, SL_BOOLEAN_FALSE) != SL_RESULT_SUCCESS)
        return 0;

    const SLEnvironmentalReverbSettings settings = SL_I3DL2_ENVIRONMENT_PRESET_DEFAULT;
    if ((*outputMixObject)->GetInterface(outputMixObject, SL_IID_ENVIRONMENTALREVERB,
                                         &slEnvironmentalReverbItf) == SL_RESULT_SUCCESS)
        (*slEnvironmentalReverbItf)->SetEnvironmentalReverbProperties(slEnvironmentalReverbItf,
                                                                      &settings);
    SLDataLocator_AndroidSimpleBufferQueue audioQueue = {SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE,
                                                         2};
    SLDataFormat_PCM pcm = {SL_DATAFORMAT_PCM, 2, SL_SAMPLINGRATE_44_1, SL_PCMSAMPLEFORMAT_FIXED_16,
                            SL_PCMSAMPLEFORMAT_FIXED_16,
                            SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT,
                            SL_BYTEORDER_LITTLEENDIAN};
    SLDataSource slDataSource = {&audioQueue, &pcm};
    SLDataLocator_OutputMix outputMix = {SL_DATALOCATOR_OUTPUTMIX, outputMixObject};
    SLDataSink dataSink = {&outputMix, NULL};
    const SLInterfaceID ids[3] = {SL_IID_BUFFERQUEUE, SL_IID_EFFECTSEND, SL_IID_VOLUME};
    const SLboolean req[3] = {SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE};
    (*engineItf)->CreateAudioPlayer(engineItf, &bqPlayerObject, &slDataSource, &dataSink, 2, ids,
                                    req);
    (*bqPlayerObject)->Realize(bqPlayerObject, SL_BOOLEAN_FALSE);
    (*bqPlayerObject)->GetInterface(bqPlayerObject, SL_IID_PLAY, &playItf);
    (*bqPlayerObject)->GetInterface(bqPlayerObject, SL_IID_BUFFERQUEUE, &bufferQueueItf);
    (*bufferQueueItf)->RegisterCallback(bufferQueueItf, bqPlayCallback, this);

    (*bqPlayerObject)->GetInterface(bqPlayerObject, SL_IID_VOLUME, &volumeItf);
    (*playItf)->SetPlayState(playItf, SL_PLAYSTATE_PLAYING);
    bqPlayCallback(bufferQueueItf, this);
    return 1;
}


void FFmpegAudio::play() {
    isPlay = 1;
    pthread_create(&p_playId, NULL, playAudio, this);
}

void FFmpegAudio::stop() {
    pthread_mutex_lock(&mutex);
    isPlay = 0;
    pthread_cond_signal(&cond);
    pthread_mutex_unlock(&mutex);
    pthread_join(p_playId, 0);
    if (playItf) {
        (*playItf)->SetPlayState(playItf, SL_PLAYSTATE_STOPPED);
        playItf = 0;
    }

    if (bqPlayerObject) {
        (*bqPlayerObject)->Destroy(bqPlayerObject);
        bqPlayerObject = 0;
        bufferQueueItf = 0;
        volumeItf = 0;
    }

    if (outputMixObject) {
        (*outputMixObject)->Destroy(outputMixObject);
        outputMixObject = 0;
    }
    if (engineObject) {
        (*engineObject)->Destroy(engineObject);
        engineObject = 0;
        engineItf = 0;
    }

    if (swrContext)
        swr_free(&swrContext);

    if (this->codecContext) {
        if (avcodec_is_open(this->codecContext))
            avcodec_close(this->codecContext);
        avcodec_free_context(&this->codecContext);
        this->codecContext = 0;
    }

}


FFmpegAudio::~FFmpegAudio() {
    if (out_buffer)
        free(out_buffer);

    for (int i = 0; i < queue.size(); i++) {
        AVPacket *packet = queue.front();
        queue.pop();
        av_free(packet);
    }

    pthread_cond_destroy(&cond);
    pthread_mutex_destroy(&mutex);
}
