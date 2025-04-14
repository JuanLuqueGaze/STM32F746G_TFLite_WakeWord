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


  int16_t expected_feature[] = {
  
    0,  0,  0,   0,   0,   0,  0,   0,   0,  0,  0,   0,   0,  0,  0,   0,  0,
    0,  0,  0,   0,   0,   0,  0,   0,   0,  0,  0,   0,   1,  19, 1,   0,  0,
    0,  0,  0,   0,   0,   0,  0,   1,   0,  1,  3,   3,   1,  1,  0,   0,  0,
    0,  0,  0,   0,   0,   0,  0,   0,   0,  0,  0,   0,   0,  0,  0,   0,  0,
    0,  0,  0,   0,   8,   89, 8,   0,   0,  0,  0,   0,   0,  0,  0,   4,  13,
    1,  6,  23,  20,  6,   4,  0,   0,   0,  0,  0,   0,   0,  0,  0,   0,  0,
    0,  0,  0,   0,   0,   0,  0,   0,   0,  0,  0,   0,   0,  19, 177, 42, 1,
    1,  0,  0,   0,   0,   2,  3,   119, 51, 5,  139, 92,  58, 58, 15,  2,  1,
    0,  0,  0,   0,   0,   0,  0,   0,   0,  0,  0,   0,   0,  0,  0,   0,  0,
    0,  0,  0,   0,   0,   13, 165, 176, 3,  1,  1,   0,   0,  1,  1,   32, 214,
    26, 19, 113, 103, 28,  22, 27,  3,   1,  0,  0,   0,   0,  0,  0,   0,  0,
    0,  0,  0,   0,   0,   0,  0,   0,   0,  0,  0,   0,   0,  0,  12,  55, 128,
    27, 1,  1,   0,   1,   4,  2,   52,  93, 10, 28,  156, 10, 21, 21,  3,  3,
    1,  0,  0,   0,   0,   0,  0,   0,   0,  0,  0,   0,   0,  0,  0,   0,  0,
    0,  0,  0,   0,   0,   0,  14,  99,  32, 65, 7,   1,   2,  2,  6,   13, 121,
    36, 15, 11,  112, 125, 14, 5,   13,  4,  4,  2,   0,   0,  0,  0,   0,  0,
    0,  0,  0,   0,   0,   0,  0,   0,   0,  0,  0,   0,   0,  0,  0,   24, 25,
    32, 5,  1,   0,   0,   0,  1,   0,   7,  5,  1,   1,   3,  3,  0,   3,  3,
    0,  0,  0,   0,   0,   0,  0,   0,   0,  0,  0,   0,   0,  0,  0,   0,  0,
    0,  0,  0,   0,   0,   0,  0,   13,  13, 5,  1,   0,   0,  0,  0,   0,  3,
    4,  1,  0,   1,   2,   3,  1,   1,   1,  4,  8,   1,   2,  1,  3,   1,  1,
    0,  1,  1,   3,   1,   0,  0,   0,   0,  0,  0,   0,   0,  0,  0,   0,  1,
    8,  2,  1,   0,   0,   0,  0,   0,   1,  1,  0,   0,   1,  1,  2,   0,  2,
    1,  0,  2,   0,   2,   2,  3,   1,   1,  0,  1,   1,   4,  5,  1,   0,  1,
    0,  0,  0,   0,   0,   0,  0,   0,   0,  0,  0,   0,   0,  0,  0,   0,  0,
    0,  0,  0,   0,   0,   0,  1,   1,   1,  0,  1,   2,   1,  0,  1,   3,  1,
    1,  3,  1,   1,   6,   2,  0,   0,   0,  0,  0,   0,   0,  0,  0,   0,  0,
    0,  2,  0,   0,   0,   0,  0,   0,   0,  0,  0,   0,   0,  0,  0,   0,  0,
    0,  0,  0,   0,   0,   0,  1,   1,   0,  1,  2,   6,   2,  4,  2,   0,  0,
    0,  0,  0,   0,   0,   0,  0,   0,   0,  0,  3,   0,   0,  0,  0,   0,  0,
    0,  0,  0,   0,   0,   0,  0,   0,   0,  0,  0,   0,   0,  0,  0,   0,  1,
    0,  0,  1,   2,   1,   1,  2,   1,   0,  0,  0,   0,   0,  0,  0,   0,  0,
    0,  0,  4,   0,   0,   0,  0,   0,   0,  0,  0,   0,   0,  0,  0,   0,  0,
    0,  0,  0,   0,   0,   0,  0,   0,   2,  1,  0,   0,   2,  3,  5,   2,  0,
    1,  0,  0,   0,   0,   0,  0,   0,   0,  0,  0,   1,   0,  0,  0,   0,  0,
    0,  0,  0,   0,   0,   0,  0,   0,   0,  0,  0,   0,   0,  0,  0,   0,  0,
    0,  0,  0,   0,   1,   2,  2,   0,   0,  0,  0,   0,   0,  0,  0,   0,  0,
    0,  0,  0,   0,   0,   0,  0,   0,   0,  0,  0,   0,   0,  0,  0,   0,  0,
    0,  0,  0,   0,   0,   0,  0,   0,   0,  1,  0,   0,   0,  0,  1,   2,  3,
    0,  0,  0,   0,   0,   0,  0,   0,   0,  0,  0,   0,   0,  0,  0,   0,  0,
    0,  0,  0,   0,   0,   0,  0,   0,   0,  0,  0,   0,   0,  0,  0,   0,  0,
    0,  0,  0,   0,   0,   1,  1,   1,   1,  0,  0,   0,   1,  0,  0,   0,  0,
    0,  0,  0,   0,   0,   0,  0,   0,   0,  0,  0,   0,   0,  0,  0,   0,  0,
    0,  0,  0,   0,   0,   0,  0,   0,   0,  0,  0,   0,   0,  0,  0,   0,  0,
    0,  0,  0,   0,   0,   0,  0,   0,   0,  0,  0,   0,   0,  0,  0,   0,  0,
    0,  0,  0,   0,   0,   0,  0,   0,   0,  0,  0,   0,   0,  0,  0,   0,  0,
    0,  0,  0,   0,   0,   0,  0,   0,   0,  0,  0,   0,   0,  0,  0,   0,  0,
    0,  0,  0,   0,   0,   0,  0,   0,   0,  0,  0,   0,   0,  0,  0,   0,  0,
    0,  0,  0,   0,   0,   0,  0,   0,   0,  0,  0,   0,   0,  0,  0,   0,  0,
    0,  0,  0,   0,   0,   0,  0,   0,   0,  0,  0,   0,   0,  0,  0,   0,  0,
    0,  0,  0,   0,   0,   0,  0,   0,   0,  0,  0,   0,   0,  0,  0,   0,  0,
    0,  0,  0,   0,   0,   0,  0,   0,   0,  0,  0,   0,   0,  0,  0,   0,  0,
    0,  0,  0,   0,   0,   0,  0,   0,   0,  0,  0,   0,   0,  0,  0,   0,  0,
    0,  0,  0,   0,   0,   0,  0,   0,   0,  0,  0,   0,   0,  0,  0,   0,  0,
    0,  0,  0,   0,   0,   0,  0,   0,   0,  0,  0,   0,   0,  0,  0,   0,  0,
    0,  0,  0,   0,   0,   0,  0,   0,   0,  0,  0,   0,   0,  0,  0,   0,  0,
    0,  0,  0,   0,   0,   0,  0,   0,   0,  0,  0,   0,   0,  0,  0,   0,  0,
    0,  0,  0,   0,   0,   0,  0,   0,   0,  0,  0,   0,   0,  0,  0,   0,  0,
    0,  0,  0,   0,   0,   0,  0,   0,   0,  0,  0,   0,   0,  0,  0,   0,  0,
    0,  0,  0,   0,   0,   0,  0,   0,   0,  0,  0,   0,   0,  0,  0,   0,  0,
    0,  0,  0,   0,   0,   0,  0,   0,   0,  0,  0,   0,   0,  0,  0,   0,  0,
    0,  0,  0,   0,   0,   0,  0,   0,   0,  0,  0,   0,   0,  0,  0,   0,  0,
    0,  0,  0,   0,   0,   0,  0,   0,   0,  0,  0,   0,   0,  0,  0,   0,  0,
    0,  0,  0,   0,   0,   0,  0,   0,   0,  0,  0,   0,   0,  0,  0,   0,  0,
    0,  0,  0,   0,   0,   0,  0,   0,   0,  0,  0,   0,   0,  0,  0,   0,
};

  
  TfLiteStatus GetAudioSamples(tflite::ErrorReporter* error_reporter,
                               int start_ms, int duration_ms,
                               int* audio_samples_size, int16_t** audio_samples) {
    for (int i = 0; i < kMaxAudioSampleSize; ++i) {
      g_dummy_audio_data[i] = expected_feature[i];
    }
        // Print the buffer data

        /*
        PrintToUart("Buffer Data:\r\n");
        for (int i = 0; i < kMaxAudioSampleSize; ++i) {
            char feature_buffer[16];
            sprintf(feature_buffer, "%d ", g_dummy_audio_data[i]);
            PrintToUart(feature_buffer);
    
            // Add a newline every 16 values for better readability
            if ((i + 1) % 16 == 0) {
                PrintToUart("\r\n");
            }
        }
        PrintToUart("\r\n"); // Add an extra newline after printing all features
    */


    *audio_samples_size = kMaxAudioSampleSize;
    *audio_samples = g_dummy_audio_data;
    return kTfLiteOk;
  }
  
  int32_t LatestAudioTimestamp() {
    g_latest_audio_timestamp += 100;
    return g_latest_audio_timestamp;
  }
