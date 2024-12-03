/*
 * sen5x.c
 *
 *  Created on: Dec 1, 2024
 *      Author: Bert Havinga, parts from KITT and Sensirion grouped together
 */
#include "stm32l0xx_hal.h"
#include "sen5x.h"
#include "main.h"
#include "sen5x_i2c.h"
#include "sensirion_common.h"
#include "sensirion_i2c.h"
#include "utils.h"
#include "i2c.h"

uint32_t sen5xReadTimer = 0;
uint8_t sen5xSamples = 0;
static sen5x_states samplesState = LIGHT_OUT;

void sen5x_Power_On(void) {
//  Debug("executing sen5x_Power_On");
  HAL_GPIO_WritePin(Boost_Enable_GPIO_Port, Boost_Enable_Pin, GPIO_PIN_SET);
  HAL_Delay(200);
  return;
}

void sen5x_Power_Off(void) {
//  Debug("executing sen5x_Power_Off");
  HAL_GPIO_WritePin(Boost_Enable_GPIO_Port, Boost_Enable_Pin, GPIO_PIN_RESET);
  return;
}

int16_t probe_sen5x(void) {
//  Debug("test for sen5x_device");
  int16_t error = 0;
  unsigned char serial_number[32];
  uint8_t serial_number_size = 32;
  unsigned char product_name[32];
  uint8_t product_name_size = 32;
  sen5x_Power_On();  // switch buck converter

  error = sen5x_device_reset();
  if (error) {
      Error("Error executing sen5x_device_reset(): %i", error);
      return error;
  }
  error = sen5x_get_serial_number(serial_number, serial_number_size);
  if (error) {
      printf("Error executing sen5x_get_serial_number(): %i\r\n", error);
      return error;
  } else {
      printf("Serial number: %s\r\n", serial_number);
  }
  error = sen5x_get_product_name(product_name, product_name_size);
  if (error) {
      printf("Error executing sen5x_get_product_name(): %i\r\n", error);
      return error;
  } else {
      printf("Product name: %s\r\n", product_name);
  }

  uint8_t firmware_major;
  uint8_t firmware_minor;
  bool firmware_debug;
  uint8_t hardware_major;
  uint8_t hardware_minor;
  uint8_t protocol_major;
  uint8_t protocol_minor;
  error = sen5x_get_version(&firmware_major, &firmware_minor, &firmware_debug,
                            &hardware_major, &hardware_minor, &protocol_major,
                            &protocol_minor);

  if (error) {
      printf("Error executing sen5x_get_version(): %i\r\n", error);
      return error;
  } else {
      printf("Firmware: %u.%u, Hardware: %u.%u\r\n", firmware_major,
             firmware_minor, hardware_major, hardware_minor);
  }


// set a temperature offset - supported by SEN54 and SEN55 sensors
//
// By default, the temperature and humidity outputs from the sensor
// are compensated for the modules self-heating. If the module is
// designed into a device, the temperature compensation might need
// to be adapted to incorporate the change in thermal coupling and
// self-heating of other device components.
//
// A guide to achieve optimal performance, including references
// to mechanical design-in examples can be found in the app note
// “SEN5x – Temperature Compensation Instruction�? at www.sensirion.com.
// Please refer to those application notes for further information
// on the advanced compensation settings used in
// `sen5x_set_temperature_offset_parameters`,
// `sen5x_set_warm_start_parameter` and
// `sen5x_set_rht_acceleration_mode`.
//
// Adjust temp_offset in degrees celsius to account for additional
// temperature offsets exceeding the SEN module's self heating.
  float temp_offset = 0.0f;
  int16_t default_slope = 0;
  uint16_t default_time_constant = 0;
  error = sen5x_set_temperature_offset_parameters(
      (int16_t)(200 * temp_offset), default_slope, default_time_constant);
  if (error) {
      Error("Error executing sen5x_set_temperature_offset_parameters(): %i", error);
      return error;
  } else {
      Info("Temperature Offset set to %.2f °C (SEN54/SEN55 only)", temp_offset);
  }
  sen5x_Power_Off();
  sen5xReadTimer  = HAL_GetTick() + 25000; // after 25 second first measurement
  return error;
}

int16_t sen5x_lightup_measurement(void) {
// Start Measurement
  int16_t error = 0;
//  Debug("entering sen5x_lightup_measurement");
  error = sen5x_start_measurement();
  if (error) {
      Error("Error executing sen5x_lightup_measurement(): %i", error);
  }
  return error;
}

int16_t sen5x_extinguish_measurement(void) {
// Stop Measurement
//  Debug("entering sen5x_extinguish_measurement");
  int16_t error = 0;
  error = sen5x_stop_measurement();
  if (error) {
    Error("Error executing sen5x_stop_measurement(): %i", error);
  }
  return error;
}

int16_t sen5x_measurement(void) {
  // Read Measurement
  int16_t error = 0;
//  sensirion_i2c_hal_sleep_usec(1000000);  // 1 measurement / second
//  Debug("entering sen5x_measurement");

  uint16_t mass_concentration_pm1p0;
  uint16_t mass_concentration_pm2p5;
  uint16_t mass_concentration_pm4p0;
  uint16_t mass_concentration_pm10p0;
  int16_t ambient_humidity;
  int16_t ambient_temperature;
  int16_t voc_index;
  int16_t nox_index;

  error = sen5x_read_measured_values(
          &mass_concentration_pm1p0, &mass_concentration_pm2p5,
          &mass_concentration_pm4p0, &mass_concentration_pm10p0,
          &ambient_humidity, &ambient_temperature, &voc_index, &nox_index);

  if (error) {
    Error("Error executing sen5x_read_measured_values(): %i", error);
    return error;
  } else {
    if (sen5xSamples == 0) {
      return 0; // first sample reads zero's
    }
    printf("Mass concentration pm1p0: %.1f µg/m³\r\n", mass_concentration_pm1p0 / 10.0f);
    printf("Mass concentration pm2p5: %.1f µg/m³\r\n", mass_concentration_pm2p5 / 10.0f);
    printf("Mass concentration pm4p0: %.1f µg/m³\r\n", mass_concentration_pm4p0 / 10.0f);
    printf("Mass concentration pm10p0: %.1f µg/m³\r\n", mass_concentration_pm10p0 / 10.0f);
    if (ambient_humidity != 0x7fff) {
      printf("Ambient humidity: %.1f %%RH\r\n", ambient_humidity / 100.0f);
    }
    if (ambient_temperature != 0x7fff) {
      printf("Ambient temperature: %.1f °C\r\n", ambient_temperature / 200.0f);
    }
    if (voc_index != 0x7fff) {
      printf("Voc index: %.1f\r\n", voc_index / 10.0f);
    }
    if (nox_index != 0x7fff) {
      printf("Nox index: %.1f\r\n", nox_index / 10.0f);
    }
  }
  return error;
}

/**
 * Release all resources initialized by sensirion_i2c_hal_init().
*/

void sensirion_i2c_hal_free(void) {
//  Debug("entering sensirion_i2c_hal_free");
  HAL_GPIO_WritePin(Boost_Enable_GPIO_Port, Boost_Enable_Pin, GPIO_PIN_RESET);
}

/**
 * Execute one read transaction on the I2C bus, reading a given number of bytes.
 * If the device does not acknowledge the read command, an error shall be
 * returned.
 *
 * @param address 7-bit I2C address to read from
 * @param data    pointer to the buffer where the data is to be stored
 * @param count   number of bytes to read from I2C and store in the buffer
 * @returns 0 on success, error code otherwise
 */

int8_t sensirion_i2c_hal_read(uint8_t address, uint8_t* data, uint16_t count) {
  return (int8_t)HAL_I2C_Master_Receive(&hi2c2, (uint16_t)(address << 1), data, count, 100);
}

/**
 * Execute one write transaction on the I2C bus, sending a given number of
 * bytes. The bytes in the supplied buffer must be sent to the given address. If
 * the slave device does not acknowledge any of the bytes, an error shall be
 * returned.
 *
 * @param address 7-bit I2C address to write to
 * @param data    pointer to the buffer containing the data to write
 * @param count   number of bytes to read from the buffer and send over I2C
 * @returns 0 on success, error code otherwise
 */

int8_t sensirion_i2c_hal_write(uint8_t address, const uint8_t* data, uint16_t count) {
  return (int8_t)HAL_I2C_Master_Transmit(&hi2c2, (uint16_t)(address << 1), (uint8_t*)data, count, 100);
}

/**
 * Sleep for a given number of microseconds. The function should delay the
 * execution for at least the given time, but may also sleep longer.
 *
 * @param useconds the sleep time in microseconds
 */

void sensirion_i2c_hal_sleep_usec(uint32_t useconds) {
  uint32_t msec = useconds / 1000;
  if (useconds % 1000 > 0) {
    msec++;
  }
  HAL_Delay(msec);
}

void sen5x_statemachine(void) {
  if (TimestampIsReached(sen5xReadTimer)) {
    switch (samplesState) {
    case LIGHT_OUT:
//      Debug(" state is LIGHT_OUT");
      sen5x_Power_On();
      if (sen5x_lightup_measurement()) {
        Error("Error executing sen5x_lightup_measurement()");
      }
      samplesState = LIGHT_ON;
      sen5xReadTimer = HAL_GetTick() + 1000;
      break;
    case LIGHT_ON:
//      Debug(" state is LIGHT_ON");
      if (sen5x_measurement()) {
        Error("Error executing sen5x_measurement()");
      }
      if (sen5xSamples >= 3) { // show 3 samples
        samplesState = SAMPLES_TAKEN;
      }
      sen5xSamples++;
      sen5xReadTimer = HAL_GetTick() + 1000;
      break;
    case SAMPLES_TAKEN:
//      Debug(" state is SAMPLES_TAKEN");
      sen5xSamples = 0;
      if (sen5x_extinguish_measurement()) {
        Error("Error executing sen5x_extinguish_measurement()");
      }
      sen5x_Power_Off();
      sen5xReadTimer = HAL_GetTick() + 59000; //
      samplesState = LIGHT_OUT;
    }
  }
}
