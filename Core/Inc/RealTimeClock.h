#ifndef RTC_H
#define RTC_H

#include "stm32l0xx_hal.h"  // Inclusie van de benodigde HAL-bibliotheken
#include "stm32l0xx_hal_rcc.h"
#include "utils.h"


extern uint8_t lasthour;
extern uint8_t lastminute;
extern uint8_t lastsecond;
extern uint8_t weekday;
extern uint8_t day;
extern uint8_t month;
extern uint8_t year;

void ParseTime(char* buffer);
// Functies voor het instellen en uitlezen van de tijd
void showTime();
void UpdateSystemUptime();
//void RTC_SetTime(uint8_t hours, uint8_t minutes, uint8_t seconds);
void RTC_SetTime(RTC_TimeTypeDef* sTime);

// Functies voor het instellen en uitlezen van de datum
//void RTC_SetDate(uint8_t weekday, uint8_t day, uint8_t month, uint8_t year);
void RTC_SetDate(RTC_DateTypeDef* sDate);
//void RTC_GetTime(uint8_t *weekday, uint8_t *day, uint8_t *month, uint8_t *year, uint8_t *hours, uint8_t *minutes, uint8_t *seconds);
void RTC_GetTime(RTC_TimeTypeDef* gTime, RTC_DateTypeDef* gDate);
// Functie om een alarm in te stellen
void RTC_SetAlarm(uint8_t hours, uint8_t minutes, uint8_t seconds);

// Functie om een wakeup timer in te stellen
void RTC_SetWakeUpTimer(uint32_t secondsOfSleep);
void Enter_Standby_Mode(void);
void Enter_Stop_Mode(void);

void InitClock(RTC_HandleTypeDef *hrtc);
void UpdateClock();

#endif  // RTC_H

