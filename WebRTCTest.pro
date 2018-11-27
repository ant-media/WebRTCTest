QT += core gui websockets multimedia multimediawidgets
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = WebRTCTest
CONFIG += console
CONFIG -= app_bundle
CONFIG += no_keywords


TEMPLATE = app

SOURCES += main.cpp \
    websocketadaptor.cpp \
    webrtcadaptor.cpp \
    videosink.cpp \
    settings.cpp \
    myvideocapturer.cc \
    file_audio_device.cc \
    myaudiodevicemoduleimp.cpp \
    mydecoder.cpp

QMAKE_CXXFLAGS += -std=gnu++11 -fno-rtti -fpermissive


WEBRTC = /home/burak/antmedia/webrtc-checkout/src
INCLUDEPATH += $${WEBRTC}
INCLUDEPATH += $${WEBRTC}/third_party/abseil-cpp
INCLUDEPATH += $${WEBRTC}/third_party/libyuv/include
INCLUDEPATH += /usr/local/include

DEFINES += WEBRTC_POSIX
DEFINES += WEBRTC_INCLUDE_INTERNAL_AUDIO_DEVICE

LIBDIR = $${WEBRTC}/out

LIBS += -L/usr/local/lib
LIBS += -lavdevice -lavfilter -lavformat -lavcodec -lswresample -lswscale -lavutil -lpthread -lm -lz
LIBS += -L/usr/lib/x86_64-linux-gnu -lopus -lvpx

#LIBS += -L$${LIBDIR}/Release/obj -lwebrtc
LIBS += -L$${LIBDIR} -lwebrtc1
LIBS += -L$${LIBDIR} -lwebrtc2

LIBS += -lX11 -lXext -latomic -ldl -lpthread -lrt

HEADERS += \
    websocketadaptor.h \
    webrtcadaptor.h \
    videosink.h \
    settings.h \
    myvideocapturer.h \
    antutils.h \
    file_audio_device.h \
    myaudiodevicemoduleimp.h \
    mydecoder.h

DISTFILES +=

