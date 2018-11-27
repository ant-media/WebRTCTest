#ifndef WEBRTCADAPTOR_H
#define WEBRTCADAPTOR_H

#define WEBRTC_LINUX 1
#define WEBRTC_POSIX 1

#include <api/peerconnectioninterface.h>
#include "api/stats/rtcstatsreport.h"
#include "websocketadaptor.h"
#include "videosink.h"
#include "myvideocapturer.h"
#include "myaudiodevicemoduleimp.h"
#include "mydecoder.h"


class WebRTCAdaptor;

class SSDO : public webrtc::SetSessionDescriptionObserver {
private:
    WebRTCAdaptor* parent;
public:
    SSDO(WebRTCAdaptor* parent);
    void OnSuccess() override;
    void OnFailure(const std::string& error) override;
};

class PCO : public webrtc::PeerConnectionObserver {
private:
    WebRTCAdaptor* parent;
public:
    PCO(WebRTCAdaptor* parent);
    void OnSignalingChange(webrtc::PeerConnectionInterface::SignalingState new_state) override;
    void OnAddStream(rtc::scoped_refptr<webrtc::MediaStreamInterface> stream) override;
    void OnRemoveStream(rtc::scoped_refptr<webrtc::MediaStreamInterface> stream) override;
    void OnDataChannel(rtc::scoped_refptr<webrtc::DataChannelInterface> data_channel) override;
    void OnRenegotiationNeeded() override;
    void OnIceConnectionChange(webrtc::PeerConnectionInterface::IceConnectionState new_state) override;
    void OnIceGatheringChange(webrtc::PeerConnectionInterface::IceGatheringState new_state) override;
    void OnIceCandidate(const webrtc::IceCandidateInterface* candidate) override;
    void OnAddTrack(
        rtc::scoped_refptr<webrtc::RtpReceiverInterface> receiver,
        const std::vector<rtc::scoped_refptr<webrtc::MediaStreamInterface>>& streams) override;
};

class CSDO : public webrtc::CreateSessionDescriptionObserver {
private:
    WebRTCAdaptor* parent;
public:
    CSDO(WebRTCAdaptor* parent);
    void OnSuccess(webrtc::SessionDescriptionInterface* desc) override;
    void OnFailure(const std::string& error) override;
};

class RSCC : public webrtc::RTCStatsCollectorCallback{
 public:
  virtual void OnStatsDelivered(const rtc::scoped_refptr<const webrtc::RTCStatsReport>& report);
};




class WebRTCAdaptor : public WebSocketListener
{
public:
    WebRTCAdaptor();

    rtc::scoped_refptr<webrtc::MyAudioDeviceModuleImpl> myAdm = webrtc::MyAudioDeviceModuleImpl::Create(webrtc::AudioDeviceModule::AudioLayer::kLinuxAlsaAudio);

    MyDecoder *fileReader;
    void gotDescription(webrtc::SessionDescriptionInterface* desc);
    void onIceCandidate(const webrtc::IceCandidateInterface* candidate);
    void init();
    void startPublishing(std::string streamId);
    void takeConfiguration(std::string streamId, std::string sdp, std::string type);
    void takeCandidate(std::string streamId, std::string mid, int mIndex, std::string candidate);
    std::unique_ptr<cricket::VideoCapturer> OpenVideoCaptureDevice();
    void AddTracks();
    void InitializePeerConnection();
    void getStats();
    void stop();

    virtual void onConnected() override;
    virtual void onMessage(std::string msg) override;

    cricket::MyVideoCapturer *capturer;
    rtc::scoped_refptr<webrtc::VideoTrackInterface> video_track_;
    WebSocketAdaptor *wsa;
    rtc::scoped_refptr<webrtc::PeerConnectionInterface> peer_connection;
    std::string sdp;
    PCO  pco;
    rtc::scoped_refptr<RSCC> rscc;
    rtc::scoped_refptr<CSDO> csdo;
    rtc::scoped_refptr<SSDO> ssdo;
    VideoSink *video_sink;
    rtc::scoped_refptr<webrtc::PeerConnectionFactoryInterface> peer_connection_factory;
    webrtc::PeerConnectionInterface::RTCConfiguration configuration;
};

#endif // WEBRTCADAPTOR_H
