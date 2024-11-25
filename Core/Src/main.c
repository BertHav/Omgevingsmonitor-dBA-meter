/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "adc.h"
#include "dma.h"
#include "i2c.h"
#include "i2s.h"
#include "usart.h"
#include "rtc.h"
#include "tim.h"
#include "usb_device.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "gadget.h"
#include "utils.h"
#include "measurement.h"
#include "globals.h"
#include "ESP.h"
#include "PowerUtils.h"
#include "usbd_cdc_if.h"
#include "statusCheck.h"
#include "RealTimeClock.h"
#include "sound_measurement.h"
#include "print_functions.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
  bool testDone = false;
  bool ESP_Programming = false;
  bool batteryEmpty = false;
  bool MeasurementBusy;
  uint8_t RxData[UART_CDC_DMABUFFERSIZE] = {0};
  uint8_t iMinute = 0;
  uint8_t lasthour = 0;
  uint8_t lastminute = 0;
  uint8_t lastsecond = 0;
  uint8_t weekday;
  uint8_t day;
  uint8_t month;
  uint8_t year;
  uint8_t myUptimeminute = 0;
  uint8_t myUptimehour = 0;
  uint16_t myUptimeday = 0;
  uint16_t IndexRxData = 0;
  uint32_t LastRxTime = 0;
  uint32_t batteryReadTimer = 0;
  uint32_t timeReadTimer = 0;
  uint32_t sleepTime = 0;
  uint16_t size = 0;

  Battery_Status charge;
  ESP_States ESP_Status;
  extern DMA_HandleTypeDef hdma_spi2_rx;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
void SetTestDone(){
  testDone = true;
//  Info("testDone true in SetTestDone\r\n");
  HAL_Delay(1000);
  SetDBLED(false, false, true);
  SetStatusLED(4000, 4000, 3000);
  SetVocLED(4000, 4000, 3000);
  HAL_Delay(1000);
  SetDBLED(false, false, false);
  SetStatusLED(4000, 4000, 4000);
  SetVocLED(4000, 4000, 4000);
  InitDone();
}
void ESP_Programming_Read_Remaining_DMA()
{
  //ESP programmer section

  if (LastRxTime != 0 && ESP_Programming)
  {
    if ((LastRxTime + 100) < HAL_GetTick()) //120
    {
      HAL_UART_DMAPause(&hlpuart1);
      size = __HAL_DMA_GET_COUNTER(hlpuart1.hdmarx);
      if (size > (UART_CDC_DMABUFFERSIZE / 2))
      {
        size = UART_CDC_DMABUFFERSIZE - size;
      }
      else
      {
        size = (UART_CDC_DMABUFFERSIZE / 2) - size;
      }
      if (size > 0)
      {
        CDC_Transmit_FS(&RxData[IndexRxData], size);
        LastRxTime = 0;
        IndexRxData += size;
      }
      HAL_UART_DMAResume(&hlpuart1);
    }
  }
}
//void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
//{
//  // printf_USB("\r\nDMA counter Full: %d\r\nDMA size Full: %d\r\n", __HAL_DMA_GET_COUNTER(hlpuart1.hdmarx), size);
//  //printf_USB("DMA size Full: %d\r\n", size);
//
//  if (huart->Instance == LPUART1)
//  {
//    CDC_Transmit_FS(&RxData[IndexRxData], (UART_CDC_DMABUFFERSIZE / 2) - size);
//
//    size = 0;
//    IndexRxData = 0;
//    LastRxTime = HAL_GetTick();
//  }
//}
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
uint8_t u1_rx_buff[16];  // rxbuffer for serial logger

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_I2C1_Init();
  MX_I2S2_Init();
  MX_USART1_UART_Init();
  MX_I2C2_Init();
  MX_TIM2_Init();
  MX_TIM3_Init();
  MX_USART4_UART_Init();
  MX_ADC_Init();
  MX_USB_DEVICE_Init();
  MX_RTC_Init();
  MX_LPUART1_UART_Init();
  MX_TIM6_Init();
  /* USER CODE BEGIN 2 */
  // General TODO 's
	/*
	 * : Put SSID in EEPROM
	 * : Turn on heater if humidity is too high
	 * : NTP sets RTC done
	 * : FFT Processing bins done
	 * : LEDs indicator for air quality
	 * : Default network: Sensor community
	 * : Different modes for outside and inside (check solar?)
	 * : Add CLI via usb-c
	 * : Network not found? Sleep
	 */
  GPIO_InitPWMLEDs(&htim2, &htim3);
  if(UserButton_Pressed()){
    EnableESPProg();
    ESP_Programming = true;
  }
  //uint32_t LedBlinkTimestamp = HAL_GetTick() + LED_BLINK_INTERVAL;
  SetVerboseLevel(VERBOSE_ALL);
  BinaryReleaseInfo();
  InitClock(&hrtc);
  Debug("Clock init done");
  HAL_UART_Receive_IT(&huart1, u1_rx_buff, 1);

  if (!soundInit(&hdma_spi2_rx, &hi2s2, &htim6, DMA1_Channel4_5_6_7_IRQn))
  {
      errorHandler(__func__, __LINE__, __FILE__);
  }

  Gadget_Init(&hi2c1, &hi2s2, &huart4, &hadc);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1) {
	  // Upkeep gadget
    if(testDone && !ESP_Programming && !batteryEmpty){
      MeasurementBusy = UpkeepGadget();
      ESP_Status = ESP_Upkeep();

    }
    if(!testDone && !ESP_Programming && !batteryEmpty){
      Gadget_Test();
    }
    Status_Upkeep();
    if(TimestampIsReached(batteryReadTimer)){
      charge = Battery_Upkeep();
      batteryReadTimer  = HAL_GetTick() + 60000;
    }
    if(charge == BATTERY_LOW || charge == BATTERY_CRITICAL){

    }
    if(charge == BATTERY_CRITICAL && ESP_Status == ESP_STATE_RESET){
      batteryEmpty = true;
    }
    else{
      batteryEmpty = false;
    }
    if(charge == BATTERY_FULL){

    }

    if(TimestampIsReached(timeReadTimer)){
      timeReadTimer  = HAL_GetTick() + 30000;
      RTC_GetTime(&lasthour, &lastminute, &lastsecond, &weekday, &day, &month, &year);
      printf("Uptime timestamp reached ±30 sec interval: %02d:%02d:%02d\r\n", lasthour, lastminute, lastsecond);
      if (iMinute != lastminute) {
        iMinute = lastminute;
        myUptimeminute++;
        printf("Systemuptime is: %d days %02d:%02d\r\n", myUptimeday, myUptimehour, myUptimeminute);
      }
      if (myUptimeminute == 60) {
        myUptimeminute = 0;
        myUptimehour++;
        if (myUptimehour == 24) {
          myUptimehour = 0;
          myUptimeday++;
        }
      }
    }

    //    if(TimestampIsReached(LedBlinkTimestamp)) {
//      // Red LED
//
//      LedBlinkTimestamp = HAL_GetTick() + LED_BLINK_INTERVAL;
//    }

    // Optional colours:
    // Red, Yellow, Magenta, White, Cyan, Blue, Green.  black ;)

    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Configure LSE Drive Capability
  */
  HAL_PWR_EnableBkUpAccess();
  __HAL_RCC_LSEDRIVE_CONFIG(RCC_LSEDRIVE_LOW);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI|RCC_OSCILLATORTYPE_LSE
                              |RCC_OSCILLATORTYPE_HSI48;
  RCC_OscInitStruct.LSEState = RCC_LSE_ON;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.HSI48State = RCC_HSI48_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART1|RCC_PERIPHCLK_LPUART1
                              |RCC_PERIPHCLK_I2C1|RCC_PERIPHCLK_RTC
                              |RCC_PERIPHCLK_USB;
  PeriphClkInit.Usart1ClockSelection = RCC_USART1CLKSOURCE_PCLK2;
  PeriphClkInit.Lpuart1ClockSelection = RCC_LPUART1CLKSOURCE_PCLK1;
  PeriphClkInit.I2c1ClockSelection = RCC_I2C1CLKSOURCE_PCLK1;
  PeriphClkInit.RTCClockSelection = RCC_RTCCLKSOURCE_LSE;
  PeriphClkInit.UsbClockSelection = RCC_USBCLKSOURCE_HSI48;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
// Provide a print interface for print_functions.
void printString(const char * str, uint16_t length)
{
    HAL_UART_Transmit(&huart1, (uint8_t*) str, length, 0xFFFF);
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
  HAL_UART_Receive_IT(&huart1, u1_rx_buff, 1);
  switch (u1_rx_buff[0]){

  case (uint8_t)'?':
    RTC_GetTime(&lasthour, &lastminute, &lastsecond, &weekday, &day, &month, &year);
    printf("System time: %02d-%02d-%02d %02d:%02d:%02d, system uptime is: %dD %02d:%02d\r\n", year, month, day, lasthour, lastminute, lastsecond, myUptimeday, myUptimehour, myUptimeminute);
    break;
  case (uint8_t)'t':
    forceNTPupdate();
  break;
  default:
     printf("Error unknown request\r\n");
  break;
  }
  HAL_UART_Receive_IT(&huart1, u1_rx_buff, 1); //Re-arm the interrupt
}



/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
