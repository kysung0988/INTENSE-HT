/*
 * adc.h
 *
 *  Created on: May 13, 2025
 *      Author: kimst
 */

#ifndef INC_ADC_H_
#define INC_ADC_H_

#include "stm32u5xx_hal.h"
#include "main.h"

void adc_channel_read_start(uint16_t *bat, int16_t *mcu, uint16_t *vref);

#endif /* INC_ADC_H_ */
