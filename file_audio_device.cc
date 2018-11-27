/*
 *  Copyright (c) 2014 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include "rtc_base/checks.h"
#include "rtc_base/logging.h"
#include "rtc_base/platform_thread.h"
#include "file_audio_device.h"
#include "system_wrappers/include/sleep.h"
#include "rtc_base/thread.h"
#include <iostream>

namespace webrtc {

const int kRecordingFixedSampleRate = 48000;
const size_t kRecordingNumChannels = 2;
const int kPlayoutFixedSampleRate = 48000;
const size_t kPlayoutNumChannels = 2;
const size_t kPlayoutBufferSize =
        kPlayoutFixedSampleRate / 100 * kPlayoutNumChannels * 2;
const size_t kRecordingBufferSize =
        kRecordingFixedSampleRate / 100 * kRecordingNumChannels * 2;

VirtualFileAudioDevice::VirtualFileAudioDevice():
                                            _ptrAudioBuffer(NULL),
                                            _recordingBuffer(NULL),
                                            _playoutBuffer(NULL),
                                            _recordingFramesLeft(0),
                                            _playoutFramesLeft(0),
                                            _recordingBufferSizeIn10MS(0),
                                            _recordingFramesIn10MS(0),
                                            _playoutFramesIn10MS(0),
                                            _playing(false),
                                            _recording(false),
                                            _playIsInitialized(false),
                                            _lastCallPlayoutMillis(0),
                                            _lastCallRecordMillis(0)
{
}

VirtualFileAudioDevice::~VirtualFileAudioDevice() {

}

int32_t VirtualFileAudioDevice::ActiveAudioLayer(
        AudioDeviceModule::AudioLayer& audioLayer) const {
      RTC_LOG(INFO) << __FUNCTION__;

    return -1;
}

AudioDeviceGeneric::InitStatus VirtualFileAudioDevice::Init() {
      RTC_LOG(INFO) << __FUNCTION__;

    return InitStatus::OK;
}

int32_t VirtualFileAudioDevice::Terminate() {
      RTC_LOG(INFO) << __FUNCTION__;

    return 0;
}

bool VirtualFileAudioDevice::Initialized() const {
      RTC_LOG(INFO) << __FUNCTION__;

    return true;
}

int16_t VirtualFileAudioDevice::PlayoutDevices() {
      RTC_LOG(INFO) << __FUNCTION__;

    return 1;
}

int16_t VirtualFileAudioDevice::RecordingDevices() {
      RTC_LOG(INFO) << __FUNCTION__;

    return 1;
}

int32_t VirtualFileAudioDevice::PlayoutDeviceName(uint16_t index,
        char name[kAdmMaxDeviceNameSize],
        char guid[kAdmMaxGuidSize]) {
      RTC_LOG(INFO) << __FUNCTION__;

    const char* kName = "dummy_device";
    const char* kGuid = "dummy_device_unique_id";
    if (index < 1) {
        memset(name, 0, kAdmMaxDeviceNameSize);
        memset(guid, 0, kAdmMaxGuidSize);
        memcpy(name, kName, strlen(kName));
        memcpy(guid, kGuid, strlen(guid));
        return 0;
    }
    return -1;
}

int32_t VirtualFileAudioDevice::RecordingDeviceName(uint16_t index,
        char name[kAdmMaxDeviceNameSize],
        char guid[kAdmMaxGuidSize]) {
      RTC_LOG(INFO) << __FUNCTION__;

    const char* kName = "dummy_device";
    const char* kGuid = "dummy_device_unique_id";
    if (index < 1) {
        memset(name, 0, kAdmMaxDeviceNameSize);
        memset(guid, 0, kAdmMaxGuidSize);
        memcpy(name, kName, strlen(kName));
        memcpy(guid, kGuid, strlen(guid));
        return 0;
    }
    return -1;
}

int32_t VirtualFileAudioDevice::SetPlayoutDevice(uint16_t index) {
      RTC_LOG(INFO) << __FUNCTION__;

    if (index == 0) {
        _playout_index = index;
        return 0;
    }
    return -1;
}

int32_t VirtualFileAudioDevice::SetPlayoutDevice(
        AudioDeviceModule::WindowsDeviceType device) {
      RTC_LOG(INFO) << __FUNCTION__;

    return -1;
}

int32_t VirtualFileAudioDevice::SetRecordingDevice(uint16_t index) {
    std::cout << __FUNCTION__ << " index: " << index;
    if (index == 0) {
        _record_index = index;
        return _record_index;
    }
    return -1;
}

int32_t VirtualFileAudioDevice::SetRecordingDevice(
        AudioDeviceModule::WindowsDeviceType device) {
      RTC_LOG(INFO) << __FUNCTION__;

    return -1;
}

int32_t VirtualFileAudioDevice::PlayoutIsAvailable(bool& available) {
      RTC_LOG(INFO) << __FUNCTION__;

    if (_playout_index == 0) {
        available = true;
        return _playout_index;
    }
    available = false;
    return -1;
}

int32_t VirtualFileAudioDevice::InitPlayout() {
      RTC_LOG(INFO) << __FUNCTION__;

    if (_ptrAudioBuffer) {
        // Update webrtc audio buffer with the selected parameters
        _ptrAudioBuffer->SetPlayoutSampleRate(kPlayoutFixedSampleRate);
        _ptrAudioBuffer->SetPlayoutChannels(kPlayoutNumChannels);
    }
    return 0;
}

bool VirtualFileAudioDevice::PlayoutIsInitialized() const {
      RTC_LOG(INFO) << __FUNCTION__;

    return _playIsInitialized;
}

int32_t VirtualFileAudioDevice::RecordingIsAvailable(bool& available) {
      RTC_LOG(INFO) << __FUNCTION__;

    if (_record_index == 0) {
        available = true;
        return _record_index;
    }
    available = false;
    return -1;
}

int32_t VirtualFileAudioDevice::InitRecording() {
      RTC_LOG(INFO) << __FUNCTION__;

    rtc::CritScope lock(&_critSect);

    if (_recording) {
        return -1;
    }

    _recordingFramesIn10MS = static_cast<size_t>(kRecordingFixedSampleRate / 100);

    if (_ptrAudioBuffer) {
        _ptrAudioBuffer->SetRecordingSampleRate(kRecordingFixedSampleRate);
        _ptrAudioBuffer->SetRecordingChannels(kRecordingNumChannels);
    }
    return 0;
}

bool VirtualFileAudioDevice::RecordingIsInitialized() const {
      RTC_LOG(INFO) << __FUNCTION__;

    return _recordingFramesIn10MS != 0;
}

int32_t VirtualFileAudioDevice::StartPlayout() {
      RTC_LOG(INFO) << __FUNCTION__;

    if (_playing) {
        return 0;
    }

    _playoutFramesIn10MS = static_cast<size_t>(kPlayoutFixedSampleRate / 100);
    _playing = true;
    _playoutFramesLeft = 0;

    if (!_playoutBuffer) {
        _playoutBuffer = new int8_t[kPlayoutBufferSize];
    }
    if (!_playoutBuffer) {
        _playing = false;
        return -1;
    }

    // PLAYOUT

    _ptrThreadPlay.reset(new rtc::PlatformThread(
            PlayThreadFunc, this, "webrtc_audio_module_play_thread"));
    _ptrThreadPlay->Start();
    _ptrThreadPlay->SetPriority(rtc::kRealtimePriority);

    std::cout << "Started playout capture: ";
    return 0;
}

int32_t VirtualFileAudioDevice::StopPlayout() {
    {
          RTC_LOG(INFO) << __FUNCTION__;

        rtc::CritScope lock(&_critSect);
        _playing = false;
    }

    // stop playout thread first
    if (_ptrThreadPlay) {
        _ptrThreadPlay->Stop();
        _ptrThreadPlay.reset();
    }

    rtc::CritScope lock(&_critSect);

    _playoutFramesLeft = 0;
    delete [] _playoutBuffer;
    _playoutBuffer = NULL;


    std::cout << "Stopped playout capture ";

    _playIsInitialized = false;
    return 0;
}

bool VirtualFileAudioDevice::Playing() const {
      RTC_LOG(INFO) << __FUNCTION__;

    return _playing;
}

int32_t VirtualFileAudioDevice::StartRecording() {
      RTC_LOG(INFO) << __FUNCTION__;

    _recording = true;

    // Make sure we only create the buffer once.
    _recordingBufferSizeIn10MS = _recordingFramesIn10MS *
            kRecordingNumChannels *
            2;
    if (!_recordingBuffer) {
        _recordingBuffer = new int8_t[_recordingBufferSizeIn10MS];
    }


/*
    _ptrThreadRec.reset(new rtc::PlatformThread(
            RecThreadFunc, this, "webrtc_audio_module_capture_thread"));

    _ptrThreadRec->Start();
    _ptrThreadRec->SetPriority(rtc::kRealtimePriority);
*/
    std::cout << "Started recording ";

    return 0;
}


int32_t VirtualFileAudioDevice::StopRecording() {
    uint64_t now = rtc::TimeMillis();
    {
          RTC_LOG(INFO) << __FUNCTION__;

        rtc::CritScope lock(&_critSect);
        _recording = false;
    }

    std::cout << "stop recording 1. interval " << rtc::TimeMillis() - now ;

     now = rtc::TimeMillis();
    if (_ptrThreadRec) {
        _ptrThreadRec->Stop();
        _ptrThreadRec.reset();
    }


    rtc::CritScope lock(&_critSect);
    _recordingFramesLeft = 0;
    if (_recordingBuffer) {
        delete [] _recordingBuffer;
        _recordingBuffer = NULL;
    }
    std::cout << "stop recording 2. interval " << rtc::TimeMillis() - now ;

    std::cout << "Stopped recording "
            << std::endl;
    return 0;
}

bool VirtualFileAudioDevice::Recording() const {
      RTC_LOG(INFO) << __FUNCTION__;

    return _recording;
}

int32_t VirtualFileAudioDevice::InitSpeaker() {
      RTC_LOG(INFO) << __FUNCTION__;

    return 0;
}

bool VirtualFileAudioDevice::SpeakerIsInitialized() const {
      RTC_LOG(INFO) << __FUNCTION__;

    return false;
}

int32_t VirtualFileAudioDevice::InitMicrophone() {   RTC_LOG(INFO) << __FUNCTION__;

return 0; }

bool VirtualFileAudioDevice::MicrophoneIsInitialized() const {
      RTC_LOG(INFO) << __FUNCTION__;

    return true; }

int32_t VirtualFileAudioDevice::SpeakerVolumeIsAvailable(bool& available) {
      RTC_LOG(INFO) << __FUNCTION__;

    return -1;
}

int32_t VirtualFileAudioDevice::SetSpeakerVolume(uint32_t volume) {
      RTC_LOG(INFO) << __FUNCTION__;

    return -1; }

int32_t VirtualFileAudioDevice::SpeakerVolume(uint32_t& volume) const {   RTC_LOG(INFO) << __FUNCTION__;

return -1; }

int32_t VirtualFileAudioDevice::MaxSpeakerVolume(uint32_t& maxVolume) const {
      RTC_LOG(INFO) << __FUNCTION__;

    return -1;
}

int32_t VirtualFileAudioDevice::MinSpeakerVolume(uint32_t& minVolume) const {
      RTC_LOG(INFO) << __FUNCTION__;

    return -1;
}


int32_t VirtualFileAudioDevice::MicrophoneVolumeIsAvailable(bool& available) {
      RTC_LOG(INFO) << __FUNCTION__;

    return -1;
}

int32_t VirtualFileAudioDevice::SetMicrophoneVolume(uint32_t volume) {   RTC_LOG(INFO) << __FUNCTION__;

return -1; }

int32_t VirtualFileAudioDevice::MicrophoneVolume(uint32_t& volume) const {
      RTC_LOG(INFO) << __FUNCTION__;

    return -1;
}

int32_t VirtualFileAudioDevice::MaxMicrophoneVolume(uint32_t& maxVolume) const {
      RTC_LOG(INFO) << __FUNCTION__;

    return -1;
}

int32_t VirtualFileAudioDevice::MinMicrophoneVolume(uint32_t& minVolume) const {
      RTC_LOG(INFO) << __FUNCTION__;

    return -1;
}


int32_t VirtualFileAudioDevice::SpeakerMuteIsAvailable(bool& available) {
      RTC_LOG(INFO) << __FUNCTION__;

    return -1; }

int32_t VirtualFileAudioDevice::SetSpeakerMute(bool enable) {
      RTC_LOG(INFO) << __FUNCTION__;

    return -1; }

int32_t VirtualFileAudioDevice::SpeakerMute(bool& enabled) const {
      RTC_LOG(INFO) << __FUNCTION__;

    return -1; }

int32_t VirtualFileAudioDevice::MicrophoneMuteIsAvailable(bool& available) {
      RTC_LOG(INFO) << __FUNCTION__;

    return -1;
}

int32_t VirtualFileAudioDevice::SetMicrophoneMute(bool enable) {
      RTC_LOG(INFO) << __FUNCTION__;

    return -1; }

int32_t VirtualFileAudioDevice::MicrophoneMute(bool& enabled) const {
      RTC_LOG(INFO) << __FUNCTION__;

    return -1; }

int32_t VirtualFileAudioDevice::StereoPlayoutIsAvailable(bool& available) {
      RTC_LOG(INFO) << __FUNCTION__;

    available = true;
    return 0;
}
int32_t VirtualFileAudioDevice::SetStereoPlayout(bool enable) {
      RTC_LOG(INFO) << __FUNCTION__;

    return 0;
}

int32_t VirtualFileAudioDevice::StereoPlayout(bool& enabled) const {
      RTC_LOG(INFO) << __FUNCTION__;

    enabled = true;
    return 0;
}

int32_t VirtualFileAudioDevice::StereoRecordingIsAvailable(bool& available) {
      RTC_LOG(INFO) << __FUNCTION__;

    available = true;
    return 0;
}

int32_t VirtualFileAudioDevice::SetStereoRecording(bool enable) {
      RTC_LOG(INFO) << __FUNCTION__;

    return 0;
}

int32_t VirtualFileAudioDevice::StereoRecording(bool& enabled) const {
      RTC_LOG(INFO) << __FUNCTION__;

    enabled = true;
    return 0;
}

int32_t VirtualFileAudioDevice::PlayoutDelay(uint16_t& delayMS) const {
    //  RTC_LOG(INFO) << __FUNCTION__;

    //delayMS = 25;
    return 0;
}

void VirtualFileAudioDevice::AttachAudioBuffer(AudioDeviceBuffer* audioBuffer) {
      RTC_LOG(INFO) << __FUNCTION__;

    rtc::CritScope lock(&_critSect);

    _ptrAudioBuffer = audioBuffer;

    // Inform the AudioBuffer about default settings for this implementation.
    // Set all values to zero here since the actual settings will be done by
    // InitPlayout and InitRecording later.
    _ptrAudioBuffer->SetRecordingSampleRate(0);
    _ptrAudioBuffer->SetPlayoutSampleRate(0);
    _ptrAudioBuffer->SetRecordingChannels(0);
    _ptrAudioBuffer->SetPlayoutChannels(0);
}

bool VirtualFileAudioDevice::PlayThreadFunc(void* pThis)
{
    return (static_cast<VirtualFileAudioDevice*>(pThis)->PlayThreadProcess());
}

bool VirtualFileAudioDevice::RecThreadFunc(void* pThis)
{
    return (static_cast<VirtualFileAudioDevice*>(pThis)->RecThreadProcess());
}

void SleepMicroSeconds(int microSecs) {
#ifdef _WIN32
  //Sleep(msecs);
#else
  struct timespec short_wait;
  struct timespec remainder;
  short_wait.tv_sec = microSecs / 1000000;
  short_wait.tv_nsec = (microSecs % 1000000) * 1000;
  nanosleep(&short_wait, &remainder);
#endif
}

bool VirtualFileAudioDevice::PlayThreadProcess()
{
    //std::cout << " PlayThreadProcess";

    if (!_playing) {
        return false;
    }

    int64_t currentTime = rtc::TimeMicros();
    _critSect.Enter();

    int64_t timeDiff = currentTime - _lastCallPlayoutMillis;
    if (_lastCallPlayoutMillis == 0 || timeDiff >= 10000)
    {
         //std::cout << " request playout data time diff: " << timeDiff << std::endl;
        _critSect.Leave();
        _ptrAudioBuffer->RequestPlayoutData(_playoutFramesIn10MS);
        _critSect.Enter();
        _lastCallPlayoutMillis = currentTime;
    }
    _critSect.Leave();

    int64_t nextCallTime = _lastCallPlayoutMillis + 10000;
    int64_t waitTime = nextCallTime - rtc::TimeMicros();

    if (waitTime > 0)
    {
        SleepMicroSeconds(waitTime);
    }


    return true;
}

bool VirtualFileAudioDevice::WriteAudioFrame(int8_t* data, size_t sample_count) {

    if (!_recording) {
        return false;
    }

    //int64_t currentTime = rtc::TimeMillis();
    _critSect.Enter();

    //if (_lastCallRecordMillis == 0 || currentTime - _lastCallRecordMillis >= 10)
    {
        //if (_inputFile.is_open())
        {
            /*
            if (_inputFile.Read(_recordingBuffer, kRecordingBufferSize) > 0) {
                _ptrAudioBuffer->SetRecordedBuffer(_recordingBuffer,
                        _recordingFramesIn10MS);
            } else {
                _inputFile.Rewind();
            }
             */
            _ptrAudioBuffer->SetRecordedBuffer(data,
                    sample_count);
            //_lastCallRecordMillis = currentTime;
            _critSect.Leave();
            _ptrAudioBuffer->DeliverRecordedData();


            //			_critSect.Enter();
        }
    }

    //	_critSect.Leave();

    //int64_t deltaTimeMillis = rtc::TimeMillis() - currentTime;
    //if (deltaTimeMillis < 10) {
    //	SleepMs(10 - deltaTimeMillis);
    //}

    return true;

}

void VirtualFileAudioDevice::NewFrameAvailable(int sampleCount) {
    _critSect.Enter();
    if (!_recording) {
        std::cout <<  " dropping audio because it is not recording ";
        _critSect.Leave();
        return;
    }
    this->newFrameSampleCount = sampleCount;

    int frameCount = newFrameSampleCount / _recordingFramesIn10MS;

    for (int i = 0; i < frameCount; i++) {
        _ptrAudioBuffer->SetRecordedBuffer(_recordingBuffer, _recordingFramesIn10MS);
        _lastCallRecordMillis = 0; //currentTime;
            //		newFrameSampleCount -= _recordingFramesIn10MS;
        _critSect.Leave();
        _ptrAudioBuffer->DeliverRecordedData();
        _critSect.Enter();
        //check recording state again because
        // recording can be set to false in the interval between mutex is released and lock above
        if (!_recording) {
            std::cout <<  " breaking audio loop because it is not recording ";
            break;
        }
    }

    _critSect.Leave();
}

bool VirtualFileAudioDevice::RecThreadProcess()
{
    //  RTC_LOG(INFO) << __FUNCTION__;

    if (!_recording) {
        return false;
    }

    //int64_t currentTime = rtc::TimeMillis();
    _critSect.Enter();



    if (newFrameSampleCount != 0) {

        //std::cout <<  " new frame sample count: " <<newFrameSampleCount << " time is: " << rtc::TimeMillis();
        //if (_inputFile.is_open())
        {
            /*
            if (_inputFile.Read(_recordingBuffer, kRecordingBufferSize) > 0) {
                _ptrAudioBuffer->SetRecordedBuffer(_recordingBuffer,
                        _recordingFramesIn10MS);
            } else {
                _inputFile.Rewind();
            }
             */

            /*
            int frameCount = newFrameSampleCount / _recordingFramesIn10MS;

            for (int i = 0; i < frameCount; i++) {
                _ptrAudioBuffer->SetRecordedBuffer(_recordingBuffer, _recordingFramesIn10MS);
                _lastCallRecordMillis = 0; //currentTime;
                newFrameSampleCount -= _recordingFramesIn10MS;
                _critSect.Leave();
                _ptrAudioBuffer->DeliverRecordedData();
                _critSect.Enter();
            }
            */

            //std::cout << "leaving loop sample count:" << newFrameSampleCount << " recordingFamesIn10Ms " << _recordingFramesIn10MS;
        }
    }

    _critSect.Leave();

    SleepMicroSeconds(100);

    return true;
}

}  // namespace webrtc
