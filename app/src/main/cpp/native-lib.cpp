#include <jni.h>
#include <string>
extern "C"
JNIEXPORT jstring JNICALL
Java_com_wzq_ffmpegdemo_ui_MainActivity_stringFromJNI(JNIEnv *env, jobject instance) {

    // TODO
    std::string hello = "Hello from C++";
    return env->NewStringUTF(hello.c_str());
}

