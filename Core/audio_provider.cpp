/* Copyright 2018 The TensorFlow Authors. All Rights Reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
==============================================================================*/

#include "audio_provider.h"
#include "uart_utils.h"
#include "micro_features_micro_model_settings.h"
#include <cstdio> 
namespace {
int16_t g_dummy_audio_data[kMaxAudioSampleSize];
int32_t g_latest_audio_timestamp = 0;
}  // namespace


constexpr int kAudioCaptureBufferSize = 512; // for 1 second of 16kHz audio
int16_t audio_capture_buffer[kAudioCaptureBufferSize];
volatile int audio_capture_write_index = 0;


TfLiteStatus GetAudioSamples(tflite::ErrorReporter* error_reporter,
  int start_ms, int duration_ms,
  int* audio_samples_size, int16_t** audio_samples) {
    const int sample_rate = 16000;  // Hz
    const int length = 512;         // what model expects
    const int start_offset = (start_ms * sample_rate) / 1000;
    
    // Handle wrap-around
    int capture_start = (audio_capture_write_index - length + kAudioCaptureBufferSize) % kAudioCaptureBufferSize;
    
    *audio_samples = &audio_capture_buffer[capture_start];
    *audio_samples_size = length;

printf("Audio buffer content: %d %d %d %d...\n", audio_capture_buffer[0], audio_capture_buffer[1], audio_capture_buffer[2], audio_capture_buffer[3]);
return kTfLiteOk;
}
void ProcessAudioData(uint8_t* data, int length) {
  // Convert the 8-bit DMA buffer to 16-bit audio samples and store in the capture buffer
  for (int i = 0; i < length; ++i) {
    audio_capture_buffer[audio_capture_write_index] = static_cast<int16_t>(data[i] - 128) << 8; // Convert to signed 16-bit
    audio_capture_write_index = (audio_capture_write_index + 1) % kAudioCaptureBufferSize; // Handle wrap-around
  }
}

int32_t LatestAudioTimestamp() {
  // Returns time in ms based on number of samples captured
  return (audio_capture_write_index / 16); // if 16kHz, then 16 samples per ms
}
