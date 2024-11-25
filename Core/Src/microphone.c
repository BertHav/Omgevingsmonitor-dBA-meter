/*
 * microphone.c
 *
 *  Created on: Jun 10, 2024
 *      Author: Joris Blankestijn
 */

#include "microphone.h"
//#include "main.h"
#include "GPIO.h"
#include "sound_measurement.h"


// TODO: include arm math libraries differently.
#define CORTEX_M0
#include "arm_math.h"
#include "arm_const_structs.h"
#include <stdlib.h>

//static I2S_HandleTypeDef* I2SHandle = NULL;
//static NrOfSamples Samples = NUMBER_OF_SAMPLES;
//static int16_t AudioRxBuffer[NUMBER_OF_SAMPLES*4] = {0};
//q15_t output[NUMBER_OF_SAMPLES*2];

#define nrOfSamples 10

static volatile uint32_t StartTime = 0;
static volatile uint32_t StartupDoneTime = 0;
static volatile bool StartUpDone = false;


float dBc = 0;
float dBcSamples[nrOfSamples];
float dBcAverage;
float dBValue = 0.0f;
uint8_t counter = 0;
bool averageReached = false;
//float OCT[10];
float sample[NUMBER_OF_SAMPLES];


//void MIC_Init(I2S_HandleTypeDef* i2SHandle) { I2SHandle = i2SHandle; }

//static void CalculateFFT(void) {
//  float32_t IQ[NR_SAMPLES_128 * 2] ={0};      /*! I/Q interleave buffer Required 2x FFT-Size */
//
//  // Hanning window
//  float32_t energyCorrectionFactor = 1.63;
//  float32_t windowingFunc[NR_SAMPLES_128] = {0, 0.00061179, 0.0024457, 0.0054971, 0.0097588, 0.01522, 0.021868, 0.029685,
//  0.038654, 0.048752, 0.059954, 0.072232, 0.085558, 0.099898, 0.11522, 0.13148, 0.14864, 0.16666, 0.1855, 0.20511,
//  0.22544, 0.24644, 0.26806, 0.29025, 0.31295, 0.33611, 0.35967, 0.38358, 0.40777, 0.43218, 0.45676, 0.48145,
//  0.50618, 0.5309, 0.55554, 0.58005, 0.60436, 0.62841, 0.65215, 0.67552, 0.69846, 0.72091, 0.74283, 0.76414,
//  0.78481, 0.80479, 0.82402, 0.84245, 0.86005, 0.87677, 0.89256, 0.9074, 0.92123, 0.93404, 0.94578, 0.95644,
//  0.96597, 0.97437, 0.9816, 0.98766, 0.99252, 0.99618, 0.99862, 0.99985, 0.99985, 0.99862, 0.99618, 0.99252,
//  0.98766, 0.9816, 0.97437, 0.96597, 0.95644, 0.94578, 0.93404, 0.92123, 0.9074, 0.89256, 0.87677, 0.86005,
//  0.84245, 0.82402, 0.80479,0.78481, 0.76414, 0.74283, 0.72091, 0.69846, 0.67552, 0.65215, 0.62841, 0.60436,
//  0.58005, 0.55554, 0.5309, 0.50618, 0.48145, 0.45676, 0.43218, 0.40777, 0.38358, 0.35967, 0.33611, 0.31295,
//  0.29025, 0.26806, 0.24644, 0.22544, 0.20511, 0.1855, 0.16666, 0.14864, 0.13148, 0.11522, 0.099898, 0.085558,
//  0.072232, 0.059954, 0.048752, 0.038654, 0.029685, 0.021868, 0.01522, 0.0097588, 0.0054971, 0.0024457, 0.00061179, 0};
//
//  // Apply the windowing function
//  for(uint16_t i = 0; i < NR_SAMPLES_128; i++) {
//    IQ[i] = (float32_t)(AudioRxBuffer[i] * windowingFunc[i]);
//  }
//
//  uint8_t ifftFlag = 0;
//  uint8_t doBitReverse = 1;
//
//  /* Process the data through the CFFT/CIFFT module */
//  arm_cfft_f32(&arm_cfft_sR_f32_len512, IQ, ifftFlag, doBitReverse);
//  /* Process the data through the Complex Magnitude Module for
//  calculating the magnitude at each bin */
//  arm_cmplx_mag_f32(IQ, FFTResult, NR_SAMPLES_128);
//
//  for(uint8_t i = 0;i < NR_SAMPLES_128; i++) {
//    FFTResult[i] *= energyCorrectionFactor;
//  }
//}

/*
static void UpdateSampleRate(uint32_t sampleRate) {
  if (I2SHandle->Init.AudioFreq == sampleRate) {
    return;
  }

  HAL_I2S_DeInit(I2SHandle);
  I2SHandle->Init.AudioFreq = sampleRate;
  HAL_I2S_Init(I2SHandle);
}

static float ConvertAudio(int16_t* data) {
  uint32_t MSP;
  uint32_t LSP;
  int32_t signedAudioValue;
  float Division = 8388607.0; //Reference?
  float adjustedAudioValue = 0;
  uint32_t audioValue = 0;
  MSP = data[0]<<9;
  LSP = (data[1]&0xFF80)>>7;
  audioValue = MSP | LSP;
  if((0x800000 & audioValue) != 0){
    audioValue = 0xFF000000 | audioValue;
  }
  signedAudioValue = (int32_t)audioValue;
  adjustedAudioValue = ((float)signedAudioValue/Division);

  //dBc = 10*log(audioValue/Division);
//  if(Sign){
//    audioValue = 0x3FFFFFFF;
//    audioValue -= (0x3FFF & data[0]) << 17;
//    audioValue -= data[1]<<1;
//    audioValue = audioValue >> 9;
//    dB = 20 * log10(audioValue / Division);
//  }
//  else{
//    audioValue += (0x3FFF & data[0]) << 8;
//    audioValue += (data[1] >> 8);
//    dB = 20 * log10(audioValue / Division);
//  }

//  audioValue = data[1] << 1| (data[0] << 17);
//  audioValue = audioValue >> 9;
//  if(audioValue < 0){
//    audioValue = 0xFFFFFFFF-audioValue;
//  }
//  dB = 20 * log10(audioValue / Division);

  return adjustedAudioValue;
}
*/
//q15_t GetOctave(uint16_t width, uint16_t minFreq){
//  q15_t retVal = 0;
//  q15_t buffer = 0;
//  float dB;
//  float Division = (32768.0)-1; //Reference?
//  for(uint16_t i = minFreq; i < minFreq+width; i++){
//    buffer += X2(output[i]);
//  }
//  arm_sqrt_q15(buffer, &retVal);
//  dB = 10*log10(retVal/Division);
//  return(dB);
//}

//void GetAllOctaves(){
//  OCT[0] = GetOctave(2, 3); // 31.5 centre should be at 4
//  OCT[1] = GetOctave(4, 6); // 63 centre should be at 8
//  OCT[2] = GetOctave(6, 13); // 125 centre should be at 16
//  OCT[3] = GetOctave(13, 27); // 250 centre should be at 33
//  OCT[4] = GetOctave(22 ,55); // 500 centre should be at 66
//  OCT[5] = GetOctave(46 , 109); // 1000 centre should be at 132
//  //OCT[6] = output[64]*0; //2000
//
//}

//void FFT(){
//  static arm_rfft_instance_q15 fft_instance;
//  arm_status status;
//  status = arm_rfft_init_q15(&fft_instance, NUMBER_OF_SAMPLES, 1, 1);
//  arm_rfft_q15(&fft_instance, sample, output);
//  arm_abs_q15(output, output, NUMBER_OF_SAMPLES);
//  GetAllOctaves();
//}

/*
void MIC_Start(uint32_t sampleRate, uint16_t nrSamples) {
  if (I2SHandle == NULL) {
    Error("Microphone is not initialised.");
    return;
  }
  SetMICIndicator();
  Debug("In mic start");
  UpdateSampleRate(sampleRate);
  Samples = (NrOfSamples)nrSamples;

  //StartTime = GetCurrentHalTicks();
  StartupDoneTime = HAL_GetTick() + 20;
  StartUpDone = false;
  DataReady = false;

  HAL_StatusTypeDef status = HAL_I2S_Receive_DMA(I2SHandle, (uint16_t*)AudioRxBuffer, NUMBER_OF_SAMPLES*2);
//      HAL_I2S_Receive_DMA(I2SHandle, (uint16_t*)AudioRxBuffer,
//          Samples >> 1); //>>1 because reading half word

  Info("Status %d", status);
}
*/

/*
float MinimalValue(uint16_t length){
  float MinVal = 1;
  int16_t i;
  for(i =0; i<length;i++){
    if(sample[i] < MinVal){
      MinVal = sample[i];
    }
  }
  return(MinVal);
}

float MaximalValue(uint16_t length){
  float MaxVal = -1;
  int16_t i;
  for(i =0; i<length;i++){
    if(sample[i] > MaxVal){
      MaxVal = sample[i];
    }
  }
  return(MaxVal);
}


bool MIC_Check(void) {
  float Max;
  float Min;
  Info("New samples");
  for (uint16_t i = 0; i < NUMBER_OF_SAMPLES; i += 1) {
    sample[i] = ConvertAudio(&AudioRxBuffer[4*i+2]);
    //Info("0x%08x", sample);
  }
  Max = MaximalValue(NUMBER_OF_SAMPLES);
  Min = MinimalValue(NUMBER_OF_SAMPLES);
  if(Max > 0.0 || Min < 0.0){
    return(true);
  }
  return(false);
}
*/

void MIC_Print(void) {
//  Info("New samples");
//  for (uint16_t i = 0; i < NUMBER_OF_SAMPLES; i += 1) {
//    sample[i] = ConvertAudio(&AudioRxBuffer[4*i+2]);
//    //Info("0x%08x", sample);
//  }
  //filter();
  //dBc = MIC_GetDB();
  dBc = dBValue;
//  Info("IN MIC_Print dBc: %02.1f", dBc);

  if(counter < nrOfSamples){
    dBcSamples[counter] = dBc;
    counter += 1;
    if ( counter == nrOfSamples ){
//      Debug("IN MIC_Print dBcSamples[counter] == %d", nrOfSamples);
    }
  }
  else{
    counter = 0;
    dBcSamples[counter] = dBc;
    counter += 1;
    averageReached = true;
  }
  if(!averageReached){
      setMic(dBc); // individual is only stored in case of no average level is available
  }
  else{
    dBcAverage = 0.0;
    for(uint8_t i=0;i<nrOfSamples;i++){
      dBcAverage += dBcSamples[i];
    }
    dBcAverage = dBcAverage/(float)nrOfSamples;
//    Debug("dBcAverage value for upload: %02.1f", dBcAverage);
    setMic(dBcAverage);
  }

//Currently is set as the individual dBc, should be set to average.
//  HAL_GPIO_WritePin(MCU_LED_C_R_GPIO_Port, MCU_LED_C_R_Pin, 1);
//  HAL_GPIO_WritePin(MCU_LED_C_G_GPIO_Port, MCU_LED_C_G_Pin, 1);
//  HAL_GPIO_WritePin(MCU_LED_C_B_GPIO_Port, MCU_LED_C_B_Pin, 1);
  if(dBc >= 90){//white
    SetDBLED(true, true, true);
  }
  if(dBc < 90 && dBc >= 80){ //red
    SetDBLED(true, false, false);
  }
  if(dBc < 80 && dBc >= 70){//yellow
    SetDBLED(true, true, false);
  }
  if(dBc < 70 && dBc >= 60){//Green
    SetDBLED(false, true, false);
  }
  if(dBc < 60 && dBc >= 50){//blue
    SetDBLED(false, false, true);
  }
  if(dBc < 50 && dBc >= 40){//light blue
    SetDBLED(false, true, true);
  }
  if(dBc < 40 && dBc >= 35){//purple
    SetDBLED(true, false, true);
  }
  if(dBc < 35){//off
    SetDBLED(false, false, false);
  }
}



bool MIC_MeasurementDone(void) {
  if(DataReady) {
    MIC_Print();
//    Debug("MIC measurement is done with %i samples.", Samples);
    ResetMICIndicator();
    return true;
  }
  return false;
}



bool MIC_TestMeasurementDone(void) {
  bool Check;
  Info("DataReady in MIC_TestMeasurementDone: %d", DataReady);
  if(DataReady) {
//    Check = MIC_Check();
    Check = micEnabled;
    Info("status micEnabled: %d",micEnabled );
    ResetMICIndicator();
    return Check;
  }
  return false;
}


// #TODO Also include half full callback
/*void HAL_I2S_RxCpltCallback(I2S_HandleTypeDef* hi2s) {
  if (hi2s != I2SHandle)
    return;

  if (StartUpDone) {
    HAL_I2S_DMAStop(I2SHandle);
    DataReady = true;
    // MIC trigger pin is used to debug when the measurement has started.
//    HAL_GPIO_WritePin(MIC_Trigger_GPIO_Port, MIC_Trigger_Pin, GPIO_PIN_RESET);
    // TODO: Use different pin for MIC_Trigger.
  } else if (TimestampIsReached(StartupDoneTime)) {
    StartUpDone = true;
//    HAL_GPIO_WritePin(MIC_Trigger_GPIO_Port, MIC_Trigger_Pin, GPIO_PIN_SET);
  }
}
*/
/*
float MIC_GetDB(void) {
    float sum = 0.0f;
    float dBValue = 0.0f;

    // Calculate the sum of the magnitudes in FFTResult
    for (uint16_t i = NUMBER_OF_SAMPLES/2; i < NUMBER_OF_SAMPLES; i++) {
      // Sum of squares of magnitudes
      sum += sample[i]*sample[i];
    }

    // Calculate the root mean square (RMS)
    float rms = sqrt(sum/(float)(NUMBER_OF_SAMPLES/2));

    // Convert the RMS value to dB
    dBValue = 20.0f * log10(rms) +120.0;

    return dBValue;
}
*/
