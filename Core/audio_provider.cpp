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


constexpr int kAudioCaptureBufferSize = 16000; // for 1 second of 16kHz audio
int16_t audio_capture_buffer[kAudioCaptureBufferSize];
volatile int audio_capture_write_index = 0;


TfLiteStatus GetAudioSamples(tflite::ErrorReporter* error_reporter,
  int start_ms, int duration_ms,
  int* audio_samples_size, int16_t** audio_samples) {
const int start_offset = (start_ms * 16); // assuming 16kHz sample rate
const int length = (duration_ms * 16);    // 16 samples per ms

// Handle wrap-around if using ring buffer
int capture_start = (audio_capture_write_index - length + kAudioCaptureBufferSize) % kAudioCaptureBufferSize;

*audio_samples = &audio_capture_buffer[capture_start];
*audio_samples_size = length;

return kTfLiteOk;
}

/*
TfLiteStatus GetAudioSamples(tflite::ErrorReporter* error_reporter,
  int start_ms, int duration_ms,
  int* audio_samples_size, int16_t** audio_samples) {
// Debug print to confirm the function is called
char debug_buffer[128];
sprintf(debug_buffer, "GetAudioSamples called with start_ms: %d, duration_ms: %d\r\n", start_ms, duration_ms);
PrintToUart(debug_buffer);

// Fill the dummy audio buffer with zeros (replace this with real microphone data)
// Replace the dummy audio buffer with ADC data
for (int i = 0; i < kMaxAudioSampleSize; ++i) {
  g_dummy_audio_data[i] = 0;  // Replace with actual ADC read function
}

// Set the audio samples and size
*audio_samples_size = kMaxAudioSampleSize;
*audio_samples = g_dummy_audio_data;

// Debug print to confirm the buffer size
sprintf(debug_buffer, "Audio sample size: %d\r\n", *audio_samples_size);
PrintToUart(debug_buffer);

// Print the first few audio samples for debugging
for (int i = 0; i < 10; ++i) {  // Print the first 10 samples
sprintf(debug_buffer, "Sample[%d]: %d\r\n", i, g_dummy_audio_data[i]);
PrintToUart(debug_buffer);
}

return kTfLiteOk;
}
*/
int32_t LatestAudioTimestamp() {
  // Returns time in ms based on number of samples captured
  return (audio_capture_write_index / 16); // if 16kHz, then 16 samples per ms
}
