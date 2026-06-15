/*
 * uart.c
 *
 *  Created on: May 8, 2025
 *      Author: kimst
 */


#include "uart.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdbool.h>
#include "ili9341.h"
#include "tim6.h"
#include "max31865.h"
#include "bsp_spi2.h"
#include "ads124s06.h"
#include "mcu_flash.h"
#include "main.h"
#include "control.h"

extern UART_HandleTypeDef huart2;
extern UART_HandleTypeDef huart1;
extern control_flag flag_control;
extern uint32_t selection_mode;
extern IWDG_HandleTypeDef hiwdg;
#ifdef __GNUC__
  /* With GCC, small printf (option LD Linker->Libraries->Small printf
     set to 'Yes') calls __io_putchar() */
  #define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
  #define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#endif /* __GNUC__ */

PUTCHAR_PROTOTYPE
{
  HAL_UART_Transmit(&huart1, (uint8_t *)&ch, 1, 0xFFFF);
  return ch;
}
uint8_t rx_data_byte;
uint8_t RxSp=0,RxEp=0; Test_Ep=0;
#define RECV_BUF_CNT 256

uint32_t ReadCnt=0,AckMsgFlag=0,NckMsgFlag=0,NckTimeOutCnt=0,AckTimeOut=0;
uint32_t StxFindCnt=9999,EnqFindCnt=9999,CrFindCnt=9999,StartCksumCnt=0,m_Cnt=0;
uint32_t ReadCntBuffer=0,asc_r_update_cnt=0;
uint8_t checkSumValue=0,receiveComplete=0,ChecksumSuccessFlag=0,Old_ReadFrameByte=0,Buf_Opcode=0,OP_CODE=0;
uint8_t StreammingUpdateFlag=0,structsum=0;
uint8_t StrToIntbuf[50],ResCheckSum=0,calcCheckSum_1=0,calcCheckSum_2=0;
uint32_t upmsdelay=0,AllStartIndex=0,AllEndIndex=0,UploadTempsetCnt=0;
char bintostrhex[50];
uint16_t sbatteryValue=0;
int16_t smcu_temp_value=0;



Protocol_CalcRecive_Frame CalcReadFrame;
Protocol_Frame ReadFrame;
Protocol_Frame WriteEFrame;
Protocol_Frame WriteMFrame;
_opcode_p opcode_p;
_opcode_f opcode_f;
Buf_Frame s_Buf_Frame;
BootTriger_Frame BootTriger;

Protocol_Frame ble_peripheral;
uint32_t ble_peripheral_cnt=0,ble_peripheral_read=0,ble_peripheral_start=0,ble_peripheral_end=0;
char temp_string_buf[50]={0,};
//float sbatteryValue=0.0,smcu_temp_value=0.0;

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{

	if(huart->Instance == USART2)
	{
		//printf("TEST\r\n");
		ReadFrame._01_byte[ReadCnt]=rx_data_byte;
		if( ReadFrame._01_byte[ReadCnt]==(int8_t)0xAA )
			AckMsgFlag=1;
		if( ReadFrame._01_byte[ReadCnt]==0x15 )
			NckMsgFlag=1;

		if(ReadFrame._01_byte[ReadCnt] == CR)
		{


			checkSumValue=0;
			CrFindCnt=ReadCnt;
			if(StxFindCnt!=9999)
				StartCksumCnt=StxFindCnt;
			if(EnqFindCnt!=9999)
				StartCksumCnt=EnqFindCnt;
				ReadCntBuffer = ReadCnt + 1;
			for(int x = StartCksumCnt; x < ReadCntBuffer; x++)
			{
				CalcReadFrame._R_01_byte[x] = ReadFrame._01_byte[x];
				ReadFrame._01_byte[x] 	= 0;
				if(x < ReadCntBuffer - 3)
				{
					checkSumValue += CalcReadFrame._R_01_byte[x];
				}
				//printf("Completes\r\n");
			}
			if( (CalcReadFrame._R_01_byte[CrFindCnt-2]== (checkSumValue>>4)+0x30) && (CalcReadFrame._R_01_byte[CrFindCnt-1]== (checkSumValue&0x0f)+0x30) )
			{
				//printf("SUCCESS 1\r\n");
				ChecksumSuccessFlag=1;
				receiveComplete=1;
				//SEGGER_RTT_printf(0,"TRI:\r\n");

				//SEGGER_RTT_printf(0,"\r\n");
			}
			else if( ((uint8_t)CalcReadFrame._R_01_byte[CrFindCnt-2]== 0xCC) && ((uint8_t)CalcReadFrame._R_01_byte[CrFindCnt-1]== 0xCC) ) // allpass checksum 0xcccc
			{
				//printf("SUCCESS 2\r\n");
				ChecksumSuccessFlag=1;
				receiveComplete=1;
			}
			else
			{
				//printf("Fail\r\n");
				ChecksumSuccessFlag=0;
				receiveComplete=0;
				//SEGGER_RTT_printf(0,"CHECKSUM_FAIL[%2x][%2x]\r\n",(checkSumValue>>4)+0x30,(checkSumValue&0x0f)+0x30);
			}
			ReadCnt=0;
			//StxFindCnt=9999;
			//EnqFindCnt=9999;
			//CrFindCnt=9999;
		}
		else
		{
			if((Old_ReadFrameByte==STX)||(Old_ReadFrameByte==ENQ))
			{
				Buf_Opcode=ReadFrame._01_byte[ReadCnt];
				if(Old_ReadFrameByte==STX)
					StxFindCnt=(ReadCnt-1);
				else
					EnqFindCnt=(ReadCnt-1);
			}
			Old_ReadFrameByte=ReadFrame._01_byte[ReadCnt];
			//SEGGER_RTT_printf(0,"R[%2x]\r\n",ReadFrame._01_byte[ReadCnt]);
			ReadCnt++;
			if(ReadCnt>=1024)
			{
				ReadCnt=0;
				StxFindCnt=9999;
				EnqFindCnt=9999;
				CrFindCnt=9999;
				receiveComplete=0;
			}
		}
		HAL_UART_Receive_IT(&huart2,&rx_data_byte,1);
	}
	if(huart->Instance == USART1)
	{
		RxEp=1;
		HAL_UART_Receive_IT(&huart1,&RxSp, 1);
	}
}

void uart1_init(void)
{
	huart1.Instance = USART1;
	huart1.Init.BaudRate = 115200;
	huart1.Init.WordLength = UART_WORDLENGTH_8B;
	huart1.Init.StopBits = UART_STOPBITS_1;
	huart1.Init.Parity = UART_PARITY_NONE;
	huart1.Init.Mode = UART_MODE_TX_RX;
	huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	huart1.Init.OverSampling = UART_OVERSAMPLING_16;
	huart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
	huart1.Init.ClockPrescaler = UART_PRESCALER_DIV1;
	huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
	if (HAL_UART_Init(&huart1) != HAL_OK)
	{
	  Error_Handler();
	}
	if (HAL_UARTEx_SetTxFifoThreshold(&huart1, UART_TXFIFO_THRESHOLD_1_8) != HAL_OK)
	{
	  Error_Handler();
	}
	if (HAL_UARTEx_SetRxFifoThreshold(&huart1, UART_RXFIFO_THRESHOLD_1_8) != HAL_OK)
	{
	  Error_Handler();
	}
	if (HAL_UARTEx_DisableFifoMode(&huart1) != HAL_OK)
	{
	  Error_Handler();
	}
	HAL_UART_Receive_IT(&huart1,&RxSp,1);

	ReadCnt=0;
	StxFindCnt=9999;
	EnqFindCnt=9999;
	CrFindCnt=9999;
	receiveComplete=0;
}

void uart2_init(void)
{
	huart2.Instance = USART2;
	huart2.Init.BaudRate = 115200;
	huart2.Init.WordLength = UART_WORDLENGTH_8B;
	huart2.Init.StopBits = UART_STOPBITS_1;
	huart2.Init.Parity = UART_PARITY_NONE;
	huart2.Init.Mode = UART_MODE_TX_RX;
	huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	huart2.Init.OverSampling = UART_OVERSAMPLING_16;
	huart2.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
	huart2.Init.ClockPrescaler = UART_PRESCALER_DIV1;
	huart2.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
	if (HAL_UART_Init(&huart2) != HAL_OK)
	{
	  Error_Handler();
	}
	if (HAL_UARTEx_SetTxFifoThreshold(&huart2, UART_TXFIFO_THRESHOLD_1_8) != HAL_OK)
	{
	  Error_Handler();
	}
	if (HAL_UARTEx_SetRxFifoThreshold(&huart2, UART_RXFIFO_THRESHOLD_1_8) != HAL_OK)
	{
	  Error_Handler();
	}
	if (HAL_UARTEx_DisableFifoMode(&huart2) != HAL_OK)
	{
	  Error_Handler();
	}
	HAL_UART_Receive_IT(&huart2,&rx_data_byte,1);//
}

void SpaceRemoveLeft(uint8_t *strptr, uint32_t lenth)
{
   uint8_t SCnt=0, buf=0;

   for( ;SCnt<lenth; SCnt++)
   {
	   if(strptr[0]==0x20)
	   {
		   buf=strptr[0];
          memcpy(strptr, strptr+1, lenth-1);
          strptr[lenth-1]=buf;
	   }
	   else
		   return;  // space 아닌 datafield가 나오면 빠져나감.
   }
}

uint8_t Protocol_M(uint8_t op_code, control_flag *flag)
{
	uint32_t Buf_setDataSet=0,Buf_samplingRate=0,Buf_timeDelay=0;
	int16_t Buf_setTriggerTemp=0;
	float Buf_transienttemp=1.5f;
	uint8_t Buf_TriggerMode[18];
	uint8_t Res=1;
	_opcode_M *ptr_opcode_M_Op_Buf;

	ptr_opcode_M_Op_Buf=(_opcode_M *)WriteMFrame._01_byte;

	memset(StrToIntbuf,0,sizeof(StrToIntbuf));
	memcpy(StrToIntbuf,ptr_opcode_M_Op_Buf->acquisition_time,5);
	Buf_setDataSet 	= atol((char*)StrToIntbuf);
	flag->i_acquisition_time=Buf_setDataSet;

	memset(StrToIntbuf,0,sizeof(StrToIntbuf));
	memcpy(StrToIntbuf,ptr_opcode_M_Op_Buf->sample_period,4);
	Buf_samplingRate 	= atol((char*)StrToIntbuf);

	memset(StrToIntbuf,0,sizeof(StrToIntbuf));
	memcpy(StrToIntbuf,ptr_opcode_M_Op_Buf->delaytime,6);
	Buf_timeDelay 	= atol((char*)StrToIntbuf)*1000;

	memset(StrToIntbuf,0,sizeof(StrToIntbuf));
	memcpy(StrToIntbuf,ptr_opcode_M_Op_Buf->trigger_temperature,4);
	Buf_setTriggerTemp 	= atol((char*)StrToIntbuf);

	memset(StrToIntbuf,0,sizeof(StrToIntbuf));
	memcpy(StrToIntbuf,ptr_opcode_M_Op_Buf->TransientTemp,5);
	Buf_transienttemp 	=(float)atof((char*)StrToIntbuf);

	memset(Buf_TriggerMode,0x20,sizeof(Buf_TriggerMode));
	memcpy(Buf_TriggerMode,ptr_opcode_M_Op_Buf->TriggerMode,18);
	SpaceRemoveLeft(Buf_TriggerMode,18);

	if( (CrFindCnt-StxFindCnt)!=234 ) // M Lenth
	{
		//printf("NOT 234__%d\r\n",(CrFindCnt-StxFindCnt));
		return 1;
	}
	if((flag->i_acquisition_time!=0)&&(Buf_samplingRate!=0))
	{
		//if(!memcmp(ptr_opcode_M->TriggerMode,"Immediately",11))			// Immediately mode
		if(!memcmp(Buf_TriggerMode,"Immediately",11))
		{
			//printf("Immediately mode Ok\r\n");
			flag->trigger 			= IMMEDIATELY; // PROTOCOL_M_IMMEDIATELY_MODE >>IMMEDIATELY
			flag->setDataSet 		= (uint32_t)(Buf_setDataSet*((float)1000/Buf_samplingRate));//회수
			flag->samplingRate	 	= Buf_samplingRate;
			flag->comState 		 	= COM_DISCONNECT_MODE;
			flag->measurementState 	= MEASUREMENT_START;

			flag->timeDelay 		= Buf_timeDelay;
			flag->setTriggerTemp	= Buf_setTriggerTemp;
			flag->Applytransient	= Buf_transienttemp;
			Res =0;
		}
		//else if(!memcmp(ptr_opcode_M->TriggerMode,"Timedelay",9))
		else if(!memcmp(Buf_TriggerMode,"Timedelay",9))
		{
			flag->trigger			= TIMEDELAY;
			flag->setDataSet 		= (uint32_t)(Buf_setDataSet*((float)1000/Buf_samplingRate));
			flag->samplingRate	 	= Buf_samplingRate;
			flag->comState 		 	= COM_DISCONNECT_MODE;
			flag->measurementState 	= MEASUREMENT_START;

			flag->timeDelay		 	= Buf_timeDelay;//<=========
			flag->setTriggerTemp	= Buf_setTriggerTemp;
			flag->Applytransient	= Buf_transienttemp;
			Res =0;
		}
		//else if(!memcmp(ptr_opcode_M->TriggerMode,"AtTempdelay",11))
		else if(!memcmp(Buf_TriggerMode,"AtTempdelay",11))
		{
			flag->trigger			= AT_TEMP_DELAY;
			flag->setDataSet 		= (uint32_t)(Buf_setDataSet*((float)1000/Buf_samplingRate));
			flag->samplingRate	 	= Buf_samplingRate;
			flag->comState 		 	= COM_DISCONNECT_MODE;
			flag->measurementState 	= MEASUREMENT_START;

			flag->timeDelay 		= Buf_timeDelay;
			flag->setTriggerTemp	= Buf_setTriggerTemp;
			flag->Applytransient	= Buf_transienttemp;
			Res =0;
		}
		//else if(!memcmp(ptr_opcode_M->TriggerMode,"Transient",9))
		else if(!memcmp(Buf_TriggerMode,"Transient",9))//온도 차가 발생하면
		{
			flag->trigger			= TEMPTRANSIENT;
			flag->setDataSet 		= (uint32_t)(Buf_setDataSet*((float)1000/Buf_samplingRate));
			flag->samplingRate	 	= Buf_samplingRate;
			flag->comState 		 	= COM_DISCONNECT_MODE;
			flag->measurementState 	= MEASUREMENT_START;

			flag->timeDelay 		= Buf_timeDelay;
			flag->setTriggerTemp	= Buf_setTriggerTemp;
			flag->Applytransient    = Buf_transienttemp;
			Res =0;
		}
		//else if(!memcmp(ptr_opcode_M->TriggerMode,"DelayorTemp",11))
		else if(!memcmp(Buf_TriggerMode,"DelayorTemp",11))
		{
			flag->trigger			= DELAY_OR_TEMPERATURE;
			flag->setDataSet 		= (uint32_t)(Buf_setDataSet*((float)1000/Buf_samplingRate));
			flag->samplingRate	 	= Buf_samplingRate;
			flag->comState 		 	= COM_DISCONNECT_MODE;
			flag->measurementState 	= MEASUREMENT_START;

			flag->timeDelay 		= Buf_timeDelay;
			flag->setTriggerTemp	= Buf_setTriggerTemp;
			flag->Applytransient    = Buf_transienttemp;
			Res =0;
		}
	}
	else
	{
		//printf("NOT MODE\r\n");
		Res=1;
	}
	return Res;
}
#if 0 // INTENSE HT 기존 Code Remove
void ble_peripheral_read_proc(uint32_t *leng, uint8_t *buf)
{
	uint32_t loop=0,i=0;
	uint8_t read_out_data=0,checksum_cal=0;
	*leng=0;
	if(ble_peripheral_read!=ble_peripheral_cnt)
	{
		read_out_data=ble_peripheral._01_byte[ble_peripheral_read];
		if(read_out_data == STX || read_out_data == ENQ)
		{
			ble_peripheral_start=ble_peripheral_read;
			ble_peripheral_read++;
			if(ble_peripheral_read>=1024)
			{
				ble_peripheral_read=0;
			}
		}
		else
		{
			if(read_out_data == CR_)
			{
				ble_peripheral_end=ble_peripheral_read+1;
				if(ble_peripheral_end<ble_peripheral_start)
				{
					//ble_peripheral_end=ble_peripheral_read+1;
					for(i=ble_peripheral_start;i<1024;i++)
					{
						buf[loop]=ble_peripheral._01_byte[i];
						ble_peripheral._01_byte[i]=0;
						loop++;
					}
					for(i=0;i<ble_peripheral_end;i++)
					{
						buf[loop]=ble_peripheral._01_byte[i];
						ble_peripheral._01_byte[i]=0;
						loop++;
					}
					for(i=0;i<loop-3;i++)
					{
						checksum_cal+=buf[i];
					}
				}
				else
				{
					//ble_peripheral_end=ble_peripheral_read+1;
					for(i=ble_peripheral_start;i<ble_peripheral_end;i++)
					{
						buf[loop]=ble_peripheral._01_byte[i];
						ble_peripheral._01_byte[i]=0;
						if(i<ble_peripheral_end-3)
						{
							checksum_cal+=buf[loop];
						}
						loop++;
					}
				}
				printf("[%ld][%ld]\r\n",ble_peripheral_start,ble_peripheral_end);
				if( (buf[loop-3]== (checksum_cal>>4)+0x30) && (buf[loop-2]== (checksum_cal&0x0f)+0x30) )
				{
					if(ble_peripheral_end<ble_peripheral_start)
					{
						*leng=(1024-ble_peripheral_start)+ble_peripheral_end;
					}
					else
					{
						*leng=(ble_peripheral_end-ble_peripheral_start);
					}
				}
				else
				{
					if(ble_peripheral_end<ble_peripheral_start)
					{
						*leng=(1024-ble_peripheral_start)+ble_peripheral_end;
					}
					else
					{
						*leng=(ble_peripheral_end-ble_peripheral_start);
					}
					printf("CHECKSUM ERROR[%ld]\r\n",*leng);
					*leng=0;
				}
				ble_peripheral_read++;
				if(ble_peripheral_read == ble_peripheral_cnt)
				{
					ble_peripheral_read=0;
					ble_peripheral_cnt=0;
				}
				else
				{
					if(ble_peripheral_read>=1024)
					{
						ble_peripheral_read=0;
					}
				}
			}
			else
			{
				ble_peripheral_read++;
				if(ble_peripheral_read>=1024)
				{
					ble_peripheral_read=0;
				}
			}
		}
	}
}

void ble_peripheral_process(uint32_t *leng, uint8_t *buf,uint32_t* tx_len, uint8_t* tx_buf,control_flag* flag)
{
	uint32_t index=0,i=0,cpy_sample=0;
	uint8_t fcs[2]={0,},fcs_cal=0;
	uint16_t intense_volt=0;
	float cal_intense_volt=0.0;
	_opcode_M *mission_packet;
	_opcode_E *status_packet;
	mission_packet=(_opcode_M*)(uint32_t)ADDR_FlASH_PAGE_A_125;
	status_packet=(_opcode_E*)(uint32_t)ADDR_FlASH_PAGE_A_126;
	Protocol_Frame wr_buf;
	*tx_len=0;
	if(flag->receive_calibration==CALIBRATION_STOP)
	{
		if(*leng!=0)
		{
			flag->ble_peri_cmd_process=0;
			switch(buf[1])
			{
				case ASC_A:
#ifdef DIRECT_3P_CAL
					if(buf[2]=='1'||buf[2]=='2'||buf[2]=='6'||buf[2]=='7'||buf[2]=='8'||buf[2]=='9')
#else
					if(buf[2]=='1'||buf[2]=='2'||buf[2]=='6'||buf[2]=='7')
#endif
					{
						flag->receive_calibration=buf[2]-0x30;
						memset(temp_string_buf,0,sizeof(temp_string_buf));
						memcpy(temp_string_buf,&buf[3],8);
						flag->receive_ref_resist=atof(temp_string_buf);
#ifdef DIRECT_3P_CAL
						if(buf[2]=='6'||buf[2]=='8'||buf[2]=='9')
						{
							if(buf[2]=='6')
							{
								flag->cal_data.ref_resist[2]=flag->receive_ref_resist*CAL_MUL_DIV_VAL_NEW;
							}
							if(buf[2]=='8')
							{
								flag->cal_data.ref_resist[3]=flag->receive_ref_resist*CAL_MUL_DIV_VAL_NEW;
							}
							if(buf[2]=='9')
							{
								flag->cal_data.ref_resist[4]=flag->receive_ref_resist*CAL_MUL_DIV_VAL_NEW;
							}
						}
						else
#endif
							flag->cal_data.ref_resist[flag->receive_calibration-1]=flag->receive_ref_resist*CAL_MUL_DIV_VAL_NEW;

						memset(temp_string_buf,0,sizeof(temp_string_buf));
						memcpy(temp_string_buf,&buf[11],4);
						flag->stable_time=atoi(temp_string_buf);
						if(flag->stable_time<=0)
						{
							flag->stable_time=1;
						}
						memset(temp_string_buf,0,sizeof(temp_string_buf));
						memcpy(temp_string_buf,&buf[15],4);
						flag->acquisition_time=atoi(temp_string_buf);
						printf("ASC_A[%8.2f][%d][%d]\r\n",flag->receive_ref_resist,flag->stable_time,flag->acquisition_time);
						flag->acquisition_time_proc=0;
						flag->stable_time_proc=0;
						if(flag->receive_calibration==CLA_COMPLETE_SEQ)
						{
							flag->cal_data.calibration_flag=0xffffffff;
							flag->acquisition_time=0;
							flag->stable_time=0;
							flag->acquisition_time_proc=1;
							flag->stable_time_proc=1;
						}
						clear_calibration_cnt();
						tx_buf[index++]=STX;tx_buf[index++]='a';
						tx_buf[index++]=flag->receive_calibration+0x30;
						tx_buf[index++]=((flag->cal_data.calibration_flag>>28)&0xf)+0x30;
						tx_buf[index++]=((flag->cal_data.calibration_flag>>24)&0xf)+0x30;
						tx_buf[index++]=((flag->cal_data.calibration_flag>>20)&0xf)+0x30;
						tx_buf[index++]=((flag->cal_data.calibration_flag>>16)&0xf)+0x30;
						tx_buf[index++]=((flag->cal_data.calibration_flag>>12)&0xf)+0x30;
						tx_buf[index++]=((flag->cal_data.calibration_flag>>8)&0xf)+0x30;
						tx_buf[index++]=((flag->cal_data.calibration_flag>>4)&0xf)+0x30;
						tx_buf[index++]=((flag->cal_data.calibration_flag)&0xf)+0x30;
						tx_buf[index++]=ETX;
						for(i=0;i<index;i++)
						{
							fcs_cal+=tx_buf[i];
						}
						fcs[0]=(fcs_cal>>4)+0x30;
						fcs[1]=(fcs_cal&0x0f)+0x30;
						tx_buf[i++]=fcs[0];
						tx_buf[i++]=fcs[1];
						tx_buf[i++]=CR_;
						*tx_len=i;
					}
					else
					{
						printf("ASC_A_NAK\r\n");
						tx_buf[index++]=STX;tx_buf[index++]='a';
						tx_buf[index++]=buf[2];
						tx_buf[index++]=0+0x30;
						tx_buf[index++]=0+0x30;
						tx_buf[index++]=0+0x30;
						tx_buf[index++]=0+0x30;
						tx_buf[index++]=0+0x30;
						tx_buf[index++]=0+0x30;
						tx_buf[index++]=0+0x30;
						tx_buf[index++]=0+0x30;
						tx_buf[index++]=ETX;
						for(i=0;i<index;i++)
						{
							fcs_cal+=tx_buf[i];
						}
						fcs[0]=(fcs_cal>>4)+0x30;
						fcs[1]=(fcs_cal&0x0f)+0x30;
						tx_buf[i++]=fcs[0];
						tx_buf[i++]=fcs[1];
						tx_buf[i++]=CR_;
						*tx_len=i;
					}
				break;
				case ASC_H:
					if(buf[2]=='3'||buf[2]=='4'||buf[2]=='5')
					{
						flag->receive_calibration=buf[2]-0x30;
						memset(temp_string_buf,0,sizeof(temp_string_buf));
						memcpy(temp_string_buf,&buf[3],8);
						flag->receive_ref_resist=atof(temp_string_buf);
						flag->cal_data.ref_resist[flag->receive_calibration-1]=flag->receive_ref_resist*CAL_MUL_DIV_VAL_NEW;
						i=11;
						printf("REF[%d]\r\n",flag->cal_data.ref_resist[flag->receive_calibration-1]);
						for(cpy_sample=0;cpy_sample<SENSOR_TOTAL_CNT;cpy_sample++)
						{
							memset(temp_string_buf,0,sizeof(temp_string_buf));
							memcpy(temp_string_buf,&buf[i],8);
							flag->cal_data.pre_cal_resist[cpy_sample][flag->receive_calibration-3]=(atof(temp_string_buf)*CAL_MUL_DIV_VAL_NEW);
							//flag->cal_data.pre_cal_resist[cpy_sample][flag->receive_calibration-3]+=flag_control.cal_data.ref_cal_resist[cpy_sample][0];
							printf("RECV_R(%d)[%d]\r\n",cpy_sample,flag->cal_data.pre_cal_resist[cpy_sample][flag->receive_calibration-3]);
							i+=8;
						}
						flag->stable_time=0;
						flag->acquisition_time=0;;
						flag->acquisition_time_proc=1;
						flag->stable_time_proc=1;
						clear_calibration_cnt();
						tx_buf[index++]=STX;tx_buf[index++]='a';
						tx_buf[index++]=flag->receive_calibration+0x30;
						tx_buf[index++]=((flag->cal_data.calibration_flag>>28)&0xf)+0x30;
						tx_buf[index++]=((flag->cal_data.calibration_flag>>24)&0xf)+0x30;
						tx_buf[index++]=((flag->cal_data.calibration_flag>>20)&0xf)+0x30;
						tx_buf[index++]=((flag->cal_data.calibration_flag>>16)&0xf)+0x30;
						tx_buf[index++]=((flag->cal_data.calibration_flag>>12)&0xf)+0x30;
						tx_buf[index++]=((flag->cal_data.calibration_flag>>8)&0xf)+0x30;
						tx_buf[index++]=((flag->cal_data.calibration_flag>>4)&0xf)+0x30;
						tx_buf[index++]=((flag->cal_data.calibration_flag)&0xf)+0x30;
						tx_buf[index++]=ETX;
						for(i=0;i<index;i++)
						{
							fcs_cal+=tx_buf[i];
						}
						fcs[0]=(fcs_cal>>4)+0x30;
						fcs[1]=(fcs_cal&0x0f)+0x30;
						tx_buf[i++]=fcs[0];
						tx_buf[i++]=fcs[1];
						tx_buf[i++]=CR_;
						*tx_len=i;
					}
					else
					{
						printf("ASC_H_NAK\r\n");
						tx_buf[index++]=STX;tx_buf[index++]='h';
						tx_buf[index++]=buf[2];
						tx_buf[index++]=0+0x30;
						tx_buf[index++]=0+0x30;
						tx_buf[index++]=0+0x30;
						tx_buf[index++]=0+0x30;
						tx_buf[index++]=0+0x30;
						tx_buf[index++]=0+0x30;
						tx_buf[index++]=0+0x30;
						tx_buf[index++]=0+0x30;
						tx_buf[index++]=ETX;
						for(i=0;i<index;i++)
						{
							fcs_cal+=tx_buf[i];
						}
						fcs[0]=(fcs_cal>>4)+0x30;
						fcs[1]=(fcs_cal&0x0f)+0x30;
						tx_buf[i++]=fcs[0];
						tx_buf[i++]=fcs[1];
						tx_buf[i++]=CR_;
						*tx_len=i;
					}
				break;

			}
		}
	}
}
#endif

void Process(void)
{
	int loop=0;
	float not_cal_temp[SENSOR_TOTAL_CNT]={0.0,},not_cal_res[SENSOR_TOTAL_CNT]={0.0,};
	if(RxEp)
	{
		RxEp=0;
		clear_adc_cnt();
		for(loop=0;loop<SENSOR_TOTAL_CNT;loop++)
		{
			select_mux((uint8_t)loop);
			//not_cal_temp[loop]=read_temp_1st(&flag_control,loop,&not_cal_res[loop]); // no cal
			not_cal_temp[loop]=read_temp_2nd(&flag_control,loop,&not_cal_res[loop]);  // 0ohm 10k cal
			//not_cal_temp[loop]=pt1000_read_temp_3point_cal(&flag_control,loop,&not_cal_res[loop]);
			//not_cal_temp[loop]=pt1000_0_read_temp_3point_cal(&flag_control,loop,&not_cal_res[loop]);
		}
		printf("32_RTD_READ==>%ld ms\r\n",get_adc_cnt());
		{
			for(loop=0;loop<SENSOR_TOTAL_CNT;loop++)
			{
				printf("INDEX(%d),RESIS[%8.2f] TEMP[%6.2f]\r\n",loop,not_cal_res[loop],not_cal_temp[loop]);
			}
		}
		//select_mux(DUMMY_CHANNEL);
		select_mux(0);
	}
	else if(Test_Ep)
	{
		for(int i=0; i< 10; i++)
		{
			clear_adc_cnt();
			for(loop=0;loop<5;loop++)
			{
				select_mux((uint8_t)loop);
				//not_cal_temp[loop]=read_temp_1st(&flag_control,loop,&not_cal_res[loop]); // no cal
				not_cal_temp[loop]=read_temp_2nd(&flag_control,loop,&not_cal_res[loop]);  // 0ohm 10k cal
				//HAL_Delay(800);
				//not_cal_temp[loop]=pt1000_read_temp_3point_cal(&flag_control,loop,&not_cal_res[loop]);
				//not_cal_temp[loop]=pt1000_0_read_temp_3point_cal(&flag_control,loop,&not_cal_res[loop]);
			}
			//printf("32_RTD_READ==>%ld ms\r\n",get_adc_cnt());
			{
				for(loop=0;loop<5;loop++)
				{
					printf("%6.2f,",not_cal_temp[loop]);
				}
				printf("\r\n");
			}
			//select_mux(DUMMY_CHANNEL);
			select_mux(0);
		}
		Test_Ep = 0;
	}
}



void Protocol_Read(control_flag *flag)
{
	//uint8_t cpy_com_state=flag->comState;
	//uint8_t cpy_meas_start=flag->measurementState;
	_opcode_M *ptr_opcode_M;
	_opcode_E *ptr_opcode_E;
	ptr_opcode_M=(_opcode_M*)M_FLASH_AREA;
	ptr_opcode_E=(_opcode_E*)E_FLASH_AREA;
	if((receiveComplete == 1)&&((StxFindCnt!=9999)||(EnqFindCnt!=9999))&&(CrFindCnt!=9999))
	{
		checkSumValue = 0;
		asc_r_update_cnt=0;
		flag->ble_com_flag=1;
		/*
		if( (CrFindCnt!=9999)&&(Buf_Opcode=='I'||Buf_Opcode=='H'))
		{
			ChecksumSuccessFlag=0;
			receiveComplete = 0;
			StxFindCnt=9999;
			EnqFindCnt=9999;
			CrFindCnt=9999;
			for(int x = 0; x < ReadCntBuffer; x++)
			{
				CalcReadFrame._R_01_byte[x] = 0;
				ReadFrame._01_byte[x]		= 0;
			}
			flag->comState=cpy_com_state;
			flag->measurementState=cpy_meas_start;
			return;
		}
		*/
		OP_CODE=Buf_Opcode;
		if(ChecksumSuccessFlag)//if check sum
		{
			//printf("ENTER\r\n");
			//set_PoweroffTimeout(60000);
			StreammingUpdateFlag=0;
			structsum=0;
			HAL_IWDG_Refresh(&hiwdg);
			printf("OP_CODE : '%x' \r\n", OP_CODE);
			switch(OP_CODE)
			{
				case ASC_A:
				case ASC_H:
					if(OP_CODE == ASC_A)
					{
						printf("CMD_A\r\n");
					}
					else
					{
						printf("CMD_H\r\n");
					}

					//flag->receive_mission=NONE_MISSION;
					//flag->receive_temp_read=NONE_TMP_READ;
					//flag->receive_calibration=CALIBRATION_START;

					m_Cnt=0;
					for(int x = (StxFindCnt); x < (CrFindCnt); x++)
					{
						WriteMFrame._01_byte[m_Cnt] = CalcReadFrame._R_01_byte[x];
						CalcReadFrame._R_01_byte[x] = 0;
						m_Cnt++;
					}
					m_Cnt = 0;
					//printf("Write M Frame: %x\r\n",WriteMFrame._01_byte[2]);
					if(WriteMFrame._01_byte[2]==0x31||WriteMFrame._01_byte[2]==0x32||WriteMFrame._01_byte[2]==0x36||WriteMFrame._01_byte[2]==0x37)
					{
						flag->receive_calibration=WriteMFrame._01_byte[2]-0x30;
						printf("Write M Frame: %x\r\n",WriteMFrame._01_byte[2]);
						memset(bintostrhex,0,sizeof(bintostrhex));
						memcpy(bintostrhex,&WriteMFrame._01_byte[3],8);
						flag->receive_ref_resist=atof(bintostrhex);

						if(flag->receive_calibration == 1||flag->receive_calibration == 2)
						{
							flag->cal_data.ref_resist[flag->receive_calibration-1]=flag->receive_ref_resist*CAL_MUL_DIV_VAL_NEW;
						}

						printf("CMD A Short measurement \r\n");
						memset(bintostrhex,0,sizeof(bintostrhex));
						memcpy(bintostrhex,&WriteMFrame._01_byte[11],4);
						flag->stable_time=atoi(bintostrhex);

						if(flag->stable_time<=0)
						{
							flag->stable_time=1;
						}
						memset(bintostrhex,0,sizeof(bintostrhex));
						memcpy(bintostrhex,&WriteMFrame._01_byte[15],4);
						flag->acquisition_time=atoi(bintostrhex);
						flag->acquisition_time_proc=0;
						flag->stable_time_proc=0;
						if(flag->receive_calibration==CLA_COMPLETE_SEQ)
						{
							flag->cal_data.calibration_flag=0xffffffff;
							flag->acquisition_time=0;
							flag->stable_time=0;
							flag->acquisition_time_proc=1;
							flag->stable_time_proc=1;
						}
						/*
						if(flag->receive_calibration==6)
						{
							flag->acquisition_time=0;
							flag->stable_time=0;
							flag->acquisition_time_proc=1;
							flag->stable_time_proc=1;
						}*/
						clear_calibration_cnt();
						//ACK Data Process<================================
					}
					else
					{
						//NAK Process<================================
					}
					if(WriteMFrame._01_byte[2]==0x33||WriteMFrame._01_byte[2]==0x34||WriteMFrame._01_byte[2]==0x35)
					{
						flag->receive_calibration=WriteMFrame._01_byte[2]-0x30;
						printf("Write M Frame: %x\r\n",WriteMFrame._01_byte[2]);
						memset(bintostrhex,0,sizeof(bintostrhex));
						memcpy(bintostrhex,&WriteMFrame._01_byte[3],8);
						flag->receive_ref_resist=atof(bintostrhex);
						flag->cal_data.ref_resist[flag->receive_calibration-1]=flag->receive_ref_resist*CAL_MUL_DIV_VAL_NEW;
						printf("Write H Frame: %x\r\n",WriteMFrame._01_byte[2]);
						printf("CMD_H\r\n");
						int i=11,j;
						for(j=0;j<SENSOR_TOTAL_CNT;j++)
						{
							memset(bintostrhex,0,sizeof(bintostrhex));
							memcpy(bintostrhex,&WriteMFrame._01_byte[i],8);
							flag->cal_data.pre_cal_resist[j][flag->receive_calibration-3]=(atof(bintostrhex)*CAL_MUL_DIV_VAL_NEW);
							i+=8;
						}
						flag->stable_time=0;
						flag->acquisition_time=0;;
						flag->acquisition_time_proc=1;
						flag->stable_time_proc=1;
						clear_calibration_cnt();
						//ACK Data Process<================================
					}
					else
					{
						//NAK Process<================================
					}

				break;
				case ASC_M: // Type: Write  INTENSE wafer mission write request(SW→INTENSE)
					printf("CMD_M\r\n");
					m_Cnt=0;
					for(int x = (StxFindCnt+2); x < (CrFindCnt-4); x++)
					{
						WriteMFrame._01_byte[m_Cnt] = CalcReadFrame._R_01_byte[x];
						CalcReadFrame._R_01_byte[x] = 0;
						m_Cnt++;
					}
					m_Cnt = 0;
					if(Protocol_M(OP_CODE,flag))
					{
						UART1_Tx(STX);
						UART1_Tx(ASC_m);
						UART1_Tx(NAK);
						ResCheckSum= STX+ASC_m+NAK+ETX;
						printf("CMD_M NAK\r\n");
					}
					else
					{
						//printf("CMD_M\r\n");
						Protocol_Erase_Page(OP_CODE);
						Protocol_Erase_Page(ASC_R);
						Protocol_Flash_Memory_Store(OP_CODE,(uint32_t*)&WriteMFrame._04_byte[0]);

						UART1_Tx(STX);
						UART1_Tx(ASC_m);
						UART1_Tx(ACK);
						ResCheckSum= STX+ASC_m+ACK+ETX;
						clear_tick_val1();
						clear_mission_cnt();
						clear_mission_start();
						//flag->DoorTrigerTimeOut=10000;
						flag->DoorTrigerTimeOut=0;
					}

				break;

				case ASC_R://STX명령어가 아닌 ENQ명열어로 오는 것 같다.
					printf("READ\r\n");
#if 0
					asc_r_update_cnt=0;
					NckTimeOutCnt=0;
					NckMsgFlag=0;
					asc_r_update_cnt=0;
					if(!memcmp( &CalcReadFrame._R_01_byte[(EnqFindCnt+2)],"ALL",3) ) // ALL050MS (streame auto)
					{
						printf("CMD_R ALL\r\n");
						StreammingUpdateFlag=0;//StreammingUpdateFlag=1;
						 // "050MS" Parsing
						 memset(bintostrhex,0,sizeof(bintostrhex));
						 memcpy(bintostrhex,&CalcReadFrame._R_01_byte[(EnqFindCnt+5)], 3 );
						 upmsdelay=(uint32_t)atol(bintostrhex);
                         // "0000" Start Index Parsing
						 memset(bintostrhex,0,sizeof(bintostrhex));
						 memcpy(bintostrhex,&CalcReadFrame._R_01_byte[(EnqFindCnt+10)], 4 );
						 AllStartIndex=(uint32_t)atol(bintostrhex);
						 if(AllStartIndex>0)
						 {
							 AllStartIndex--;
						 }
                         // "0100" End Index Parsing
						 memset(bintostrhex,0,sizeof(bintostrhex));
						 memcpy(bintostrhex,&CalcReadFrame._R_01_byte[(EnqFindCnt+14)], 4 );
						 AllEndIndex=(uint32_t)atol(bintostrhex);
						 if(AllEndIndex>0)
						 {
							 AllEndIndex--;
						 }

						 //printf("upmsdelay:%ld\r\n",upmsdelay);
						 for(UploadTempsetCnt=AllStartIndex;UploadTempsetCnt<=(AllEndIndex+1);UploadTempsetCnt++) //0 >>1
						 //for(UploadTempsetCnt=AllStartIndex;UploadTempsetCnt<=(AllEndIndex);UploadTempsetCnt++)
						 {
#if 0
							 // Resend when nak is received & nak
							 if(NckMsgFlag==1)//BLE NAK
							 {
							 	HAL_Delay(150); //wait other nak
								NckMsgFlag=0;
								asc_r_update_cnt=0;
								if(UploadTempsetCnt>0)
									UploadTempsetCnt--;
							 }
#endif
							 UART1_Tx(STX);
							 UART1_Tx(ASC_r);
							 structsum=0;
							 //수정 필요 부분 온도데이터 + 배터리 + ㄷ다합쳐서 저장할 것
							 Opcod_r_TempUpload_Task(ADDR_FlASH_PAGE_B_000+UploadTempsetCnt*FLASH_SEGMENT,   //  0x80(64EA) >>> 0xB0(88EA)
									 	 	 	 	 ADDR_FlASH_PAGE_A_105+UploadTempsetCnt*2,  // trigger sensor size :2
													 ADDR_FlASH_PAGE_A_105+UploadTempsetCnt*2,
													 ADDR_FlASH_PAGE_A_118+UploadTempsetCnt*2);  // battery vale size : 2
							 // Streaming End Command
							 if( UploadTempsetCnt==(AllEndIndex+1) )
							 //if( UploadTempsetCnt==(AllEndIndex) )
							 {
							 	memcpy(opcode_r.index,"END ",4);
							 }
							 HAL_IWDG_Refresh(&hiwdg);
							 structsum=struct_send(opcode_r.index,sizeof(_opcode_r),upmsdelay,structsum);
							 //if( (AckTimeOut>ACKTIMEOUT_MSEC)||(NckTimeOutCnt>NCKTIMEOUT_ESC_CNT))
							//	 break;
							 ResCheckSum= STX+ASC_r+structsum+ETX;
							 UART1_Tx(ETX);
							 UART1_Tx((ResCheckSum>>4)+0x30);  //crc ResCheckSum
							 UART1_Tx((ResCheckSum&0x0f)+0x30);  //crc
							 UART1_Tx(CR);
							 HAL_Delay(50);
							 printf("R:%ld\r\n",asc_r_update_cnt);
							 //printf("UART_T:%d\r\n",asc_r_update_cnt);
						 }
					}
					else if( !memcmp( &CalcReadFrame._R_01_byte[(EnqFindCnt+2)],"REAL",4) ) // Realtime Mode
					{
						printf("CMD_R REAL\r\n");
						//===========ADD==============
						sensor_ldo_on();
						ReadRealSensorAndSaveBuf(&opcode_r_rt);

						HAL_GPIO_WritePin(LDO_EN0_GPIO_Port,LDO_EN0_Pin,GPIO_PIN_RESET);
						HAL_GPIO_WritePin(LDO_EN1_GPIO_Port,LDO_EN1_Pin,GPIO_PIN_RESET);
						HAL_GPIO_WritePin(LDO_EN2_GPIO_Port,LDO_EN2_Pin,GPIO_PIN_RESET);

						UART1_Tx(STX);
						UART1_Tx(ASC_r);
						structsum=struct_send(opcode_r_rt.index,sizeof(_opcode_r),150,0);
					}
					else  /// inex number
					{
						//StreammingUpdateFlag=1; // 240508 added by sgs
						printf("CMD_R INDEX\r\n");
						UART1_Tx(STX);
						UART1_Tx(ASC_r);
						memset(bintostrhex,0,sizeof(bintostrhex));
						memcpy(bintostrhex,&CalcReadFrame._R_01_byte[(EnqFindCnt+2)], 8 );

						UploadTempsetCnt= atol(bintostrhex);
						if(UploadTempsetCnt>=0)
						{
							UploadTempsetCnt--;
						}

						Opcod_r_TempUpload_Task(ADDR_FlASH_PAGE_B_000+UploadTempsetCnt*FLASH_SEGMENT ,
												ADDR_FlASH_PAGE_A_105+UploadTempsetCnt*2,   // trigger sensor size :2
												ADDR_FlASH_PAGE_A_105+UploadTempsetCnt*2,
												ADDR_FlASH_PAGE_A_118+UploadTempsetCnt*2);   // battery vale size : 2
						structsum=struct_send(opcode_r.index,sizeof(_opcode_r),150,0);
					}

					ResCheckSum= STX+ASC_r+structsum+ETX;
#endif
				break;
				case ASC_F:// Type: Read Factory setting request(SW→INTENSE)
					printf("CMD_F\r\n");

					asc_r_update_cnt=0;
					NckTimeOutCnt=0;
					NckMsgFlag=0;
					//FOR TEST
					StreammingUpdateFlag=0;//StreammingUpdateFlag=1;
					memcpy(opcode_f.sequencenumber,ptr_opcode_M->sequencenumber,8);
					memcpy(opcode_f.serialnumber,ptr_opcode_E->serialnumber,30);
					memcpy(opcode_f.mission_name,ptr_opcode_M->mission_name,40);
					memcpy(opcode_f.mission_complete_status,ptr_opcode_M->mission_complete_status,7);
					memcpy(opcode_f.sample_period,ptr_opcode_M->sample_period,4);
					memcpy(opcode_f.acquisition_time,ptr_opcode_M->acquisition_time,5);
					memcpy(opcode_f.trigger_temperature,ptr_opcode_M->trigger_temperature,4);
					memcpy(opcode_f.delaytime,ptr_opcode_M->delaytime,6);
					memcpy(opcode_f.TriggerMode,ptr_opcode_M->TriggerMode,18);
					memcpy(opcode_f.TransientTemp,ptr_opcode_M->TransientTemp,5);
					memcpy(opcode_f.intensefirmwareversion,FIRMWARE_VER,5);
					memcpy(opcode_f.intensewafersize,ptr_opcode_E->intensewafersize,5);
					memcpy(opcode_f.permittedusagetime,ptr_opcode_E->permittedusagetime,11);
					memcpy(opcode_f.hoursofused,ptr_opcode_E->hoursofused,11);

					adc_channel_read_start(&sbatteryValue,&smcu_temp_value);
					memset(bintostrhex,0,sizeof(bintostrhex));
					sprintf(bintostrhex,"%04X",sbatteryValue);
					memcpy(opcode_f.waferbattervoltage,bintostrhex,4);
					//memcpy(opcode_f.sensorfailure,"00FFFFFFFFFFFFFFFFFFFFFFFFFFFFFF",32);
					//memcpy(opcode_f.sensorfailure,flag->sensor_status,32);
					//for(int k=0,m=31;k<32;k++,m--)
					//{
					//	opcode_f.sensorfailure[k]=flag->sensor_status[m];
					//}
					uint8_t m=15,t=0;
					for(int k=0;k<32;k+=2)
					{
						t=((flag->sensor_status[m]>>4)&0x0f);
						if(t>=10)
						{
							t+=0x37;
						}
						else
						{
							t+=0x30;
						}
						opcode_f.sensorfailure[k]=t;
						t=(flag->sensor_status[m]&0x0f);
						if(t>=10)
						{
							t+=0x37;
						}
						else
						{
							t+=0x30;
						}
						opcode_f.sensorfailure[k+1]=t;
						m--;
					}
					//if(!HAL_GPIO_ReadPin(BAT_CHK_GPIO_Port,BAT_CHK_Pin))
					if(HAL_GPIO_ReadPin(BAT_DIR_GPIO_Port,BAT_DIR_Pin))
					{
						memcpy(opcode_f.intensechargingstatus," CHARGING",9);
					}
					else
					{
						memcpy(opcode_f.intensechargingstatus,"UNCHARGED",9);
					}
					memcpy(opcode_f.Missionstartdate,ptr_opcode_M->MissionStartDate,12);
					memcpy(opcode_f.Toolid,ptr_opcode_M->Toolid,15);
					memcpy(opcode_f.SubToolid,ptr_opcode_M->SubToolid,15);

					memset(opcode_f.Reserved1,0x20,30);
					memset(opcode_f.Reserved2,0x20,30);
					UART1_Tx(STX);
					UART1_Tx(ASC_f);
					structsum=0;
					structsum=struct_send(opcode_f.sequencenumber,sizeof(opcode_f),200,0);
					ResCheckSum= STX+ASC_f+structsum+ETX;

				break;
				case ASC_E: // Type:Write  INTENSE state information setting[SW(admin)→INTENSE]
					printf("CMD_E\r\n");
					UART1_Tx(STX);
					UART1_Tx(ASC_e);
					UART1_Tx(ACK);

					Protocol_Erase_Page(ASC_E);  // check 할 것
					Protocol_Erase_Page(ASC_E);

					m_Cnt = 0;
					for(int x = (StxFindCnt+2); x < (CrFindCnt-4); x++)
					{
						WriteEFrame._01_byte[m_Cnt] = CalcReadFrame._R_01_byte[x];
						CalcReadFrame._R_01_byte[x] = 0;
						printf("[%d]:%02x / %c\r\n",m_Cnt,WriteEFrame._01_byte[m_Cnt], WriteEFrame._01_byte[m_Cnt]);
						m_Cnt++;
					}

					memcpy(&WriteEFrame._01_byte[16],FIRMWARE_VER,5);
					memcpy(&WriteEFrame._01_byte[21],"  300",5);
					m_Cnt = 0;
					Protocol_Flash_Memory_Store(OP_CODE,(uint32_t*)WriteEFrame._04_byte);
					ResCheckSum= STX+ASC_e+ACK+ETX;
				break;
				case ASC_G: // Request "Go SleepMode"
					printf("CMD_G\r\n");

					UART1_Tx(STX);
					UART1_Tx(ASC_g);
					ResCheckSum= STX+ASC_g+ETX;
					UART1_Tx(ETX);
					UART1_Tx((ResCheckSum>>4)+0x30);  //crc ResCheckSum
					UART1_Tx((ResCheckSum&0x0f)+0x30);  //crc
					UART1_Tx(CR);

					//LL_mDelay(500);
					//LL_GPIO_ResetOutputPin(GPIOA, nRF52811_LDO_EN_Pin); // BLE Ldo Pin Reset(Power Off)
					//LL_mDelay(100);
					//HAL_GPIO_WritePin(LDO_EN4_GPIO_Port, LDO_EN4_Pin, GPIO_PIN_RESET);
					//printf("MAKE POWER DOWN MODE FUNCTION ===================================\r\n");
					HAL_Delay(200);
					Go_StandbyMode();  // 이거 살려야됨

				break;
				case ASC_K: // Request MISSION name Change "READ OK"
					printf("CMD_K\r\n");

					MissionNameChangeREADOK();
					//LL_mDelay(100);
					UART1_Tx(STX);
					UART1_Tx(ASC_k);
					ResCheckSum= STX+ASC_k+ETX;

				break;
				case ASC_Q: // Request MISSION name Change "READY"
					printf("CMD_Q\r\n");

					MissionCompleteStausChangeReady();
					//LL_mDelay(100);
					UART1_Tx(STX);
					UART1_Tx(ASC_q);
					ResCheckSum= STX+ASC_q+ETX;

				break;
				case ASC_T: // Type: Read INTENSE wafer temperature sensor calibration data request[SW(admin)→INTENSE] -> A 대체
					printf("CMD_T\r\n");

					UART1_Tx(STX);
					UART1_Tx(ASC_t);
					ResCheckSum= STX+ASC_t+ETX;

					if(Test_Ep == 0)
					{
						Test_Ep = 1;
					}
					else
					{
						Test_Ep = 0;
					}

				break;
				case ASC_S: // Type: Write INTENSE Temp Sensor calibration value setting[SW(admin)→INTENSE] -> H 대체
					printf("CMD_S\r\n");

					UART1_Tx(STX);
					UART1_Tx(ASC_s);
					UART1_Tx(ACK);
					//Protocol_Flash_Memory_Read(OP_CODE);
					ResCheckSum= STX+ASC_s+ACK+ETX;

				break;
				case ASC_O: // Type:Write  INTENSE Add usage time setting
					// IFA DoorTriger Timeout
					//-DoorTrigerTimeOut=CalcReadFrame._R_01_byte[(EnqFindCnt+2)];
					//--Protocol_Erase_Page(OP_CODE);

					// "00" IFA TimeOut(00~99)
					printf("CMD_O\r\n");

					memset(bintostrhex,0,sizeof(bintostrhex));
					memcpy(bintostrhex,&CalcReadFrame._R_01_byte[(EnqFindCnt+2)], 4 );
					flag->DoorTrigerTimeOut=atoi(bintostrhex);
					flag->DoorTrigerTimeOut=flag->DoorTrigerTimeOut*10;
					printf("Door:%ld\r\n",flag->DoorTrigerTimeOut);
					UART1_Tx(STX);
					UART1_Tx(ASC_o);
					UART1_Tx(ACK);
					ResCheckSum= STX+ASC_o+ACK+ETX;

				break;
				case ASC_P:   // Eco
					printf("CMD_P\r\n");


					UART1_Tx(STX);
					UART1_Tx(ASC_p);

					//if(!HAL_GPIO_ReadPin(BAT_CHK_GPIO_Port,BAT_CHK_Pin))//if(!(LL_GPIO_ReadInputPort(GPIOA)&0x00000020)) // if charge....Wait...(LOW:Charge, HIGH:No Charge)
					if(HAL_GPIO_ReadPin(BAT_DIR_GPIO_Port,BAT_DIR_Pin))
					{
						opcode_p.intensechargestatus='C';
					}
					else
						opcode_p.intensechargestatus='U';
					ptr_opcode_E->intensefirmwareversion[0] = '1';
					ptr_opcode_E->intensefirmwareversion[1] = '.';
					ptr_opcode_E->intensefirmwareversion[2] = '1';
					ptr_opcode_E->intensefirmwareversion[3] = '.';
					ptr_opcode_E->intensefirmwareversion[4] = '1';

					memcpy(opcode_p.intensefirmwareversion,ptr_opcode_E->intensefirmwareversion,5);
					memcpy(opcode_p.intensewafersize,ptr_opcode_E->intensewafersize,5);
					memcpy(opcode_p.permittedusagetime,ptr_opcode_E->permittedusagetime,11);
					memcpy(opcode_p.hoursofused,ptr_opcode_E->hoursofused,11);
					memcpy(opcode_p.serialnumber,ptr_opcode_E->serialnumber,30);
					// bat voltage...
					adc_channel_read_start(&sbatteryValue,&smcu_temp_value);
					//printf("ADC:%x\r\n",sbatteryValue);
					memset(bintostrhex,0,sizeof(bintostrhex));
					sprintf(bintostrhex,"%04X",sbatteryValue);
					memcpy(opcode_p.waferbattervoltage,bintostrhex,4);

					structsum=struct_send(&opcode_p.intensechargestatus,sizeof(opcode_p),50,0);
					ResCheckSum= STX+ASC_p+structsum+ETX;

				break;
				case ASC_B:
					printf("CMD_B\r\n");

					UART1_Tx(STX);
					UART1_Tx(ASC_b);
					UART1_Tx(ACK);
					//--Protocol_Flash_Memory_Read(OP_CODE);
					//--if(ADMIN_KEY_ARRAY)
					UART1_Tx(ETX);
					ResCheckSum= STX+ASC_b+ACK+ETX;
					UART1_Tx((ResCheckSum>>4)+0x30);  //crc ResCheckSum
					UART1_Tx((ResCheckSum&0x0f)+0x30);  //crc
					UART1_Tx(CR);
					if(1)
					{
						//ACK_Answer_Check(OP_CODE);
						HAL_Delay(200);
						boottrigerSet();
						HAL_NVIC_SystemReset();
					}
					else
					{
						//NAK_Answer_Check(OP_CODE);
					}
					ResCheckSum= STX+ASC_b+ACK+ETX;

				break;
				case ASC_Z:  // Only Reset...
					printf("CMD_Z\r\n");

					MissionCompleteStausChangeReady();
					UART1_Tx(STX);
					UART1_Tx(ASC_z);
					UART1_Tx(ACK);
					UART1_Tx(ETX);
					ResCheckSum= STX+ASC_z+ACK+ETX;
					UART1_Tx((ResCheckSum>>4)+0x30);  //crc ResCheckSum
					UART1_Tx((ResCheckSum&0x0f)+0x30);  //crc
					UART1_Tx(CR);
					printf("\r\n");
					//LL_mDelay(300);
					OP_CODE=0;
				  	// BLE Ldo Pin Reset(Power Off)
					//LL_GPIO_ResetOutputPin(GPIOA, nRF52811_LDO_EN_Pin);
					HAL_Delay(200);
					//HAL_GPIO_WritePin(LDO_EN4_GPIO_Port, LDO_EN4_Pin, GPIO_PIN_RESET);
					HAL_NVIC_SystemReset();

				break;
				default:
				break;
			}
			if( memcmp( &CalcReadFrame._R_01_byte[(EnqFindCnt+2)],"ALL",3) )  // not same  '0'   else != '0'
			{
				UART1_Tx(ETX);
				UART1_Tx((ResCheckSum>>4)+0x30);  //crc ResCheckSum
				UART1_Tx((ResCheckSum&0x0f)+0x30);  //crc
				UART1_Tx(CR);
				//printf("TESTTESTTEST\r\n");
			}

			ReadCnt 					= 0;
			calcCheckSum_1 				= 0;
			calcCheckSum_2 				= 0;
			OP_CODE 					= 0;
			for(int x = 0; x < ReadCntBuffer; x++)
			{
				CalcReadFrame._R_01_byte[x] = 0x20;
				//ReadFrame._R_01_byte[x]		= 0;
			}
		}
		else
		{
			OP_CODE = CalcReadFrame._R_01_byte[OPCODE_ARRAY];
			NAK_Answer_Check(OP_CODE);

			ReadCnt 					= 0;
			calcCheckSum_1 				= 0;
			calcCheckSum_2 				= 0;
			OP_CODE 					= 0;
			for(int x = 0; x < ReadCntBuffer; x++)
			{
				CalcReadFrame._R_01_byte[x] = 0;
				ReadFrame._01_byte[x]		= 0;
			}
		}
		ChecksumSuccessFlag=0;
		receiveComplete = 0;
		StxFindCnt=9999;
		EnqFindCnt=9999;
		CrFindCnt=9999;
	}
}



void UART1_Tx(char send_data)
{
	uint8_t ch=send_data;
	HAL_UART_Transmit(&huart2, (uint8_t *)&ch, 1, 1000);
	//asc_r_update_cnt++;
}

uint8_t struct_send(uint8_t *ptr_array,uint32_t lenth,uint32_t senddelay,uint8_t initialsum)
{
	uint8_t s_sum=initialsum;
	uint32_t i=0;
	//--HAL_Delay(senddelay); //  senddelay >> 100
	//printf("LENG:%d\r\n",lenth);
	for(i=0; i<lenth; i++)
	{
		UART1_Tx((uint8_t)ptr_array[i]);
		//printf("struct_send: %d\r\n",(uint8_t)ptr_array[i]);
		s_sum+=ptr_array[i];

		if((asc_r_update_cnt%100)==0 && (asc_r_update_cnt>0))  //220 >>230
		{
#if 0
			 // Olny r All Cmd
			//-if( !memcmp( &CalcReadFrame._R_01_byte[(EnqFindCnt+2)],"ALL",3) )
			if(StreammingUpdateFlag==1)
			{
				for(AckMsgFlag=0,AckTimeOut=0; AckMsgFlag!=1 ; AckTimeOut++)
				{
					if(AckTimeOut>ACKTIMEOUT_MSEC)  // timeout 4s
						return 0;  //breka >>>return 0
					// NAK occurs when receiving less than BLE 230 byte..
					if( NckMsgFlag==1 )
					{
						NckTimeOutCnt++;
						return 0;
					}
					HAL_Delay(1);
				};
				// Ack Received
				NckTimeOutCnt=0;
				HAL_Delay(10);
			}
			else
				HAL_Delay(senddelay); // senddelay >> 100
#else
		//		SEGGER_RTT_printf(0,"LENGTEST:%d\r\n",i);
		//HAL_Delay(100);//HAL_Delay(senddelay); // senddelay >> 100
		}
#endif
	}
    return s_sum;
}

void MissionNameChangeREADOK(void)
{
	uint32_t rw_read_addr 	= M_FLASH_AREA;//FLASH_USER_PROTOCOL_M_ADDR_START;
	_opcode_M *ptr_opcode_M_Buf;

	memset(s_Buf_Frame._01_byte,0,sizeof(s_Buf_Frame._01_byte));

	// Read M Flash
	for(int x = 0; x < 256; x++) //128 >>> 256
	{
		s_Buf_Frame._01_byte[x]	=	*(__IO uint8_t*) ( rw_read_addr + (1 * x));
		//s_Buf_Frame._01_byte[x]	=(uint8_t) *(ptr_add + (1 * x));
	}

	ptr_opcode_M_Buf=(_opcode_M *)s_Buf_Frame._01_byte;

	// WRITE READ OK...
    memcpy(ptr_opcode_M_Buf->mission_complete_status,"READ OK",7);
    //memcpy(&s_Buf_Frame._01_byte[30],"READ OK ",8);

    Protocol_Erase_Page(ASC_M);
    //HAL_Delay(50);
    Protocol_Flash_Memory_Store(ASC_M,s_Buf_Frame._04_byte);
#if 0
    // FLASH WRITE...
    HAL_FLASH_Unlock();
	{
		//--result = HAL_FLASH_Program(FLASH_TYPEPROGRAM_BURST, rw_read_addr,(unsigned int )(__IO uint8_t *)(s_Buf_Frame._01_byte));	// flash memory write	128 byte 씩 작성됨

		HAL_FLASH_Program(FLASH_TYPEPROGRAM_BURST, rw_read_addr,(unsigned int )(__IO uint32_t *)(s_Buf_Frame._04_byte));
		HAL_Delay(50);
		HAL_FLASH_Lock();

		HAL_Delay(50);
		HAL_FLASH_Unlock();
		rw_read_addr = FLASH_USER_PROTOCOL_M_ADDR_START + (0x80) ;
		HAL_FLASH_Program(FLASH_TYPEPROGRAM_BURST, rw_read_addr,(unsigned int )(__IO uint32_t *)(s_Buf_Frame._04_byte + 0x20)); // 4bytx32(0x20)=128
	}
	HAL_Delay(50);
	HAL_FLASH_Lock();
#endif
}


void MissionCompleteStausChangeReady(void)
{
	uint32_t rw_read_addr 	= M_FLASH_AREA;//FLASH_USER_PROTOCOL_M_ADDR_START;
	_opcode_M *ptr_opcode_M_Buf;

	memset(s_Buf_Frame._01_byte,0,sizeof(s_Buf_Frame._01_byte));

	// Read M Flash
	for(int x = 0; x < 256; x++) //128 >>> 256
	{
		s_Buf_Frame._01_byte[x]	=	*(__IO uint8_t*) ( rw_read_addr + (1 * x));
	}

	ptr_opcode_M_Buf=(_opcode_M *)s_Buf_Frame._01_byte;

	// WRITE READ OK...
    memcpy(ptr_opcode_M_Buf->mission_complete_status,"  READY",7);

    Protocol_Erase_Page(ASC_M);
    //HAL_Delay(50);
    Protocol_Flash_Memory_Store(ASC_M,s_Buf_Frame._04_byte);
#if 0
    // FLASH WRITE...
    HAL_FLASH_Unlock();
	{
		//--result = HAL_FLASH_Program(FLASH_TYPEPROGRAM_BURST, rw_read_addr,(unsigned int )(__IO uint8_t *)(s_Buf_Frame._01_byte));	// flash memory write	128 byte 씩 작성됨

		result = HAL_FLASH_Program(FLASH_TYPEPROGRAM_BURST, rw_read_addr,(unsigned int )(__IO uint32_t *)(s_Buf_Frame._04_byte));
		HAL_Delay(50);
		HAL_FLASH_Lock();

		HAL_Delay(50);
		HAL_FLASH_Unlock();
		rw_read_addr = FLASH_USER_PROTOCOL_M_ADDR_START + (0x80) ;
		result = HAL_FLASH_Program(FLASH_TYPEPROGRAM_BURST, rw_read_addr,(unsigned int )(__IO uint32_t *)(s_Buf_Frame._04_byte + 0x20)); // 4bytx32(0x20)=128

	}
	HAL_Delay(50);
	HAL_FLASH_Lock();
#endif
}

void boottrigerSet(void)
{
	 HAL_FLASH_Unlock();
	 //HAL_StatusTypeDef result;
	 unsigned int  write_addr = ADDR_FlASH_PAGE_A_015;

	 //memset(&BootTriger,0,sizeof(Temperature_Frame));
	 BootTriger._04_byte[0]=0;

	 //MCU 2 BANK 1,024 kByte = abot 1 M Byte / flash memory 1 page = 8 kByte / Temp 1 data = 16 bit = 2 byte * 64 = 128 byte(1set = 64 data)
	 //MCU 1 BANK -> Tamp 1 set data about 8,160 set save possible
	 HAL_FLASH_Program(FLASH_TYPEPROGRAM_BURST, write_addr,(unsigned int )(__IO uint32_t *)(BootTriger._04_byte)); // flash memory write
	 HAL_Delay(10);

	 /*
	 if (result != HAL_OK)
	 {
	  printf("Flash Write Error 0x%02x\n", result);
	 }
	 else
	 {
	  printf("Flash Write at  0x%08xOK \n", write_addr);
	 }
	 */
	 //printf("Save Data Set %02d \n", data_count + 1);
	 //HAL_Delay(50);  // Bluetooth hardfault prevention
	 HAL_FLASH_Lock();
}

void Go_StandbyMode(void)
{
	//  STANDBYMode
	//MX_PowerMode_GPIO_Init();

	HAL_SuspendTick();
	//__HAL_RCC_PWR_CLK_ENABLE(); // Enable power peripheral

	__HAL_PWR_CLEAR_FLAG(PWR_WAKEUP_FLAG2);
	//HAL_GPIO_WritePin(LDO_EN4_GPIO_Port,LDO_EN4_Pin,GPIO_PIN_RESET);
	HAL_UART_MspDeInit(&huart1);
	//HAL_GPIO_WritePin(LDO_EN0_GPIO_Port,LDO_EN0_Pin,GPIO_PIN_RESET);
	//HAL_GPIO_WritePin(LDO_EN1_GPIO_Port,LDO_EN1_Pin,GPIO_PIN_RESET);
	//HAL_GPIO_WritePin(LDO_EN2_GPIO_Port,LDO_EN2_Pin,GPIO_PIN_RESET);
	//HAL_GPIO_WritePin(LDO_EN3_GPIO_Port,LDO_EN3_Pin,GPIO_PIN_RESET);
	HAL_IWDG_Refresh(&hiwdg);
	//i2c_io_deinit();
	//HAL_GPIO_WritePin(MCU_EN_GPIO_Port,MCU_EN_Pin,GPIO_PIN_RESET);
	HAL_PWR_EnterSTANDBYMode();
}


void NAK_Answer_Check(uint8_t op_code)
{
	ResCheckSum=0;
	UART1_Tx(STX);
	ResCheckSum+=STX;
	switch(op_code)
	{
		case ASC_M:
			UART1_Tx(ASC_m);
			ResCheckSum+=ASC_m;
			break;
		case ASC_R:
			UART1_Tx(ASC_r);
			ResCheckSum+=ASC_r;
			break;
		case ASC_F:
			UART1_Tx(ASC_f);
			ResCheckSum+=ASC_f;
			break;
		case ASC_E:
			UART1_Tx(ASC_e);
			ResCheckSum+=ASC_e;
			break;
		case ASC_T:
			UART1_Tx(ASC_t);
			ResCheckSum+=ASC_t;
			break;
		case ASC_S:
			UART1_Tx(ASC_s);
			ResCheckSum+=ASC_s;
			break;
		case ASC_O:
			UART1_Tx(ASC_o);
			ResCheckSum+=ASC_o;
			break;
		case ASC_B:
			UART1_Tx(ASC_b);
			ResCheckSum+=ASC_b;
			break;
		default:
			UART1_Tx(ASC_j);
			ResCheckSum+=ASC_j;
			break;
	}
	UART1_Tx(NAK);
	ResCheckSum+=NAK;

	UART1_Tx(ETX);
	ResCheckSum+=ETX;

	UART1_Tx((ResCheckSum>>4)+0x30);  //crc ResCheckSum
	UART1_Tx((ResCheckSum&0x0f)+0x30);  //crc
	UART1_Tx(CR);
}

