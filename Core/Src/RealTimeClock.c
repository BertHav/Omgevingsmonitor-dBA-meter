#include "rtc.h"
#include "RealTimeClock.h"
#include "sen5x.h"
#include <string.h>
#include <stdlib.h>

typedef struct {
  uint8_t Hours;
  uint8_t Minutes;
  uint8_t Seconds;
}Clock;

bool firstTimeUpdate = true;
//static Clock RealTime = {.Hours = 0, .Minutes = 0, .Seconds = 0};
//static Clock NextAction = {.Hours = 0, .Minutes = 0, .Seconds = 0};
static const char *dayNames[7] = {  "Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun"}; // 1 to 7
static const char *monthNames[12] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"}; // 1 to 12
static RTC_HandleTypeDef * RealTime_Handle;
char systemUptime[16] = {0};

uint8_t iMinute = 0;
uint8_t myUptimeminute = 0;
uint8_t myUptimehour = 0;
uint16_t myUptimeday = 0;

void showTime() {
  RTC_TimeTypeDef currentTime;
  RTC_DateTypeDef currentDate;
  RTC_GetTime(&currentTime, &currentDate);
  printf("System time: %02d-%02d-%02d %02dh:%02dm:%02ds, system uptime is: %dd %02dh:%02dm\r\n",
      currentDate.Year, currentDate.Month, currentDate.Date, currentTime.Hours, currentTime.Minutes,
      currentTime.Seconds, myUptimeday, myUptimehour, myUptimeminute);
}

void setiMinute(uint8_t minute) {
  iMinute = minute;
  Debug("iMinute set to: %d", iMinute);
}

void UpdateSystemUptime() {
  RTC_TimeTypeDef currentTime;
  RTC_DateTypeDef currentDate;
  RTC_GetTime(&currentTime, &currentDate);
  if (iMinute != currentTime.Minutes) {
    iMinute = currentTime.Minutes;
    myUptimeminute++;
  }
  if (myUptimeminute == 60) {
    myUptimeminute = 0;
    myUptimehour++;
    if (myUptimehour == 24) {
      myUptimehour = 0;
      myUptimeday++;
    }
  }
  Info("Current time is: %02d:%02d:%02d System uptime is: %dd %02dh:%02dm",
      currentTime.Hours, currentTime.Minutes, currentTime.Seconds, myUptimeday, myUptimehour, myUptimeminute);
}

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
  RTC_TimeTypeDef currentTime;
  RTC_DateTypeDef currentDate;
  RTC_GetTime(&currentTime, &currentDate);
  currentTime.Hours = aBuff2int(buffer, 24, 25);
  currentTime.Minutes = aBuff2int(buffer, 27, 28);
  currentTime.Seconds = aBuff2int(buffer, 30, 31);
  currentDate.Year = aBuff2int(buffer, 35, 36);
  currentDate.Month = aBuff2int(buffer, 17, 19);
  currentDate.Date = aBuff2int(buffer, 21,22);
  currentDate.WeekDay = aBuff2int(buffer, 13, 15);
  Debug("Current RTC time before update is: %02dh:%02dm:%02ds", currentTime.Hours , currentTime.Minutes, currentTime.Seconds);
  RTC_SetTime(&currentTime);
  RTC_SetDate(&currentDate);
  if (currentDate.WeekDay == 2) {
    reset_fanCleaningDone();
  }
//  Debug("PARSETIME parameters => weekday: %d, year: %d, month: %d, day: %d, hours: %d, minutes: %d, seconds: %d", weekday, year, month, day, hours, minutes, seconds);
}

// Functie om de tijd in te stellen
void RTC_SetTime(RTC_TimeTypeDef* sTime) {
    sTime->TimeFormat = RTC_HOURFORMAT_24;
    sTime->DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
    sTime->StoreOperation = RTC_STOREOPERATION_RESET;
    
    if (HAL_RTC_SetTime(RealTime_Handle, sTime, RTC_FORMAT_BIN) != HAL_OK) {
        // Foutafhandeling
      Error("Error setting time to RTC");
    }
    if (firstTimeUpdate) {
      firstTimeUpdate = false;
      setiMinute(sTime->Minutes);
    }
}

// Functie om de datum in te stellen
void RTC_SetDate(RTC_DateTypeDef* sDate) {
    if (HAL_RTC_SetDate(RealTime_Handle, sDate, RTC_FORMAT_BIN) != HAL_OK) {
        // Foutafhandeling
      Error("Error setting date to RTC");
    }
}

// Functie om de tijd uit te lezen
void RTC_GetTime(RTC_TimeTypeDef* gTime, RTC_DateTypeDef* gDate) {
    if (HAL_RTC_GetTime(RealTime_Handle, gTime, RTC_FORMAT_BIN) != HAL_OK) {
      Error("Error getting time from RTC");
    }
    if (HAL_RTC_GetDate(RealTime_Handle, gDate, RTC_FORMAT_BIN) != HAL_OK) {
      Error("Error getting date from RTC");
    }
}

uint8_t RTC_GetWeekday(void) {
  RTC_TimeTypeDef currentTime;
  RTC_DateTypeDef currentDate;
  RTC_GetTime(&currentTime, &currentDate);
  return currentDate.WeekDay;
}

// Functie om een alarm in te stellen
void RTC_SetAlarm(uint8_t hours, uint8_t minutes, uint8_t seconds) {
    RTC_AlarmTypeDef sAlarm = {0};

/*
// alarm elk uur
sAlarm.AlarmMask  = (RTC_ALARMMASK_DATEWEEKDAY|RTC_ALARMMASK_HOURS) ;
sAlarm.AlarmTime.Seconds = 0;
sAlarm.AlarmTime.Minutes = 0;
sAlarm.AlarmTime.SubSeconds = 0;
sAlarm.AlarmSubSecondMask   = RTC_ALARMSUBSECONDMASK_NONE;
ALARM_ActivateAlarm(&sAlarm);

 */

    sAlarm.AlarmTime.Hours = hours;
    sAlarm.AlarmTime.Minutes = minutes;
    sAlarm.AlarmTime.Seconds = seconds;
    sAlarm.AlarmTime.TimeFormat = RTC_HOURFORMAT12_AM;
    sAlarm.Alarm = RTC_ALARM_A;

    if (HAL_RTC_SetAlarm_IT(RealTime_Handle, &sAlarm, RTC_FORMAT_BIN) != HAL_OK) {
        // Foutafhandeling
      Error("Error activating interrupt voor RTC Alarm time");
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
}
