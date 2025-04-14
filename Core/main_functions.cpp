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

#include "main_functions.h"
#include "audio_provider.h"
#include "command_responder.h"
#include "uart_utils.h"
#include "feature_provider.h"
#include "stm32746g_discovery.h"
#include "micro_features_micro_model_settings.h"
#include "micro_features_tiny_conv_micro_features_model_data.h"
#include "lcd.h"
#include "recognize_commands.h"
#include "tensorflow/lite/micro/kernels/micro_ops.h"
#include "tensorflow/lite/micro/micro_error_reporter.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/micro/micro_mutable_op_resolver.h"
#include "tensorflow/lite/schema/schema_generated.h"
#include "tensorflow/lite/version.h"
#include "yes_fixed_data.h"

// Model declaration
namespace {
  tflite::ErrorReporter* error_reporter = nullptr;
  const tflite::Model* model = nullptr;
  tflite::MicroInterpreter* interpreter = nullptr;
  TfLiteTensor* model_input = nullptr;
  FeatureProvider* feature_provider = nullptr;
  RecognizeCommands* recognizer = nullptr;
  int32_t previous_time = 0;
  
  // Create an area of memory to use for input, output, and intermediate arrays.
  // The size of this will depend on the model you're using, and may need to be
  // determined by experimentation.
  constexpr int kTensorArenaSize = 10 * 1024;
  uint8_t tensor_arena[kTensorArenaSize]; //This is done to avoid errors in the model
  }  // namespace
  

// Definitions of objets and buffers used in the code
#define AUDIO_BUFFER_SIZE 2048
uint8_t dma_audio_buffer[AUDIO_BUFFER_SIZE];
SAI_HandleTypeDef hsai_BlockA1;
DMA_HandleTypeDef hdma_sai1_a;
UART_HandleTypeDef DebugUartHandler;



  // This is the setup function, executed once at startup.
  void setup() {
    
	BSP_LED_Init(LED_GREEN);  //Initilize the LED, which can be used for debugging

  uart1_init(); //Initialize the uart
  PrintToUart("UART Initialized\r\n"); //Debug message for the UART initialization

  MX_SAI1_Init();      // Initialize the SAI peripheral and outputs a message to verify it

  // Tensorflow code, instantiate the error reporter to report errors to the UART
  static tflite::MicroErrorReporter micro_error_reporter;
  error_reporter = &micro_error_reporter;


  /* This is a function of the library stm32f7xx_hal_sai.c. There should not be any issue with it because it was already
  defined in the library. What this function does is initialize the SAI peripheral to receive data using DMA. */
  HAL_StatusTypeDef SAI_Receive_DMA_Debug = HAL_SAI_Receive_DMA(&hsai_BlockA1, (uint8_t*)dma_audio_buffer, AUDIO_BUFFER_SIZE);

  if (SAI_Receive_DMA_Debug != HAL_OK) {
    PrintToUart("HAL_SAI_Receive_DMA failed\r\n");
    error_handler();  
  }
  else
  {
    PrintToUart("HAL_SAI_Receive_DMA succeded\r\n");
  }

  /* Map the model into a usable data structure. This doesn't involve any copying or parsing, 
  it's a very lightweight operation. It's just assign the model to a tflite struct model */
  model = tflite::GetModel(g_tiny_conv_micro_features_model_data);
  if (model->version() != TFLITE_SCHEMA_VERSION) {
    PrintToUart("Version is not correct\r\n");
    error_handler();
    return;
  }
  else
  {
    PrintToUart("Model assigned correctly\r\n");
  }
  /* Code copied from the tensorflow official repository
  // Pull in only the operation implementations we need.
  // This relies on a complete list of all the ops needed by this graph.
  // An easier approach is to just use the AllOpsResolver, but this will
  // incur some penalty in code space for op implementations that are not
  // needed by this graph.
  //
  // tflite::ops::micro::AllOpsResolver resolver;
  // NOLINTNEXTLINE(runtime-global-variables)
  */


  static tflite::MicroMutableOpResolver micro_mutable_op_resolver;
  TfLiteStatus BuiltinOperator_StatusDebug_1 = micro_mutable_op_resolver.AddBuiltin(
      tflite::BuiltinOperator_DEPTHWISE_CONV_2D,
      tflite::ops::micro::Register_DEPTHWISE_CONV_2D());
  TfLiteStatus BuiltinOperator_StatusDebug_2 = micro_mutable_op_resolver.AddBuiltin(
      tflite::BuiltinOperator_FULLY_CONNECTED,
      tflite::ops::micro::Register_FULLY_CONNECTED());
  TfLiteStatus BuiltinOperator_StatusDebug_3 =micro_mutable_op_resolver.AddBuiltin(tflite::BuiltinOperator_SOFTMAX,
                                       tflite::ops::micro::Register_SOFTMAX());

  //Checks if the layers are correctly assigned to the ops resolver. If not, it will enter the error handler.
  if (BuiltinOperator_StatusDebug_1 != kTfLiteOk) {
    PrintToUart("First layer wrongly assigned\r\n");
    error_handler();  
  }
  else
  {
    PrintToUart("First layer correctly assigned\r\n");
  } 
  
  if (BuiltinOperator_StatusDebug_1 != kTfLiteOk) {
    PrintToUart("Second layer wrongly assigned\r\n");
    error_handler();  
  }
  else
  {
    PrintToUart("Second layer correctly assigned\r\n");
  }      

  if (BuiltinOperator_StatusDebug_1 != kTfLiteOk) {
    PrintToUart("Third layer wrongly assigned\r\n");
    error_handler();  
  }
  else
  {
    PrintToUart("Third layer correctly assigned\r\n");
  }      

  PrintToUart("Ops resolvers working\r\n"); //Debug for ops resolver
  
  // Build an interpreter to run the model with.
  static tflite::MicroInterpreter static_interpreter(
      model, micro_mutable_op_resolver, tensor_arena, kTensorArenaSize,
      error_reporter);
  interpreter = &static_interpreter;
  
  // Allocate memory from the tensor_arena for the model's tensors.
  TfLiteStatus allocate_status = interpreter->AllocateTensors();
  if (allocate_status != kTfLiteOk) {
    PrintToUart("AllocateTensors() failed\r\n");
    error_handler();
  }
  else{
    PrintToUart("AllocateTensors() succeded\r\n");
  }
  
  // Get information about the memory area to use for the model's input.
  model_input = interpreter->input(0);


  if ((model_input->dims->size != 4) || (model_input->dims->data[0] != 1) ||
      (model_input->dims->data[1] != kFeatureSliceCount) ||
      (model_input->dims->data[2] != kFeatureSliceSize) ||
      (model_input->type != kTfLiteUInt8)) {
        PrintToUart("Bad input tensor parameters in model\r\n");
        error_handler();
      }

// Copy the fixed input data into the model's input tensor
  memcpy(model_input->data.uint8, yes_buffer, kFeatureElementCount);


  PrintToUart("Inputs assigned\r\n");
  
  /* Prepare to access the audio spectrograms from a microphone or other source
  that will provide the inputs to the neural network.
  NOLINTNEXTLINE(runtime-global-variables) */



  //This line seems to be critical in order to find the error
  //It wasn't that important, but the input kFeatureElementCount is important and the other variables inside the file that defines it also. 
  
  /* Feature Provider is a tflite class which is used to provide the input data to the model in a suitable format.
     With kFeatureElementCount we define the number of elements in the input data (the size of the input vector).
     With model_input->data.uint8 we define the input data type, which is uint8_t in this case and we create a pointer
     to the input of the model. That makes the variable feature_provider to directly point to that memory location*/
  static FeatureProvider static_feature_provider(kFeatureElementCount,
                                                model_input->data.uint8);
  feature_provider = &static_feature_provider;
  /* RecognizeCommands is also a tflite class. It helps with the audio processing. It's a quite complex class, with status 
  and structs inside. Later, we'll call some of them for debugging. */
  static RecognizeCommands static_recognizer(error_reporter);
  recognizer = &static_recognizer;
  
  previous_time = 0;

  PrintToUart("Setup completed\r\n");
  }
  
  // The name of this function is important for Arduino compatibility.
  void loop() {
    // Fetch the spectrogram for the current time.
    // It might be failing because it doesn't get the timestamp correctly
    const int32_t current_time = LatestAudioTimestamp();
    // I print the current time
    char buffer[64];
    sprintf(buffer, "Current time: %ld\r\n", current_time);
    PrintToUart(buffer);
    
    int how_many_new_slices = 0;

    // Here should be the main problem
    
    TfLiteStatus feature_status = feature_provider->PopulateFeatureData(
        error_reporter, previous_time, current_time, &how_many_new_slices);
    if (feature_status != kTfLiteOk) {
      PrintToUart("Feature generation failed\r\n");
      error_handler();
      return;
    }

    // Print the feature data
    PrintToUart("Feature Data:\r\n");
    for (int i = 0; i < kFeatureElementCount; ++i) {
        char feature_buffer[16];
        sprintf(feature_buffer, "%d ", model_input->data.uint8[i]);
        PrintToUart(feature_buffer);

        // Add a newline every 16 values for better readability
        if ((i + 1) % 16 == 0) {
            PrintToUart("\r\n");
        }
    }
    PrintToUart("\r\n"); // Add an extra newline after printing all features

    previous_time = current_time;
    // If no new audio samples have been received since last time, don't bother
    // running the network model.
    
    /*
    if (how_many_new_slices == 0) {
      PrintToUart("No new slices\r\n");
      return;
    }*/
  
    // Run the model on the spectrogram input and make sure it succeeds.
    TfLiteStatus invoke_status = interpreter->Invoke();
    if (invoke_status != kTfLiteOk) {
      PrintToUart("Invoke failed\r\n");
      error_handler();
      return;
    }
  
    // Obtain a pointer to the output tensor
    TfLiteTensor* output = interpreter->output(0);
    // Determine whether a command was recognized based on the output of inference
    const char* found_command = nullptr;
    uint8_t score = 0;
    bool is_new_command = false;
    TfLiteStatus process_status = recognizer->ProcessLatestResults(
        output, current_time, &found_command, &score, &is_new_command);
    if (process_status != kTfLiteOk) {
      PrintToUart("RecognizeCommands::ProcessLatestResults() failed\r\n");
      error_handler();
      return;
    }



    // Print the recognized command, score, and whether it's a new command
char result_buffer[128];
sprintf(result_buffer, "Command: %s, Score: %u, Is New: %s\r\n",
        found_command, score, is_new_command ? "true" : "false");
PrintToUart(result_buffer);


    // Do something based on the recognized command. The default implementation
    // just prints to the error console, but you should replace this with your
    // own function for a real application.
    RespondToCommand(error_reporter, current_time, found_command, score,
                     is_new_command);



    PrintToUart("Loop completed\r\n");
  }


  void system_clock_config(void)
 {
   RCC_OscInitTypeDef RCC_OscInitStruct = {0};
   RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};


   __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

    // Enable HSE Oscillator and activate PLL with HSE as source
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState       = RCC_HSE_ON;
    RCC_OscInitStruct.PLL.PLLState   = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource  = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLM       = 25;
    RCC_OscInitStruct.PLL.PLLN       = 400;
    RCC_OscInitStruct.PLL.PLLP       = RCC_PLLP_DIV2;
    RCC_OscInitStruct.PLL.PLLQ       = 9;

    if(HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
   {
     error_handler();
   }

   /** Activate the Over-Drive mode
   */
   if(HAL_PWREx_EnableOverDrive() != HAL_OK)
   {
     error_handler();
   }

   /** Initializes the CPU, AHB and APB buses clocks
   */

    // Initializes the CPU, AHB and APB busses clocks
   RCC_ClkInitStruct.ClockType      = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
   RCC_ClkInitStruct.SYSCLKSource   = RCC_SYSCLKSOURCE_PLLCLK;
   RCC_ClkInitStruct.AHBCLKDivider  = RCC_SYSCLK_DIV1;
   RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
   RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

   if(HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_6) != HAL_OK)
   {
     error_handler();
   }
 }


static void uart1_init(void)
{
	DebugUartHandler.Instance        = DISCOVERY_COM1;
 	DebugUartHandler.Init.BaudRate   = 9600;
 	DebugUartHandler.Init.WordLength = UART_WORDLENGTH_8B;
 	DebugUartHandler.Init.StopBits   = UART_STOPBITS_1;
 	DebugUartHandler.Init.Parity     = UART_PARITY_NONE;
 	DebugUartHandler.Init.HwFlowCtl  = UART_HWCONTROL_NONE;
 	DebugUartHandler.Init.Mode       = UART_MODE_TX_RX;
 	DebugUartHandler.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
 
 	if(HAL_UART_DeInit(&DebugUartHandler) != HAL_OK)
 	{
 		error_handler();
 	}
   else
  {
     PrintToUart("UART DeInit working correctly\r\n");
  }

 	if(HAL_UART_Init(&DebugUartHandler) != HAL_OK)
 	{
 	    error_handler();
 	}
  else
  {
      PrintToUart("UART Init working correctly\r\n");
 	}


}

static void error_handler(void)
 {
  const char* error_msg = "Error occurred. Entering error handler.\n";
  HAL_UART_Transmit(&DebugUartHandler, (uint8_t*)error_msg, strlen(error_msg), HAL_MAX_DELAY);
  BSP_LED_On(LED_GREEN);
  while (1);
 }


static void cpu_cache_enable(void){
  // Enable I-Cache
     SCB_EnableICache();
 
   /* USER CODE END Error_Handler_Debug */
     // Enable D-Cache
     SCB_EnableDCache();
    }


    void HAL_SAI_MspInit(SAI_HandleTypeDef* hsai) {
      if (hsai->Instance == SAI1_Block_A) {
        __HAL_RCC_SAI1_CLK_ENABLE();
        __HAL_RCC_DMA2_CLK_ENABLE(); // DMA2 is used for SAI1
    
        // Configure GPIOs (e.g., for data pin SD, clock SCK, etc.)
        // You can copy from CubeMX example or reference manual.
        // Example: Use PE6 for SAI1_SD_A (data line)
        __HAL_RCC_GPIOE_CLK_ENABLE();
        GPIO_InitTypeDef GPIO_InitStruct = {0};
        GPIO_InitStruct.Pin = GPIO_PIN_6;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
        GPIO_InitStruct.Alternate = GPIO_AF6_SAI1;
        HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);
    
        // DMA Configuration for SAI1 Block A RX
        hdma_sai1_a.Instance = DMA2_Stream1;
        hdma_sai1_a.Init.Channel = DMA_CHANNEL_0;
        hdma_sai1_a.Init.Direction = DMA_PERIPH_TO_MEMORY;
        hdma_sai1_a.Init.PeriphInc = DMA_PINC_DISABLE;
        hdma_sai1_a.Init.MemInc = DMA_MINC_ENABLE;
        hdma_sai1_a.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
        hdma_sai1_a.Init.MemDataAlignment = DMA_PDATAALIGN_HALFWORD;
        hdma_sai1_a.Init.Mode = DMA_CIRCULAR;
        hdma_sai1_a.Init.Priority = DMA_PRIORITY_HIGH;
        hdma_sai1_a.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    
        HAL_DMA_Init(&hdma_sai1_a);
        __HAL_LINKDMA(hsai, hdmarx, hdma_sai1_a);
    
        // Enable DMA interrupt
        HAL_NVIC_SetPriority(DMA2_Stream1_IRQn, 0, 0);
        HAL_NVIC_EnableIRQ(DMA2_Stream1_IRQn);
      }
    }
    


void MX_SAI1_Init(void) {
  hsai_BlockA1.Instance = SAI1_Block_A;
  hsai_BlockA1.Init.AudioMode = SAI_MODEMASTER_RX;
  hsai_BlockA1.Init.Synchro = SAI_ASYNCHRONOUS;
  hsai_BlockA1.Init.OutputDrive = SAI_OUTPUTDRIVE_DISABLE;
  hsai_BlockA1.Init.NoDivider = SAI_MASTERDIVIDER_ENABLE;
  hsai_BlockA1.Init.FIFOThreshold = SAI_FIFOTHRESHOLD_1QF;
  hsai_BlockA1.Init.AudioFrequency = SAI_AUDIO_FREQUENCY_16K;
  hsai_BlockA1.Init.SynchroExt = SAI_SYNCEXT_DISABLE;
  hsai_BlockA1.Init.MonoStereoMode = SAI_MONOMODE;
  hsai_BlockA1.Init.CompandingMode = SAI_NOCOMPANDING;
  hsai_BlockA1.Init.TriState = SAI_OUTPUT_NOTRELEASED;

  if (HAL_SAI_Init(&hsai_BlockA1) != HAL_OK) {
    error_handler();
  }
  else {
    PrintToUart("SAI1 Initialized successfully\r\n");
  }
}

