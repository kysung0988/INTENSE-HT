/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "app_display.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "tim6.h"
#include "uart.h"
#include <stdio.h>
#include "ili9341.h"
#include "adc.h"
#include "max31865.h"
#include "ads124s06.h"
#include "control.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc1;

IWDG_HandleTypeDef hiwdg;

SPI_HandleTypeDef hspi2;

TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim6;

UART_HandleTypeDef huart1;
UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_IWDG_Init(void);
static void MX_SPI2_Init(void);
static void MX_TIM6_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_ADC1_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

void sensor_ldo_on(void)
{
	//HAL_GPIO_WritePin(LDO_EN0_GPIO_Port,LDO_EN0_Pin,GPIO_PIN_SET);
	//HAL_GPIO_WritePin(LDO_EN1_GPIO_Port,LDO_EN1_Pin,GPIO_PIN_SET);
	//HAL_GPIO_WritePin(LDO_EN2_GPIO_Port,LDO_EN2_Pin,GPIO_PIN_SET);
	//HAL_Delay(50);
	//multi_tmp_sensor_config(0x0120);
	//HAL_Delay(130);
}

control_flag flag_control;
uint32_t selection_mode=0;
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */


int main(void)
{

  /* USER CODE BEGIN 1 */
#if 1
	flag_control.ble_peri_command=0;
	flag_control.mission_proc_cnt=0;flag_control.receive_calibration=CALIBRATION_STOP;
	flag_control.receive_mission=NONE_MISSION;flag_control.receive_temp_read=NONE_TMP_READ;
	flag_control.cal_data.calibration_flag=0xffffffff;
	flag_control.ble_com_flag = 0;
#endif
#if 0
	control_flag flag_control={
						.mission_proc_cnt=0,
						.comState=1,
						.measurementState=0,
						.samplingRate=1000,
						.setDataSet=0,
						.setTriggerTemp=0,
						.timeDelay=0,
						.trigger=0,
						.DoorTrigerTimeOut=10000,
						.i_acquisition_time=0,
						.sensor_status={0,},
						.ble_com_flag=0,
						.mission_start_flag=0,
						.ble_peri_command=0,

						.receive_calibration=CALIBRATION_STOP,
						.receive_mission=NONE_MISSION,
						.receive_temp_read=NONE_TMP_READ,
						.cal_data.calibration_flag=0xffffffff
					};

	//ADD
	uint32_t temp_acq_cnt=0,flash_128_cnt=0,bat_tri_flash_page=0;
	Temperature_Frame 	ReadTemp;
	ReadTempBat_Frame 	ReadTriggerSensor;
	ReadTempBat_Frame 	ReadBatteryVol;

	uint8_t charging_status=0,attemp_trigger_high=0,attemp_trigger_low=0;
	uint8_t transient_flag=0,transient_flag_tri=0,tmp_fail_cnt=0;
	int16_t main_tri_temp=0;
    uint16_t main_bat_value=0;
	//
#endif
	uint32_t ble_peri_leng=0,ble_peri_tx_leng=0,rst_status=0;
	uint8_t ble_peri_buf[1024]={0,},ble_peri_tx_buf[1024]={0,},main_strToint[10]={0,},tmp_fail_cnt=0;
	_opcode_M *ptr_mission_flash;
	uint8_t main_oneshot_flag=0;


	for(tmp_fail_cnt=0;tmp_fail_cnt<32;tmp_fail_cnt++)
	{
		flag_control.sensor_status[tmp_fail_cnt]=0;
	}
	tmp_fail_cnt=0;

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_IWDG_Init();
  MX_SPI2_Init();
  MX_TIM6_Init();
  MX_USART1_UART_Init();
  MX_USART2_UART_Init();
  MX_ADC1_Init();

  if (__HAL_PWR_GET_FLAG(PWR_FLAG_SBF) != RESET)
  {
	  /* Clear Standby flag */
      __HAL_PWR_CLEAR_FLAG(PWR_FLAG_SBF);
      /* Check and Clear the Wakeup flag */
      if (__HAL_PWR_GET_FLAG(PWR_WAKEUP_FLAG2) != RESET)
      {
    	  __HAL_PWR_CLEAR_FLAG(PWR_WAKEUP_FLAG2);
      }
  }
  //MX_DISPLAY_Init();
  /* USER CODE BEGIN 2 */
  uart1_init();
  uart2_init();
  flash_init();
  tim6_init();
  printf("HIGH POWER START\r\n");

  //ILI9341_INIT_SET();

  //ILI9341_Fill_Screen(YELLOW);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  clear_time_cnt();
  read_cal_data(&flag_control);
  ads124s06_init();
  tmp_fail_cnt=tmp_sensor_config(&flag_control);
  printf("FAIL:%d\r\n",tmp_fail_cnt);
  for(tmp_fail_cnt=0;tmp_fail_cnt<32;tmp_fail_cnt++)
  {
  	ble_peri_leng++;
  	printf("%2d:[%2x] ",tmp_fail_cnt,flag_control.sensor_status[tmp_fail_cnt]);
  	if((ble_peri_leng%8)==0 )
  	{
  		printf("\r\n");
  	}
  }
  ble_peri_leng=0;
  for(tmp_fail_cnt=0;tmp_fail_cnt<SENSOR_TOTAL_CNT;tmp_fail_cnt++)
  {
	  ble_peri_leng++;
    	printf("%2d:[%2x] ",tmp_fail_cnt,flag_control.rtd_status[tmp_fail_cnt]);
    	if((ble_peri_leng%8)==0 )
    	{
    		printf("\r\n");
    	}
  }
  ble_peri_leng=0;
  //select_mux(DUMMY_CHANNEL);
  select_mux(0);
  HAL_GPIO_WritePin(BLE_EN_GPIO_Port,BLE_EN_Pin,GPIO_PIN_SET);
  //SystemCoreClockUpdate();
  //printf("\r\n Clock: %lu \r\n", SystemCoreClock);
  while (1)
  {
	  HAL_IWDG_Refresh(&hiwdg);
    /* USER CODE END WHILE */

      //MX_DISPLAY_Process(); // 필요없음 vlfdydjqtdma
    /* USER CODE BEGIN 3 */
	  if(HAL_GPIO_ReadPin(SPI2_RD_GPIO_Port, SPI2_RD_Pin)==GPIO_PIN_SET)
	  {
		  HAL_GPIO_WritePin(G_LED_GPIO_Port,G_LED_Pin, GPIO_PIN_SET);
  	  }
  	  else
  	  {
  		  HAL_GPIO_WritePin(G_LED_GPIO_Port,G_LED_Pin, GPIO_PIN_RESET);
  	  }
	  Protocol_Read(&flag_control);

#if 0
	  //
	  ble_peripheral_read_proc(&ble_peri_leng,ble_peri_buf);
	  ble_peripheral_process(&ble_peri_leng,ble_peri_buf,&ble_peri_tx_leng,ble_peri_tx_buf,&flag_control);
#endif
	  if(flag_control.receive_mission==NONE_MISSION && flag_control.receive_temp_read==NONE_TMP_READ && flag_control.receive_calibration!=CALIBRATION_STOP)
	  {
		  //printf("CAL Start\r\n");
		  calibration_proc(&flag_control);
	  }
	  Process();

#if 0
	  if((flag_control.measurementState==1 && flag_control.DoorTrigerTimeOut==0)|| temp_acq_cnt != 0)
	  {

		  //Wrtie_Hours_of_used(&flag_control);
		  if(charging_status && temp_acq_cnt != 0)
		  {

			  flag_control.measurementState=0;
			  if(flash_128_cnt!=0)
			  {
				 // Store_tri_bat_data(1, bat_tri_flash_page,(uint32_t*)ReadTriggerSensor._04_byte);
				  //Store_tri_bat_data(0, bat_tri_flash_page,(uint32_t*)ReadBatteryVol._04_byte);

			  }
			  MissionSUccessStore();
			 // Wrtie_Hours_of_used(&flag_control);
			  printf("MISSION_CHARGING_INTERRUPT\r\n");
			  //HAL_GPIO_WritePin(LDO_EN4_GPIO_Port,LDO_EN4_Pin,GPIO_PIN_SET);
			  huart1.gState=HAL_UART_STATE_RESET;
			  uart1_init();
		  }
		  else
		  {
			  switch(flag_control.trigger)
			  {
				  case IMMEDIATELY:
					  if(charging_status==0)
					  {
						  HAL_UART_MspDeInit(&huart1);
						  //HAL_GPIO_WritePin(LDO_EN4_GPIO_Port,LDO_EN4_Pin,GPIO_PIN_RESET);ASC_c
					  }
					  else
					  {
						  //HAL_GPIO_WritePin(LDO_EN4_GPIO_Port,LDO_EN4_Pin,GPIO_PIN_SET);
						  huart1.gState=HAL_UART_STATE_RESET;
						  uart1_init();
						  flag_control.DoorTrigerTimeOut=10000;
					  }
					  if(temp_acq_cnt < flag_control.setDataSet)
					  {
						  printf("temp_acq_cnt %d \r\n",temp_acq_cnt);
						  if(temp_acq_cnt==0)
						  {
							  sensor_ldo_on();
							  printf("\r\nsampling:%ld\r\n",temp_acq_cnt);
							  clear_mission_cnt();
							  memset(ReadTemp._04_byte,0xFFFFffff,sizeof(ReadTemp._04_byte));
							  //ReadSensor_temp(&ReadTemp,&main_tri_temp,&main_bat_value,temp_i2c_ack_flag);
							  memset(ReadTriggerSensor._S_02_byte,0xFFFF,sizeof(ReadTriggerSensor._S_02_byte));
							  memset(ReadBatteryVol._S_02_byte,0xFFFF,sizeof(ReadBatteryVol._S_02_byte));
							  ReadTriggerSensor._S_02_byte[flash_128_cnt]=main_tri_temp;
							  ReadBatteryVol._S_02_byte[flash_128_cnt]=main_bat_value