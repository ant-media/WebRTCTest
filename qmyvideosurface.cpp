#include "qmyvideosurface.h"
#include <QDir>

QMyVideoSurface::QMyVideoSurface(QString fileName) {
    QString url = QDir::homePath() + QString("/test/")+QString(fileName);
    player = new QMediaPlayer;
    player->setMedia(QUrl::fromLocalFile(url));
    player->setVolume(10);
    player->setVideoOutput(this);
}

void QMyVideoSurface::setCapturer(cricket::MyVideoCapturer *vc){
    videoCapturer = vc;
}

QList<QVideoFrame::PixelFormat> QMyVideoSurface::supportedPixelFormats(QAbstractVideoBuffer::HandleType handleType) const
{
    Q_UNUSED(handleType);

    // Return the formats you will support
    QList<QVideoFrame::PixelFormat> list;
    list<< QVideoFrame::Format_RGB565;
    list<< QVideoFrame::Format_ARGB32;
    list<< QVideoFrame::Format_BGRA32;
    list<< QVideoFrame::Format_RGB32;

    return list;
}

bool QMyVideoSurface::present(const QVideoFrame &frm)
{
    QVideoFrame frame(frm);  // make a copy we can call map (non-const) on
    frame.map(QAbstractVideoBuffer::ReadOnly);
    videoCapturer->VideoCaptured((unsigned char *)frame.bits(), frm.width(), frm.height());

    //QImage::Format imageFormat = QVideoFrame::imageFormatFromPixelFormat(frame.pixelFormat());
    //QImage img( frame.bits(), frame.width(), frame.height(), frame.bytesPerLine(), imageFormat);
    //img.save("deneme.png", "png", 100);

    count ++;
    frame.unmap();
    return true;
}

void QMyVideoSurface::play()
{
    player->play();
}


