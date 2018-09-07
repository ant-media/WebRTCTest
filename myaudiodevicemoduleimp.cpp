#include "myaudiodevicemoduleimp.h"

#include "modules/audio_device/audio_device_config.h"
#include "modules/audio_device/audio_device_generic.h"
#include "rtc_base/checks.h"
#include "rtc_base/logging.h"
#include "rtc_base/refcount.h"
#include "rtc_base/refcountedobject.h"
#include "system_wrappers/include/metrics.h"

#include "file_audio_device.h"

#define CHECKinitialized_() \
  {                         \
    if (!initialized_) {    \
      return -1;            \
    }                       \
  }

#define CHECKinitialized__BOOL() \
  {                              \
    if (!initialized_) {         \
      return false;              \
    }                            \
  }

namespace webrtc {

// static
rtc::scoped_refptr<AudioDeviceModule> AudioDeviceModule::Create(
    const AudioLayer audio_layer) {
  RTC_LOG(INFO) << __FUNCTION__;

  // The "AudioDeviceModule::kWindowsCoreAudio2" audio layer has its own
  // dedicated factory method which should be used instead.
  if (audio_layer == AudioDeviceModule::kWindowsCoreAudio2) {
    RTC_LOG(LS_ERROR) << "Use the CreateWindowsCoreAudioAudioDeviceModule() "
                         "factory method instead for this option.";
    return nullptr;
  }

  // Create the generic reference counted (platform independent) implementation.
  rtc::scoped_refptr<MyAudioDeviceModuleImpl> audioDevice(
      new rtc::RefCountedObject<MyAudioDeviceModuleImpl>(audio_layer));

  // Ensure that the current platform is supported.
  if (audioDevice->CheckPlatform() == -1) {
    return nullptr;
  }

  // Create the platform-dependent implementation.
  if (audioDevice->CreatePlatformSpecificObjects() == -1) {
    return nullptr;
  }

  // Ensure that the generic audio buffer can communicate with the platform
  // specific parts.
  if (audioDevice->AttachAudioBuffer() == -1) {
    return nullptr;
  }

  return audioDevice;
}

// TODO(bugs.webrtc.org/7306): deprecated.
rtc::scoped_refptr<AudioDeviceModule> AudioDeviceModule::Create(
    const int32_t id,
    const AudioLayer audio_layer) {
  RTC_LOG(INFO) << __FUNCTION__;
  return AudioDeviceModule::Create(audio_layer);
}

MyAudioDeviceModuleImpl::MyAudioDeviceModuleImpl(const AudioLayer audioLayer)
    : audio_layer_(audioLayer) {
  RTC_LOG(INFO) << __FUNCTION__;
}

int32_t MyAudioDeviceModuleImpl::CheckPlatform() {

  // Ensure that the current platform is supported
  PlatformType platform(kPlatformLinux);

  platform_type_ = platform;
  return 0;
}

int32_t MyAudioDeviceModuleImpl::CreatePlatformSpecificObjects() {
   audio_device_.reset(new VirtualFileAudioDevice());

  return 0;
}

int32_t MyAudioDeviceModuleImpl::AttachAudioBuffer() {
  RTC_LOG(INFO) << __FUNCTION__;
  audio_device_->AttachAudioBuffer(&audio_device_buffer_);
  return 0;
}

MyAudioDeviceModuleImpl::~MyAudioDeviceModuleImpl() {
  RTC_LOG(INFO) << __FUNCTION__;
}

int32_t MyAudioDeviceModuleImpl::ActiveAudioLayer(AudioLayer* audioLayer) const {
  RTC_LOG(INFO) << __FUNCTION__;
  AudioLayer activeAudio;
  if (audio_device_->ActiveAudioLayer(activeAudio) == -1) {
    return -1;
  }
  *audioLayer = activeAudio;
  return 0;
}

int32_t MyAudioDeviceModuleImpl::Init() {
  RTC_LOG(INFO) << __FUNCTION__;
  if (initialized_)
    return 0;
  RTC_CHECK(audio_device_);
  AudioDeviceGeneric::InitStatus status = audio_device_->Init();
  RTC_HISTOGRAM_ENUMERATION(
      "WebRTC.Audio.InitializationResult", static_cast<int>(status),
      static_cast<int>(AudioDeviceGeneric::InitStatus::NUM_STATUSES));
  if (status != AudioDeviceGeneric::InitStatus::OK) {
    RTC_LOG(LS_ERROR) << "Audio device initialization failed.";
    return -1;
  }
  initialized_ = true;
  return 0;
}

int32_t MyAudioDeviceModuleImpl::Terminate() {
  RTC_LOG(INFO) << __FUNCTION__;
  if (!initialized_)
    return 0;
  if (audio_device_->Terminate() == -1) {
    return -1;
  }
  initialized_ = false;
  return 0;
}

bool MyAudioDeviceModuleImpl::Initialized() const {
  RTC_LOG(INFO) << __FUNCTION__ << ": " << initialized_;
  return initialized_;
}

int32_t MyAudioDeviceModuleImpl::InitSpeaker() {
  RTC_LOG(INFO) << __FUNCTION__;
  CHECKinitialized_();
  return audio_device_->InitSpeaker();
}

int32_t MyAudioDeviceModuleImpl::InitMicrophone() {
  RTC_LOG(INFO) << __FUNCTION__;
  CHECKinitialized_();
  return audio_device_->InitMicrophone();
}

int32_t MyAudioDeviceModuleImpl::SpeakerVolumeIsAvailable(bool* available) {
  RTC_LOG(INFO) << __FUNCTION__;
  CHECKinitialized_();
  bool isAvailable = false;
  if (audio_device_->SpeakerVolumeIsAvailable(isAvailable) == -1) {
    return -1;
  }
  *available = isAvailable;
  RTC_LOG(INFO) << "output: " << isAvailable;
  return 0;
}

int32_t MyAudioDeviceModuleImpl::SetSpeakerVolume(uint32_t volume) {
  RTC_LOG(INFO) << __FUNCTION__ << "(" << volume << ")";
  CHECKinitialized_();
  return audio_device_->SetSpeakerVolume(volume);
}

int32_t MyAudioDeviceModuleImpl::SpeakerVolume(uint32_t* volume) const {
  RTC_LOG(INFO) << __FUNCTION__;
  CHECKinitialized_();
  uint32_t level = 0;
  if (audio_device_->SpeakerVolume(level) == -1) {
    return -1;
  }
  *volume = level;
  RTC_LOG(INFO) << "output: " << *volume;
  return 0;
}

bool MyAudioDeviceModuleImpl::SpeakerIsInitialized() const {
  RTC_LOG(INFO) << __FUNCTION__;
  CHECKinitialized__BOOL();
  bool isInitialized = audio_device_->SpeakerIsInitialized();
  RTC_LOG(INFO) << "output: " << isInitialized;
  return isInitialized;
}

bool MyAudioDeviceModuleImpl::MicrophoneIsInitialized() const {
  RTC_LOG(INFO) << __FUNCTION__;
  CHECKinitialized__BOOL();
  bool isInitialized = audio_device_->MicrophoneIsInitialized();
  RTC_LOG(INFO) << "output: " << isInitialized;
  return isInitialized;
}

int32_t MyAudioDeviceModuleImpl::MaxSpeakerVolume(uint32_t* maxVolume) const {
  CHECKinitialized_();
  uint32_t maxVol = 0;
  if (audio_device_->MaxSpeakerVolume(maxVol) == -1) {
    return -1;
  }
  *maxVolume = maxVol;
  return 0;
}

int32_t MyAudioDeviceModuleImpl::MinSpeakerVolume(uint32_t* minVolume) const {
  CHECKinitialized_();
  uint32_t minVol = 0;
  if (audio_device_->MinSpeakerVolume(minVol) == -1) {
    return -1;
  }
  *minVolume = minVol;
  return 0;
}

int32_t MyAudioDeviceModuleImpl::SpeakerMuteIsAvailable(bool* available) {
  RTC_LOG(INFO) << __FUNCTION__;
  CHECKinitialized_();
  bool isAvailable = false;
  if (audio_device_->SpeakerMuteIsAvailable(isAvailable) == -1) {
    return -1;
  }
  *available = isAvailable;
  RTC_LOG(INFO) << "output: " << isAvailable;
  return 0;
}

int32_t MyAudioDeviceModuleImpl::SetSpeakerMute(bool enable) {
  RTC_LOG(INFO) << __FUNCTION__ << "(" << enable << ")";
  CHECKinitialized_();
  return audio_device_->SetSpeakerMute(enable);
}

int32_t MyAudioDeviceModuleImpl::SpeakerMute(bool* enabled) const {
  RTC_LOG(INFO) << __FUNCTION__;
  CHECKinitialized_();
  bool muted = false;
  if (audio_device_->SpeakerMute(muted) == -1) {
    return -1;
  }
  *enabled = muted;
  RTC_LOG(INFO) << "output: " << muted;
  return 0;
}

int32_t MyAudioDeviceModuleImpl::MicrophoneMuteIsAvailable(bool* available) {
  RTC_LOG(INFO) << __FUNCTION__;
  CHECKinitialized_();
  bool isAvailable = false;
  if (audio_device_->MicrophoneMuteIsAvailable(isAvailable) == -1) {
    return -1;
  }
  *available = isAvailable;
  RTC_LOG(INFO) << "output: " << isAvailable;
  return 0;
}

int32_t MyAudioDeviceModuleImpl::SetMicrophoneMute(bool enable) {
  RTC_LOG(INFO) << __FUNCTION__ << "(" << enable << ")";
  CHECKinitialized_();
  return (audio_device_->SetMicrophoneMute(enable));
}

int32_t MyAudioDeviceModuleImpl::MicrophoneMute(bool* enabled) const {
  RTC_LOG(INFO) << __FUNCTION__;
  CHECKinitialized_();
  bool muted = false;
  if (audio_device_->MicrophoneMute(muted) == -1) {
    return -1;
  }
  *enabled = muted;
  RTC_LOG(INFO) << "output: " << muted;
  return 0;
}

int32_t MyAudioDeviceModuleImpl::MicrophoneVolumeIsAvailable(bool* available) {
  RTC_LOG(INFO) << __FUNCTION__;
  CHECKinitialized_();
  bool isAvailable = false;
  if (audio_device_->MicrophoneVolumeIsAvailable(isAvailable) == -1) {
    return -1;
  }
  *available = isAvailable;
  RTC_LOG(INFO) << "output: " << isAvailable;
  return 0;
}

int32_t MyAudioDeviceModuleImpl::SetMicrophoneVolume(uint32_t volume) {
  RTC_LOG(INFO) << __FUNCTION__ << "(" << volume << ")";
  CHECKinitialized_();
  return (audio_device_->SetMicrophoneVolume(volume));
}

int32_t MyAudioDeviceModuleImpl::MicrophoneVolume(uint32_t* volume) const {
  RTC_LOG(INFO) << __FUNCTION__;
  CHECKinitialized_();
  uint32_t level = 0;
  if (audio_device_->MicrophoneVolume(level) == -1) {
    return -1;
  }
  *volume = level;
  RTC_LOG(INFO) << "output: " << *volume;
  return 0;
}

int32_t MyAudioDeviceModuleImpl::StereoRecordingIsAvailable(
    bool* available) const {
  RTC_LOG(INFO) << __FUNCTION__;
  CHECKinitialized_();
  bool isAvailable = false;
  if (audio_device_->StereoRecordingIsAvailable(isAvailable) == -1) {
    return -1;
  }
  *available = isAvailable;
  RTC_LOG(INFO) << "output: " << isAvailable;
  return 0;
}

int32_t MyAudioDeviceModuleImpl::SetStereoRecording(bool enable) {
  RTC_LOG(INFO) << __FUNCTION__ << "(" << enable << ")";
  CHECKinitialized_();
  if (audio_device_->RecordingIsInitialized()) {
    RTC_LOG(WARNING) << "recording in stereo is not supported";
    return -1;
  }
  if (audio_device_->SetStereoRecording(enable) == -1) {
    RTC_LOG(WARNING) << "failed to change stereo recording";
    return -1;
  }
  int8_t nChannels(1);
  if (enable) {
    nChannels = 2;
  }
  audio_device_buffer_.SetRecordingChannels(nChannels);
  return 0;
}

int32_t MyAudioDeviceModuleImpl::StereoRecording(bool* enabled) const {
  RTC_LOG(INFO) << __FUNCTION__;
  CHECKinitialized_();
  bool stereo = false;
  if (audio_device_->StereoRecording(stereo) == -1) {
    return -1;
  }
  *enabled = stereo;
  RTC_LOG(INFO) << "output: " << stereo;
  return 0;
}

int32_t MyAudioDeviceModuleImpl::StereoPlayoutIsAvailable(bool* available) const {
  RTC_LOG(INFO) << __FUNCTION__;
  CHECKinitialized_();
  bool isAvailable = false;
  if (audio_device_->StereoPlayoutIsAvailable(isAvailable) == -1) {
    return -1;
  }
  *available = isAvailable;
  RTC_LOG(INFO) << "output: " << isAvailable;
  return 0;
}

int32_t MyAudioDeviceModuleImpl::SetStereoPlayout(bool enable) {
  RTC_LOG(INFO) << __FUNCTION__ << "(" << enable << ")";
  CHECKinitialized_();
  if (audio_device_->PlayoutIsInitialized()) {
    RTC_LOG(LERROR)
        << "unable to set stereo mode while playing side is initialized";
    return -1;
  }
  if (audio_device_->SetStereoPlayout(enable)) {
    RTC_LOG(WARNING) << "stereo playout is not supported";
    return -1;
  }
  int8_t nChannels(1);
  if (enable) {
    nChannels = 2;
  }
  audio_device_buffer_.SetPlayoutChannels(nChannels);
  return 0;
}

int32_t MyAudioDeviceModuleImpl::StereoPlayout(bool* enabled) const {
  RTC_LOG(INFO) << __FUNCTION__;
  CHECKinitialized_();
  bool stereo = false;
  if (audio_device_->StereoPlayout(stereo) == -1) {
    return -1;
  }
  *enabled = stereo;
  RTC_LOG(INFO) << "output: " << stereo;
  return 0;
}

int32_t MyAudioDeviceModuleImpl::PlayoutIsAvailable(bool* available) {
  RTC_LOG(INFO) << __FUNCTION__;
  CHECKinitialized_();
  bool isAvailable = false;
  if (audio_device_->PlayoutIsAvailable(isAvailable) == -1) {
    return -1;
  }
  *available = isAvailable;
  RTC_LOG(INFO) << "output: " << isAvailable;
  return 0;
}

int32_t MyAudioDeviceModuleImpl::RecordingIsAvailable(bool* available) {
  RTC_LOG(INFO) << __FUNCTION__;
  CHECKinitialized_();
  bool isAvailable = false;
  if (audio_device_->RecordingIsAvailable(isAvailable) == -1) {
    return -1;
  }
  *available = isAvailable;
  RTC_LOG(INFO) << "output: " << isAvailable;
  return 0;
}

int32_t MyAudioDeviceModuleImpl::MaxMicrophoneVolume(uint32_t* maxVolume) const {
  CHECKinitialized_();
  uint32_t maxVol(0);
  if (audio_device_->MaxMicrophoneVolume(maxVol) == -1) {
    return -1;
  }
  *maxVolume = maxVol;
  return 0;
}

int32_t MyAudioDeviceModuleImpl::MinMicrophoneVolume(uint32_t* minVolume) const {
  CHECKinitialized_();
  uint32_t minVol(0);
  if (audio_device_->MinMicrophoneVolume(minVol) == -1) {
    return -1;
  }
  *minVolume = minVol;
  return 0;
}

int16_t MyAudioDeviceModuleImpl::PlayoutDevices() {
  RTC_LOG(INFO) << __FUNCTION__;
  CHECKinitialized_();
  uint16_t nPlayoutDevices = audio_device_->PlayoutDevices();
  RTC_LOG(INFO) << "output: " << nPlayoutDevices;
  return (int16_t)(nPlayoutDevices);
}

int32_t MyAudioDeviceModuleImpl::SetPlayoutDevice(uint16_t index) {
  RTC_LOG(INFO) << __FUNCTION__ << "(" << index << ")";
  CHECKinitialized_();
  return audio_device_->SetPlayoutDevice(index);
}

int32_t MyAudioDeviceModuleImpl::SetPlayoutDevice(WindowsDeviceType device) {
  RTC_LOG(INFO) << __FUNCTION__;
  CHECKinitialized_();
  return audio_device_->SetPlayoutDevice(device);
}

int32_t MyAudioDeviceModuleImpl::PlayoutDeviceName(
    uint16_t index,
    char name[kAdmMaxDeviceNameSize],
    char guid[kAdmMaxGuidSize]) {
  RTC_LOG(INFO) << __FUNCTION__ << "(" << index << ", ...)";
  CHECKinitialized_();
  if (name == NULL) {
    return -1;
  }
  if (audio_device_->PlayoutDeviceName(index, name, guid) == -1) {
    return -1;
  }
  if (name != NULL) {
    RTC_LOG(INFO) << "output: name = " << name;
  }
  if (guid != NULL) {
    RTC_LOG(INFO) << "output: guid = " << guid;
  }
  return 0;
}

int32_t MyAudioDeviceModuleImpl::RecordingDeviceName(
    uint16_t index,
    char name[kAdmMaxDeviceNameSize],
    char guid[kAdmMaxGuidSize]) {
  RTC_LOG(INFO) << __FUNCTION__ << "(" << index << ", ...)";
  CHECKinitialized_();
  if (name == NULL) {
    return -1;
  }
  if (audio_device_->RecordingDeviceName(index, name, guid) == -1) {
    return -1;
  }
  if (name != NULL) {
    RTC_LOG(INFO) << "output: name = " << name;
  }
  if (guid != NULL) {
    RTC_LOG(INFO) << "output: guid = " << guid;
  }
  return 0;
}

int16_t MyAudioDeviceModuleImpl::RecordingDevices() {
  RTC_LOG(INFO) << __FUNCTION__;
  CHECKinitialized_();
  uint16_t nRecordingDevices = audio_device_->RecordingDevices();
  RTC_LOG(INFO) << "output: " << nRecordingDevices;
  return (int16_t)nRecordingDevices;
}

int32_t MyAudioDeviceModuleImpl::SetRecordingDevice(uint16_t index) {
  RTC_LOG(INFO) << __FUNCTION__ << "(" << index << ")";
  CHECKinitialized_();
  return audio_device_->SetRecordingDevice(index);
}

int32_t MyAudioDeviceModuleImpl::SetRecordingDevice(WindowsDeviceType device) {
  RTC_LOG(INFO) << __FUNCTION__;
  CHECKinitialized_();
  return audio_device_->SetRecordingDevice(device);
}

int32_t MyAudioDeviceModuleImpl::InitPlayout() {
  RTC_LOG(INFO) << __FUNCTION__;
  CHECKinitialized_();
  if (PlayoutIsInitialized()) {
    return 0;
  }
  int32_t result = audio_device_->InitPlayout();
  RTC_LOG(INFO) << "output: " << result;
  RTC_HISTOGRAM_BOOLEAN("WebRTC.Audio.InitPlayoutSuccess",
                        static_cast<int>(result == 0));
  return result;
}

int32_t MyAudioDeviceModuleImpl::InitRecording() {
  RTC_LOG(INFO) << __FUNCTION__;
  CHECKinitialized_();
  if (RecordingIsInitialized()) {
    return 0;
  }
  int32_t result = audio_device_->InitRecording();
  RTC_LOG(INFO) << "output: " << result;
  RTC_HISTOGRAM_BOOLEAN("WebRTC.Audio.InitRecordingSuccess",
                        static_cast<int>(result == 0));
  return result;
}

bool MyAudioDeviceModuleImpl::PlayoutIsInitialized() const {
  RTC_LOG(INFO) << __FUNCTION__;
  CHECKinitialized__BOOL();
  return audio_device_->PlayoutIsInitialized();
}

bool MyAudioDeviceModuleImpl::RecordingIsInitialized() const {
  RTC_LOG(INFO) << __FUNCTION__;
  CHECKinitialized__BOOL();
  return audio_device_->RecordingIsInitialized();
}

int32_t MyAudioDeviceModuleImpl::StartPlayout() {
  RTC_LOG(INFO) << __FUNCTION__;
  CHECKinitialized_();
  if (Playing()) {
    return 0;
  }
  audio_device_buffer_.StartPlayout();
  int32_t result = audio_device_->StartPlayout();
  RTC_LOG(INFO) << "output: " << result;
  RTC_HISTOGRAM_BOOLEAN("WebRTC.Audio.StartPlayoutSuccess",
                        static_cast<int>(result == 0));
  return result;
}

int32_t MyAudioDeviceModuleImpl::StopPlayout() {
  RTC_LOG(INFO) << __FUNCTION__;
  CHECKinitialized_();
  int32_t result = audio_device_->StopPlayout();
  audio_device_buffer_.StopPlayout();
  RTC_LOG(INFO) << "output: " << result;
  RTC_HISTOGRAM_BOOLEAN("WebRTC.Audio.StopPlayoutSuccess",
                        static_cast<int>(result == 0));
  return result;
}

bool MyAudioDeviceModuleImpl::Playing() const {
  RTC_LOG(INFO) << __FUNCTION__;
  CHECKinitialized__BOOL();
  return audio_device_->Playing();
}

int32_t MyAudioDeviceModuleImpl::StartRecording() {
  RTC_LOG(INFO) << __FUNCTION__;
  CHECKinitialized_();
  if (Recording()) {
    return 0;
  }
  audio_device_buffer_.StartRecording();
  int32_t result = audio_device_->StartRecording();
  RTC_LOG(INFO) << "output: " << result;
  RTC_HISTOGRAM_BOOLEAN("WebRTC.Audio.StartRecordingSuccess",
                        static_cast<int>(result == 0));
  return result;
}

int32_t MyAudioDeviceModuleImpl::StopRecording() {
  RTC_LOG(INFO) << __FUNCTION__;
  CHECKinitialized_();
  int32_t result = audio_device_->StopRecording();
  audio_device_buffer_.StopRecording();
  RTC_LOG(INFO) << "output: " << result;
  RTC_HISTOGRAM_BOOLEAN("WebRTC.Audio.StopRecordingSuccess",
                        static_cast<int>(result == 0));
  return result;
}

bool MyAudioDeviceModuleImpl::Recording() const {
  RTC_LOG(INFO) << __FUNCTION__;
  CHECKinitialized__BOOL();
  return audio_device_->Recording();
}

int32_t MyAudioDeviceModuleImpl::RegisterAudioCallback(
    AudioTransport* audioCallback) {
  RTC_LOG(INFO) << __FUNCTION__;
  return audio_device_buffer_.RegisterAudioCallback(audioCallback);
}

int32_t MyAudioDeviceModuleImpl::PlayoutDelay(uint16_t* delayMS) const {
  CHECKinitialized_();
  uint16_t delay = 0;
  if (audio_device_->PlayoutDelay(delay) == -1) {
    RTC_LOG(LERROR) << "failed to retrieve the playout delay";
    return -1;
  }
  *delayMS = delay;
  return 0;
}

bool MyAudioDeviceModuleImpl::BuiltInAECIsAvailable() const {
  RTC_LOG(INFO) << __FUNCTION__;
  CHECKinitialized__BOOL();
  bool isAvailable = audio_device_->BuiltInAECIsAvailable();
  RTC_LOG(INFO) << "output: " << isAvailable;
  return isAvailable;
}

int32_t MyAudioDeviceModuleImpl::EnableBuiltInAEC(bool enable) {
  RTC_LOG(INFO) << __FUNCTION__ << "(" << enable << ")";
  CHECKinitialized_();
  int32_t ok = audio_device_->EnableBuiltInAEC(enable);
  RTC_LOG(INFO) << "output: " << ok;
  return ok;
}

bool MyAudioDeviceModuleImpl::BuiltInAGCIsAvailable() const {
  RTC_LOG(INFO) << __FUNCTION__;
  CHECKinitialized__BOOL();
  bool isAvailable = audio_device_->BuiltInAGCIsAvailable();
  RTC_LOG(INFO) << "output: " << isAvailable;
  return isAvailable;
}

int32_t MyAudioDeviceModuleImpl::EnableBuiltInAGC(bool enable) {
  RTC_LOG(INFO) << __FUNCTION__ << "(" << enable << ")";
  CHECKinitialized_();
  int32_t ok = audio_device_->EnableBuiltInAGC(enable);
  RTC_LOG(INFO) << "output: " << ok;
  return ok;
}

bool MyAudioDeviceModuleImpl::BuiltInNSIsAvailable() const {
  RTC_LOG(INFO) << __FUNCTION__;
  CHECKinitialized__BOOL();
  bool isAvailable = audio_device_->BuiltInNSIsAvailable();
  RTC_LOG(INFO) << "output: " << isAvailable;
  return isAvailable;
}

int32_t MyAudioDeviceModuleImpl::EnableBuiltInNS(bool enable) {
  RTC_LOG(INFO) << __FUNCTION__ << "(" << enable << ")";
  CHECKinitialized_();
  int32_t ok = audio_device_->EnableBuiltInNS(enable);
  RTC_LOG(INFO) << "output: " << ok;
  return ok;
}

MyAudioDeviceModuleImpl::PlatformType MyAudioDeviceModuleImpl::Platform() const {
  RTC_LOG(INFO) << __FUNCTION__;
  return platform_type_;
}

AudioDeviceModule::AudioLayer MyAudioDeviceModuleImpl::PlatformAudioLayer()
    const {
  RTC_LOG(INFO) << __FUNCTION__;
  return audio_layer_;
}

}  // namespace webrtc

/*
void CustomAudioDeviceModule::newFrameAvailable(int sample_count) {
    VirtualFileAudioDevice* audioDevice = (VirtualFileAudioDevice*)audio_device_.get();
    audioDevice->NewFrameAvailable(sample_count);
}

bool CustomAudioDeviceModule::WriteAudioFrame(int8_t* data, size_t sample_count) {
    //std::cout << __FUNCTION__ << rtc::TimeMillis();
    VirtualFileAudioDevice* audioDevice = (VirtualFileAudioDevice*)audio_device_.get();
    return audioDevice->WriteAudioFrame(data, sample_count);
}
*/
