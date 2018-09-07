
#include "myvideocapturer.h"
#include "stdio.h"
#include "rtc_base/arraysize.h"
#include "third_party/libyuv/include/libyuv/convert.h"
#include <iostream>
#include "antutils.h"
#include <QImage>
namespace cricket {

pthread_t g_pthread;

MyVideoCapturer::MyVideoCapturer(bool is_screencast)
    : running_(false),
      is_screencast_(is_screencast){
    // Default supported formats. Use ResetSupportedFormats to over write.
    using cricket::VideoFormat;
    static const VideoFormat formats[] = {
        {1280, 720, VideoFormat::FpsToInterval(30), cricket::FOURCC_I420},
        {640, 480, VideoFormat::FpsToInterval(30), cricket::FOURCC_I420},
        {320, 240, VideoFormat::FpsToInterval(30), cricket::FOURCC_I420},
        {160, 120, VideoFormat::FpsToInterval(30), cricket::FOURCC_I420},
        {1280, 720, VideoFormat::FpsToInterval(60), cricket::FOURCC_I420},
    };

    ResetSupportedFormats({&formats[0], &formats[arraysize(formats)]});
}

MyVideoCapturer::MyVideoCapturer() : MyVideoCapturer(false) {}

MyVideoCapturer::~MyVideoCapturer() {
    SignalDestroyed(this);
}

void MyVideoCapturer::ResetSupportedFormats(const std::vector<cricket::VideoFormat>& formats) {
    SetSupportedFormats(formats);
}

bool MyVideoCapturer::CaptureFrame(const webrtc::VideoFrame& frame) {
    if (!running_) {
        return false;
    }

    OnFrame(frame, frame.width(), frame.height());

    return true;
}

cricket::CaptureState MyVideoCapturer::Start(const cricket::VideoFormat& format) {
    videoFormat = new VideoFormat(format);
    running_ = true;
    SetCaptureState(cricket::CS_RUNNING);
    m_startThread = rtc::Thread::Current();

    return cricket::CS_RUNNING;
}

const VideoFormat* MyVideoCapturer::GetCaptureFormat() {
    return videoFormat;
}


void MyVideoCapturer::Stop() {
    running_ = false;
    SetCaptureFormat(NULL);
    SetCaptureState(cricket::CS_STOPPED);
}

bool MyVideoCapturer::IsRunning() {
    return running_;
}

bool MyVideoCapturer::IsScreencast() const {
    return is_screencast_;
}

bool MyVideoCapturer::GetPreferredFourccs(std::vector<uint32_t>* fourccs) {
    fourccs->push_back(cricket::FOURCC_I420);
    fourccs->push_back(cricket::FOURCC_MJPG);
    return true;
}

void MyVideoCapturer::VideoCaptured(unsigned char* data, int width, int height) {

    rtc::scoped_refptr<webrtc::I420Buffer> buffer(webrtc::I420Buffer::Create(640, 480));
    buffer->InitializeData();

    libyuv::ARGBToI420((uint8_t*)data, 640 * 4, buffer->MutableDataY(), buffer->StrideY(),
                       buffer->MutableDataU(), buffer->StrideU(),
                       buffer->MutableDataV(), buffer->StrideV(),
                       buffer->width(), buffer->height());

    webrtc::VideoFrame frame =  webrtc::VideoFrame(buffer, webrtc::VideoRotation::kVideoRotation_0, 30);

    CaptureFrame(frame);
}





}  // namespace cricket
