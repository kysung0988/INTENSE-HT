/*
 * adc1.h
 *
 *  Created on: Nov 15, 2024
 *      Author: kimst
 */

#ifndef INC_ADC1_H_
#define INC_ADC1_H_

#include "stm32u5xx_hal.h"


void adc_channel_read_start(uint16_t *bat, int16_t *mcu);


#endif /* INC_ADC1_H_ */
