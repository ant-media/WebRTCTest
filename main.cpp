#include <rtc_base/physicalsocketserver.h>
#include <rtc_base/ssladapter.h>
#include <rtc_base/thread.h>
#include <webrtcadaptor.h>

#include <thread>
#include <cstdlib>
#include <csignal>
#include <thread>

#include "videosink.h"
#include "settings.h"

#include "qmyvideosurface.h"
#include <QThread>
#include <QApplication>

WebRTCAdaptor *adaptor;
QApplication *app;
class CustomRunnable : public rtc::Runnable {
public:
    void Run(rtc::Thread* subthread) override {

        adaptor = new WebRTCAdaptor();
        subthread->Run();
    }
};

class StatWriter : public QThread
{
private:
    void run()
    {
        while(true) {
            sleep(5);
            printf("\n");
            adaptor->getStats();
        }
    }
};

void doexit(){
    printf("exiting\n");
//    adaptor->stop();
    sleep(1);
    exit(0);
}

void doexitsignal(int i){
    doexit();
}

int main(int argc, char* argv[]) {
    if(!Settings::Parse(argc, argv)) {
        Settings::PrintUsage();
        return 0;
    }

    std::atexit(doexit);
    //^C
    signal(SIGINT, doexitsignal);
    //abort()
    signal(SIGABRT, doexitsignal);
    //sent by "kill" command
    signal(SIGTERM, doexitsignal);
    //^Z
    signal(SIGTSTP, doexitsignal);

    std::unique_ptr<rtc::Thread> thread;
    rtc::PhysicalSocketServer socket_server;
    thread.reset(new rtc::Thread(&socket_server));
    rtc::InitializeSSL();
    CustomRunnable runnable;
    thread->Start(&runnable);

    sleep(1);

    //std::thread t1(StatWriterTask);
    //t1.join();

    QApplication a(argc, argv);

    StatWriter t;
    t.start();

    app = &a;
    VideoSink *vs = new VideoSink();

    adaptor->video_sink = vs;
    if(Settings::mode == Settings::Mode::Publisher) {
        vs->setTrack(adaptor->video_track_);
    }


    if(Settings::mode == Settings::Mode::Publisher && Settings::streamSource.compare("camera") != 0) {
        QMyVideoSurface *surface = new QMyVideoSurface(QString(Settings::streamSource.c_str()));
        surface->setCapturer(adaptor->capturer);
        surface->play();
    }

    adaptor->init();

    a.exec();

    //printf("after exec\n");
    thread.reset();
    rtc::CleanupSSL();

    return 0;
}
