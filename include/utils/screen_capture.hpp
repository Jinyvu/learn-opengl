#ifndef SCREEN_CAPTURE_H
#define SCREEN_CAPTURE_H

extern "C"
{
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
}

#include <stdexcept>

enum EScreenCaptureFormat {
    EScreenCaptureFormat_MP4,
    EScreenCaptureFormat_STREAM,
};

class ScreenCapture
{
public:
    ScreenCapture(unsigned int width, unsigned int height);
    ~ScreenCapture();
    void openOutputContext(const char *filename);
    void encodeFrame(uint8_t *data);
    void release();

private:
    AVFormatContext *fmt_ctx;
    AVCodecContext *enc_ctx;
    SwsContext *img_convert_ctx;
    AVFrame *yuv_frame;
    AVPacket *pkt;
    AVStream *out_stream;
    unsigned int width;
    unsigned int height;
    int64_t frame_counter;
};

ScreenCapture::ScreenCapture(unsigned int width, unsigned int height)
    : fmt_ctx(nullptr), enc_ctx(nullptr), img_convert_ctx(nullptr),
      yuv_frame(nullptr), pkt(nullptr), out_stream(nullptr),
      width(width), height(height), frame_counter(0)
{
    AVCodec *codec = const_cast<AVCodec *>(avcodec_find_encoder(AV_CODEC_ID_H264));
    if (!codec)
    {
        throw std::runtime_error("Necessary encoder not found.");
    }

    enc_ctx = avcodec_alloc_context3(codec);
    if (!enc_ctx)
    {
        throw std::runtime_error("Failed to allocate the encoder context.");
    }

    enc_ctx->height = height;
    enc_ctx->width = width;
    enc_ctx->sample_aspect_ratio = {1, 1};
    enc_ctx->pix_fmt = AV_PIX_FMT_YUV420P;
    enc_ctx->time_base = {1, 30};
    enc_ctx->gop_size = 10;
    enc_ctx->max_b_frames = 1;

    if (avcodec_open2(enc_ctx, codec, nullptr) < 0)
    {
        avcodec_free_context(&enc_ctx);
        throw std::runtime_error("Could not open encoder.");
    }

    img_convert_ctx = sws_getContext(width, height, AV_PIX_FMT_RGB24,
                                     width, height, AV_PIX_FMT_YUV420P,
                                     SWS_BICUBIC, nullptr, nullptr, nullptr);

    if (!img_convert_ctx)
    {
        avcodec_free_context(&enc_ctx);
        throw std::runtime_error("Failed to initialize the conversion context.");
    }

    yuv_frame = av_frame_alloc();
    if (!yuv_frame)
    {
        avcodec_free_context(&enc_ctx);
        sws_freeContext(img_convert_ctx);
        throw std::runtime_error("Failed to allocate video frame.");
    }

    yuv_frame->format = enc_ctx->pix_fmt;
    yuv_frame->width = enc_ctx->width;
    yuv_frame->height = enc_ctx->height;

    if (av_image_alloc(yuv_frame->data, yuv_frame->linesize, enc_ctx->width, enc_ctx->height, enc_ctx->pix_fmt, 32) < 0)
    {
        av_frame_free(&yuv_frame);
        avcodec_free_context(&enc_ctx);
        sws_freeContext(img_convert_ctx);
        throw std::runtime_error("Could not allocate raw picture buffer.");
    }

    pkt = av_packet_alloc();
    if (!pkt)
    {
        av_frame_free(&yuv_frame);
        avcodec_free_context(&enc_ctx);
        sws_freeContext(img_convert_ctx);
        throw std::runtime_error("Failed to allocate packet.");
    }
}

ScreenCapture::~ScreenCapture()
{
    release();
}

void ScreenCapture::openOutputContext(const char *path)
{
    avformat_alloc_output_context2(&fmt_ctx, nullptr, nullptr, path);
    if (!fmt_ctx)
    {
        throw std::runtime_error("Could not create output context.");
    }

    AVOutputFormat *fmt = const_cast<AVOutputFormat *>(fmt_ctx->oformat);
    out_stream = avformat_new_stream(fmt_ctx, nullptr);
    if (!out_stream)
    {
        avformat_free_context(fmt_ctx);
        throw std::runtime_error("Failed to allocate output stream.");
    }

    out_stream->time_base = {1, 30};

    if (avcodec_parameters_from_context(out_stream->codecpar, enc_ctx) < 0)
    {
        avformat_free_context(fmt_ctx);
        throw std::runtime_error("Failed to copy encoder parameters to output stream.");
    }

    if (!(fmt->flags & AVFMT_NOFILE))
    {
        if (avio_open(&fmt_ctx->pb, path, AVIO_FLAG_WRITE) < 0)
        {
            avformat_free_context(fmt_ctx);
            throw std::runtime_error("Could not open output file.");
        }
    }

    if (avformat_write_header(fmt_ctx, nullptr) < 0)
    {
        avio_close(fmt_ctx->pb);
        avformat_free_context(fmt_ctx);
        throw std::runtime_error("Error occurred when opening output file.");
    }
}

void ScreenCapture::encodeFrame(uint8_t *data)
{
    // Y flip
    uint8_t *srcSlices[1] = { data + (height - 1) * width * 3 };
    int srcStride[1] = { -static_cast<int>(width) * 3};

    sws_scale(img_convert_ctx, srcSlices, srcStride, 0, height, yuv_frame->data, yuv_frame->linesize);

    yuv_frame->pts = frame_counter++;

    if (avcodec_send_frame(enc_ctx, yuv_frame) < 0)
    {
        throw std::runtime_error("Error sending a frame for encoding.");
    }

    while (avcodec_receive_packet(enc_ctx, pkt) == 0)
    {
        av_packet_rescale_ts(pkt, enc_ctx->time_base, out_stream->time_base);
        pkt->stream_index = out_stream->index;

        if (av_interleaved_write_frame(fmt_ctx, pkt) < 0)
        {
            av_packet_unref(pkt);
            throw std::runtime_error("Error while writing video frame.");
        }
        av_packet_unref(pkt);
    }
}

void ScreenCapture::release()
{
    if (fmt_ctx)
    {
        av_write_trailer(fmt_ctx);
        if (enc_ctx)
        {
            avcodec_free_context(&enc_ctx);
        }
        if (pkt)
        {
            av_packet_free(&pkt);
        }
        if (yuv_frame)
        {
            av_frame_free(&yuv_frame);
        }
        if (img_convert_ctx)
        {
            sws_freeContext(img_convert_ctx);
        }
        if (!(fmt_ctx->oformat->flags & AVFMT_NOFILE))
        {
            avio_close(fmt_ctx->pb);
        }
        avformat_free_context(fmt_ctx);
    }
}

#endif