#include "webrtcadaptor.h"

#include <iostream>
#include <memory>
#include <mutex>
#include <queue>
#include <string>
#include <thread>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonValue>
#include "settings.h"

#include <api/audio_codecs/builtin_audio_decoder_factory.h>
#include <api/audio_codecs/builtin_audio_encoder_factory.h>
#include "api/video_codecs/builtin_video_decoder_factory.h"
#include "api/video_codecs/builtin_video_encoder_factory.h"
#include <api/peerconnectioninterface.h>
#include <api/mediaconstraintsinterface.h>
#include <rtc_base/flags.h>
#include <rtc_base/physicalsocketserver.h>
#include <rtc_base/ssladapter.h>
#include <rtc_base/thread.h>

#include "media/engine/webrtcvideocapturerfactory.h"
#include "modules/audio_device/include/audio_device.h"
#include "modules/audio_processing/include/audio_processing.h"
#include "modules/video_capture/video_capture_factory.h"
#include "myvideocapturer.h"
//#include "audiosink.h"


#include <iostream>
#include <memory>
#include <mutex>
#include <queue>
#include <string>
#include <thread>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonValue>


SSDO::SSDO(WebRTCAdaptor* parent) {
    this->parent = parent;
}

void SSDO::OnSuccess() {
    //std::cout << std::this_thread::get_id() << ":"<< "SetSessionDescriptionObserver::OnSuccess" << std::endl;
}

void SSDO::OnFailure(const std::string& error)  {
    //std::cout << std::this_thread::get_id() << ":"<< "SetSessionDescriptionObserver::OnFailure" << std::endl << error << std::endl;
}

PCO::PCO(WebRTCAdaptor* parent){
    this->parent = parent;
}

void PCO::OnSignalingChange(webrtc::PeerConnectionInterface::SignalingState new_state)  {
    std::cout << std::this_thread::get_id() << ":"<< "PeerConnectionObserver::SignalingChange(" << new_state << ")" << std::endl;
}

void PCO::OnAddStream(rtc::scoped_refptr<webrtc::MediaStreamInterface> stream)  {
    //std::cout << std::this_thread::get_id() << ":"<< "PeerConnectionObserver::AddStream" << std::endl;
}

void PCO::OnRemoveStream(rtc::scoped_refptr<webrtc::MediaStreamInterface> stream)  {
    //std::cout << std::this_thread::get_id() << ":"<< "PeerConnectionObserver::RemoveStream" << std::endl;
}

void PCO::OnDataChannel(rtc::scoped_refptr<webrtc::DataChannelInterface> data_channel)  {
    //std::cout << std::this_thread::get_id() << ":"<< "PeerConnectionObserver::DataChannel()" << std::endl;
}

void PCO::OnRenegotiationNeeded()  {
    //std::cout << std::this_thread::get_id() << ":"<< "PeerConnectionObserver::RenegotiationNeeded" << std::endl;
}

void PCO::OnIceConnectionChange(webrtc::PeerConnectionInterface::IceConnectionState new_state)  {
    std::cout << std::this_thread::get_id() << ":"<< "PeerConnectionObserver::IceConnectionChange(" << new_state << ")" << std::endl;

    if(new_state == webrtc::PeerConnectionInterface::IceConnectionState::kIceConnectionCompleted) {
        parent->fileReader->start();
    }
}

void PCO::OnIceGatheringChange(webrtc::PeerConnectionInterface::IceGatheringState new_state)  {
    //std::cout << std::this_thread::get_id() << ":"<< "PeerConnectionObserver::IceGatheringChange(" << new_state << ")" << std::endl;
}

void PCO::OnIceCandidate(const webrtc::IceCandidateInterface* candidate)  {
    //std::cout << std::this_thread::get_id() << ":"<< "PeerConnectionObserver::IceCandidate" << std::endl;
    parent->onIceCandidate(candidate);
}

void PCO::OnAddTrack(
        rtc::scoped_refptr<webrtc::RtpReceiverInterface> receiver,
        const std::vector<rtc::scoped_refptr<webrtc::MediaStreamInterface>>& streams){

    std::cout << std::this_thread::get_id() << ":"<< "PeerConnectionObserver::OnAddTrack" << std::endl;

    webrtc::MediaStreamTrackInterface* track = receiver->track().release();

    if (track->kind() == webrtc::MediaStreamTrackInterface::kVideoKind) {
        auto* video_track = static_cast<webrtc::VideoTrackInterface*>(track);
        parent->video_sink->setTrack(video_track);
    }
    track->Release();
}

CSDO::CSDO(WebRTCAdaptor* parent){
    this->parent = parent;
}

void CSDO::OnSuccess(webrtc::SessionDescriptionInterface* desc)  {
    //std::cout << std::this_thread::get_id() << ":"<< "CreateSessionDescriptionObserver::OnSuccess" << std::endl;
    parent->gotDescription(desc);
}

void CSDO::OnFailure(const std::string& error)  {
    //std::cout << std::this_thread::get_id() << ":"<< "CreateSessionDescriptionObserver::OnFailure" << std::endl << error << std::endl;
}

void RSCC::OnStatsDelivered(const rtc::scoped_refptr<const webrtc::RTCStatsReport>& report) {
    printf("on stats:%s\n",report->ToJson().c_str());
}


WebRTCAdaptor::WebRTCAdaptor() :
    pco(this),
    csdo(new rtc::RefCountedObject<CSDO>(this)),
    rscc(new rtc::RefCountedObject<RSCC>()),
    ssdo(new rtc::RefCountedObject<SSDO>(this))
{
    InitializePeerConnection();
}

void WebRTCAdaptor::init() {
    wsa = new WebSocketAdaptor();
    wsa->listener = this;
    wsa->Connect();
}

void WebRTCAdaptor::onIceCandidate(const webrtc::IceCandidateInterface* candidate) {
    std::string candidate_str;
    candidate->ToString(&candidate_str);
    //todo: must be qjsonarray
    QJsonDocument doc;
    QJsonObject json;
    json["candidate"] = candidate_str.c_str();
    json["sdpMid"] = candidate->sdp_mid().c_str();
    json["sdpMLineIndex"] = candidate->sdp_mline_index();
    doc.setObject(json);
}

void WebRTCAdaptor::takeCandidate(std::string streamId, std::string mid, int mIndex, std::string candidate) {
    webrtc::SdpParseError err_sdp;
    webrtc::IceCandidateInterface* ice = webrtc::CreateIceCandidate(mid, mIndex, candidate, &err_sdp);
    if (!err_sdp.line.empty() && !err_sdp.description.empty()) {
        std::cout << "Error on CreateIceCandidate" << std::endl
                  << err_sdp.line << std::endl
                  << err_sdp.description << std::endl;
        return;
    }
    peer_connection->AddIceCandidate(ice);
}

void WebRTCAdaptor::takeConfiguration(std::string streamId, std::string sdp, std::string type) {
    InitializePeerConnection();

    webrtc::SdpParseError error;
    webrtc::SessionDescriptionInterface* session_description = webrtc::CreateSessionDescription(type, sdp, &error);
    if (!session_description) {
        std::cout << "Can't parse received session description message. "
                  << "SdpParseError was: " << error.description<<std::endl;
        return;
    }
    peer_connection->SetRemoteDescription(ssdo, session_description);

    if(type.compare("offer") == 0) {
        peer_connection->CreateAnswer(csdo, webrtc::PeerConnectionInterface::RTCOfferAnswerOptions());
    }
}


void WebRTCAdaptor::startPublishing(std::string streamId){
    webrtc::PeerConnectionInterface::RTCOfferAnswerOptions options(1, 1, true, false, true);
    peer_connection->CreateOffer(csdo, options);
}

void WebRTCAdaptor::gotDescription(webrtc::SessionDescriptionInterface* desc)
{
    peer_connection->SetLocalDescription(ssdo, desc);
    std::string sdp;
    desc->ToString(&sdp);

    QJsonDocument doc;
    QJsonObject json;

    json["command"] = "takeConfiguration";
    json["streamId"] = Settings::streamId.c_str();
    json["type"] = webrtc::SdpTypeToString(desc->GetType());
    json["sdp"] = sdp.c_str();


    doc.setObject(json);

    wsa->MessageToSend(QString::fromStdString(doc.toJson().toStdString()));
}

void WebRTCAdaptor::onConnected(){
    QJsonDocument doc;
    QJsonObject json;

    if(Settings::mode == Settings::Mode::Player){
        json["command"] = "play";
        json["streamId"] = Settings::streamId.c_str();
    }
    else{
        json["command"] = "publish";
        json["streamId"] = Settings::streamId.c_str();
        json["video"] = true;
        json["audio"] = true;
    }

    doc.setObject(json);

    wsa->MessageToSend(QString::fromStdString(doc.toJson().toStdString()));
}

void WebRTCAdaptor::onMessage(std::string msg)
{
    //printf("onMessage:%s\n", msg);

    QString json = msg.c_str();
    QJsonParseError * error = new QJsonParseError;
    QJsonDocument doc = QJsonDocument::fromJson(json.toUtf8(), error);

    QJsonObject data = doc.object();
    QString command = data["command"].toString();

    if (command.compare("start") == 0)
    {
        std::string streamId = data["streamId"].toString().toStdString();
        startPublishing(streamId);
    }
    else if (command.compare("takeCandidate") == 0) {
        std::string streamId = data["streamId"].toString().toStdString();
        std::string id = data["id"].toString().toStdString();
        int label = data["label"].toInt();
        std::string candidate = data["candidate"].toString().toStdString();
        takeCandidate(streamId, id, label, candidate);

    } else if (command.compare("takeConfiguration") == 0) {
        std::string streamId = data["streamId"].toString().toStdString();
        std::string sdp = data["sdp"].toString().toStdString();
        std::string type = data["type"].toString().toStdString();
        takeConfiguration(streamId, sdp, type);

    }
    else if (command.compare("stop") == 0) {
        std::string streamId = data["streamId"].toString().toStdString();
        //closePeerConnection(streamId);
    }
    else if (command.compare("error") == 0) {
        std::string definition = data["definition"].toString().toStdString();
        //callbackError(definition);
    }
    else if (command.compare("notification") == 0) {
        std::string definition = data["definition"].toString().toStdString();
        //callback(obj.definition, obj);
    }
    else if (command.compare("streamInformation") == 0) {
        //callback(obj.command, obj);
    }
    else if (command.compare("publish_started") == 0) {

    }
}

std::unique_ptr<cricket::VideoCapturer> WebRTCAdaptor::OpenVideoCaptureDevice() {
    std::vector<std::string> device_names;
    {
        std::unique_ptr<webrtc::VideoCaptureModule::DeviceInfo> info(webrtc::VideoCaptureFactory::CreateDeviceInfo());
        if (!info) {
            return nullptr;
        }
        int num_devices = info->NumberOfDevices();
        for (int i = 0; i < num_devices; ++i) {
            const uint32_t kSize = 256;
            char name[kSize] = {0};
            char id[kSize] = {0};
            if (info->GetDeviceName(i, name, kSize, id, kSize) != -1) {
                device_names.push_back(name);
            }
        }
    }

    cricket::WebRtcVideoDeviceCapturerFactory factory;
    std::unique_ptr<cricket::VideoCapturer> capturer;
    for (const auto& name : device_names) {
        capturer = factory.Create(cricket::Device(name, 0));
        if (capturer) {
            break;
        }
    }
    return capturer;
}

void WebRTCAdaptor::AddTracks() {
    if (!peer_connection->GetSenders().empty()) {
        return;  // Already added tracks.
    }

    rtc::scoped_refptr<webrtc::AudioTrackInterface> audio_track(
        peer_connection_factory->CreateAudioTrack(
            "audio", peer_connection_factory->CreateAudioSource(
                             cricket::AudioOptions())));
    //audio_track->AddSink(new AudioSink());
    myAdm->InitRecording();
    myAdm->StartRecording();

    auto result_or_error = peer_connection->AddTrack(audio_track, {Settings::streamId});

    if (!result_or_error.ok()) {
        printf("\n \n  errr\n\n");
        RTC_LOG(LS_ERROR) << "Failed to add audio track to PeerConnection: "
                          << result_or_error.error().message();
    }


    if(Settings::mode == Settings::Mode::Publisher && Settings::streamSource.compare("camera") != 0) {
        capturer = new cricket::MyVideoCapturer(true);
        rtc::scoped_refptr<webrtc::VideoTrackInterface>  vt;
        rtc::scoped_refptr<webrtc::VideoTrackSourceInterface> vts = peer_connection_factory->CreateVideoSource(capturer);
        vt = peer_connection_factory->CreateVideoTrack("video", vts);

        video_track_ = vt.get();
        auto result_or_error = peer_connection->AddTrack(video_track_, {Settings::streamId});

        if (!result_or_error.ok()) {
            RTC_LOG(LS_ERROR) << "Failed to add video track to PeerConnection: "
                              << result_or_error.error().message();
        }
    }
    else {
        std::unique_ptr<cricket::VideoCapturer> video_device = OpenVideoCaptureDevice();
        if (video_device) {
            rtc::scoped_refptr<webrtc::VideoTrackInterface>  vt;
            rtc::scoped_refptr<webrtc::VideoTrackSourceInterface> vts = peer_connection_factory->CreateVideoSource(std::move(video_device), nullptr);
            vt = peer_connection_factory->CreateVideoTrack("video", vts);

            video_track_ = vt.get();
            auto result_or_error = peer_connection->AddTrack(video_track_, {"myStream"});

            if (!result_or_error.ok()) {
                RTC_LOG(LS_ERROR) << "Failed to add video track to PeerConnection: "
                                  << result_or_error.error().message();
            }

        } else {
            RTC_LOG(LS_ERROR) << "OpenVideoCaptureDevice failed";
        }
    }

}

void WebRTCAdaptor::InitializePeerConnection() {
    if(peer_connection_factory == NULL){
       if(Settings::mode == Settings::Mode::Publisher && Settings::streamSource.compare("camera") == 0) {
           peer_connection_factory = webrtc::CreatePeerConnectionFactory(
                       nullptr /* network_thread */, nullptr /* worker_thread */,
                       nullptr /* signaling_thread */, nullptr /* default_adm */,
                       webrtc::CreateBuiltinAudioEncoderFactory(),
                       webrtc::CreateBuiltinAudioDecoderFactory(),
                       webrtc::CreateBuiltinVideoEncoderFactory(),
                       webrtc::CreateBuiltinVideoDecoderFactory(), nullptr /* audio_mixer */,
                       nullptr /* audio_processing */);
       }
       else {
        peer_connection_factory = webrtc::CreatePeerConnectionFactory(
                    nullptr /* network_thread */, nullptr /* worker_thread */,
                    nullptr /* signaling_thread */,  myAdm,
                    webrtc::CreateBuiltinAudioEncoderFactory(),
                    webrtc::CreateBuiltinAudioDecoderFactory(),
                    webrtc::CreateBuiltinVideoEncoderFactory(),
                    webrtc::CreateBuiltinVideoDecoderFactory(), nullptr /* audio_mixer */,
                    nullptr /* audio_processing */);
       }

        webrtc::PeerConnectionInterface::RTCConfiguration config;
        config.sdp_semantics = webrtc::SdpSemantics::kUnifiedPlan;
        config.enable_dtls_srtp = true;
        webrtc::PeerConnectionInterface::IceServer server;
        server.uri = "stun:stun.l.google.com:19302";
        config.servers.push_back(server);

        peer_connection = peer_connection_factory->CreatePeerConnection(config, nullptr, nullptr, &pco);

        if(Settings::mode == Settings::Mode::Publisher) {
            AddTracks();
        }
    }
}

void WebRTCAdaptor::getStats() {
    peer_connection->GetStats(rscc.get());

}

void WebRTCAdaptor::stop() {
    getStats();

    QJsonDocument doc;
    QJsonObject json;

    json["command"] = "stop";
    json["streamId"] = Settings::streamId.c_str();

    doc.setObject(json);
    wsa->MessageToSend(QString::fromStdString(doc.toJson().toStdString()));

    //peer_connection->Close();
}

