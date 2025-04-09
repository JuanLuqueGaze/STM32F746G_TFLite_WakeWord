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


  int8_t expected_feature[] = {
    126, 103, 124, 102, 124, 102, 123, 100, 118, 97, 118, 100, 118, 98,
    121, 100, 121, 98,  117, 91,  96,  74,  54,  87, 100, 87,  109, 92,
    91,  80,  64,  55,  83,  74,  74,  78,  114, 95, 101, 81,
};

  
  TfLiteStatus GetAudioSamples(tflite::ErrorReporter* error_reporter,
                               int start_ms, int duration_ms,
                               int* audio_samples_size, int16_t** audio_samples) {
    for (int i = 0; i < kMaxAudioSampleSize; ++i) {
      g_dummy_audio_data[i] = expected_feature[i];
    }
    *audio_samples_size = kMaxAudioSampleSize;
    *audio_samples = g_dummy_audio_data;
    return kTfLiteOk;
  }
  
  int32_t LatestAudioTimestamp() {
    g_latest_audio_timestamp += 100;
    return g_latest_audio_timestamp;
  }
