#include <jni.h>
#include <string>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <unistd.h>

extern "C" {

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <android/asset_manager_jni.h>
#include <android/asset_manager.h>
#include <android/log.h>


constexpr char *FILES_DIR = "FILES_DIR_PATH";

std::string readFile(std::string filePath) {
    std::ifstream ifs(filePath);
    std::stringstream ss;

    ss << ifs.rdbuf();

    return ss.str();
}

JNIEXPORT jstring JNICALL
Java_com_alittlelost_ffmpegaudioloading_MainActivity_stringFromJNI(
        JNIEnv *env,
        jobject /* this */) {

    std::string filesPath = getenv(FILES_DIR);

    __android_log_print(ANDROID_LOG_INFO, "Main", "Loaded data: %s",
                        readFile(filesPath + "/" + "metro.wav").c_str());

    std::string hello = avcodec_configuration();
    return env->NewStringUTF(hello.c_str());
}

}
