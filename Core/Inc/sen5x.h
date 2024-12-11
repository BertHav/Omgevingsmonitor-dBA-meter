/*
 * sen5x.h
 *
 *  Created on: Dec 1, 2024
 *      Author: itsme
 */


#ifndef INC_SEN5X_H_
#define INC_SEN5X_H_

#include "stm32l0xx_hal.h"

#define SEN5X_FAN_SPEED_ERROR   0x00200000
#define SEN5X_FAN_CLEANING      0x00080000
#define SEN5X_GAS_SENSOR_ERROR  0x00000080
#define SEN5X_RHT_ERROR         0x00000040
#define SEN5X_LASER_ERROR       0x00000020
#define SEN5X_FAN_BLOCKED_ERROR 0x00000010


typedef enum {
  S5X_DISABLED,
  LIGHT_OUT,
  LIGHT_ON,
  CHECK_SEN5X,
  CLEAN_FAN,
  SAMPLES_TAKEN
}sen5x_states;

typedef struct
{
  uint16_t mass_concentration_pm1p0;
  uint16_t mass_concentration_pm2p5;
  uint16_t mass_concentration_pm4p0;
  uint16_t mass_concentration_pm10p0;
  int16_t ambient_humidity;
  int16_t ambient_temperature;
  int16_t voc_index;
  int16_t nox_index;
} SEN5X_DateTypeDef;

extern SEN5X_DateTypeDef sen5x_data;
extern uint32_t sen5xReadTimer;
extern uint8_t sen5xErrors;

void sen5x_Power_On(void);
void sen5x_Power_Off(void);
void reset_fanCleaningDone(void);
int16_t probe_sen5x(void);
int16_t sen5x_lightup_measurement(void);
int16_t sen5x_measurement(void);
void sensirion_i2c_hal_free(void);
int8_t sensirion_i2c_hal_read(uint8_t address, uint8_t* data, uint16_t count);
int8_t sensirion_i2c_hal_write(uint8_t address, const uint8_t* data, uint16_t count);
void sensirion_i2c_hal_sleep_usec(uint32_t useconds);
void sen5x_statemachine(uint8_t delayfactor);


#endif /* INC_SEN5X_H_ */