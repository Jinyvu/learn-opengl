#ifndef VIDEO_FRAME_LOADER
#define VIDEO_FRAME_LOADER

#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#include <loader/stb_image.h>
#endif

extern "C"
{
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
}

#include <GLFW/glfw3.h>
#include <stdexcept>

class VideoFrameLoader
{
public:
    VideoFrameLoader(const char *videoFileName);
    ~VideoFrameLoader();
    bool extractFrame(bool loop = false);
    void displayFrame(GLuint textureID, unsigned int i);

private:
    char *filename;
    AVFormatContext *formatContext;
    AVCodecContext *codecContext;
    AVFrame *frame;
    SwsContext *sws_ctx;
    int videoStreamIndex;

    AVFormatContext *openVideoFile(const char *filename);
    AVCodecContext *initializeCodecContext(AVFormatContext *formatContext);
    bool readFrame();
    void reset();
};

VideoFrameLoader::VideoFrameLoader(const char *videoFileName)
{
    filename = new char[strlen(videoFileName) + 1];
    strcpy(filename, videoFileName);
    videoStreamIndex = -1;

    formatContext = openVideoFile(filename);
    if (!formatContext)
    {
        fprintf(stderr, "Could not open video file.\n");
        throw std::runtime_error("Could not open video file.");
    }

    codecContext = initializeCodecContext(formatContext);
    if (!codecContext)
    {
        fprintf(stderr, "Could not initialize codec context.\n");
        throw std::runtime_error("Could not initialize codec context.");
    }

    frame = av_frame_alloc();

    sws_ctx = sws_getContext(codecContext->width, codecContext->height, codecContext->pix_fmt,
                             codecContext->width, codecContext->height, AV_PIX_FMT_RGB24,
                             SWS_BILINEAR, nullptr, nullptr, nullptr);
}

VideoFrameLoader::~VideoFrameLoader()
{
    delete[] filename;
    av_frame_free(&frame);
    avcodec_free_context(&codecContext);
    avformat_close_input(&formatContext);
    avformat_free_context(formatContext);
    sws_freeContext(sws_ctx);
}

AVFormatContext *VideoFrameLoader::openVideoFile(const char *filename)
{
    AVFormatContext *formatContext = avformat_alloc_context();
    if (avformat_open_input(&formatContext, filename, nullptr, nullptr) != 0)
    {
        return nullptr;
    }
    if (avformat_find_stream_info(formatContext, nullptr) < 0)
    {
        return nullptr;
    }
    return formatContext;
}

AVCodecContext *VideoFrameLoader::initializeCodecContext(AVFormatContext *formatContext)
{
    const AVCodec *codec = nullptr;
    AVCodecParameters *codecParameters = nullptr;
    videoStreamIndex = -1;

    // 找到视频流索引
    for (unsigned int i = 0; i < formatContext->nb_streams; i++)
    {
        AVCodecParameters *pLocalCodecParameters = formatContext->streams[i]->codecpar;
        if (pLocalCodecParameters->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            codec = avcodec_find_decoder(pLocalCodecParameters->codec_id);
            codecParameters = pLocalCodecParameters;
            videoStreamIndex = i;
            break;
        }
    }

    if (!codec || videoStreamIndex == -1)
    {
        return nullptr;
    }

    AVCodecContext *codecContext = avcodec_alloc_context3(codec);
    if (avcodec_parameters_to_context(codecContext, codecParameters) < 0)
    {
        return nullptr;
    }
    if (avcodec_open2(codecContext, codec, nullptr) < 0)
    {
        return nullptr;
    }
    return codecContext;
}

bool VideoFrameLoader::extractFrame(bool loop)
{
    if (!loop)
    {
        return readFrame();
    }
    else
    {
        const int readFrameResult = readFrame();
        if (readFrameResult)
            return true;

        reset();
        return readFrame();
    }
}

void VideoFrameLoader::displayFrame(GLuint textureID, unsigned int i)
{
    int width = frame->width;
    int height = frame->height;

    uint8_t *data[4];
    int linesize[4];
    av_image_alloc(data, linesize, width, height, AV_PIX_FMT_RGB24, 1);

    sws_scale(sws_ctx, frame->data, frame->linesize, 0, height, data, linesize);

    glActiveTexture(GL_TEXTURE0 + i);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data[0]);
    glGenerateMipmap(GL_TEXTURE_2D);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    av_freep(&data[0]);
}

bool VideoFrameLoader::readFrame()
{
    AVPacket *packet = av_packet_alloc();

    while (av_read_frame(formatContext, packet) >= 0)
    {
        if (packet->stream_index == videoStreamIndex)
        {
            if (avcodec_send_packet(codecContext, packet) >= 0)
            {
                if (avcodec_receive_frame(codecContext, frame) >= 0)
                {
                    av_packet_unref(packet);
                    av_packet_free(&packet);
                    return true;
                }
            }
        }
        av_packet_unref(packet);
    }

    av_packet_free(&packet);
    return false;
}

void VideoFrameLoader::reset()
{
    if (av_seek_frame(formatContext, videoStreamIndex, 0, AVSEEK_FLAG_BACKWARD) < 0)
    {
        throw std::runtime_error("Error seeking to the beginning of the video.");
    }
    avcodec_flush_buffers(codecContext);
}

#endif