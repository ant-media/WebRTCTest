QT += core gui websockets multimedia multimediawidgets
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = WebRTCTest
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

SOURCES += main.cpp \
    websocketadaptor.cpp \
    webrtcadaptor.cpp \
    videosink.cpp \
    settings.cpp \
    myvideocapturer.cc \
    qmyvideosurface.cpp \
    file_audio_device.cc \
    myaudiodevicemoduleimp.cpp

QMAKE_CXXFLAGS += -std=gnu++11 -fno-rtti


WEBRTC = /home/burak/webrtc/precompiled
INCLUDEPATH += $${WEBRTC}/include
INCLUDEPATH += $${WEBRTC}/include/third_party/abseil-cpp
INCLUDEPATH += $${WEBRTC}/include/third_party/libyuv/include



DEFINES += WEBRTC_POSIX
DEFINES += WEBRTC_INCLUDE_INTERNAL_AUDIO_DEVICE

LIBDIR = /home/burak/webrtc/webrtcbuilds/out/src/out/Release/obj

#LIBS += -L$${LIBDIR} -lwebrtc
#LIBS += -L$${LIBDIR}/rtc_base -lrtc_base
#LIBS += -L$${LIBDIR}/rtc_base -lrtc_base_generic
#LIBS += -L$${LIBDIR}/rtc_base/stringutils/stringencode.o


LIBS += -L$${WEBRTC}/lib/Release -lwebrtc_full
LIBS += -L$${LIBDIR}/third_party/ffmpeg -lffmpeg_yasm
LIBS += -L$${LIBDIR}/third_party/ffmpeg -lffmpeg_internal
LIBS += -L$${LIBDIR}/third_party/openh264 -lopenh264_common_yasm
LIBS += -L$${LIBDIR}/third_party/openh264 -lopenh264_encoder_yasm
LIBS += -L$${LIBDIR}/third_party/openh264 -lopenh264_processing_yasm
LIBS += -L$${LIBDIR}/rtc_base -lrtc_base

LIBS += -lX11 -lXcomposite -lXext -lXrender -latomic -ldl -lpthread -lrt

HEADERS += \
    websocketadaptor.h \
    webrtcadaptor.h \
    videosink.h \
    settings.h \
    myvideocapturer.h \
    qmyvideosurface.h \
    antutils.h \
    file_audio_device.h \
    myaudiodevicemoduleimp.h

DISTFILES +=

