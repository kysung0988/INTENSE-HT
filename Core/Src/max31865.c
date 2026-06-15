/*
 * max31865.c
 *
 *  Created on: May 12, 2025
 *      Author: kimst
 */


/***************************************************************************//**
 *   @file   max31865.c
 *   @brief  Implementation of MAX31865 Driver.
 *   @author JSanBuen (jose.sanbuenaventura@analog.com)
 *   @author MSosa (marcpaolo.sosa@analog.com)
********************************************************************************
 * Copyright 2023(c) Analog Devices, Inc.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. Neither the name of Analog Devices, Inc. nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY ANALOG DEVICES, INC. “AS IS” AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
 * EVENT SHALL ANALOG DEVICES, INC. BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*******************************************************************************/

#include <errno.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include "max31865.h"
#include "math.h"
#include "bsp_spi2.h"

/******************************************************************************/
void max31865_init(void)
{
	max31865_set_wires(0);
	max31865_enable_bias(0);
	max31865_auto_convert(0);
	max31865_set_threshold(0x0000,0xffff);
	max31865_clear_fault();
}


int max31865_reg_update(uint8_t reg_addr,uint8_t reg_update, uint8_t or_mask)
{
	uint8_t temp;
	int ret;

	ret = max31865_read(reg_addr, &temp);
	if (ret<0)
	{
		printf("SPI2_UPDATE_ERROR\r\n");
		return ret;
	}

	if (or_mask)
		temp |= reg_update;
	else
		temp &= reg_update;

	return max31865_write(reg_addr, temp);
}

int max31865_read(uint8_t reg_addr,uint8_t *reg_data)
{
	uint8_t raw_array[2];
	int ret;

	raw_array[0] = reg_addr;
	raw_array[1] = 0;

	if ((reg_addr & MAX31865_READ_MASK) != reg_addr)
		reg_addr &= MAX31865_READ_MASK;

	if (reg_addr < MAX31865_CONFIG_REG || reg_addr > MAX31865_FAULTSTAT_REG)
		return -EINVAL;
	RTD_CS_LOW();
	ret = BSP_SPI2_Send(raw_array,1);
	ret +=BSP_SPI2_Recv(&raw_array[1],1);
	RTD_CS_HIGH();
	if (ret<0)
	{
		printf("SPI2_READ_ERROR\r\n");
		return ret;
	}

	*reg_data = raw_array[1];

	return 0;
}

int max31865_write(uint8_t reg_addr,uint8_t reg_data)
{
	int ret;
	uint8_t raw_array[2];

	if (reg_addr < MAX31865_CONFIG_REG || reg_addr >= MAX31865_FAULTSTAT_REG)
		return -EINVAL;

	if (reg_addr == MAX31865_RTDMSB_REG || reg_addr == MAX31865_RTDLSB_REG)
		return -EINVAL;

	reg_addr |= MAX31865_WRITE_MASK;

	raw_array[0] = reg_addr;
	raw_array[1] = reg_data;

	RTD_CS_LOW();
	ret = BSP_SPI2_Send(raw_array,2);
	RTD_CS_HIGH();
	if (ret<0)
	{
		printf("SPI2_WRITE_ERROR\r\n");
		return ret;
	}

	return ret;
}

int max31865_clear_fault(void)
{
	int ret;

	ret = max31865_reg_update(MAX31865_CONFIG_REG,MAX31865_CONFIG_CLRFAULT_MASK, false);
	if (ret<0)
	{
		printf("SPI2_CLEAR_ERROR\r\n");
		return ret;
	}

	return max31865_reg_update(MAX31865_CONFIG_REG,MAX31865_CONFIG_FAULTSTAT, true);
}


int max31865_enable_bias(uint8_t bias_en)
{
	if (bias_en)
		return max31865_reg_update(MAX31865_CONFIG_REG, MAX31865_CONFIG_BIAS,true);

	return max31865_reg_update(MAX31865_CONFIG_REG, ~MAX31865_CONFIG_BIAS,false);
}


int max31865_auto_convert(uint8_t auto_conv_en)
{
	if (auto_conv_en)
		return max31865_reg_update(MAX31865_CONFIG_REG,MAX31865_CONFIG_MODEAUTO, true);

	return max31865_reg_update(MAX31865_CONFIG_REG, MAX31865_CONFIG_MODEOFF,false);
}

int max31865_enable_50Hz(uint8_t filt_en)
{
	if (filt_en)
		return max31865_reg_update(MAX31865_CONFIG_REG,MAX31865_CONFIG_FILT50HZ, true);

	return max31865_reg_update(MAX31865_CONFIG_REG,MAX31865_CONFIG_FILT60HZ, false);
}

int max31865_set_threshold(uint16_t lower,uint16_t upper)
{
	uint16_t lsb;
	uint16_t msb;
	int ret;

	lsb = (upper)&0x00ff;
	msb = (upper>>8)&0x00ff;

	ret = max31865_write(MAX31865_HFAULTMSB_REG, (uint8_t)msb);
	if (ret<0)
		return ret;

	ret = max31865_write(MAX31865_HFAULTLSB_REG, (uint8_t)lsb);
	if (ret<0)
		return ret;

	lsb = (lower)&0x00ff;
	msb = (lower>>8)&0x00ff;

	ret = max31865_write(MAX31865_LFAULTMSB_REG, (uint8_t)msb);
	if (ret<0)
	{
		printf("SPI2_SET_THRES_ERROR\r\n");
		return ret;
	}

	return max31865_write(MAX31865_LFAULTLSB_REG, (uint8_t)lsb);
}

int max31865_get_lower_threshold(uint16_t *low_threshold)
{
	uint8_t low_msb;
	uint8_t low_lsb;
	int ret;

	ret = max31865_read(MAX31865_LFAULTMSB_REG, &low_msb);
	if (ret<0)
	{
		printf("SPI2_LOW_THR_ERROR\r\n");
		return ret;
	}
	ret = max31865_read(MAX31865_LFAULTLSB_REG, &low_lsb);
	if (ret<0)
	{
		printf("SPI2_LOW_THR_ERROR2\r\n");
		return ret;
	}
	*low_threshold = (low_msb << 8) | low_lsb;

	return 0;
}


int max31865_get_upper_threshold(uint16_t *up_threshold)
{
	uint8_t high_msb;
	uint8_t high_lsb;
	int ret;

	ret = max31865_read(MAX31865_HFAULTMSB_REG, &high_msb);
	if (ret<0)
	{
		printf("SPI2_UP_THR_ERROR\r\n");
		return ret;
	}
	ret = max31865_read(MAX31865_HFAULTLSB_REG, &high_lsb);
	if (ret<0)
	{
		printf("SPI2_UP_THR_ERROR2\r\n");
		return ret;
	}
	*up_threshold = (high_msb << 8) | high_lsb;

	return 0;
}

int max31865_set_wires(uint8_t is_odd_wire)
{
	if (is_odd_wire)
		return max31865_reg_update(MAX31865_CONFIG_REG, MAX31865_CONFIG_3WIRE,true);

	return max31865_reg_update(MAX31865_CONFIG_REG, MAX31865_CONFIG_2_4WIRE,false);
}


int max31865_read_rtd(uint16_t *rtd_reg)
{
	uint8_t reg_data;
	int ret;

	ret = max31865_clear_fault();
	if (ret<0)
	{
		printf("RTD_READ1_ERROR\r\n");
		return ret;
	}
	ret = max31865_enable_bias(true);
	if (ret<0)
	{
		printf("RTD_READ2_ERROR\r\n");
		return ret;
	}
	HAL_Delay(5);
	ret = max31865_reg_update(MAX31865_CONFIG_REG, MAX31865_CONFIG_1SHOT,true);
	if (ret<0)
	{
		printf("RTD_READ3_ERROR\r\n");
		return ret;
	}
	HAL_Delay(55);

	ret = max31865_read(MAX31865_RTDMSB_REG, &reg_data);
	if (ret<0)
	{
		printf("RTD_READ4_ERROR\r\n");
		return ret;
	}
	*rtd_reg = reg_data << 8;

	ret = max31865_read(MAX31865_RTDLSB_REG, &reg_data);
	if (ret<0)
	{
		printf("RTD_READ5_ERROR\r\n");
		return ret;
	}
	*rtd_reg = *rtd_reg | (uint16_t)reg_data;
	*rtd_reg >>= 1;

	return max31865_enable_bias(false);
}

#define RTD_A 3.9083e-3
#define RTD_B -5.775e-7

float max31865_rtd_to_temperature(uint16_t rtd)
{
	float Z1, Z2, Z3, Z4, Rt, temp;
	Rt = (float)rtd;
	Rt /= 32768;
	Rt *= 1000.0;

	//Rt-=5.0;

	Z1 = -RTD_A;
	Z2 = RTD_A * RTD_A - (4 * RTD_B);
	Z3 = (4 * RTD_B) / 200.0;
	Z4 = 2 * RTD_B;

	temp = Z2 + (Z3 * Rt);
	temp = (sqrt(temp) + Z1) / Z4;
	if (temp >= 0)
	{
		//printf("Positive(%f)\r\n",temp);
	    return temp;
	}
	Rt /= 200.0;//100.0;
	Rt *= 100.0;//100; // normalize to 100 ohm

	float rpoly = Rt;

	temp = -242.02;
	temp += 2.2228 * rpoly;
	rpoly *= Rt; // square
	temp += 2.5859e-3 * rpoly;
	rpoly *= Rt; // ^3
	temp -= 4.8260e-6 * rpoly;
	rpoly *= Rt; // ^4
	temp -= 2.8183e-8 * rpoly;
	rpoly *= Rt; // ^5
	temp += 1.5243e-10 * rpoly;
	printf("Negative(%f)\r\n",temp);
	return temp;
}

float max31865_rtd_to_resist(void)
{
	float rt=0.0;
	uint16_t read_rtd=0;

	max31865_read_rtd(&read_rtd);

	rt=(float)read_rtd;
	rt/=32768;
	rt*=1000.0;

	return rt;

}

void solve_system_3x3(float a[3][3], float b[3], float x[3])
{
    int i=0, j=0, k=0,pivot_row=0;
    double factor=0.0,temp=0.0,temp_b=0.0,sum_ajx=0.0;

    for (k = 0; k < 2; k++)
    {
        pivot_row = k;
        for (i = k + 1; i < 3; i++)
        {
            if (fabs(a[i][k]) > fabs(a[pivot_row][k]))
            {
                pivot_row = i;
            }
        }

        if (pivot_row != k)
        {
            for (j = k; j < 3; j++)
            {
                temp = a[k][j];
                a[k][j] = a[pivot_row][j];
                a[pivot_row][j] = temp;
            }
            temp_b = b[k];
            b[k] = b[pivot_row];
            b[pivot_row] = temp_b;
        }

        if (fabs(a[k][k]) < 1e-9)
        {
            printf("=========>ERROR_CAL(1)\r\n");
            return;
        }

        for (i = k + 1; i < 3; i++)
        {
            factor = a[i][k] / a[k][k];
            for (j = k; j < 3; j++)
            {
                a[i][j] -= factor * a[k][j];
            }
            b[i] -= factor * b[k];
        }
    }

    if (fabs(a[2][2]) < 1e-9)
    {
        printf("=========>ERROR_CAL(2)\r\n");
        return;
    }

    for (i = 2; i >= 0; i--)
    {
        sum_ajx = 0.0;
        for (j = i + 1; j < 3; j++)
        {
            sum_ajx += a[i][j] * x[j];
        }
        x[i] = (b[i] - sum_ajx) / a[i][i];
    }
}


float read_temp_without_cal(void)
{
	uint16_t read_rtd=0;
	float Z1, Z2, Z3, Z4, Rt, temp;

	max31865_read_rtd(&read_rtd);

	Rt = (float)read_rtd;
	Rt /= 32768;
	Rt *= 1000.0;

	Z1 = -RTD_A;
	Z2 = RTD_A * RTD_A - (4 * RTD_B);
	Z3 = (4 * RTD_B) / 200.0;
	Z4 = 2 * RTD_B;

	temp = Z2 + (Z3 * Rt);
	temp = (sqrt(temp) + Z1) / Z4;
	if (temp >= 0)
	{
		//printf("Positive(%f)\r\n",temp);
		return temp;
	}
	Rt /= 200.0;//100.0;
	Rt *= 100.0;//100; // normalize to 100 ohm

	float rpoly = Rt;

	temp = -242.02;
	temp += 2.2228 * rpoly;
	rpoly *= Rt; // square
	temp += 2.5859e-3 * rpoly;
	rpoly *= Rt; // ^3
	temp -= 4.8260e-6 * rpoly;
	rpoly *= Rt; // ^4
	temp -= 2.8183e-8 * rpoly;
	rpoly *= Rt; // ^5
	temp += 1.5243e-10 * rpoly;
	//printf("Negative(%f)\r\n",temp);
	return temp;
}










