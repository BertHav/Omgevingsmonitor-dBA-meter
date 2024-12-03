/*
 * sen5x.h
 *
 *  Created on: Dec 1, 2024
 *      Author: itsme
 */

#include "stm32l0xx_hal.h"


#ifndef INC_SEN5X_H_
#define INC_SEN5X_H_

typedef enum {
  LIGHT_OUT,
  LIGHT_ON,
  SAMPLES_TAKEN
}sen5x_states;

void sen5x_Power_On(void);
void sen5x_Power_Off(void);
int16_t probe_sen5x(void);
int16_t sen5x_lightup_measurement(void);
int16_t sen5x_measurement(void);
void sensirion_i2c_hal_free(void);
int8_t sensirion_i2c_hal_read(uint8_t address, uint8_t* data, uint16_t count);
int8_t sensirion_i2c_hal_write(uint8_t address, const uint8_t* data, uint16_t count);
void sensirion_i2c_hal_sleep_usec(uint32_t useconds);
void sen5x_statemachine(void);


#endif /* INC_SEN5X_H_ */
