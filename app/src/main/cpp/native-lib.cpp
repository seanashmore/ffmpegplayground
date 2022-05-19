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
#include <libavutil/audio_fifo.h>


constexpr char *FILES_DIR = "FILES_DIR_PATH";

std::string readFile(std::string filePath) {
    std::ifstream ifs(filePath);
    std::stringstream ss;

    ss << ifs.rdbuf();

    return ss.str();
}

int decode_packet(AVPacket *pPacket, AVCodecContext *pCodecContext, AVFrame *pFrame, AVStream *pStream) {
    int frameCount = 0;

    int response = avcodec_send_packet(pCodecContext, pPacket);

    if (response < 0) {
        __android_log_print(ANDROID_LOG_ERROR, "Main", "Error while sending packet to decoder: %s",
                            av_err2str(response));
        return response;
    }

    AVAudioFifo* fifo = av_audio_fifo_alloc(AV_SAMPLE_FMT_FLT, pStream->codecpar->channels, 512);

    while (response >= 0) {
        response = avcodec_receive_frame(pCodecContext, pFrame);

        if(response == AVERROR(EAGAIN)) {
            response = avcodec_send_packet(pCodecContext, pPacket);
        } else if (response == AVERROR_EOF) {
            break;
        } else if (response < 0) {
            __android_log_print(ANDROID_LOG_ERROR, "Main",
                                "Error while receiving frame from decoder: %s",
                                av_err2str(response));
        }

        if (response >= 0) {
            frameCount++;
            __android_log_print(ANDROID_LOG_INFO, "Main",
                                "Frame %d (sample rate=%d size=%d bytes, format=%d) pts %lld key_frame %d",
                                pCodecContext->frame_number,
                                pFrame->sample_rate,
                                pFrame->pkt_size,
                                pFrame->format,
                                pFrame->pts,
                                pFrame->key_frame
            );

            av_audio_fifo_write(fifo, (void**)pFrame, pFrame->nb_samples);
            av_frame_unref(pFrame);
        }
    }


}

JNIEXPORT jstring JNICALL
Java_com_alittlelost_ffmpegaudioloading_MainActivity_stringFromJNI(
        JNIEnv *env,
        jobject /* this */) {

    std::string filesPath = getenv(FILES_DIR);
    std::string metroFilePath = filesPath + "/" + "metro.wav";

    __android_log_print(ANDROID_LOG_INFO, "Main", "Loaded data: %s",
                        readFile(filesPath + "/" + "metro.wav").c_str());


    const char *metroPath = metroFilePath.c_str();
    //AVFormat holds the header info from the format
    AVFormatContext *pFormatContext = avformat_alloc_context();
    if (!pFormatContext) {
        __android_log_print(ANDROID_LOG_ERROR, "Main",
                            "Failed to allocate memory for format context");
    }

    __android_log_print(ANDROID_LOG_INFO, "Main",
                        "metro.wav path: %s", metroPath);

    //Open the file & read the header
    int result = avformat_open_input(&pFormatContext, metroPath, nullptr, nullptr);
    if (result != 0) {
        __android_log_print(ANDROID_LOG_ERROR, "Main",
                            "Failed to open metro.wav. Error: %s", av_err2str(result));
    }

    __android_log_print(ANDROID_LOG_INFO,
                        "Main",
                        "format %s, duration %lld us, bit_rate %lld",
                        pFormatContext->iformat->name,
                        pFormatContext->duration,
                        pFormatContext->bit_rate);

    //Read packets from the format to get stream info
    //This populates pFormatContext->streams
    //of equal size to pFormatContext->nb_streams
    result = avformat_find_stream_info(pFormatContext, nullptr);
    if (result < 0) {
        __android_log_print(ANDROID_LOG_ERROR, "Main",
                            "Failed to get stream info. Error: %s", av_err2str(result));
    }


    AVCodec *pCodec = nullptr;
    AVCodecParameters *pCodecParameters = nullptr;

    int audio_stream_index = -1;
    AVStream *pStream = nullptr;

    for (int i = 0; i < pFormatContext->nb_streams; i++) {
        AVCodecParameters *pLocalCodecParameters = nullptr;
        pLocalCodecParameters = pFormatContext->streams[i]->codecpar;
        pStream = pFormatContext->streams[i];

        AVCodec *pLocalCodec = nullptr;

        pLocalCodec = avcodec_find_decoder(pLocalCodecParameters->codec_id);

        if (pLocalCodec == nullptr) {
            __android_log_print(ANDROID_LOG_ERROR, "Main", "ERROR unsupported codec");
            continue;
        }

        if (pLocalCodecParameters->codec_type == AVMEDIA_TYPE_AUDIO) {
            if (audio_stream_index == -1) {
                audio_stream_index = i;
                pCodec = pLocalCodec;
                pCodecParameters = pLocalCodecParameters;
            }

            __android_log_print(ANDROID_LOG_INFO,
                                "Main",
                                "Audio codec %d channels, sample rate: %d",
                                pLocalCodecParameters->channels,
                                pLocalCodecParameters->sample_rate);
        }
    } //for-loop

    if (audio_stream_index == -1) {
        __android_log_print(ANDROID_LOG_ERROR, "Main", "File does not contain an audio stream!");
    }


    //AVCodecContext
    AVCodecContext *pCodecContext = avcodec_alloc_context3(pCodec);
    if (!pCodecContext) {
        __android_log_print(ANDROID_LOG_ERROR, "Main",
                            "Failed to allocate memory for AVCodecContext");
    }

    //Fill context based on values from codec params
    if (avcodec_parameters_to_context(pCodecContext, pCodecParameters) < 0) {
        __android_log_print(ANDROID_LOG_ERROR, "Main",
                            "Failed to copy codec params to codec context");
    }

    if (avcodec_open2(pCodecContext, pCodec, nullptr) < 0) {
        __android_log_print(ANDROID_LOG_ERROR, "Main", "Failed to open codec with avcodec_open2");
    }

    AVFrame *pFrame = av_frame_alloc();
    if (!pFrame) {
        __android_log_print(ANDROID_LOG_ERROR, "Main", "Failed to allocate memory for AVFrame");
    }

    AVPacket *pPacket = av_packet_alloc();
    if (!pPacket) {
        __android_log_print(ANDROID_LOG_ERROR, "Main", "Failed to allocate memory for AVPacket");
    }

    int response = 0;
    int packets_to_process = 0;

    //Fill the packet with data from the stream
    while (av_read_frame(pFormatContext, pPacket) >= 0) {
        //if its our audio stream
        if (pPacket->stream_index == audio_stream_index) {
            //TODO: Decode packet
            decode_packet(pPacket, pCodecContext, pFrame, pStream);
        }
    }


    std::string hello = avcodec_configuration();
    return env->NewStringUTF(hello.c_str());
}

}
