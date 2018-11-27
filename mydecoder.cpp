#include "mydecoder.h"
#include "iostream"
#include "QTime"

QQueue<QByteArray> videoQ;
QQueue<QByteArray> audioQ;

int width, height;

cricket::MyVideoCapturer *videoCapturer;
webrtc::MyAudioDeviceModuleImpl* myadm;

MyDecoder::MyDecoder(QString fileName, cricket::MyVideoCapturer *vc, webrtc::MyAudioDeviceModuleImpl* adm)
{
    reader.init(fileName);
    videoCapturer = vc;
    myadm = adm;
}

void MyDecoder::start()
{
    reader.start();
    vSender.start();
    aSender.start();
}

bool ReaderThread::init(QString fileName) {
    int ret;
    const char* cp = fileName.toLocal8Bit().data();
    /* open input file, and allocate format context */

    AVDictionary *d = NULL;
        av_dict_set(&d, "protocol_whitelist","file,udp,tcp,rtp,rtsp", 0);

    if (avformat_open_input(&fmt_ctx, cp, NULL, &d) < 0) {
        std::cout<<"Could not open source file:"<<fileName.toStdString()<<std::endl;
        return false;
    }

    /* retrieve stream information */
    if (avformat_find_stream_info(fmt_ctx, NULL) < 0) {
        std::cout<<"Could not ind stream information"<<std::endl;
        return false;
    }

    if (open_codec_context(&video_stream_idx, &video_dec_ctx, fmt_ctx, AVMEDIA_TYPE_VIDEO) >= 0) {
        video_stream = fmt_ctx->streams[video_stream_idx];

        /* allocate image where the decoded image will be put */
        width = video_dec_ctx->width;
        height = video_dec_ctx->height;
        pix_fmt = video_dec_ctx->pix_fmt;
        ret = av_image_alloc(video_dst_data, video_dst_linesize,
                             width, height, pix_fmt, 1);
        if (ret < 0) {
            std::cout<<"Could not allocate raw video buffer"<<std::endl;
            return false;
        }
        video_dst_bufsize = ret;
    }

    if (open_codec_context(&audio_stream_idx, &audio_dec_ctx, fmt_ctx, AVMEDIA_TYPE_AUDIO) >= 0) {
        audio_stream = fmt_ctx->streams[audio_stream_idx];
    }

    /* dump input information to stderr */
    av_dump_format(fmt_ctx, 0, fileName.toStdString().c_str(), 0);

    if (!audio_stream && !video_stream) {
        std::cout<<"Could not find audio or video stream in the input, aborting"<<std::endl;
        return false;
    }

    frame = av_frame_alloc();
    if (!frame) {
        std::cout<<"Could not allocate frame"<<std::endl;
        return false;
    }

    /* initialize packet, set data to NULL, let the demuxer fill it */
    av_init_packet(&pkt);
    pkt.data = NULL;
    pkt.size = 0;
}



int ReaderThread::open_codec_context(int *stream_idx, AVCodecContext **dec_ctx, AVFormatContext *fmt_ctx, enum AVMediaType type)
{
    int ret, stream_index;
    AVStream *st;
    AVCodec *dec = NULL;
    AVDictionary *opts = NULL;

    ret = av_find_best_stream(fmt_ctx, type, -1, -1, NULL, 0);
    if (ret < 0) {
        std::cout<<"Could not find stream in input file:"<<av_get_media_type_string(type)<<std::endl;
        return ret;
    } else {
        stream_index = ret;
        st = fmt_ctx->streams[stream_index];

        /* find decoder for the stream */
        dec = avcodec_find_decoder(st->codecpar->codec_id);
        if (!dec) {
            std::cout<<"Failed to find codec:"<<av_get_media_type_string(type)<<std::endl;
            return AVERROR(EINVAL);
        }

        /* Allocate a codec context for the decoder */
        *dec_ctx = avcodec_alloc_context3(dec);
        if (!*dec_ctx) {
            std::cout<<"Failed to allocate the codec context"<<av_get_media_type_string(type)<<std::endl;
            return AVERROR(ENOMEM);
        }

        /* Copy codec parameters from input stream to output codec context */
        if ((ret = avcodec_parameters_to_context(*dec_ctx, st->codecpar)) < 0) {
            std::cout<<"Failed to copy codec parameters to decoder context"<<av_get_media_type_string(type)<<std::endl;
            return ret;
        }

        /* Init the decoders, with or without reference counting */
        av_dict_set(&opts, "refcounted_frames", refcount ? "1" : "0", 0);
        if ((ret = avcodec_open2(*dec_ctx, dec, &opts)) < 0) {
            std::cout<<"Failed to open codec"<<av_get_media_type_string(type)<<std::endl;
            return ret;
        }
        *stream_idx = stream_index;
    }

    return 0;
}

int ReaderThread::get_format_from_sample_fmt(const char **fmt, enum AVSampleFormat sample_fmt)
{
    int i;
    struct sample_fmt_entry {
        enum AVSampleFormat sample_fmt; const char *fmt_be, *fmt_le;
    }
    sample_fmt_entries[] = {
    { AV_SAMPLE_FMT_U8,  "u8",    "u8"    },
    { AV_SAMPLE_FMT_S16, "s16be", "s16le" },
    { AV_SAMPLE_FMT_S32, "s32be", "s32le" },
    { AV_SAMPLE_FMT_FLT, "f32be", "f32le" },
    { AV_SAMPLE_FMT_DBL, "f64be", "f64le" },};
    *fmt = NULL;

    for (i = 0; i < FF_ARRAY_ELEMS(sample_fmt_entries); i++) {
        struct sample_fmt_entry *entry = &sample_fmt_entries[i];
        if (sample_fmt == entry->sample_fmt) {
            *fmt = AV_NE(entry->fmt_be, entry->fmt_le);
            return 0;
        }
    }

    std::cout<<"sample format is not supported as output format:"<<av_get_sample_fmt_name(sample_fmt)<<std::endl;
    return -1;
}

int ReaderThread::decodePacket(int *got_frame, int cached) {
    int ret = 0;
    int decoded = pkt.size;
    *got_frame = 0;

    if (pkt.stream_index == video_stream_idx) {
        /* decode video frame */
        ret = avcodec_decode_video2(video_dec_ctx, frame, got_frame, &pkt);
        if (ret < 0) {
            std::cout<<"Error decoding video frame:"<<ret<<std::endl;
            return ret;
        }

        if (*got_frame) {
            if (frame->width != width || frame->height != height || frame->format != pix_fmt) {
                std::cout<<"Error in decoded video frame"<<std::endl;
                return -1;
            }

            /*
            printf("video_frame%s n:%d coded_n:%d pts:%d\n",
                   cached ? "(cached)" : "",
                   video_frame_count++, frame->coded_picture_number,
                   frame->pts);
            */

         //copy decoded frame to destination buffer: this is required since rawvideo expects non aligned data
            av_image_copy(video_dst_data, video_dst_linesize,
                          (const uint8_t **)(frame->data), frame->linesize,
                          pix_fmt, width, height);

            QByteArray vFrm((char *)video_dst_data[0], video_dst_bufsize);
            videoQ.enqueue(vFrm);

        }
    }
    else if (pkt.stream_index == audio_stream_idx) {
        /* decode audio frame */
        ret = avcodec_decode_audio4(audio_dec_ctx, frame, got_frame, &pkt);
        if (ret < 0) {
            std::cout<<"Error decoding audio frame:"<<ret<<std::endl;
            return ret;
        }

        decoded = FFMIN(ret, pkt.size);

        if (*got_frame) {
            size_t unpadded_linesize = frame->nb_samples * av_get_bytes_per_sample((AVSampleFormat)frame->format);

            /*
            printf("audio_frame%s n:%d nb_samples:%d pts:%d\n",
                   cached ? "(cached)" : "",
                   audio_frame_count++, frame->nb_samples,
                   frame->pts);
            */

            //audio data is 20ms length. webrtc waits 10ms. devide it into 2 parts
            char * audioData20ms = (char *)frame->extended_data[0];
            QByteArray aFrm1(audioData20ms, unpadded_linesize/2);
            audioQ.enqueue(aFrm1);
            QByteArray aFrm2(audioData20ms+unpadded_linesize/2, unpadded_linesize/2);
            audioQ.enqueue(aFrm2);
        }
    }

    /* If we use frame reference counting, we own the data and need
 * to de-reference it when we don't use it anymore */
    if (*got_frame && refcount)
        av_frame_unref(frame);

    return decoded;
}

void ReaderThread::run() {
    int ret, got_frame;
    /* read frames from the file */
    while (av_read_frame(fmt_ctx, &pkt) >= 0) {
        //if both Qs have 1 sec data, wait for a while
        if(audioQ.size() > 100 && videoQ.size()>24) {
       //     printf("reader waits v:%d a:%d\n", videoQ.size(), audioQ.size());
            msleep(800);
        }
        AVPacket orig_pkt = pkt;
        do {
            ret = decodePacket(&got_frame, 0);
            if (ret < 0)
                break;
            pkt.data += ret;
            pkt.size -= ret;
        } while (pkt.size > 0);
        av_packet_unref(&orig_pkt);
    }
}

void VideoSenderThread::run() {
    while(true) {
        if(!videoQ.isEmpty()) {
            QByteArray video = videoQ.dequeue();
            videoCapturer->VideoCaptured(video.data(), width, height);
            //std::cout<<"I get video still we have:"<<videoQ.size()<<std::endl;
            msleep(1000/24);
        }
    }
}
void AudioSenderThread::run() {
    qint64 sendStartTime, wait;
    while(true) {
        if(!audioQ.isEmpty()) {
            sendStartTime = QDateTime::currentMSecsSinceEpoch();
            QByteArray audio = audioQ.dequeue();
            myadm->WriteAudioFrame(audio.data(), audio.size()/4);
            //devide 4 to find sample per channel (each sample 16bit=2byte and there is 2 channel)

            //std::cout<<"I get audio still we have:"<<audioQ.size()<<std::endl;
            wait = sendStartTime+10-QDateTime::currentMSecsSinceEpoch();
            //std::cout<<"wait for:"<<wait<<std::endl;
            msleep(wait);
        }
    }
}
