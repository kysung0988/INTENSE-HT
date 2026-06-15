/*
 * tim6.h
 *
 *  Created on: May 8, 2025
 *      Author: kimst
 */

#ifndef INC_TIM6_H_
#define INC_TIM6_H_

#include "stm32u5xx_hal.h"

void tim6_init(void);
uint32_t get_time_cnt(void);
void clear_time_cnt(void);
uint32_t get_adc_cnt(void);
void clear_adc_cnt(void);
uint32_t get_cal_cnt(void);
void clear_cal_cnt(void);
uint32_t get_calibration_cnt(void);
void clear_calibration_cnt(void);
void clear_tick_val1(void);
void clear_mission_cnt(void);
void clear_mission_start(void);

#endif /* INC_TIM6_H_ */
