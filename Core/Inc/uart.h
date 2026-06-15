/*
 * uart.h
 *
 *  Created on: May 8, 2025
 *      Author: kimst
 */

#ifndef INC_UART_H_
#define INC_UART_H_

#include "stm32u5xx_hal.h"
#include "main.h"

#define FLASH_SEGMENT	0x80

typedef union
{
	int32_t _04_byte[256];
	int8_t  _01_byte[1024];
}Protocol_Frame;

#define NONE_MISSION			0x00
#define IMMEDIATELY				0x01
#define	TIMEDELAY				0x02
#define	AT_TEMP_DELAY			0x03
#define	TEMPTRANSIENT			0x04
#define	DELAY_OR_TEMPERATURE  	0x05

#define	COM_CONNECT_MODE			0x01
#define	COM_DISCONNECT_MODE			0x00
#define	MEASUREMENT_START			0x01
#define	MEASUREMENT_END				0x00

#define NONE_TMP_READ			0x00
#define ALL_TMP_READ			0x01
#define REAL_TMP_READ			0x02
#define IDX_TEMP_READ			0x03

#define CALIBRATION_STOP		0x00
#define CALIBRATION_START		0x01

typedef union
{
	uint8_t  _01_byte[FLASH_SEGMENT]; // 88EA Sensor(Exclude TriggerSensor, Battery )
	int16_t	 _S_02_byte[FLASH_SEGMENT/2];  //x58 ( 88EA Senor) (Exclude TriggerSensor, Battery )
	uint16_t _02_byte[FLASH_SEGMENT/2];//uint16_t _02_byte[0x58]; //  64 sensor data>>89 sensor, 88EA(x58, Exclude Trigger, Battery )
	uint32_t _04_byte[FLASH_SEGMENT/4]; //  89EA(Sensor)  >> 90EA(89EA)
}Temperature_Frame;

typedef union
{
	uint8_t  _01_byte[FLASH_SEGMENT]; // 128 byte
	int16_t	 _S_02_byte[FLASH_SEGMENT/2];
	uint16_t _02_byte[FLASH_SEGMENT/2]; //  64(0x40,64EA_Sensor) sensor data>>89 sensor   89EA(Sensor), Exclude Trigger, Battery Inf
	uint32_t _04_byte[FLASH_SEGMENT/4]; //  64 sensor data
}ReadTempBat_Frame;

typedef struct __attribute__((__packed__))
{
  unsigned char sequencenumber[8];
  unsigned char serialnumber[30];
  unsigned char mission_name[40];
  unsigned char mission_complete_status[7];
  unsigned char sample_period[4];
  unsigned char acquisition_time[5];
  unsigned char trigger_temperature[4];
  unsigned char delaytime[6];
  unsigned char TriggerMode[18];
  unsigned char TransientTemp[5];
  unsigned char MissionStartDate[12];
  unsigned char Toolid[15];
  unsigned char SubToolid[15];
  unsigned char Reserved1[30];
  unsigned char Reserved2[30];
  //unsigned char padding[3];
} _opcode_M;

typedef struct __attribute__((__packed__))
{
  unsigned char sequencenumber[8];
  unsigned char serialnumber[30];
  unsigned char mission_name[40];
  unsigned char mission_complete_status[7];
  unsigned char sample_period[4];
  unsigned char acquisition_time[5];
  unsigned char trigger_temperature[4];
  unsigned char delaytime[6];
  unsigned char TriggerMode[18];
  unsigned char TransientTemp[5];
  unsigned char intensefirmwareversion[5];
  unsigned char intensewafersize[5];
  unsigned char permittedusagetime[11];
  unsigned char hoursofused[11];
  unsigned char waferbattervoltage[4];
  unsigned char sensorfailure[32];
  unsigned char intensechargingstatus[9];
  unsigned char Missionstartdate[12];
  unsigned char Toolid[15];
  unsigned char SubToolid[15];
  unsigned char Reserved1[30];
  unsigned char Reserved2[30];
//unsigned char padding;
} _opcode_f;

typedef struct __attribute__((__packed__))
{
  unsigned char admin_password[16];
  unsigned char intensefirmwareversion[5];
  unsigned char intensewafersize[5];
  unsigned char permittedusagetime[11];
  unsigned char hoursofused[11];
  unsigned char serialnumber[30];
  unsigned char reserved[18];
} _opcode_E;

typedef union
{
	uint8_t		_01_byte[1024];
	uint32_t	_04_byte[256];
}Buf_Frame;

typedef union
{
	uint32_t	_04_byte[128];
}BootTriger_Frame;

typedef struct __attribute__((__packed__))
{
  unsigned char intensechargestatus;
  unsigned char intensefirmwareversion[5];
  unsigned char intensewafersize[5];
  unsigned char permittedusagetime[11];
  unsigned char hoursofused[11];
  unsigned char serialnumber[30];
  unsigned char waferbattervoltage[4];
} _opcode_p;


#define FIRMWARE_VER  		"1.0.0"

/*OP-CODE Define(ASC2)*/
#define NUL  		0x00		// NULL text
#define SOH 		0x01		// Start of header
#define STX  		0x02		// Start if text
#define ETX  		0x03		// End of text
#define EOT			0x04		// End of transmission
#define ENQ			0x05		// ENQuiry
#define ACK  		0x06		// ACKnowledge
#define CR 			0x0D		// Carriage return
#define NAK 		0x15		// Negative Acknowledge
#define DLE 		0x10		// Delete
#define ASC_A		0x41
#define ASC_a		0x61
#define ASC_B		0x42
#define ASC_b		0x62
#define ASC_I 		0x49
#define ASC_i		0x69
#define ASC_MINUS	0x2D
#define ASC_PLUS   	0x2B
#define ASC_N		0x4E
#define ASC_Y		0x59
#define	ASC_H		0x48
#define	ASC_9		0x39
#define	ASC_6		0x36
#define ASC_0		0x30
#define ASC_M		0x4D		// Type: Write  INTENSE wafer mission write request(SW?INTENSE)
#define ASC_m		0x6D
#define ASC_R		0x52		// Type: Write  INTENSE wafer measured temperature data request(SW?INTENSE)
#define ASC_r		0x72		// Type: Write  INTENSE wafer measured temperature data response(INTENSE?SW)
#define ASC_G		0x47		// Type: Write  Request "Go SleepMode"
#define ASC_g		0x67		// Type: Write  Response "Go SleepMode"
#define ASC_K		0x4B		// Type: Write  Request MISSION Rename "SUCCESS" To "READ OK"
#define ASC_k		0x6B		// Type: Write  Response MISSION Rename "SUCCESS" To "READ OK"
#define ASC_Q		0x51		// Type: Write  Request MISSION Rename "SUCCESS" To "READY"
#define ASC_q		0x71		// Type: Write  Response MISSION Rename "SUCCESS" To "READY"
#define ASC_j		0x6A
#define	ASC_P		0x50
#define	ASC_p		0x70
#define	ASC_Z		0x5A
#define	ASC_z		0x7A
#define ASC_F		0x46		// Type: Write  Factory setting request(SW?INTENSE)
#define ASC_f		0x66		// Type: Write  Factory setting response(INTENSE?SW)
#define ASC_E		0x45
#define ASC_e		0x65
#define ASC_T		0x54		// Type: Write  INTENSE wafer temperature sensor calibration data request(SW?INTENSE)
#define ASC_t		0x74
#define ASC_S		0x53
#define	ASC_s		0x73
#define	ASC_O		0x4F
#define	ASC_o		0x6F
#define ASC_c		0x63
#define ASC_d		0x64
#define ASC_J		0x4A
#define ASC_U		0x55
#define ASC_u		0x75
#define ASC_V		0x56
#define ASC_v		0x76
#define ASC_W		0x57
#define ASC_w		0x77
#define ASC_X		0x58
#define ASC_x		0x78
#define ASC_CAL		0xee

#define OPCODE_ARRAY				0x01



typedef union
{
	int32_t 	_R_04_byte[256];
	float		_R_F_04_byte[256];
	int16_t 	_R_02_byte[512];
	uint8_t		_R_01_u_byte[1024];
	int8_t 		_R_01_byte[1024];
	char   		_R_C_01_byte[1024];
}Protocol_CalcRecive_Frame;



void uart1_init(void);
void uart2_init(void);
//void ble_peripheral_read_proc(uint32_t *leng, uint8_t *buf);
//void ble_peripheral_process(uint32_t *leng, uint8_t *buf,uint32_t* tx_len, uint8_t* tx_buf,control_flag* flag);
//void Process(void);
void Protocol_Read(control_flag *flag);
uint8_t struct_send(uint8_t *ptr_array,uint32_t lenth,uint32_t senddelay,uint8_t initialsum);
void UART1_Tx(char send_data);
void Go_StandbyMode(void);
void MissionNameChangeREADOK(void);
void MissionCompleteStausChangeReady(void);
void boottrigerSet(void);
void SpaceRemoveLeft(uint8_t *strptr, uint32_t lenth);
void NAK_Answer_Check(uint8_t op_code);




#endif /* INC_UART_H_ */
