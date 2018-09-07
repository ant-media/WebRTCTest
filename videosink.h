#ifndef VIDEOSINK_H
#define VIDEOSINK_H

#include "api/mediastreaminterface.h"
#include "api/video/video_frame.h"
#include "media/base/mediachannel.h"
#include "media/base/videocommon.h"

#include <QImage>
#include <QLabel>


class VideoSink : public rtc::VideoSinkInterface<webrtc::VideoFrame> {
public:
    QLabel *label;
    VideoSink();
    void setTrack(webrtc::VideoTrackInterface* track_to_render);
    void saveFrame(QImage &img);
    void OnFrame(const webrtc::VideoFrame& frame) override;


    std::unique_ptr<uint8_t[]> image_;
    int frameCount;
};

#endif // VIDEOSINK_H
