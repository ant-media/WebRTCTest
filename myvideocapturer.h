/*
 *  Copyright (c) 2004 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef MYVIDEOCAPTURER_H_
#define MYVIDEOCAPTURER_H_

#include <string.h>

#include <memory>
#include <vector>

#include "api/video/i420_buffer.h"
#include "api/video/video_frame.h"
#include "media/base/videocapturer.h"
#include "media/base/videocommon.h"
#include "rtc_base/task_queue_for_test.h"
#include "rtc_base/timeutils.h"
#include <rtc_base/thread.h>

namespace cricket {

// My video capturer that allows the test to manually pump in frames.
class MyVideoCapturer : public cricket::VideoCapturer {
 public:
  explicit MyVideoCapturer(bool is_screencast);
  MyVideoCapturer();

  ~MyVideoCapturer() override;

  void ResetSupportedFormats(const std::vector<cricket::VideoFormat>& formats);
  sigslot::signal1<MyVideoCapturer*> SignalDestroyed;

  cricket::CaptureState Start(const cricket::VideoFormat& format) override;
  void Stop() override;
  bool IsRunning() override;
  bool IsScreencast() const override;
  bool GetPreferredFourccs(std::vector<uint32_t>* fourccs) override;


  void VideoCaptured(unsigned char* data, int width, int height);

  const VideoFormat* GetCaptureFormat();
  rtc::Thread*  m_startThread; //video capture thread

 private:
  bool CaptureFrame(const webrtc::VideoFrame& frame);

  bool running_;
  const bool is_screencast_;
  // Duplicates MyFrameSource::rotation_, but needed to support
  // SetRotation before Start.
  VideoFormat *videoFormat;
};

}  // namespace cricket


#endif
