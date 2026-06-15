/*
 * adc.c
 *
 *  Created on: May 13, 2025
 *      Author: kimst
 */


#include "adc.h"

extern ADC_HandleTypeDef hadc1;
__IO uint16_t aADCxConvertedData[3]={0,0,0},adc_vref=0,bat_volt=0;
int16_t mcu_temp=0;

void adc_channel_sel(uint8_t ch)
{
	ADC_ChannelConfTypeDef sConfig = {0};
	if(ch==0)
	{
		sConfig.Channel = ADC_CHANNEL_VREFINT;
		sConfig.Rank = ADC_REGULAR_RANK_1;
		sConfig.SamplingTime = ADC_SAMPLETIME_36CYCLES;
		sConfig.SingleDiff = ADC_SINGLE_ENDED;
		sConfig.OffsetNumber = ADC_OFFSET_NONE;
		sConfig.Offset = 0;
		if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
		{
			Error_Handler();
		}
	}
	else if(ch==1)
	{
		sConfig.Channel = ADC_CHANNEL_TEMPSENSOR;
		sConfig.Rank = ADC_REGULAR_RANK_1;
		sConfig.SamplingTime = ADC_SAMPLETIME_36CYCLES;
		sConfig.SingleDiff = ADC_SINGLE_ENDED;
		sConfig.OffsetNumber = ADC_OFFSET_NONE;
		sConfig.Offset = 0;
		if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
		{
			Error_Handler();
		}
	}
	else
	{
		sConfig.Channel = ADC_CHANNEL_VBAT;
		sConfig.Rank = ADC_REGULAR_RANK_1;
		sConfig.SamplingTime = ADC_SAMPLETIME_36CYCLES;
		sConfig.SingleDiff = ADC_SINGLE_ENDED;
		sConfig.OffsetNumber = ADC_OFFSET_NONE;
		sConfig.Offset = 0;
		if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
		{
			Error_Handler();
		}
	}
}

void adc_channel_read_start(uint16_t *bat, int16_t *mcu, uint16_t *vref)
{
	adc_channel_sel(0);
	HAL_ADC_Start(&hadc1);
	HAL_ADC_PollForConversion(&hadc1, 1000);
	aADCxConvertedData[0]=HAL_ADC_GetValue(&hadc1);
	HAL_ADC_Stop(&hadc1);
	adc_vref=(VREFINT_CAL_VREF * (*VREFINT_CAL_ADDR)) / aADCxConvertedData[0];

	adc_channel_sel(1);
	HAL_ADC_Start(&hadc1);
	HAL_ADC_PollForConversion(&hadc1, 1000);
	aADCxConvertedData[1]=HAL_ADC_GetValue(&hadc1);
	HAL_ADC_Stop(&hadc1);
	mcu_temp=__LL_ADC_CALC_TEMPERATURE(ADC1,adc_vref,aADCxConvertedData[1],LL_ADC_RESOLUTION_14B);

	adc_channel_sel(2);
	HAL_ADC_Start(&hadc1);
	HAL_ADC_PollForConversion(&hadc1, 1000);
	aADCxConvertedData[2]=HAL_ADC_GetValue(&hadc1);
	HAL_ADC_Stop(&hadc1);
	bat_volt=__LL_ADC_CALC_DATA_TO_VOLTAGE(ADC1, adc_vref, aADCxConvertedData[2], LL_ADC_RESOLUTION_14B);


	*bat=aADCxConvertedData[2];//bat_volt;
	*mcu=mcu_temp;
#if 0
	*bat=bat_volt;
	*mcu=mcu_temp;
	*vref=adc_vref;
#endif
	//printf("VREF:%d,VAT:%d mV,TEMP:%d \r\n",adc_vref,bat_volt*4,mcu_temp);

}
