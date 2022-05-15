#include <jni.h>
#include <string>
#include <android/log.h>

extern "C" {

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <android/asset_manager_jni.h>
#include <android/asset_manager.h>

AAssetManager *assetManager;

JNIEXPORT jstring JNICALL
Java_com_alittlelost_ffmpegaudioloading_MainActivity_stringFromJNI(
        JNIEnv *env,
        jobject /* this */) {

    std::string hello = avcodec_configuration();
    return env->NewStringUTF(hello.c_str());
}

JNIEXPORT jboolean JNICALL
Java_com_alittlelost_ffmpegaudioloading_MainActivity_setAssetManager(JNIEnv *env,
                                                                     jobject instance,
                                                                     jobject jAssetManager) {
    assetManager = AAssetManager_fromJava(env, jAssetManager);

    if (assetManager == nullptr) {
        __android_log_print(ANDROID_LOG_ERROR, "Main", "Failed to set assetManager");
        return false;
    }

    return true;
}

JNIEXPORT void JNICALL
Java_com_alittlelost_ffmpegaudioloading_MainActivity_loadAsset(JNIEnv *env,
                                                               jobject instance,
                                                               jstring assetName) {
    const char* nativeFileName = env->GetStringUTFChars(assetName, nullptr);
    AAsset *asset = AAssetManager_open(assetManager, nativeFileName, AASSET_MODE_UNKNOWN);

    if(!asset) {
        __android_log_print(ANDROID_LOG_ERROR, "Main", "Failed to open asset %s", nativeFileName);
    } else {
        __android_log_print(ANDROID_LOG_INFO, "Main", "Successfully opened asset %s", nativeFileName);
        AAsset_close(asset);
    }
}

}

