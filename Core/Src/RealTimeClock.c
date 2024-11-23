#include "rtc.h"
#include "RealTimeClock.h"
#include <string.h>
#include <stdlib.h>

typedef struct {
  uint8_t Hours;
  uint8_t Minutes;
  uint8_t Seconds;
}Clock;

static Clock RealTime = {.Hours = 0, .Minutes = 0, .Seconds = 0};
//static Clock NextAction = {.Hours = 0, .Minutes = 0, .Seconds = 0};
static const char *dayNames[7] = {  "Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun"}; // 1 to 7
static const char *monthNames[12] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"}; // 1 to 12
static RTC_HandleTypeDef * RealTime_Handle;

//  0         1         2         3  3
//  0         0         0         0  3
//  +CIPSNTPTIME:Thu Jan  1 01:00:03 1970
//OK
uint8_t aBuff2int(char* aBuff, uint8_t start, uint8_t stop) {
  char iBuff[5];
  uint8_t i;
  uint8_t pos = 0;
  for (i = start; i <= stop ; i++){
    iBuff[pos] = aBuff[i];
    pos++;
  }
  iBuff[pos] = '\0';
  if (start < 20) {
    if (start == 17) { //month
      for (i = 0; i < 12; i++) {
        if (strcmp( iBuff, monthNames[i]) == 0) {
          return i+1;
        }
      }
    }
    else {
      //day
      for (i = 0; i < 7; i++) {
        if (strcmp( iBuff, dayNames[i]) == 0) {
          return i+1;
        }
      }
    }
  }
  return atoi(iBuff);
}

void ParseTime(char* buffer) {
  uint8_t year, month, day, weekday, hours, minutes, seconds;
  hours = aBuff2int(buffer, 24, 25);
  minutes = aBuff2int(buffer, 27, 28);
  seconds = aBuff2int(buffer, 30, 31);
  RTC_SetTime(hours, minutes, seconds);
  year = aBuff2int(buffer, 35, 36);
  month = aBuff2int(buffer, 17, 19);
  day = aBuff2int(buffer, 21,22);
  weekday = aBuff2int(buffer, 13, 15);
  RTC_SetDate(weekday, day, month, year);
  printf("PARSETIME parameters => weekday: %d, year: %d, month: %d, day: %d, hours: %d, minutes: %d, seconds: %d\r\n", weekday, year, month, day, hours, minutes, seconds);
}

// Functie om de tijd in te stellen
void RTC_SetTime(uint8_t hours, uint8_t minutes, uint8_t seconds) {
    RTC_TimeTypeDef sTime = {0};

    sTime.Hours = hours;
    sTime.Minutes = minutes;
    sTime.Seconds = seconds;
    sTime.TimeFormat = RTC_HOURFORMAT_24;
    sTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
    sTime.StoreOperation = RTC_STOREOPERATION_RESET;
    
    if (HAL_RTC_SetTime(RealTime_Handle, &sTime, RTC_FORMAT_BIN) != HAL_OK) {
        // Foutafhandeling
      Debug("Error setting time to RTC");
    }
}

// Functie om de datum in te stellen
void RTC_SetDate(uint8_t weekday, uint8_t day, uint8_t month, uint8_t year) {
    RTC_DateTypeDef sDate = {0};

    sDate.WeekDay = weekday;
    sDate.Date = day;
    sDate.Month = month;
    sDate.Year = year;  // Jaartal zonder eeuw (bv. 2024 -> 24)

    if (HAL_RTC_SetDate(RealTime_Handle, &sDate, RTC_FORMAT_BIN) != HAL_OK) {
        // Foutafhandeling
      Debug("Error setting date to RTC");
    }
}

// Functie om de tijd uit te lezen
void RTC_GetTime(uint8_t *hours, uint8_t *minutes, uint8_t *seconds, uint8_t *weekday, uint8_t *day, uint8_t *month, uint8_t *year) {
    RTC_TimeTypeDef gTime = {0};
    RTC_DateTypeDef gDate = {0};

    if (HAL_RTC_GetTime(RealTime_Handle, &gTime, RTC_FORMAT_BIN) != HAL_OK) {
      Debug("Error getting time from RTC");
    }
    if (HAL_RTC_GetDate(RealTime_Handle, &gDate, RTC_FORMAT_BIN) != HAL_OK) {
      Debug("Error getting date from RTC");
    }
    *hours = gTime.Hours;
    *minutes = gTime.Minutes;
    *seconds = gTime.Seconds;
    *weekday = gDate.WeekDay;
    *day = gDate.Date;
    *month = gDate.Month;
    *year = gDate.Year;
}

// Functie om een alarm in te stellen
void RTC_SetAlarm(uint8_t hours, uint8_t minutes, uint8_t seconds) {
    RTC_AlarmTypeDef sAlarm = {0};

    sAlarm.AlarmTime.Hours = hours;
    sAlarm.AlarmTime.Minutes = minutes;
    sAlarm.AlarmTime.Seconds = seconds;
    sAlarm.AlarmTime.TimeFormat = RTC_HOURFORMAT12_AM;
    sAlarm.Alarm = RTC_ALARM_A;

    if (HAL_RTC_SetAlarm_IT(RealTime_Handle, &sAlarm, RTC_FORMAT_BIN) != HAL_OK) {
        // Foutafhandeling
    }
}

// // Callback voor als het alarm afgaat
// void HAL_RTC_AlarmAEventCallback(RTC_HandleTypeDef *hrtc) {
//     // Doe iets wanneer het alarm afgaat
//     HAL_GPIO_TogglePin(LED_C_Red_GPIO_Port, LED_C_Red_Pin);
// }

void RTC_SetWakeUpTimer(uint32_t secondsOfSleep)
{
    //Switch of the timer to reset it
    HAL_RTCEx_DeactivateWakeUpTimer(&hrtc);

    //Set the prescale so that the clock will be 1Hz
    HAL_RTCEx_SetWakeUpTimer_IT(RealTime_Handle, secondsOfSleep, RTC_WAKEUPCLOCK_CK_SPRE_16BITS);

    HAL_NVIC_SetPriority(RTC_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(RTC_IRQn);
}

void Enter_Standby_Mode(void)
{
    // Schakel de clock voor de Power Controller in
    __HAL_RCC_PWR_CLK_ENABLE();

    // Schakel Standby Mode in
    HAL_PWR_EnterSTANDBYMode();
}

void Enter_Stop_Mode(void)
{
    // Schakel de clock voor de Power Controller in
    __HAL_RCC_PWR_CLK_ENABLE();

    // Instellen op Stop mode met low-power regulator
    HAL_PWR_EnterSTOPMode(PWR_LOWPOWERREGULATOR_ON, PWR_STOPENTRY_WFI);
}

void InitClock(RTC_HandleTypeDef* h_hrtc){
  RealTime_Handle = h_hrtc;
  RTC_SetTime(RealTime.Hours, RealTime.Minutes, RealTime.Seconds);
}
