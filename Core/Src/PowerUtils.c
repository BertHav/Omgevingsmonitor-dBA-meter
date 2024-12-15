/*
 * PowerUtils.c
 *
 *  Created on: Sep 4, 2024
 *      Author: andri
 *
 */

#include "PowerUtils.h"

/* function to read the actual battery voltage */

  ADC_ChannelConfTypeDef sConfig = {0};
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  uint32_t Vref = 2915;   // reference voltage in mV

float ReadBatteryVoltage(void){
  /* Channel 14 is de battery voltage */
  static uint32_t value = 0;
  static float trueValue = 0;
  sConfig.Channel = ADC_CHANNEL_14;
  sConfig.Rank = ADC_RANK_CHANNEL_NUMBER;
  if (HAL_ADC_ConfigChannel(&hadc, &sConfig) != HAL_OK) {
    Error_Handler();
  }
  HAL_ADC_Start(&hadc);
  HAL_ADC_PollForConversion(&hadc, 1);
  value = (HAL_ADC_GetValue(&hadc)*Vref*2)/4095;
  trueValue = (float)value/1000.0;
  /* Disable Channel 14 */
  sConfig.Channel = ADC_CHANNEL_14;
  sConfig.Rank = ADC_RANK_NONE;
  if (HAL_ADC_ConfigChannel(&hadc, &sConfig) != HAL_OK) {
    Error_Handler();
  }
  return trueValue;
 }

/* function to read the actual battery voltage */

float ReadSolarVoltage(void){
  /* Channel 15 is the Solar voltage */
  static uint32_t value = 0;
  static float trueValue = 0;
  sConfig.Channel = ADC_CHANNEL_15;
  sConfig.Rank = ADC_RANK_CHANNEL_NUMBER;
  if (HAL_ADC_ConfigChannel(&hadc, &sConfig) != HAL_OK) {
    Error_Handler();
  }
  HAL_ADC_Start(&hadc);
  HAL_ADC_PollForConversion(&hadc, 1);
  value = (HAL_ADC_GetValue(&hadc)*Vref*3)/4095;
  trueValue = (float)value / 1000.0;
  /* Disable Channel 14 */
  sConfig.Channel = ADC_CHANNEL_15;
  sConfig.Rank = ADC_RANK_NONE;
  if (HAL_ADC_ConfigChannel(&hadc, &sConfig) != HAL_OK) {
    Error_Handler();
  }
  return trueValue;
 }

// True if 5V on USB Connector
bool Check_USB_PowerOn(void){
  return (HAL_GPIO_ReadPin(VusbDetect_GPIO_Port, VusbDetect_Pin));
}

// Returns Battery status in TypeDef
// TypeDef ChargeStatus:
//  CHARGING_OFF  = 0
//  CHARGING_ON   = 1
//  CHARGING_FULL = 2
//  Function returns Chargestatus

ChargeStatus Read_Charge_Status(void){
  bool PulledUpStatus   = true  ;
  bool PulledDownStatus = false ;
  //Read with Pull Up
  //Charge_Pin_Up();
  GPIO_InitStruct.Pin = Charger_status_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(Charger_status_GPIO_Port, &GPIO_InitStruct);

  PulledUpStatus = (HAL_GPIO_ReadPin(Charger_status_GPIO_Port, Charger_status_Pin));

  //Read with Pull Down
  // Charge_Pin_Down();
  GPIO_InitStruct.Pin = Charger_status_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  HAL_GPIO_Init(Charger_status_GPIO_Port, &GPIO_InitStruct);

  PulledDownStatus = (HAL_GPIO_ReadPin(Charger_status_GPIO_Port, Charger_status_Pin));

  if (PulledUpStatus==false) {
    return CHARGING_ON;
  }
  if (PulledDownStatus==true){
    return CHARGING_FULL;
  }
  return CHARGING_OFF;
}

/* Button operations
 * User Button A  ( Boot0 PIN  and PA15)
 * User Button B  ( User button PD2 )
 */

// True if Userbutton A ( Boot ) is pressed
// It is NOT Possible to read the Boot0 pin after powerup / Reset!!
// So a copy is hard wired to PA15

bool     BootButton_Pressed(void){
  return   (HAL_GPIO_ReadPin(BOOT0_GPIO_Port, BOOT0_Pin));
}

// True if Userbutton B is pressed
bool     UserButton_Pressed(void){
  return   (!HAL_GPIO_ReadPin(User_Button_GPIO_Port, User_Button_Pin));
}

/* (1) Enable the peripheral clock of GPIOA */
/* (2) Select input mode (00) on GPIOA pin 0 */
/* (3) Select Port A for pin 0 extended interrupt by writing 0000
 in EXTI0 (reset value) */
/* (4) Configure the corresponding mask bit in the EXTI_IMR register */
/* (5) Configure the Trigger Selection bits of the Interrupt line
 on rising edge */
/* (6) Configure the Trigger Selection bits of the Interrupt line
 on falling edge */
//RCC->IOPENR |= RCC_IOPENR_GPIOAEN; /* (1) */
//GPIOA->MODER = (GPIOA->MODER & ~(GPIO_MODER_MODE0)); /* (2) */
//SYSCFG->EXTICR[0] &= (uint16_t)~SYSCFG_EXTICR1_EXTI0_PA; /* (3) */
//EXTI->IMR |= 0x0001; /* (4) */
//EXTI->RTSR |= 0x0001; /* (5) */
//EXTI->FTSR |= 0x0001; /* (6) */
/* Configure NVIC for Extended Interrupt */
/* (7) Enable Interrupt on EXTI0_1 */
/* (8) Set priority for EXTI0_1 */
//NVIC_EnableIRQ(EXTI0_1_IRQn); /* (7) */
//NVIC_SetPriority(EXTI0_1_IRQn,0); /* (8) */

