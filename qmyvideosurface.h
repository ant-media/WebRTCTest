#ifndef QMYVIDEOSURFACE_H
#define QMYVIDEOSURFACE_H

#include "myvideocapturer.h"
#include <QAbstractVideoSurface>
#include <QMediaPlayer>

class QMyVideoSurface : public QAbstractVideoSurface
{
public:
    cricket::MyVideoCapturer *videoCapturer;
    QMyVideoSurface(QString fileName);
    QMediaPlayer *player;

    void setCapturer(cricket::MyVideoCapturer *vc);
    QList<QVideoFrame::PixelFormat> supportedPixelFormats(QAbstractVideoBuffer::HandleType handleType = QAbstractVideoBuffer::NoHandle) const;
    bool present(const QVideoFrame &frame);
    void play();

    int count = 0;
};

#endif // QMYVIDEOSURFACE_H
