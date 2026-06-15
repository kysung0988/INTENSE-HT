/*
 * control.h
 *
 *  Created on: Mar 6, 2026
 *      Author: kimst
 */

#ifndef INC_CONTROL_H_
#define INC_CONTROL_H_

#include <stdint.h>
#include "main.h"

void calibration_proc(control_flag* flag);
void read_cal_data(control_flag* flag);
void rtd_calibration_func(control_flag* flag);
void rtd_calibration_func_200C_test(control_flag* flag);
void rtd_calibration_func_init(control_flag* flag);
void rtd_calibration_func_test(control_flag* flag);

void cal_data_flash_write(CAL_DATA *cal);
void cal_data_restore(uint8_t clear,CAL_DATA *cal_flag);
void flash_init(void);

#endif /* INC_CONTROL_H_ */
