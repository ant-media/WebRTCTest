#ifndef MYDECODER_H
#define MYDECODER_H

#include "QQueue"
#include "QThread"
#include "QByteArray"
#include "myvideocapturer.h"
#include "myaudiodevicemoduleimp.h"

extern "C" {
    #include <libavutil/samplefmt.h>
    #include <libavutil/timestamp.h>
    #include <libavutil/imgutils.h>
    #include <libavformat/avformat.h>
}

class ReaderThread : public QThread
{
    Q_OBJECT
public:
    bool init(QString fileName);

private:

    AVFormatContext *fmt_ctx=NULL;
    AVCodecContext *video_dec_ctx = NULL, *audio_dec_ctx;
    enum AVPixelFormat pix_fmt;
    AVStream *video_stream = NULL, *audio_stream = NULL;
    int video_stream_idx = -1, audio_stream_idx = -1;
    AVFrame *frame = NULL;
    AVPacket pkt;
    int video_frame_count = 0;
    int audio_frame_count = 0;
    int refcount = 0;
    uint8_t *video_dst_data[4] = {NULL};
    int video_dst_linesize[4];
    int video_dst_bufsize;

    int open_codec_context(int *stream_idx, AVCodecContext **dec_ctx, AVFormatContext *fmt_ctx, enum AVMediaType type);
    int get_format_from_sample_fmt(const char **fmt, enum AVSampleFormat sample_fmt);
    int decodePacket(int *got_frame, int cached);
    void run();
};

class VideoSenderThread : public QThread
{
    Q_OBJECT

private:
    void run();
};

class AudioSenderThread : public QThread
{
    Q_OBJECT
private:
    void run();
};

class MyDecoder
{
public:
    ReaderThread reader;
    VideoSenderThread vSender;
    AudioSenderThread aSender;
    MyDecoder(QString fileName, cricket::MyVideoCapturer *vc, webrtc::MyAudioDeviceModuleImpl* adm);
    void start();
};

#endif // MYDECODER_H
