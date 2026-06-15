/*
 * control.c
 *
 *  Created on: Mar 6, 2026
 *      Author: kimst
 */


#include "control.h"
#include "uart.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "tim6.h"
#include "ads124s06.h"
#include "mcu_flash.h"
//uint8_t sensor_flag[SENSOR_TOTAL_CNT+1]={0,};
float simulation_trigger_value=25.0;

typedef union
{
	uint8_t byte_1[128];
	int32_t byte_4[32];
}_Temp_save_buf;
_Temp_save_buf temp_save_buf;


void flash_init(void)
{
	_opcode_M *int_flash;
	_opcode_M *wr_flash;
	_opcode_E *wr_flash_2;
	Buf_Frame tmp_buf;
	int_flash=(_opcode_M*)M_FLASH_AREA;

    if(int_flash->sequencenumber[0]==0xff&&int_flash->sequencenumber[1]==0xff&&int_flash->sequencenumber[2]==0xff&&int_flash->sequencenumber[3]==0xff&& \
	   int_flash->sequencenumber[4]==0xff&&int_flash->sequencenumber[5]==0xff&&int_flash->sequencenumber[6]==0xff&&int_flash->sequencenumber[7]==0xff)

	{
		printf("FACTROY SETTING START\r\n");
		wr_flash=(_opcode_M *)tmp_buf._01_byte;
		memcpy(wr_flash->sequencenumber,"00000000",8);
		memcpy(wr_flash->serialnumber,"               INTENSE-FACTORY",30);
		memcpy(wr_flash->mission_name,"                         INTENSE-FACTORY",40);
		memcpy(wr_flash->mission_complete_status,"  READY",7);
		memcpy(wr_flash->sample_period,"1000",4);
		memcpy(wr_flash->acquisition_time,"00000",5);
		memcpy(wr_flash->trigger_temperature,"   0",4);
		memcpy(wr_flash->delaytime,"     0",6);
		memcpy(wr_flash->TriggerMode,"       Immediately",18);
		memcpy(wr_flash->TransientTemp,"  1.5",5);
		memcpy(wr_flash->MissionStartDate,"MANUFACTURE-",12);
		memcpy(wr_flash->Toolid,"            FST",15);
		memcpy(wr_flash->SubToolid,"    MANUFACTURE",15);
		memcpy(wr_flash->Reserved1,"                              ",30);
		memcpy(wr_flash->Reserved2,"                              ",30);

		Protocol_Erase_Page(ASC_M);
		Protocol_Flash_Memory_Store(ASC_M,tmp_buf._04_byte);

		wr_flash_2=(_opcode_E *)tmp_buf._01_byte;
		memcpy(wr_flash_2->admin_password,"PA0123456789ABCD",16);
		memcpy(wr_flash_2->intensefirmwareversion,FIRMWARE_VER,5);
		memcpy(wr_flash_2->intensewafersize,"  300",5);
		memcpy(wr_flash_2->permittedusagetime,"      14400",11);
		memcpy(wr_flash_2->hoursofused,"          0",11);
		memcpy(wr_flash_2->serialnumber,"               INTENSE-FACTORY",30);
		memcpy(wr_flash_2->reserved,"                  ",18);

		Protocol_Erase_Page(ASC_E);
		Protocol_Flash_Memory_Store(ASC_E,tmp_buf._04_byte);
	}
}


void cal_data_flash_write(CAL_DATA *cal)
{
	Buf_Frame buf;
	unsigned int  write_addr,offset=0,i=0;
	memcpy(buf._01_byte,(Buf_Frame*)cal,sizeof(CAL_DATA));
	HAL_FLASH_Unlock();
	write_addr = ADDR_FlASH_PAGE_A_127;
	offset=0;
	for(i=0;i<16;i++)
	{
		//HAL_FLASH_Program(FLASH_TYPEPROGRAM_BURST, write_addr,(uint32_t)(__IO uint32_t *)(buf._04_byte+offset));//128Byte
		//err_flag=HAL_FLASH_Program(FLASH_TYPEPROGRAM_BURST, write_addr,(unsigned int )(__IO uint32_t *)(buf._04_byte+offset));//128Byte
		HAL_FLASH_Program(FLASH_TYPEPROGRAM_BURST, write_addr,(uint32_t)(buf._04_byte+offset));
		write_addr = write_addr + (0x80);
		offset = offset+0x20;
	}
	HAL_FLASH_Lock();
}

void cal_data_restore(uint8_t clear,CAL_DATA *cal_flag)
{
	Protocol_Erase_Page(ASC_CAL);
	if(clear<CLA_COMPLETE_SEQ)
	{
		cal_data_flash_write(cal_flag);
	}
}

void calibration_proc(control_flag* flag)
{
	uint8_t i=0;
	float tmp_value=0.0,tmp_res=0.0;
	static float resistor_buf[SENSOR_TOTAL_CNT]={0.0,};
	float cal_tmp_value=0.0;
	uint32_t tmp_flag=0x00000000;
	if(get_calibration_cnt()>=1000)
	{
		clear_calibration_cnt();
		if(flag->stable_time>flag->stable_time_proc)
		{
			flag->stable_time_proc++;
			printf("STA[%d]	",flag->stable_time_proc);
			for(i=0;i<SENSOR_TOTAL_CNT;i++)
			{
			#ifdef DIRECT_3P_CAL
				if(flag->receive_calibration==6||flag->receive_calibration==8||flag->receive_calibration==9)
				{
					//tmp_value=sel_readsensor_temp_resistor(i,&tmp_res,flag,2);
					tmp_value=sel_readsensor_temp_resistor(i,&tmp_res,flag,1);
				}
				else
			#endif
				tmp_value=sel_readsensor_temp_resistor(i,&tmp_res,flag,1);
				printf("%7.2f	",tmp_res);
				resistor_buf[i]=0.0;
			}
			printf("\r\n");
		}
		else
		{
			if(flag->acquisition_time>flag->acquisition_time_proc)
			{
				flag->acquisition_time_proc++;
				printf("AQU[%d]	",flag->acquisition_time_proc);
				for(i=0;i<SENSOR_TOTAL_CNT;i++)
				{
				#ifdef DIRECT_3P_CAL
					if(flag->receive_calibration==6||flag->receive_calibration==8||flag->receive_calibration==9)
					{
						//tmp_value=sel_readsensor_temp_resistor(i,&tmp_res,flag,2);
						tmp_value=sel_readsensor_temp_resistor(i,&tmp_res,flag,1);
					}
					else
				#endif
					tmp_value=sel_readsensor_temp_resistor(i,&tmp_res,flag,1);
					printf("%7.2f	",tmp_res);
					resistor_buf[i]+=tmp_res;
					//printf("Total:%6.2f\r\n",resistor_buf[i]);
				}
				printf("\r\n");
			}
			else
			{
				if(flag->receive_calibration<6) //6CLA_COMPLETE_SEQ
				{
					cal_tmp_value=(float)flag->acquisition_time_proc;
					//printf("Divide:%6.2f	",cal_tmp_value);
					for(i=0;i<SENSOR_TOTAL_CNT;i++)
					{
						if(flag->receive_calibration==1)
						{
							resistor_buf[i]=resistor_buf[i]/cal_tmp_value;
							//printf("%6.2f	",resistor_buf[i]);
							flag->cal_data.ref_cal_resist[i][flag->receive_calibration-1]=(uint32_t)((resistor_buf[i]+CAL_ADD_OFFSET_NEW)*CAL_MUL_DIV_VAL_NEW);
						}
						if(flag->receive_calibration==2)
						{
							resistor_buf[i]=resistor_buf[i]/cal_tmp_value;
							//printf("%6.2f	",resistor_buf[i]);
							flag->cal_data.ref_cal_resist[i][flag->receive_calibration-1]=(uint32_t)((resistor_buf[i]+CAL_ADD_OFFSET_NEW)*CAL_MUL_DIV_VAL_NEW);
						}
						//flag->cal_data.pre_cal_resist[i][flag->receive_calibration-1]=(uint32_t)((resistor_buf[i])*CAL_MUL_DIV_VAL);
					#ifdef DIRECT_3P_CAL
						if(flag->receive_calibration==6)
						{
							resistor_buf[i]=resistor_buf[i]/cal_tmp_value;
							//printf("%6.2f	",resistor_buf[i]);
							flag->cal_data.pre_cal_resist[i][0]=(uint32_t)((resistor_buf[i]+CAL_ADD_OFFSET_NEW)*CAL_MUL_DIV_VAL_NEW);
						}
					#endif
					}
					#ifdef DIRECT_3P_CAL
					if(flag->receive_calibration==6)
					{
						flag->receive_calibration=3;
					}
					#endif
					//printf("CALFLAG1=%08x\r\n",flag->cal_data.calibration_flag);
					tmp_flag=tmp_flag|(1<<(flag->receive_calibration-1));
					flag->cal_data.calibration_flag=flag->cal_data.calibration_flag&~tmp_flag;
					//printf("CALFLAG2=%08x\r\n",flag->cal_data.calibration_flag);
				}
			#ifdef DIRECT_3P_CAL
				if(flag->receive_calibration==8)
				{
					cal_tmp_value=(float)flag->acquisition_time_proc;
					for(i=0;i<SENSOR_TOTAL_CNT;i++)
					{
						resistor_buf[i]=resistor_buf[i]/cal_tmp_value;
						//printf("%6.2f	",resistor_buf[i]);
						flag->cal_data.pre_cal_resist[i][1]=(uint32_t)((resistor_buf[i]+CAL_ADD_OFFSET_NEW)*CAL_MUL_DIV_VAL_NEW);
					}
					flag->receive_calibration=4;
					tmp_flag=tmp_flag|(1<<(flag->receive_calibration-1));
					flag->cal_data.calibration_flag=flag->cal_data.calibration_flag&~tmp_flag;
				}
				if(flag->receive_calibration==9)
				{
					cal_tmp_value=(float)flag->acquisition_time_proc;
					for(i=0;i<SENSOR_TOTAL_CNT;i++)
					{
						resistor_buf[i]=resistor_buf[i]/cal_tmp_value;
						//printf("%6.2f	",resistor_buf[i]);
						flag->cal_data.pre_cal_resist[i][2]=(uint32_t)((resistor_buf[i]+CAL_ADD_OFFSET_NEW)*CAL_MUL_DIV_VAL_NEW);
					}
					flag->receive_calibration=5;
					tmp_flag=tmp_flag|(1<<(flag->receive_calibration-1));
					flag->cal_data.calibration_flag=flag->cal_data.calibration_flag&~tmp_flag;
				}
			#endif
				cal_data_restore(flag->receive_calibration,&flag->cal_data);
				flag->receive_calibration=CALIBRATION_STOP;
				printf("Acquisition Complete\r\n");
				read_cal_data(flag);
				if(flag->cal_data.calibration_flag==CAL_COMPLETE_FLAG_INIT)
				{
					rtd_calibration_func_init(flag);
					cal_data_restore(flag->receive_calibration,&flag->cal_data);
					read_cal_data(flag);
				}
				if(flag->cal_data.calibration_flag==CAL_COMPLETE_FLAG)
				{
#ifdef DIRECT_3P_CAL
					//rtd_calibration_func_test(flag);
					rtd_calibration_func(flag);

#else
					//rtd_calibration_func(flag);
					rtd_calibration_func_200C_test(flag);
#endif


					cal_data_restore(flag->receive_calibration,&flag->cal_data);
					read_cal_data(flag);
				}
			}
		}
	}
}

void rtd_calibration_func_init(control_flag* flag)
{
	int i=0;
	float temp=0.0;
	float val_1=0.0,val_2=0.0;
	for(i=0;i<SENSOR_TOTAL_CNT;i++)
	{
		flag->cal_data.ref_cal_resist[i][2]=flag->cal_data.ref_cal_resist[i][1]-flag->cal_data.ref_cal_resist[i][0];
		temp=flag->cal_data.ref_resist[1];//*CAL_MUL_DIV_VAL_NEW;
		val_1=(float)(temp/flag->cal_data.ref_cal_resist[i][2]);
		val_2=(float)(flag->cal_data.cal_resist[i][0]*val_1);
		flag->cal_data.cal_resist[i][1]=(uint32_t)val_2;
		flag->cal_data.cal_resist[i][2]=0;
	}
	printf("RTD CALIBRATION INIT\r\n");
}

void rtd_calibration_func_test(control_flag* flag)
{
	double a[3][3],b[3];
	double ref_res_res[3];
	int i=0,j=0;
	float ref_res[3]={0.0,},pre_res[3]={0.0,};
	for(j=0;j<SENSOR_TOTAL_CNT;j++)
	{
		pre_res[0]=(float)(flag->cal_data.pre_cal_resist[j][0]/CAL_MUL_DIV_VAL_NEW);
		pre_res[1]=(float)(flag->cal_data.pre_cal_resist[j][1]/CAL_MUL_DIV_VAL_NEW);
		pre_res[2]=(float)(flag->cal_data.pre_cal_resist[j][2]/CAL_MUL_DIV_VAL_NEW);
		ref_res[0]=(float)(flag->cal_data.ref_resist[2]/CAL_MUL_DIV_VAL_NEW);
		ref_res[1]=(float)(flag->cal_data.ref_resist[3]/CAL_MUL_DIV_VAL_NEW);
		ref_res[2]=(float)(flag->cal_data.ref_resist[4]/CAL_MUL_DIV_VAL_NEW);

		for (i = 0; i < 3; i++)
		{
			a[i][0] = pre_res[i]*pre_res[i];//pow(pre_res[i], 2);
			a[i][1] = pre_res[i];
			a[i][2] = 1.0;
			b[i] = ref_res[i];
		}
		solve_system_3x3(a,b,ref_res_res);
		printf("1[%.10e][%f][%f]\r\n",ref_res_res[0],ref_res_res[1],ref_res_res[2]);//[%.10e]
		if(ref_res_res[0]<0)
		{
			flag->cal_data.cal_coeff[j][0]=(ref_res_res[0]-CAL_ADD_OFFSET_UP)*CAL_MUL_DIV_VAL_UP;
		}
		else
		{
			flag->cal_data.cal_coeff[j][0]=(ref_res_res[0]+CAL_ADD_OFFSET_UP)*CAL_MUL_DIV_VAL_UP;
		}
		if(ref_res_res[1]<0)
		{
			flag->cal_data.cal_coeff[j][1]=(ref_res_res[1]-CAL_ADD_OFFSET_UP)*CAL_MUL_DIV_VAL;
		}
		else
		{
			flag->cal_data.cal_coeff[j][1]=(ref_res_res[1]+CAL_ADD_OFFSET_UP)*CAL_MUL_DIV_VAL;
		}
		if(ref_res_res[2]<0)
		{
			flag->cal_data.cal_coeff[j][2]=(ref_res_res[2]-CAL_ADD_OFFSET_UP)*CAL_MUL_DIV_VAL;
		}
		else
		{
			flag->cal_data.cal_coeff[j][2]=(ref_res_res[2]+CAL_ADD_OFFSET)*CAL_MUL_DIV_VAL;
		}
		printf("2[%ld][%ld][%ld]\r\n",flag->cal_data.cal_coeff[j][0],flag->cal_data.cal_coeff[j][1],flag->cal_data.cal_coeff[j][2]);
		printf("3[%lx][%lx][%lx]\r\n",flag->cal_data.cal_coeff[j][0],flag->cal_data.cal_coeff[j][1],flag->cal_data.cal_coeff[j][2]);
	}
	printf("RTD CALIBRATION Complete\r\n");

}

void rtd_calibration_func(control_flag* flag)
{
	double a[3][3],b[3];
	double ref_res_res[3];
	int i=0,j=0;
	float ref_res[3]={0.0,},pre_res[3]={0.0,};
	float meas_tmp=0.0,std_tmp=0.0,scale_factor=0.0,scale_factor_1=0.0,scale_factor_2=0.0;
	//float tmp_200=1758.56,cal_tmp_200=0.0,tmp_410=2505.33,cal_tmp_410=0.0;
	//float tmp_200=13850.55,cal_tmp_200=0.0,tmp_410=17585.60,cal_tmp_410=0.0;
	float tmp_200=1758.56*8,cal_tmp_200=0.0,tmp_410=2505.33*8,cal_tmp_410=0.0;
	for(j=0;j<SENSOR_TOTAL_CNT;j++)
	{
		pre_res[0]=(float)(flag->cal_data.pre_cal_resist[j][0]/CAL_MUL_DIV_VAL_NEW);
		pre_res[1]=(float)(flag->cal_data.pre_cal_resist[j][1]/CAL_MUL_DIV_VAL_NEW);
		pre_res[2]=(float)(flag->cal_data.pre_cal_resist[j][2]/CAL_MUL_DIV_VAL_NEW);
		ref_res[0]=(float)(flag->cal_data.ref_resist[2]/CAL_MUL_DIV_VAL_NEW);
		ref_res[1]=(float)(flag->cal_data.ref_resist[3]/CAL_MUL_DIV_VAL_NEW);
		ref_res[2]=(float)(flag->cal_data.ref_resist[4]/CAL_MUL_DIV_VAL_NEW);

		std_tmp=ref_res[2];//50
		meas_tmp=pre_res[2];
		scale_factor=meas_tmp/std_tmp;
		scale_factor_1=scale_factor*0.9993;//0.9985;//0.999;//
		scale_factor_2=scale_factor*0.9983;//0.9965;//0.998;//
		cal_tmp_200=tmp_200*scale_factor_1;
		cal_tmp_410=tmp_410*scale_factor_2;

		printf("[%d],std:%f,meas:%f,factor:(%f)(%f),(%f),cal_200:%f,cal_410:%f\r\n",\
				j,std_tmp,meas_tmp,scale_factor,scale_factor_1,scale_factor_1,cal_tmp_200,cal_tmp_410);
		ref_res[1]=tmp_200;
		ref_res[2]=tmp_410;
		pre_res[1]=cal_tmp_200;
		pre_res[2]=cal_tmp_410;

		for (i = 0; i < 3; i++)
		{
			a[i][0] = pre_res[i]*pre_res[i];//pow(pre_res[i], 2);
			a[i][1] = pre_res[i];
			a[i][2] = 1.0;
			b[i] = ref_res[i];
		}
		solve_system_3x3(a,b,ref_res_res);
		b[0]=ref_res_res[0];b[1]=ref_res_res[1];b[2]=ref_res_res[2];
		cal_tmp_200=(-ref_res_res[1] + sqrt(ref_res_res[1]*ref_res_res[1] - 4*ref_res_res[0]*(ref_res_res[2] - tmp_200))) / (2*ref_res_res[0]);
		cal_tmp_410=(-ref_res_res[1] + sqrt(ref_res_res[1]*ref_res_res[1] - 4*ref_res_res[0]*(ref_res_res[2] - tmp_410))) / (2*ref_res_res[0]);
		ref_res[1]=tmp_200;
		ref_res[2]=tmp_410;
		pre_res[1]=cal_tmp_200;
		pre_res[2]=cal_tmp_410;
		printf("[%d],std:%f,meas:%f,cal_200:%f,cal_410:%f\r\n",j,std_tmp,meas_tmp,cal_tmp_200,cal_tmp_410);

		for (i = 0; i < 3; i++) // AI
		{
			a[i][0] = pre_res[i]*pre_res[i];//pow(pre_res[i], 2);
			a[i][1] = pre_res[i];
			a[i][2] = 1.0;
			b[i] = ref_res[i];
		}
		solve_system_3x3(a,b,ref_res_res);
		printf("1[%.10e][%f][%f]\r\n",ref_res_res[0],ref_res_res[1],ref_res_res[2]);//[%.10e]
		if(ref_res_res[0]<0)
		{
			flag->cal_data.cal_coeff[j][0]=(ref_res_res[0]-CAL_ADD_OFFSET_UP)*CAL_MUL_DIV_VAL_UP;
		}
		else
		{
			flag->cal_data.cal_coeff[j][0]=(ref_res_res[0]+CAL_ADD_OFFSET_UP)*CAL_MUL_DIV_VAL_UP;
		}
		if(ref_res_res[1]<0)
		{
			flag->cal_data.cal_coeff[j][1]=(ref_res_res[1]-CAL_ADD_OFFSET_UP)*CAL_MUL_DIV_VAL;
		}
		else
		{
			flag->cal_data.cal_coeff[j][1]=(ref_res_res[1]+CAL_ADD_OFFSET_UP)*CAL_MUL_DIV_VAL;
		}
		if(ref_res_res[2]<0)
		{
			flag->cal_data.cal_coeff[j][2]=(ref_res_res[2]-CAL_ADD_OFFSET_UP)*CAL_MUL_DIV_VAL;
		}
		else
		{
			flag->cal_data.cal_coeff[j][2]=(ref_res_res[2]+CAL_ADD_OFFSET)*CAL_MUL_DIV_VAL;
		}
		printf("2[%ld][%ld][%ld]\r\n",flag->cal_data.cal_coeff[j][0],flag->cal_data.cal_coeff[j][1],flag->cal_data.cal_coeff[j][2]);
		printf("3[%lx][%lx][%lx]\r\n",flag->cal_data.cal_coeff[j][0],flag->cal_data.cal_coeff[j][1],flag->cal_data.cal_coeff[j][2]);
	}
	printf("RTD CALIBRATION Complete\r\n");
}

void rtd_calibration_func_200C_test(control_flag* flag)
{
	double a[3][3],b[3];
	double ref_res_res[3];
	int i=0,j=0;
	float ref_res[3]={0.0,},pre_res[3]={0.0,};
	float meas_tmp=0.0,std_tmp=0.0,scale_factor=0.0,scale_factor_1=0.0,scale_factor_2=0.0;
	//float tmp_100=1385.06*8,cal_tmp_100=0.0,tmp_200=1758.56*8,cal_tmp_200=0.0;
	float tmp_100=13850.55,cal_tmp_100=0.0,tmp_200=17585.60,cal_tmp_200=0.0;
	for(j=0;j<SENSOR_TOTAL_CNT;j++)
	{
		pre_res[0]=(float)(flag->cal_data.pre_cal_resist[j][0]/CAL_MUL_DIV_VAL_NEW);
		pre_res[1]=(float)(flag->cal_data.pre_cal_resist[j][1]/CAL_MUL_DIV_VAL_NEW);
		pre_res[2]=(float)(flag->cal_data.pre_cal_resist[j][2]/CAL_MUL_DIV_VAL_NEW);
		ref_res[0]=(float)(flag->cal_data.ref_resist[2]/CAL_MUL_DIV_VAL_NEW);
		ref_res[1]=(float)(flag->cal_data.ref_resist[3]/CAL_MUL_DIV_VAL_NEW);
		ref_res[2]=(float)(flag->cal_data.ref_resist[4]/CAL_MUL_DIV_VAL_NEW);

		std_tmp=ref_res[2];//50
		meas_tmp=pre_res[2];
		scale_factor=meas_tmp/std_tmp;
		//Original
		scale_factor_1=scale_factor*0.9997;
		scale_factor_2=scale_factor*0.9987;
		//HIGH
		//scale_factor_1=scale_factor;
		//scale_factor_2=scale_factor;
		//low
		//scale_factor_1=scale_factor*0.9994;
		//scale_factor_2=scale_factor*0.9984;

		cal_tmp_100=tmp_100*scale_factor_1;
		cal_tmp_200=tmp_200*scale_factor_2;

		printf("[%d],std:%f,meas:%f,factor:(%f)(%f),(%f),cal_200:%f,cal_410:%f\r\n",\
				j,std_tmp,meas_tmp,scale_factor,scale_factor_1,scale_factor_1,cal_tmp_100,cal_tmp_200);
		ref_res[1]=tmp_100;
		ref_res[2]=tmp_200;
		pre_res[1]=cal_tmp_100;
		pre_res[2]=cal_tmp_200;

		for (i = 0; i < 3; i++)
		{
			a[i][0] = pre_res[i]*pre_res[i];//pow(pre_res[i], 2);
			a[i][1] = pre_res[i];
			a[i][2] = 1.0;
			b[i] = ref_res[i];
		}
		solve_system_3x3(a,b,ref_res_res);
		b[0]=ref_res_res[0];b[1]=ref_res_res[1];b[2]=ref_res_res[2];
		cal_tmp_100=(-ref_res_res[1] + sqrt(ref_res_res[1]*ref_res_res[1] - 4*ref_res_res[0]*(ref_res_res[2] - tmp_100))) / (2*ref_res_res[0]);
		cal_tmp_200=(-ref_res_res[1] + sqrt(ref_res_res[1]*ref_res_res[1] - 4*ref_res_res[0]*(ref_res_res[2] - tmp_200))) / (2*ref_res_res[0]);
		ref_res[1]=tmp_100;
		ref_res[2]=tmp_200;
		pre_res[1]=cal_tmp_100;
		pre_res[2]=cal_tmp_200;
		printf("[%d],std:%f,meas:%f,cal_200:%f,cal_410:%f\r\n",j,std_tmp,meas_tmp,cal_tmp_100,cal_tmp_200);

		for (i = 0; i < 3; i++)
		{
			a[i][0] = pre_res[i]*pre_res[i];//pow(pre_res[i], 2);
			a[i][1] = pre_res[i];
			a[i][2] = 1.0;
			b[i] = ref_res[i];
		}
		solve_system_3x3(a,b,ref_res_res);
		printf("1[%.10e][%f][%f]\r\n",ref_res_res[0],ref_res_res[1],ref_res_res[2]);//[%.10e]
		if(ref_res_res[0]<0)
		{
			flag->cal_data.cal_coeff[j][0]=(ref_res_res[0]-CAL_ADD_OFFSET_UP)*CAL_MUL_DIV_VAL_UP;
		}
		else
		{
			flag->cal_data.cal_coeff[j][0]=(ref_res_res[0]+CAL_ADD_OFFSET_UP)*CAL_MUL_DIV_VAL_UP;
		}
		if(ref_res_res[1]<0)
		{
			flag->cal_data.cal_coeff[j][1]=(ref_res_res[1]-CAL_ADD_OFFSET_UP)*CAL_MUL_DIV_VAL;
		}
		else
		{
			flag->cal_data.cal_coeff[j][1]=(ref_res_res[1]+CAL_ADD_OFFSET_UP)*CAL_MUL_DIV_VAL;
		}
		if(ref_res_res[2]<0)
		{
			flag->cal_data.cal_coeff[j][2]=(ref_res_res[2]-CAL_ADD_OFFSET_UP)*CAL_MUL_DIV_VAL;
		}
		else
		{
			flag->cal_data.cal_coeff[j][2]=(ref_res_res[2]+CAL_ADD_OFFSET)*CAL_MUL_DIV_VAL;
		}
		printf("2[%ld][%ld][%ld]\r\n",flag->cal_data.cal_coeff[j][0],flag->cal_data.cal_coeff[j][1],flag->cal_data.cal_coeff[j][2]);
		printf("3[%lx][%lx][%lx]\r\n",flag->cal_data.cal_coeff[j][0],flag->cal_data.cal_coeff[j][1],flag->cal_data.cal_coeff[j][2]);
	}
	printf("RTD CALIBRATION Complete\r\n");
}

void read_cal_data(control_flag* flag)
{
	uint8_t i=0;
	CAL_DATA *cal;
	cal=(CAL_DATA*)ADDR_FlASH_PAGE_A_127;
	memcpy(&flag->cal_data,cal,sizeof(CAL_DATA));
	printf("cal_flag:[%08lx]\r\n",flag->cal_data.calibration_flag);
	printf("RES[%ld][%ld][%ld][%ld][%ld]\r\n",flag->cal_data.ref_resist[0],flag->cal_data.ref_resist[1],flag->cal_data.ref_resist[2],flag->cal_data.ref_resist[3],flag->cal_data.ref_resist[4]);
	for(i=0;i<SENSOR_TOTAL_CNT;i++)
	{
		printf("ref(%2d)[%07ld][%07ld][%07ld]\r\n",i,flag->cal_data.ref_cal_resist[i][0],flag->cal_data.ref_cal_resist[i][1],flag->cal_data.ref_cal_resist[i][2]);
		printf("pre(%2d)[%07ld][%07ld][%07ld]\r\n",i,flag->cal_data.pre_cal_resist[i][0],flag->cal_data.pre_cal_resist[i][1],flag->cal_data.pre_cal_resist[i][2]);
		//flag->cal_data.cal_coeff[i][0]=0.000001;
		//printf("cof(%2d)[%07d][%07d][%07d]\r\n",i,flag->cal_data.cal_coeff[i][0],flag->cal_data.cal_coeff[i][1],flag->cal_data.cal_coeff[i][2]);
		//printf("cal(%2d)[%07ld][%07ld][%07ld]\r\n",i,flag->cal_data.cal_resist[i][0],flag->cal_data.cal_resist[i][1],flag->cal_data.cal_resist[i][2]);
		//if(flag->cal_data.calibration_flag!=CAL_COMPLETE_FLAG)

		flag->cal_data.cal_resist[i][0]=(uint32_t)(24900.0*CAL_MUL_DIV_VAL_NEW);

			//flag->cal_data.cal_resist[i][1]=(uint32_t)(24900.0*CAL_MUL_DIV_VAL_NEW);
			//flag->cal_data.cal_resist[i][2]=(uint32_t)(24900.0*CAL_MUL_DIV_VAL_NEW);

		printf("cof(%2d)[%07ld][%07ld][%07ld]\r\n",i,flag->cal_data.cal_coeff[i][0],flag->cal_data.cal_coeff[i][1],flag->cal_data.cal_coeff[i][2]);
		printf("cal(%2d)[%07ld][%07ld][%07ld]\r\n",i,flag->cal_data.cal_resist[i][0],flag->cal_data.cal_resist[i][1],flag->cal_data.cal_resist[i][2]);
		printf("\r\n");
	}
}

