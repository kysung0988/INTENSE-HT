/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32u5xx_hal.h"

#include "lcd_io.h"
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);
void MX_TIM2_Init(void);

/* USER CODE BEGIN EFP */
void sensor_ldo_on(void);

#define SENSOR_TOTAL_CNT	5//32//10//4//10
#define CAL_MUL_DIV_VAL		1000000.0f//100000.0f
#define CAL_ADD_OFFSET		0.0000005f

#define CAL_MUL_DIV_VAL_UP		100000000.0f//100000.0f
#define CAL_ADD_OFFSET_UP			0.000000005f

#define CAL_MUL_DIV_VAL_NEW		1000.0f
#define CAL_ADD_OFFSET_NEW		0.0005f

#define CAL_COMPLETE_FLAG_INIT	0xFFFFFFFC
#define CAL_COMPLETE_FLAG	0xFFFFFFE0//0xfffffff8
#define CLA_COMPLETE_SEQ		7//4

#define DUMMY_CHANNEL 32

//#define DIRECT_3P_CAL


typedef struct __attribute__((__packed__))
{
	uint32_t calibration_flag;
	uint32_t ref_resist[5];//0:0.0, 1: 10000.0, 3~5: specification
	uint32_t ref_cal_resist[SENSOR_TOTAL_CNT][3];//[0]: 0ohm offset, [1]: 10000.0 real, [2]:[1]-[0]
	uint32_t pre_cal_resist[SENSOR_TOTAL_CNT][3];//3~5 read value
	int32_t cal_coeff[SENSOR_TOTAL_CNT][3];
	uint32_t cal_resist[SENSOR_TOTAL_CNT][3];//24.9K Resist Cal Value[24900000][Cal_value][0]
}CAL_DATA;

typedef struct// __attribute__((__packed__))
{
	//uint8_t ble_conn_flag;
	uint8_t ble_peri_cmd_process;
	uint8_t ble_peri_command;
	uint16_t ble_peri_cmd_length;
	uint8_t ble_peri_cmd_buf[1024];
	//uint8_t sensor_status[32];
	uint8_t charge_status_flag;

	uint8_t receive_mission;
	//uint32_t i_acquisition_time;
	uint32_t i_acquisition_time_cpy;
	uint32_t i_sample_period;
	uint32_t i_delay_time;
	uint32_t i_trigger_temp;
	float i_transi_temp;
	uint8_t i_mode[18];
	uint32_t mission_proc_cnt;
	//int32_t DoorTrigerTimeOut;

	uint8_t attemp_trigger_high;
	uint8_t attemp_trigger_low;
	uint8_t mission_flag_tri;
	uint8_t transient_flag;

	uint8_t receive_temp_read;
	uint32_t read_proc_cnt;
	uint32_t read_start_idx;
	uint32_t read_end_idx;

	uint8_t receive_calibration;
	float receive_ref_resist;
	float receive_real_resist;
	uint16_t stable_time;
	uint16_t stable_time_proc;
	uint16_t acquisition_time;
	uint16_t acquisition_time_proc;
	uint8_t rtd_status[SENSOR_TOTAL_CNT];

	CAL_DATA cal_data;
	//uint8_t ble_com_flag;
	//////////////////////////////////////////////// add
	uint8_t trigger;
	uint8_t comState;
	uint32_t samplingRate;
	uint8_t measurementState;
	uint32_t setDataSet;
	uint32_t timeDelay;
	int16_t setTriggerTemp;
	float Applytransient;
	uint32_t DoorTrigerTimeOut;
	uint32_t i_acquisition_time;
	uint8_t sensor_status[32];
	uint8_t ble_com_flag;
	uint8_t mission_start_flag;
	////////////////////////////////////////////////
}control_flag;

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define BLE_CMD1_Pin GPIO_PIN_0
#define BLE_CMD1_GPIO_Port GPIOF
#define BLE_CMD2_Pin GPIO_PIN_1
#define BLE_CMD2_GPIO_Port GPIOF
#define SPI2_CS_Pin GPIO_PIN_0
#define SPI2_CS_GPIO_Port GPIOC
#define SPI2_RD_Pin GPIO_PIN_3
#define SPI2_RD_GPIO_Port GPIOC
#define SPI1_CS_Pin GPIO_PIN_5
#define SPI1_CS_GPIO_Port GPIOA
#define CH_STAT_Pin GPIO_PIN_1
#define CH_STAT_