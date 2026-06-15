/*
 * i2c_gpio.c
 *
 *  Created on: Jan 3, 2025
 *      Author: kimst
 */

#include "i2c_gpio.h"
#include "main.h"
#include <stdio.h>
#include <string.h>
#include "adc1.h"

#define I2C_DELAY_VAL	100
#define I2C_WAIT_ACK	100

#define TMP117_I2C_ADDR_G		0x90  //GND
#define TMP117_I2C_ADDR_V		0x92  //V+
#define TMP117_I2C_ADDR_D		0x94  //SDA
#define TMP117_I2C_ADDR_C		0x96  //SCL

#define I2C_ADDR_SET	1
#define I2C_DATA_SET	0
#define BLANK_DATA		0
#define I2C_WRITE		0
#define I2C_READ		1

Temp_Port TMP_LIST[SENSOR_TOTAL_CNT+1]=
{
	{0,0,0,0,0},
#ifdef NOTCH
	{TMP117_I2C_ADDR_G,G1_SDA_GPIO_Port,G1_SDA_Pin,G1_SCL_GPIO_Port,G1_SCL_Pin},// S1(0)
	{TMP117_I2C_ADDR_C,G2_SDA_GPIO_Port,G2_SDA_Pin,G2_SCL_GPIO_Port,G2_SCL_Pin},// S2
	{TMP117_I2C_ADDR_D,G2_SDA_GPIO_Port,G2_SDA_Pin,G2_SCL_GPIO_Port,G2_SCL_Pin},// S3
	{TMP117_I2C_ADDR_G,G2_SDA_GPIO_Port,G2_SDA_Pin,G2_SCL_GPIO_Port,G2_SCL_Pin},// S4
	{TMP117_I2C_ADDR_V,G2_SDA_GPIO_Port,G2_SDA_Pin,G2_SCL_GPIO_Port,G2_SCL_Pin},// S5(4)

	{TMP117_I2C_ADDR_D,G5_SDA_GPIO_Port,G5_SDA_Pin,G5_SCL_GPIO_Port,G5_SCL_Pin},  // S6(5)
	{TMP117_I2C_ADDR_D,G3_SDA_GPIO_Port,G3_SDA_Pin,G3_SCL_GPIO_Port,G3_SCL_Pin},  // S7
	{TMP117_I2C_ADDR_D,G10_SDA_GPIO_Port,G10_SDA_Pin,G10_SCL_GPIO_Port,G10_SCL_Pin},// S8
	{TMP117_I2C_ADDR_D,G12_SDA_GPIO_Port,G12_SDA_Pin,G12_SCL_GPIO_Port,G12_SCL_Pin},// S9
	{TMP117_I2C_ADDR_D,G13_SDA_GPIO_Port,G13_SDA_Pin,G13_SCL_GPIO_Port,G13_SCL_Pin},  // S10(9)

	{TMP117_I2C_ADDR_D,G14_SDA_GPIO_Port,G14_SDA_Pin,G14_SCL_GPIO_Port,G14_SCL_Pin},  // S11(10)
	{TMP117_I2C_ADDR_D,G8_SDA_GPIO_Port,G8_SDA_Pin,G8_SCL_GPIO_Port,G8_SCL_Pin},  // S12
	{TMP117_I2C_ADDR_D,G7_SDA_GPIO_Port,G7_SDA_Pin,G7_SCL_GPIO_Port,G7_SCL_Pin},  // S13
	{TMP117_I2C_ADDR_C,G5_SDA_GPIO_Port,G5_SDA_Pin,G5_SCL_GPIO_Port,G5_SCL_Pin},// S14
	{TMP117_I2C_ADDR_C,G3_SDA_GPIO_Port,G3_SDA_Pin,G3_SCL_GPIO_Port,G3_SCL_Pin},// S15(14)

	{TMP117_I2C_ADDR_C,G10_SDA_GPIO_Port,G10_SDA_Pin,G10_SCL_GPIO_Port,G10_SCL_Pin},// S16(15)
	{TMP117_I2C_ADDR_C,G12_SDA_GPIO_Port,G12_SDA_Pin,G12_SCL_GPIO_Port,G12_SCL_Pin},// S17
	{TMP117_I2C_ADDR_C,G13_SDA_GPIO_Port,G13_SDA_Pin,G13_SCL_GPIO_Port,G13_SCL_Pin},  // S18
	{TMP117_I2C_ADDR_C,G14_SDA_GPIO_Port,G14_SDA_Pin,G14_SCL_GPIO_Port,G14_SCL_Pin},  // S19
	{TMP117_I2C_ADDR_C,G8_SDA_GPIO_Port,G8_SDA_Pin,G8_SCL_GPIO_Port,G8_SCL_Pin},  // S20(19)

	{TMP117_I2C_ADDR_C,G7_SDA_GPIO_Port,G7_SDA_Pin,G7_SCL_GPIO_Port,G7_SCL_Pin}, // S21
	{TMP117_I2C_ADDR_V,G5_SDA_GPIO_Port,G5_SDA_Pin,G5_SCL_GPIO_Port,G5_SCL_Pin}, // S22
	{TMP117_I2C_ADDR_D,G4_SDA_GPIO_Port,G4_SDA_Pin,G4_SCL_GPIO_Port,G4_SCL_Pin}, // S23
	{TMP117_I2C_ADDR_V,G10_SDA_GPIO_Port,G10_SDA_Pin,G10_SCL_GPIO_Port,G10_SCL_Pin}, // S24
	{TMP117_I2C_ADDR_D,G11_SDA_GPIO_Port,G11_SDA_Pin,G11_SCL_GPIO_Port,G11_SCL_Pin}, // S25(24)

	{TMP117_I2C_ADDR_V,G13_SDA_GPIO_Port,G13_SDA_Pin,G13_SCL_GPIO_Port,G13_SCL_Pin},   // S26
	{TMP117_I2C_ADDR_D,G9_SDA_GPIO_Port,G9_SDA_Pin,G9_SCL_GPIO_Port,G9_SCL_Pin},   // S27
	{TMP117_I2C_ADDR_V,G8_SDA_GPIO_Port,G8_SDA_Pin,G8_SCL_GPIO_Port,G8_SCL_Pin},   // S28
	{TMP117_I2C_ADDR_D,G6_SDA_GPIO_Port,G6_SDA_Pin,G6_SCL_GPIO_Port,G6_SCL_Pin},  // S29
	{TMP117_I2C_ADDR_C,G4_SDA_GPIO_Port,G4_SDA_Pin,G4_SCL_GPIO_Port,G4_SCL_Pin}, // S30(29)

	{TMP117_I2C_ADDR_C,G11_SDA_GPIO_Port,G11_SDA_Pin,G11_SCL_GPIO_Port,G11_SCL_Pin}, // S31
	{TMP117_I2C_ADDR_C,G9_SDA_GPIO_Port,G9_SDA_Pin,G9_SCL_GPIO_Port,G9_SCL_Pin}, // S32
	{TMP117_I2C_ADDR_C,G6_SDA_GPIO_Port,G6_SDA_Pin,G6_SCL_GPIO_Port,G6_SCL_Pin}, // S33
	{TMP117_I2C_ADDR_V,G4_SDA_GPIO_Port,G4_SDA_Pin,G4_SCL_GPIO_Port,G4_SCL_Pin},   // S34
	{TMP117_I2C_ADDR_V,G11_SDA_GPIO_Port,G11_SDA_Pin,G11_SCL_GPIO_Port,G11_SCL_Pin},   // S35(34)

	{TMP117_I2C_ADDR_V,G9_SDA_GPIO_Port,G9_SDA_Pin,G9_SCL_GPIO_Port,G9_SCL_Pin},   // S36
	{TMP117_I2C_ADDR_V,G6_SDA_GPIO_Port,G6_SDA_Pin,G6_SCL_GPIO_Port,G6_SCL_Pin}, // S37
	{TMP117_I2C_ADDR_G,G5_SDA_GPIO_Port,G5_SDA_Pin,G5_SCL_GPIO_Port,G5_SCL_Pin}, // S38
	{TMP117_I2C_ADDR_V,G3_SDA_GPIO_Port,G3_SDA_Pin,G3_SCL_GPIO_Port,G3_SCL_Pin}, // S39
	{TMP117_I2C_ADDR_G,G4_SDA_GPIO_Port,G4_SDA_Pin,G4_SCL_GPIO_Port,G4_SCL_Pin}, // S40(39)

	{TMP117_I2C_ADDR_G,G3_SDA_GPIO_Port,G3_SDA_Pin,G3_SCL_GPIO_Port,G3_SCL_Pin}, // S41
	{TMP117_I2C_ADDR_G,G10_SDA_GPIO_Port,G10_SDA_Pin,G10_SCL_GPIO_Port,G10_SCL_Pin},   // S42
	{TMP117_I2C_ADDR_V,G12_SDA_GPIO_Port,G12_SDA_Pin,G12_SCL_GPIO_Port,G12_SCL_Pin},   // S43
	{TMP117_I2C_ADDR_G,G11_SDA_GPIO_Port,G11_SDA_Pin,G11_SCL_GPIO_Port,G11_SCL_Pin},   // S44
	{TMP117_I2C_ADDR_G,G12_SDA_GPIO_Port,G12_SDA_Pin,G12_SCL_GPIO_Port,G12_SCL_Pin}, // S45(44)

	{TMP117_I2C_ADDR_G,G13_SDA_GPIO_Port,G13_SDA_Pin,G13_SCL_GPIO_Port,G13_SCL_Pin}, // S46
	{TMP117_I2C_ADDR_V,G14_SDA_GPIO_Port,G14_SDA_Pin,G14_SCL_GPIO_Port,G14_SCL_Pin}, // S47
	{TMP117_I2C_ADDR_G,G9_SDA_GPIO_Port,G9_SDA_Pin,G9_SCL_GPIO_Port,G9_SCL_Pin}, // S48
	{TMP117_I2C_ADDR_G,G14_SDA_GPIO_Port,G14_SDA_Pin,G14_SCL_GPIO_Port,G14_SCL_Pin}, // S49
	{TMP117_I2C_ADDR_G,G8_SDA_GPIO_Port,G8_SDA_Pin,G8_SCL_GPIO_Port,G8_SCL_Pin},   // S50(49)

	{TMP117_I2C_ADDR_V,G7_SDA_GPIO_Port,G7_SDA_Pin,G7_SCL_GPIO_Port,G7_SCL_Pin},   // S51
	{TMP117_I2C_ADDR_G,G6_SDA_GPIO_Port,G6_SDA_Pin,G6_SCL_GPIO_Port,G6_SCL_Pin},   // S52
	{TMP117_I2C_ADDR_G,G7_SDA_GPIO_Port,G7_SDA_Pin,G7_SCL_GPIO_Port,G7_SCL_Pin}   // S53

#else
	{TMP117_I2C_ADDR_G,G1_SDA_GPIO_Port,G1_SDA_Pin,G1_SCL_GPIO_Port,G1_SCL_Pin},// S1(0)
	{TMP117_I2C_ADDR_D,G2_SDA_GPIO_Port,G2_SDA_Pin,G2_SCL_GPIO_Port,G2_SCL_Pin},// S2
	{TMP117_I2C_ADDR_G,G2_SDA_GPIO_Port,G2_SDA_Pin,G2_SCL_GPIO_Port,G2_SCL_Pin},// S3
	{TMP117_I2C_ADDR_V,G2_SDA_GPIO_Port,G2_SDA_Pin,G2_SCL_GPIO_Port,G2_SCL_Pin},// S4
	{TMP117_I2C_ADDR_C,G2_SDA_GPIO_Port,G2_SDA_Pin,G2_SCL_GPIO_Port,G2_SCL_Pin},// S5(4)

	{TMP117_I2C_ADDR_D,G10_SDA_GPIO_Port,G10_SDA_Pin,G10_SCL_GPIO_Port,G10_SCL_Pin},  // S6(5)
	{TMP117_I2C_ADDR_D,G12_SDA_GPIO_Port,G12_SDA_Pin,G12_SCL_GPIO_Port,G12_SCL_Pin},  // S7
	{TMP117_I2C_ADDR_D,G13_SDA_GPIO_Port,G13_SDA_Pin,G13_SCL_GPIO_Port,G13_SCL_Pin},// S8
	{TMP117_I2C_ADDR_D,G14_SDA_GPIO_Port,G14_SDA_Pin,G14_SCL_GPIO_Port,G14_SCL_Pin},// S9
	{TMP117_I2C_ADDR_D,G8_SDA_GPIO_Port,G8_SDA_Pin,G8_SCL_GPIO_Port,G8_SCL_Pin},  // S10(9)

	{TMP117_I2C_ADDR_D,G7_SDA_GPIO_Port,G7_SDA_Pin,G7_SCL_GPIO_Port,G7_SCL_Pin},  // S11(10)
	{TMP117_I2C_ADDR_D,G5_SDA_GPIO_Port,G5_SDA_Pin,G5_SCL_GPIO_Port,G5_SCL_Pin},  // S12
	{TMP117_I2C_ADDR_D,G3_SDA_GPIO_Port,G3_SDA_Pin,G3_SCL_GPIO_Port,G3_SCL_Pin},  // S13
	{TMP117_I2C_ADDR_C,G10_SDA_GPIO_Port,G10_SDA_Pin,G10_SCL_GPIO_Port,G10_SCL_Pin},// S14
	{TMP117_I2C_ADDR_C,G12_SDA_GPIO_Port,G12_SDA_Pin,G12_SCL_GPIO_Port,G12_SCL_Pin},// S15(14)

	{TMP117_I2C_ADDR_C,G13_SDA_GPIO_Port,G13_SDA_Pin,G13_SCL_GPIO_Port,G13_SCL_Pin},// S16(15)
	{TMP117_I2C_ADDR_C,G14_SDA_GPIO_Port,G14_SDA_Pin,G14_SCL_GPIO_Port,G14_SCL_Pin},// S17
	{TMP117_I2C_ADDR_C,G8_SDA_GPIO_Port,G8_SDA_Pin,G8_SCL_GPIO_Port,G8_SCL_Pin},  // S18
	{TMP117_I2C_ADDR_C,G7_SDA_GPIO_Port,G7_SDA_Pin,G7_SCL_GPIO_Port,G7_SCL_Pin},  // S19
	{TMP117_I2C_ADDR_C,G5_SDA_GPIO_Port,G5_SDA_Pin,G5_SCL_GPIO_Port,G5_SCL_Pin},  // S20(19)

	{TMP117_I2C_ADDR_C,G3_SDA_GPIO_Port,G3_SDA_Pin,G3_SCL_GPIO_Port,G3_SCL_Pin}, // S21
	{TMP117_I2C_ADDR_V,G10_SDA_GPIO_Port,G10_SDA_Pin,G10_SCL_GPIO_Port,G10_SCL_Pin}, // S22
	{TMP117_I2C_ADDR_D,G11_SDA_GPIO_Port,G11_SDA_Pin,G11_SCL_GPIO_Port,G11_SCL_Pin}, // S23
	{TMP117_I2C_ADDR_V,G13_SDA_GPIO_Port,G13_SDA_Pin,G13_SCL_GPIO_Port,G13_SCL_Pin}, // S24
	{TMP117_I2C_ADDR_D,G9_SDA_GPIO_Port,G9_SDA_Pin,G9_SCL_GPIO_Port,G9_SCL_Pin}, // S25(24)

	{TMP117_I2C_ADDR_V,G8_SDA_GPIO_Port,G8_SDA_Pin,G8_SCL_GPIO_Port,G8_SCL_Pin},   // S26
	{TMP117_I2C_ADDR_D,G6_SDA_GPIO_Port,G6_SDA_Pin,G6_SCL_GPIO_Port,G6_SCL_Pin},   // S27
	{TMP117_I2C_ADDR_V,G5_SDA_GPIO_Port,G5_SDA_Pin,G5_SCL_GPIO_Port,G5_SCL_Pin},   // S28
	{TMP117_I2C_ADDR_D,G4_SDA_GPIO_Port,G4_SDA_Pin,G4_SCL_GPIO_Port,G4_SCL_Pin},  // S29
	{TMP117_I2C_ADDR_C,G11_SDA_GPIO_Port,G11_SDA_Pin,G11_SCL_GPIO_Port,G11_SCL_Pin}, // S30(29)

	{TMP117_I2C_ADDR_C,G9_SDA_GPIO_Port,G9_SDA_Pin,G9_SCL_GPIO_Port,G9_SCL_Pin}, // S31
	{TMP117_I2C_ADDR_C,G6_SDA_GPIO_Port,G6_SDA_Pin,G6_SCL_GPIO_Port,G6_SCL_Pin}, // S32
	{TMP117_I2C_ADDR_C,G4_SDA_GPIO_Port,G4_SDA_Pin,G4_SCL_GPIO_Port,G4_SCL_Pin}, // S33
	{TMP117_I2C_ADDR_V,G11_SDA_GPIO_Port,G11_SDA_Pin,G11_SCL_GPIO_Port,G11_SCL_Pin},   // S34
	{TMP117_I2C_ADDR_V,G9_SDA_GPIO_Port,G9_SDA_Pin,G9_SCL_GPIO_Port,G9_SCL_Pin},   // S35(34)

	{TMP117_I2C_ADDR_V,G6_SDA_GPIO_Port,G6_SDA_Pin,G6_SCL_GPIO_Port,G6_SCL_Pin},   // S36
	{TMP117_I2C_ADDR_V,G4_SDA_GPIO_Port,G4_SDA_Pin,G4_SCL_GPIO_Port,G4_SCL_Pin}, // S37
	{TMP117_I2C_ADDR_G,G10_SDA_GPIO_Port,G10_SDA_Pin,G10_SCL_GPIO_Port,G10_SCL_Pin}, // S38
	{TMP117_I2C_ADDR_V,G12_SDA_GPIO_Port,G12_SDA_Pin,G12_SCL_GPIO_Port,G12_SCL_Pin}, // S39
	{TMP117_I2C_ADDR_G,G11_SDA_GPIO_Port,G11_SDA_Pin,G11_SCL_GPIO_Port,G11_SCL_Pin}, // S40(39)

	{TMP117_I2C_ADDR_G,G12_SDA_GPIO_Port,G12_SDA_Pin,G12_SCL_GPIO_Port,G12_SCL_Pin}, // S41
	{TMP117_I2C_ADDR_G,G13_SDA_GPIO_Port,G13_SDA_Pin,G13_SCL_GPIO_Port,G13_SCL_Pin},   // S42
	{TMP117_I2C_ADDR_V,G14_SDA_GPIO_Port,G14_SDA_Pin,G14_SCL_GPIO_Port,G14_SCL_Pin},   // S43
	{TMP117_I2C_ADDR_G,G9_SDA_GPIO_Port,G9_SDA_Pin,G9_SCL_GPIO_Port,G9_SCL_Pin},   // S44
	{TMP117_I2C_ADDR_G,G14_SDA_GPIO_Port,G14_SDA_Pin,G14_SCL_GPIO_Port,G14_SCL_Pin}, // S45(44)

	{TMP117_I2C_ADDR_G,G8_SDA_GPIO_Port,G8_SDA_Pin,G8_SCL_GPIO_Port,G8_SCL_Pin}, // S46
	{TMP117_I2C_ADDR_V,G7_SDA_GPIO_Port,G7_SDA_Pin,G7_SCL_GPIO_Port,G7_SCL_Pin}, // S47
	{TMP117_I2C_ADDR_G,G6_SDA_GPIO_Port,G6_SDA_Pin,G6_SCL_GPIO_Port,G6_SCL_Pin}, // S48
	{TMP117_I2C_ADDR_G,G7_SDA_GPIO_Port,G7_SDA_Pin,G7_SCL_GPIO_Port,G7_SCL_Pin}, // S49
	{TMP117_I2C_ADDR_G,G5_SDA_GPIO_Port,G5_SDA_Pin,G5_SCL_GPIO_Port,G5_SCL_Pin},   // S50(49)

	{TMP117_I2C_ADDR_V,G3_SDA_GPIO_Port,G3_SDA_Pin,G3_SCL_GPIO_Port,G3_SCL_Pin},   // S51
	{TMP117_I2C_ADDR_G,G4_SDA_GPIO_Port,G4_SDA_Pin,G4_SCL_GPIO_Port,G4_SCL_Pin}   // S52
#endif
};

uint8_t Sensorlocation[4][MULRSCNT]=
{
#ifdef NOTCH
	{ 1, 2, 7,23, 6,29,13,12,27, 8,25, 9,10,11},
	{ 3,15,30,14,33,21,20,32,16,31,17,18,19, 0},
	{ 4,39,34,22,37,51,28,36,24,35,43,26,47, 0},
	{ 5,41,40,38,52,53,50,48,42,44,45,46,49, 0}
#else
	{ 1, 2,13,29,12,27,11,10,25, 6,23, 7, 8, 9},
	{ 3,21,33,20,32,19,18,31,14,30,15,16,17, 0},
	{ 4,51,37,28,36,47,26,35,22,34,39,24,43, 0},
	{ 5,52,50,48,49,46,44,38,40,41,42,45, 0, 0}
#endif
};
#ifdef NOTCH
uint32_t ack_check_value[4]={0x00003fff,0x00001fff,0x00001fff,0x00001fff};
#else
uint32_t ack_check_value[4]={0x00003fff,0x00001fff,0x00001fff,0x00000fff};
#endif

uint8_t sensor_flag[SENSOR_TOTAL_CNT+1]={0,};
uint16_t StoredReadSensor[MULRSCNT*4]={0,};
uint16_t pre_StoredReadSensor[MULRSCNT*4]={0,};
uint16_t config_reg_set[MULRSCNT*4]={0,};

void i2c_delay(uint32_t delay)
{
	while(delay--)
	{

	}
}

void sda_in(uint8_t ch)
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	GPIO_InitStruct.Pin = TMP_LIST[ch].sda_pin;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(TMP_LIST[ch].sda_port, &GPIO_InitStruct);

}

void m_sda_in(uint8_t row, uint8_t col_cnt)
{
	uint8_t cnt=0,sensor_sel=0;
	for(cnt=0;cnt<col_cnt;cnt++)
	{
		sensor_sel=Sensorlocation[row][cnt];
		if((sensor_sel!=0)&&(sensor_flag[sensor_sel]!=0))
		{
			sda_in(sensor_sel);
		}
	}
}

void sda_out(uint8_t ch)
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	HAL_GPIO_WritePin(TMP_LIST[ch].sda_port, TMP_LIST[ch].sda_pin, GPIO_PIN_SET);

	GPIO_InitStruct.Pin = TMP_LIST[ch].sda_pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(TMP_LIST[ch].sda_port, &GPIO_InitStruct);
}

void m_sda_out(uint8_t row,uint8_t col_cnt)
{
	uint8_t cnt=0,sensor_sel=0;
	for(cnt=0;cnt<col_cnt;cnt++)
	{
		sensor_sel=Sensorlocation[row][cnt];
		if((sensor_sel!=0)&&(sensor_flag[sensor_sel]!=0))
		{
			sda_out(sensor_sel);
		}
	}
}

void m_sda_high(uint8_t row,uint8_t col_cnt)
{
	uint8_t cnt=0,sensor_sel=0;
	for(cnt=0;cnt<col_cnt;cnt++)
	{
		sensor_sel=Sensorlocation[row][cnt];
		if((sensor_sel!=0)&&(sensor_flag[sensor_sel]!=0))
		{
			HAL_GPIO_WritePin(TMP_LIST[sensor_sel].sda_port, TMP_LIST[sensor_sel].sda_pin, GPIO_PIN_SET);
		}
	}
}

void m_sda_low(uint8_t row,uint8_t col_cnt)
{
	uint8_t cnt=0,sensor_sel=0;
	for(cnt=0;cnt<col_cnt;cnt++)
	{
		sensor_sel=Sensorlocation[row][cnt];
		if((sensor_sel!=0)&&(sensor_flag[sensor_sel]!=0))
		{
			HAL_GPIO_WritePin(TMP_LIST[sensor_sel].sda_port, TMP_LIST[sensor_sel].sda_pin, GPIO_PIN_RESET);
		}
	}
}

void m_scl_high(uint8_t row,uint8_t col_cnt)
{
	uint8_t cnt=0,sensor_sel=0;
	for(cnt=0;cnt<col_cnt;cnt++)
	{
		sensor_sel=Sensorlocation[row][cnt];
		if((sensor_sel!=0)&&(sensor_flag[sensor_sel]!=0))
		{
			HAL_GPIO_WritePin(TMP_LIST[sensor_sel].scl_port, TMP_LIST[sensor_sel].scl_pin, GPIO_PIN_SET);
		}
	}
}

void m_scl_low(uint8_t row,uint8_t col_cnt)
{
	uint8_t cnt=0,sensor_sel=0;
	for(cnt=0;cnt<col_cnt;cnt++)
	{
		sensor_sel=Sensorlocation[row][cnt];
		if((sensor_sel!=0)&&(sensor_flag[sensor_sel]!=0))
		{
			HAL_GPIO_WritePin(TMP_LIST[sensor_sel].scl_port, TMP_LIST[sensor_sel].scl_pin, GPIO_PIN_RESET);
		}
	}
}

uint32_t m_gpio_readpin(uint8_t row,uint8_t col_cnt)
{
	uint8_t cnt=0,sensor_sel=0;
	uint32_t return_value=0;
	for(cnt=0;cnt<col_cnt;cnt++)
	{
		sensor_sel=Sensorlocation[row][cnt];
		if((sensor_sel!=0)&&(sensor_flag[sensor_sel]!=0))
		{
			if(HAL_GPIO_ReadPin(TMP_LIST[sensor_sel].sda_port,TMP_LIST[sensor_sel].sda_pin)==GPIO_PIN_RESET)
			{
				return_value|=1<<cnt;
			}
		}

		else
		{
			//printf("clear\r\n");
			return_value|=1<<cnt;
		}

	}
	return return_value;
}

void i2c_start(uint8_t ch)
{
	sda_out(ch);
	HAL_GPIO_WritePin(TMP_LIST[ch].sda_port, TMP_LIST[ch].sda_pin, GPIO_PIN_SET);//SDA_H
	//i2c_delay(I2C_DELAY_VAL);
	HAL_GPIO_WritePin(TMP_LIST[ch].scl_port, TMP_LIST[ch].scl_pin, GPIO_PIN_SET);//SCL_H
	i2c_delay(I2C_DELAY_VAL);
	HAL_GPIO_WritePin(TMP_LIST[ch].sda_port, TMP_LIST[ch].sda_pin, GPIO_PIN_RESET);//SDA_L
	i2c_delay(I2C_DELAY_VAL);
	HAL_GPIO_WritePin(TMP_LIST[ch].scl_port, TMP_LIST[ch].scl_pin, GPIO_PIN_RESET);//SCL_L
	//i2c_delay(I2C_DELAY_VAL);
}

void m_i2c_start(uint8_t row,uint8_t col_cnt)
{
	m_sda_out(row,col_cnt);
	m_sda_high(row,col_cnt);
	m_scl_high(row,col_cnt);
	i2c_delay(I2C_DELAY_VAL);
	m_sda_low(row,col_cnt);
	i2c_delay(I2C_DELAY_VAL);
	m_scl_low(row,col_cnt);
}


void i2c_stop(uint8_t ch)
{
	sda_out(ch);
	HAL_GPIO_WritePin(TMP_LIST[ch].sda_port, TMP_LIST[ch].sda_pin, GPIO_PIN_RESET);//SDA_L
	i2c_delay(I2C_DELAY_VAL);
	HAL_GPIO_WritePin(TMP_LIST[ch].scl_port, TMP_LIST[ch].scl_pin, GPIO_PIN_SET);//SCL_H
	i2c_delay(I2C_DELAY_VAL);
	HAL_GPIO_WritePin(TMP_LIST[ch].sda_port, TMP_LIST[ch].sda_pin, GPIO_PIN_SET);//SDA_H
	i2c_delay(I2C_DELAY_VAL);
	//HAL_GPIO_WritePin(TMP_LIST[ch].scl_port, TMP_LIST[ch].scl_pin, GPIO_PIN_RESET);//SCL_L
}

void m_i2c_stop(uint8_t row,uint8_t col_cnt)
{
	m_sda_out(row,col_cnt);
	m_sda_low(row,col_cnt);
	i2c_delay(I2C_DELAY_VAL);
	m_scl_high(row,col_cnt);
	i2c_delay(I2C_DELAY_VAL);
	m_sda_high(row,col_cnt);
	//i2c_delay(I2C_DELAY_VAL);
	//m_scl_low(row,col_cnt);
}

uint8_t i2c_wait_ack(uint8_t ch,uint32_t wait_value)
{
	uint32_t wait_v=0;
	sda_in(ch);
	HAL_GPIO_WritePin(TMP_LIST[ch].scl_port, TMP_LIST[ch].scl_pin, GPIO_PIN_RESET);//SCL_L
	i2c_delay(I2C_DELAY_VAL);
	HAL_GPIO_WritePin(TMP_LIST[ch].scl_port, TMP_LIST[ch].scl_pin, GPIO_PIN_SET);//SCL_H
	i2c_delay(I2C_DELAY_VAL);
	while(HAL_GPIO_ReadPin(TMP_LIST[ch].sda_port,TMP_LIST[ch].sda_pin))
	{
		wait_v++;
		if(wait_v>=wait_value)
		{
			//i2c_stop(ch);
			HAL_GPIO_WritePin(TMP_LIST[ch].scl_port, TMP_LIST[ch].scl_pin, GPIO_PIN_RESET);//SCL_L
			return 1;
		}
	}
	HAL_GPIO_WritePin(TMP_LIST[ch].scl_port, TMP_LIST[ch].scl_pin, GPIO_PIN_RESET);//SCL_L
	return 0;
}

uint32_t m_i2c_wait_ack(uint8_t row,uint8_t col_cnt,uint32_t wait_value)
{
	uint32_t ack_flag=0,wait_v=0;
	m_sda_in(row,col_cnt);
	m_scl_low(row,col_cnt);
	i2c_delay(I2C_DELAY_VAL);
	m_scl_high(row,col_cnt);
	i2c_delay(I2C_DELAY_VAL);
	while(ack_flag!=ack_check_value[row])//while(ack_flag!=0x01ffffff)
	{
		ack_flag=m_gpio_readpin(row,col_cnt);
		wait_v++;
		if(wait_v>=wait_value)
		{
			m_scl_low(row,col_cnt);//m_i2c_stop(row,col_cnt);
			//printf("wait_time\r\n");
			return ack_flag;
		}
		//printf("WAIT:%x\r\n",ack_flag);
	}
	m_scl_low(row,col_cnt);
	return ack_flag;
}

void i2c_ack(uint8_t ch)
{
	HAL_GPIO_WritePin(TMP_LIST[ch].scl_port, TMP_LIST[ch].scl_pin, GPIO_PIN_RESET);//SCL_L
	sda_out(ch);
	HAL_GPIO_WritePin(TMP_LIST[ch].sda_port, TMP_LIST[ch].sda_pin, GPIO_PIN_RESET);//SDA_L
	i2c_delay(I2C_DELAY_VAL);
	HAL_GPIO_WritePin(TMP_LIST[ch].scl_port, TMP_LIST[ch].scl_pin, GPIO_PIN_SET);//SCL_H
	i2c_delay(I2C_DELAY_VAL);
	HAL_GPIO_WritePin(TMP_LIST[ch].scl_port, TMP_LIST[ch].scl_pin, GPIO_PIN_RESET);//SCL_L
}

void m_i2c_ack(uint8_t row,uint8_t col_cnt)
{
	m_scl_low(row,col_cnt);
	m_sda_out(row,col_cnt);
	m_sda_low(row,col_cnt);
	i2c_delay(I2C_DELAY_VAL);
	m_scl_high(row,col_cnt);
	i2c_delay(I2C_DELAY_VAL);
	m_scl_low(row,col_cnt);
}

void i2c_nack(uint8_t ch)
{
	HAL_GPIO_WritePin(TMP_LIST[ch].scl_port, TMP_LIST[ch].scl_pin, GPIO_PIN_RESET);//SCL_L
	sda_out(ch);
	HAL_GPIO_WritePin(TMP_LIST[ch].sda_port, TMP_LIST[ch].sda_pin, GPIO_PIN_SET);//SDA_H
	i2c_delay(I2C_DELAY_VAL);
	HAL_GPIO_WritePin(TMP_LIST[ch].scl_port, TMP_LIST[ch].scl_pin, GPIO_PIN_SET);//SCL_H
	i2c_delay(I2C_DELAY_VAL);
	HAL_GPIO_WritePin(TMP_LIST[ch].scl_port, TMP_LIST[ch].scl_pin, GPIO_PIN_RESET);//SCL_L
}

void m_i2c_nack(uint8_t row,uint8_t col_cnt)
{
	m_scl_low(row,col_cnt);
	m_sda_out(row,col_cnt);
	m_sda_high(row,col_cnt);
	i2c_delay(I2C_DELAY_VAL);
	m_scl_high(row,col_cnt);
	i2c_delay(I2C_DELAY_VAL);
	m_scl_low(row,col_cnt);
}

void i2c_wr_byte(uint8_t ch, uint8_t data)
{
	uint8_t loop=0,cpy_data=data;
	sda_out(ch);
	//HAL_GPIO_WritePin(TMP_LIST[ch].scl_port, TMP_LIST[ch].scl_pin, GPIO_PIN_RESET);//SCL_L
	//i2c_delay(I2C_DELAY_VAL);
	for(loop=0;loop<8;loop++)
	{
		if((cpy_data&0x80)==0x80)
		{
			HAL_GPIO_WritePin(TMP_LIST[ch].sda_port, TMP_LIST[ch].sda_pin, GPIO_PIN_SET);//SDA_H
		}
		else
		{
			HAL_GPIO_WritePin(TMP_LIST[ch].sda_port, TMP_LIST[ch].sda_pin, GPIO_PIN_RESET);//SDA_L
		}
		cpy_data<<=1;
		i2c_delay(I2C_DELAY_VAL);
		HAL_GPIO_WritePin(TMP_LIST[ch].scl_port, TMP_LIST[ch].scl_pin, GPIO_PIN_SET);//SCL_H
		i2c_delay(I2C_DELAY_VAL);
		HAL_GPIO_WritePin(TMP_LIST[ch].scl_port, TMP_LIST[ch].scl_pin, GPIO_PIN_RESET);//SCL_L
		i2c_delay(I2C_DELAY_VAL);
	}
}

void m_i2c_wr_byte(uint8_t row,uint8_t col_cnt,uint8_t addr,uint8_t data,uint8_t rw)
{
	uint8_t loop=0,cnt=0,cpy_data[25]={0,},sensor_sel=0;
	if(addr!=0)
	{
		for(cnt=0;cnt<col_cnt;cnt++)
		{
			sensor_sel=Sensorlocation[row][cnt];
			if((sensor_sel!=0)&&(sensor_flag[sensor_sel]!=0))
			{
				if(rw!=0)//Read
				{
					cpy_data[cnt]=(TMP_LIST[sensor_sel].addr|0x01);
				}
				else//Write
				{
					cpy_data[cnt]=TMP_LIST[sensor_sel].addr;
				}
			}
			else
			{
				cpy_data[cnt]=0;;
			}
		}
	}
	else
	{
		for(cnt=0;cnt<col_cnt;cnt++)
		{
			sensor_sel=Sensorlocation[row][cnt];
			if((sensor_sel!=0)&&(sensor_flag[sensor_sel]!=0))
			{
				cpy_data[cnt]=data;
			}
			else
			{
				cpy_data[cnt]=0;;
			}
			//printf("0x%02x\r\n",cpy_data[cnt]);
		}
	}
	m_sda_out(row,col_cnt);
	//m_scl_low(row,col_cnt);
	for(loop=0;loop<8;loop++)
	{
		for(cnt=0;cnt<col_cnt;cnt++)
		{
			sensor_sel=Sensorlocation[row][cnt];
			if((sensor_sel!=0)&&(sensor_flag[sensor_sel]!=0))
			{
				if((cpy_data[cnt]&0x80)==0x80)
				{
					HAL_GPIO_WritePin(TMP_LIST[sensor_sel].sda_port, TMP_LIST[sensor_sel].sda_pin, GPIO_PIN_SET);//SDA_H
				}
				else
				{
					HAL_GPIO_WritePin(TMP_LIST[sensor_sel].sda_port, TMP_LIST[sensor_sel].sda_pin, GPIO_PIN_RESET);//SDA_L
				}
			}
			cpy_data[cnt]<<=1;
		}
		i2c_delay(I2C_DELAY_VAL);
		m_scl_high(row,col_cnt);
		i2c_delay(I2C_DELAY_VAL);
		m_scl_low(row,col_cnt);
	}
}

uint8_t i2c_rd_byte(uint8_t ch, uint8_t ack)
{
	uint8_t loop=0,cpy_data=0;
	sda_in(ch);
	for(loop=0;loop<8;loop++)
	{
		HAL_GPIO_WritePin(TMP_LIST[ch].scl_port, TMP_LIST[ch].scl_pin, GPIO_PIN_RESET);//SCL_L
		i2c_delay(I2C_DELAY_VAL);
		HAL_GPIO_WritePin(TMP_LIST[ch].scl_port, TMP_LIST[ch].scl_pin, GPIO_PIN_SET);//SCL_H
		i2c_delay(I2C_DELAY_VAL);
		cpy_data<<=1;
		if(HAL_GPIO_ReadPin(TMP_LIST[ch].sda_port,TMP_LIST[ch].sda_pin))
		{
			cpy_data++;
		}
		//i2c_delay(I2C_DELAY_VAL);
	}
	if(ack==1)
	{
		i2c_ack(ch);
	}
	else
	{
		i2c_nack(ch);
	}
	return cpy_data;
}

void m_i2c_rd_byte(uint8_t row,uint8_t col_cnt,uint8_t ack, uint8_t* buf)
{
	uint8_t loop=0,cnt=0,sensor_sel=0,high_cnt=0,high_cnt_loop=0;
	m_sda_in(row,col_cnt);
	for(loop=0;loop<8;loop++)
	{
		m_scl_low(row,col_cnt);
		i2c_delay(I2C_DELAY_VAL);
		m_scl_high(row,col_cnt);
		i2c_delay(I2C_DELAY_VAL);
		for(cnt=0;cnt<col_cnt;cnt++)
		{
			buf[cnt]<<=1;
			sensor_sel=Sensorlocation[row][cnt];
			high_cnt=0;
			if((sensor_sel!=0)&&(sensor_flag[sensor_sel]!=0))
			{
#if 0
				if(HAL_GPIO_ReadPin(TMP_LIST[sensor_sel].sda_port,TMP_LIST[sensor_sel].sda_pin))
				{
					buf[cnt]++;
				}
#endif
				for(high_cnt_loop=0;high_cnt_loop<5;high_cnt_loop++)
				{
					if(HAL_GPIO_ReadPin(TMP_LIST[sensor_sel].sda_port,TMP_LIST[sensor_sel].sda_pin))
					{
						high_cnt++;
					}
					i2c_delay(10);
				}
				if(high_cnt>=3)
				{
					buf[cnt]++;
				}
			}
		}
		//i2c_delay(I2C_DELAY_VAL);
	}
	if(ack==1)
	{
		m_i2c_ack(row,col_cnt);
	}
	else
	{
		m_i2c_nack(row,col_cnt);
	}
}

uint8_t i2c_wr_reg(uint8_t ch,uint8_t reg, uint16_t value)
{
	uint8_t wr_buf[2]={0,},return_value=0;
	wr_buf[0]=(value&0xff00)>>8;
	wr_buf[1]=value&0x00ff;
	i2c_start(ch);
	i2c_wr_byte(ch,TMP_LIST[ch].addr);
	return_value=i2c_wait_ack(ch,I2C_WAIT_ACK);
	i2c_wr_byte(ch,reg);
	return_value+=i2c_wait_ack(ch,I2C_WAIT_ACK);
	i2c_wr_byte(ch,wr_buf[0]);
	return_value+=i2c_wait_ack(ch,I2C_WAIT_ACK);
	i2c_wr_byte(ch,wr_buf[1]);
	return_value+=i2c_wait_ack(ch,I2C_WAIT_ACK);
	i2c_stop(ch);
	return return_value;
}

uint32_t m_i2c_wr_reg(uint8_t row,uint8_t col_cnt,uint8_t reg, uint16_t value)
{
	uint8_t wr_buf[2]={0,};
	uint32_t return_value=0x01ffffff;
	wr_buf[0]=(value&0xff00)>>8;
	wr_buf[1]=value&0x00ff;
	m_i2c_start(row,col_cnt);
	m_i2c_wr_byte(row,col_cnt,I2C_ADDR_SET,BLANK_DATA,I2C_WRITE);
	return_value&=m_i2c_wait_ack(row,col_cnt,I2C_WAIT_ACK);
	m_i2c_wr_byte(row,col_cnt,I2C_DATA_SET,reg,I2C_WRITE);
	return_value&=m_i2c_wait_ack(row,col_cnt,I2C_WAIT_ACK);
	m_i2c_wr_byte(row,col_cnt,I2C_DATA_SET,wr_buf[0],I2C_WRITE);
	return_value&=m_i2c_wait_ack(row,col_cnt,I2C_WAIT_ACK);
	m_i2c_wr_byte(row,col_cnt,I2C_DATA_SET,wr_buf[1],I2C_WRITE);
	return_value&=m_i2c_wait_ack(row,col_cnt,I2C_WAIT_ACK);
	m_i2c_stop(row,col_cnt);
	return return_value;
}

uint8_t i2c_rd_reg(uint8_t ch,uint8_t reg, uint16_t* value)
{
	uint8_t  rd_buf[2]={0,},return_value=0;
	i2c_start(ch);
	i2c_wr_byte(ch,TMP_LIST[ch].addr);
	return_value=i2c_wait_ack(ch,I2C_WAIT_ACK);
	i2c_wr_byte(ch,reg);
	return_value+=i2c_wait_ack(ch,I2C_WAIT_ACK);

	i2c_start(ch);
	i2c_wr_byte(ch,TMP_LIST[ch].addr|0x01);
	return_value+=i2c_wait_ack(ch,I2C_WAIT_ACK);
	rd_buf[1]=i2c_rd_byte(ch,1);
	rd_buf[0]=i2c_rd_byte(ch,0);

	*value=((rd_buf[1]<<8)&0xff00)|(rd_buf[0]&0x00ff);
	i2c_stop(ch);

	return return_value;
}

uint32_t m_i2c_rd_reg(uint8_t row,uint8_t col_cnt,uint8_t reg, uint16_t* value)
{
	uint32_t return_value=ack_check_value[row];//0x01ffffff;
	uint8_t mul_buf[2][25]={{0,},{0,}};
	m_i2c_start(row,col_cnt);
	m_i2c_wr_byte(row,col_cnt,I2C_ADDR_SET,BLANK_DATA,I2C_WRITE);
	return_value&=m_i2c_wait_ack(row,col_cnt,I2C_WAIT_ACK);
	//printf("ACK1:%lx\r\n",return_value);
	m_i2c_wr_byte(row,col_cnt,I2C_DATA_SET,reg,I2C_WRITE);
	return_value&=m_i2c_wait_ack(row,col_cnt,I2C_WAIT_ACK);
	//printf("ACK2:%lx\r\n",return_value);
	m_i2c_start(row,col_cnt);
	m_i2c_wr_byte(row,col_cnt,I2C_ADDR_SET,BLANK_DATA,I2C_READ);
	return_value&=m_i2c_wait_ack(row,col_cnt,I2C_WAIT_ACK);
	//printf("ACK3:%lx\r\n",return_value);
	m_i2c_rd_byte(row,col_cnt,1, mul_buf[1]);
	m_i2c_rd_byte(row,col_cnt,0, mul_buf[0]);
	m_i2c_stop(row, col_cnt);

	for(int i=0;i<col_cnt;i++)
	{
		value[i]=((mul_buf[1][i]<<8)&0xff00);
		value[i]|=(mul_buf[0][i]&0x00ff);
	}
	//printf("END\r\n");
	return return_value;
}

uint8_t tmp_sensor_config(uint16_t config,uint8_t *buff)
{
	uint8_t i=0,fail_flag=0;
	uint16_t read_value=0;
	uint8_t q=0,r=0;
	for(i=1;i<SENSOR_TOTAL_CNT+1;i++)
	{
		i2c_wr_reg(i,0x01,config);
		i2c_rd_reg(i,0x01,&read_value);
		if(read_value!=config)
		{
			printf("FAIL_SENSOR:%d\r\n",i);
			fail_flag++;
		}
		else
		{
			sensor_flag[i]=0x01;
			q=(i-1)/8;
			r=(i-1)%8;
			buff[q]|=1<<r;
		}
	}
	return fail_flag;
}

uint8_t multi_tmp_sensor_config(uint16_t config)
{
	uint8_t i=0;
#if 0
	for(i=0;i<4;i++)
	{
		m_i2c_rd_reg(i,MULRSCNT,0x01,&config_reg_set[i*MULRSCNT]);
	}
	printf("PRE_READ\r\n");
	for(i=0;i<14;i++)
	{
		printf("[%x]",config_reg_set[i]);
	}
	printf("\r\n");
	for(i=14;i<28;i++)
	{
		printf("[%x]",config_reg_set[i]);
	}
	printf("\r\n");
	for(i=28;i<42;i++)
	{
		printf("[%x]",config_reg_set[i]);
	}
	printf("\r\n");
	for(i=42;i<56;i++)
	{
		printf("[%x]",config_reg_set[i]);
	}
	printf("\r\n");
#endif
	for(i=0;i<4;i++)
	{
		m_i2c_wr_reg(i,MULRSCNT,0x01,0x0120);
	}
#if 0
	for(i=0;i<4;i++)
	{
		m_i2c_rd_reg(i,MULRSCNT,0x01,&config_reg_set[i*MULRSCNT]);
	}
	printf("WRITE_AND_READ\r\n");
	for(i=0;i<14;i++)
	{
		printf("[%x]",config_reg_set[i]);
		config_reg_set[i]=0;
	}
	printf("\r\n");
	for(i=14;i<28;i++)
	{
		printf("[%x]",config_reg_set[i]);
		config_reg_set[i]=0;
	}
	printf("\r\n");
	for(i=28;i<42;i++)
	{
		printf("[%x]",config_reg_set[i]);
		config_reg_set[i]=0;
	}
	printf("\r\n");
	for(i=42;i<56;i++)
	{
		printf("[%x]",config_reg_set[i]);
		config_reg_set[i]=0;
	}
	printf("\r\n");
#endif
	return 0;
}

uint16_t test_sensor_id_read(uint8_t sel)
{
	uint16_t read_value=0;
	i2c_rd_reg(sel,0x0F,&read_value);
	return read_value;
}

void tmp_all_read(void)
{
	uint8_t i=0;
	uint16_t read_value=0;
	for(i=1;i<SENSOR_TOTAL_CNT+1;i++)
	{
		i2c_rd_reg(i,0x00,&read_value);
		//printf("FAIL_SENSOR:%d\r\n",i+1);
	}
}

float tri_sensor_temp(void)
{
	uint16_t read_value=0;
	int16_t cpy_read_value=0;
	float return_value=0.0;

	i2c_rd_reg(1,0x00,&read_value);

	cpy_read_value=(int16_t)read_value;

	//return_value = (float)read_value*0.0078125;
	return_value = (float)cpy_read_value*0.0078125;

	return return_value;
}

char i2c_bintostrhex[50];
void ReadRealSensorAndSaveBuf(_opcode_r *opcode_r_rt)
{
	uint8_t i=0,j=0;
	uint16_t bat_value=0;
	int16_t mcu_temp_value=0;
	memcpy(opcode_r_rt->index,"9999",4);
	//samplingRate=500;
	//float temper=0.0;
	for(i=0;i<4;i++)
	{
		m_i2c_rd_reg(i,MULRSCNT,0x00,&StoredReadSensor[i*MULRSCNT]);
		for(j=0;j<MULRSCNT;j++)
		{
			if(Sensorlocation[i][j]==1) // missionTriggerSensor
			{
				//-ReadTriggerSensor._S_02_byte[TriBatWriteCnt]=multiReadSensor[k];
				memset(i2c_bintostrhex,0,sizeof(i2c_bintostrhex));
				//Real Temp
				sprintf(i2c_bintostrhex,"%04X",StoredReadSensor[(i*MULRSCNT)+j]);
				//TEST
				//sprintf(i2c_bintostrhex,"%04X",Sensorlocation[i][j]);

				memcpy(opcode_r_rt->wafertriggertemperature,i2c_bintostrhex,4);
				//memcpy(opcode_r_rt->mcureferncetemperature,i2c_bintostrhex,4);
				//temper=(float)(StoredReadSensor[(i*25)+j])*0.0078125;
				//printf("GET_1=%3.2f\r\n",temper);
			}
			else
			{
				if(Sensorlocation[i][j]!=0)
				{
					//-ReadTemp._S_02_byte[Sensorlocation[ReadingSeq][k]]=multiReadSensor[k];
					memset(i2c_bintostrhex,0,sizeof(i2c_bintostrhex));
					//Real Temp
					sprintf(i2c_bintostrhex,"%04X",StoredReadSensor[(i*MULRSCNT)+j]);
					//TEST
					//sprintf(i2c_bintostrhex,"%04X",Sensorlocation[i][j]);
					memcpy(&opcode_r_rt->waferTempSaveMemory[(Sensorlocation[i][j]-2)*4],i2c_bintostrhex,4);
				}
			}
		}
	}
	adc_channel_read_start(&bat_value,&mcu_temp_value);
	memset(i2c_bintostrhex,0,sizeof(i2c_bintostrhex));
	sprintf(i2c_bintostrhex,"%04X",bat_value);
	memcpy(&opcode_r_rt->batteryvoltage,i2c_bintostrhex,4);

	memset(i2c_bintostrhex,0,sizeof(i2c_bintostrhex));
	sprintf(i2c_bintostrhex,"%04X",mcu_temp_value);
	memcpy(opcode_r_rt->mcureferncetemperature,i2c_bintostrhex,4);
}

void ReadSensor_temp(Temperature_Frame* frame,int16_t* tri_temp,uint16_t* bat_value, uint32_t* ack_data)
{
	uint8_t i=0,j=0,k=0;
	uint16_t bat_value_read=0;
	int16_t mcu_temp_value=0;
	memset(StoredReadSensor,0,sizeof(StoredReadSensor));
	for(i=0;i<4;i++)
	{
		ack_data[i]=m_i2c_rd_reg(i,MULRSCNT,0x00,&StoredReadSensor[i*MULRSCNT]);
		if(ack_data[i]!=ack_check_value[i])
		{
			ack_data[i]=m_i2c_rd_reg(i,MULRSCNT,0x00,&StoredReadSensor[i*MULRSCNT]);
		}
		if(ack_data[i]!=ack_check_value[i])
		{
			for(k=0;k<MULRSCNT;k++)
			{
				if(!(ack_data[i]&(1<<k)))
				{
					StoredReadSensor[(i*MULRSCNT)+k]=pre_StoredReadSensor[(i*MULRSCNT)+k];
				}
			}
		}
		for(j=0;j<MULRSCNT;j++)
		{
			if(Sensorlocation[i][j]==1) // missionTriggerSensor
			{
				*tri_temp=StoredReadSensor[(i*MULRSCNT)+j];
				pre_StoredReadSensor[(i*MULRSCNT)+j]=StoredReadSensor[(i*MULRSCNT)+j];
				//TEST
				//*tri_temp=Sensorlocation[i][j];
			}
			else
			{
				if(Sensorlocation[i][j]!=0)
				{
					frame->_S_02_byte[(Sensorlocation[i][j])-2]=StoredReadSensor[(i*MULRSCNT)+j];
					pre_StoredReadSensor[(i*MULRSCNT)+j]=StoredReadSensor[(i*MULRSCNT)+j];
					//TEST
					//frame->_S_02_byte[(Sensorlocation[i][j])-2]=Sensorlocation[i][j];
				}
			}
		}
	}
	adc_channel_read_start(&bat_value_read,&mcu_temp_value);
	*bat_value=bat_value_read;//0xABCD;//
}

void test_ReadSensor_temp(Temperature_Frame* frame,int16_t* tri_temp,uint16_t* bat_value, uint32_t* ack_data)
{
	uint8_t i=0,j=0,k=0;
	uint16_t bat_value_read=0;
	int16_t mcu_temp_value=0;
	memset(StoredReadSensor,0,sizeof(StoredReadSensor));
	for(i=0;i<4;i++)
	{
		ack_data[i]=m_i2c_rd_reg(i,MULRSCNT,0x00,&StoredReadSensor[i*MULRSCNT]);
		if(ack_data[i]!=ack_check_value[i])
		{
			ack_data[i]=m_i2c_rd_reg(i,MULRSCNT,0x00,&StoredReadSensor[i*MULRSCNT]);
		}
		if(ack_data[i]!=ack_check_value[i])
		{
			for(k=0;k<MULRSCNT;k++)
			{
				if(!(ack_data[i]&(1<<k)))
				{
					StoredReadSensor[(i*MULRSCNT)+k]=pre_StoredReadSensor[(i*MULRSCNT)+k];
				}
			}
		}
		for(j=0;j<MULRSCNT;j++)
		{
			if(Sensorlocation[i][j]==1) // missionTriggerSensor
			{
				*tri_temp=StoredReadSensor[(i*MULRSCNT)+j];
				pre_StoredReadSensor[(i*MULRSCNT)+j]=StoredReadSensor[(i*MULRSCNT)+j];
				//TEST
				*tri_temp=Sensorlocation[i][j];
			}
			else
			{
				if(Sensorlocation[i][j]!=0)
				{
					frame->_S_02_byte[(Sensorlocation[i][j])-2]=StoredReadSensor[(i*MULRSCNT)+j];
					pre_StoredReadSensor[(i*MULRSCNT)+j]=StoredReadSensor[(i*MULRSCNT)+j];
					//TEST
					frame->_S_02_byte[(Sensorlocation[i][j])-2]=Sensorlocation[i][j];
				}
			}
		}
	}
	adc_channel_read_start(&bat_value_read,&mcu_temp_value);
	*bat_value=0xABCD;//bat_value_read;
}

void i2c_io_deinit(void)
{
	HAL_GPIO_DeInit(GPIOD, G4_SCL_Pin|G4_SDA_Pin|G5_SDA_Pin|G6_SDA_Pin|G6_SCL_Pin);
	HAL_GPIO_DeInit(GPIOC, G3_SDA_Pin|G10_SDA_Pin|G11_SCL_Pin);
	HAL_GPIO_DeInit(GPIOA, G3_SCL_Pin);
	HAL_GPIO_DeInit(GPIOE, G10_SCL_Pin|G14_SDA_Pin|G9_SDA_Pin|G14_SCL_Pin|G9_SCL_Pin);
	HAL_GPIO_DeInit(GPIOF, G11_SDA_Pin|G12_SCL_Pin|G12_SDA_Pin|G13_SDA_Pin|G1_SDA_Pin|G2_SDA_Pin|G1_SCL_Pin);
	HAL_GPIO_DeInit(GPIOH, G13_SCL_Pin);
	HAL_GPIO_DeInit(GPIOB, G5_SCL_Pin|G8_SDA_Pin|G7_SDA_Pin);
	HAL_GPIO_DeInit(GPIOG, G2_SCL_Pin|G8_SCL_Pin|G7_SCL_Pin);

}

float sel_sensor_temp(uint8_t ch)
{
	int16_t read_value=0;
	float return_value=0.0;
	uint8_t flag=0;

	flag=i2c_rd_reg(ch,0x00,&read_value);
	//printf("flag:%x\r\n",flag);

	return_value = (float)(read_value*0.0078125);
	return return_value;
}
