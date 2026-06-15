/*
 * max31865.h
 *
 *  Created on: May 12, 2025
 *      Author: kimst
 */

#ifndef INC_MAX31865_H_
#define INC_MAX31865_H_

#include <stdint.h>
#include <stdbool.h>
#include "stm32u5xx_hal.h"
#include "main.h"

#if 0
#define MAX31865_READ_MASK  			0x7F
#define MAX31865_WRITE_MASK 			0x80

#define MAX31865_CONFIG_BIAS 			0x80
#define MAX31865_CONFIG_MODEAUTO 		0x40
#define MAX31865_CONFIG_MODEOFF 		0xBF
#define MAX31865_CONFIG_1SHOT 			0x20
#define MAX31865_CONFIG_3WIRE 			0x10
#define MAX31865_CONFIG_2_4WIRE 		0xEF
#define MAX31865_CONFIG_CLRFAULT_MASK 	0xD3
#define MAX31865_CONFIG_FAULTSTAT 		0x02
#define MAX31865_CONFIG_FILT50HZ 		0x01
#define MAX31865_CONFIG_FILT60HZ 		0xFE

#define MAX31865_CONFIG_REG 			0x00
#define MAX31865_RTDMSB_REG 			0x01
#define MAX31865_RTDLSB_REG 			0x02
#define MAX31865_HFAULTMSB_REG 			0x03
#define MAX31865_HFAULTLSB_REG 			0x04
#define MAX31865_LFAULTMSB_REG 			0x05
#define MAX31865_LFAULTLSB_REG 			0x06
#define MAX31865_FAULTSTAT_REG 			0x07




void max31865_init(void);
int max31865_read(uint8_t reg_addr,uint8_t *reg_data);
int max31865_reg_update(uint8_t reg_addr,uint8_t reg_update, uint8_t or_mask);
int max31865_write(uint8_t reg_addr,uint8_t reg_data);
int max31865_clear_fault(void);
int max31865_enable_bias(uint8_t en);
int max31865_auto_convert(uint8_t auto_conv_en);
int max31865_enable_50Hz(uint8_t filt_en);
int max31865_set_threshold(uint16_t lower,uint16_t upper);
int max31865_get_lower_threshold(uint16_t *low_threshold);
int max31865_get_upper_threshold(uint16_t *up_threshold);
int max31865_set_wires(uint8_t is_odd_wire);
int max31865_read_rtd(uint16_t *rtd_reg);
float max31865_rtd_to_temperature(uint16_t rtd);
float max31865_rtd_to_resist(void);
void calibaration(control_flag *flag_control);
float read_temp_without_cal(void);
float read_temp_with_cal(control_flag *flag_control);

#endif

#endif /* INC_MAX31865_H_ */
