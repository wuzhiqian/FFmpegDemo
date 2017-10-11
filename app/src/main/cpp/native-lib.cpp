#include <jni.h>
#include <string>



JNIEXPORT void JNICALL
Java_com_wzq_ffmpegdemo_puller_utils_Puller_release(JNIEnv *env, jobject instance) {

    // TODO

}

JNIEXPORT void JNICALL
Java_com_wzq_ffmpegdemo_puller_utils_Puller_playNative(JNIEnv *env, jobject instance,
                                                       jstring path_) {
    const char *path = env->GetStringUTFChars(path_, 0);

    // TODO

    env->ReleaseStringUTFChars(path_, path);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_wzq_ffmpegdemo_puller_utils_Puller_display(JNIEnv *env, jobject instance,
                                                    jobject surface) {

    // TODO

}


