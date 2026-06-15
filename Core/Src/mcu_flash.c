/*
 * mcu_flash.c
 *
 *  Created on: Apr 1, 2026
 *      Author: kimst
 */


#include "mcu_flash.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void Protocol_Erase_Page(uint8_t op_code)
{
	FLASH_EraseInitTypeDef EraseInitStruct;
	uint32_t PageError=0;
	if(op_code == ASC_M)
	{
		EraseInitStruct.Banks       = FLASH_BANK_1;
		EraseInitStruct.TypeErase   = FLASH_TYPEERASE_PAGES; // 0x00
		EraseInitStruct.Page        = FLASH_USER_PROTOCOL_M_ADDR_START_PAGE;
		EraseInitStruct.NbPages     = 1;
		HAL_FLASH_Unlock();
		if (HAL_FLASHEx_Erase(&EraseInitStruct, &PageError) != HAL_OK)
		{
			//printf("Erase Error\r\n");
		}
		HAL_FLASH_Lock();
	}
	else if(op_code == ASC_E)
	{
		EraseInitStruct.Banks       = FLASH_BANK_1;
		EraseInitStruct.TypeErase   = FLASH_TYPEERASE_PAGES; // 0x00
		EraseInitStruct.Page        = FLASH_USER_PROTOCOL_E_ADDR_START_PAGE;
		EraseInitStruct.NbPages     = 1;
		HAL_FLASH_Unlock();
		if (HAL_FLASHEx_Erase(&EraseInitStruct, &PageError) != HAL_OK)
		{
			printf("Erase Error \n");
		}
		else
		{
			printf("Success \n");
		}

		HAL_FLASH_Lock();
	}
	else if(op_code == ASC_R)
	{
		EraseInitStruct.Banks       = FLASH_BANK_2;
		EraseInitStruct.TypeErase   = FLASH_TYPEERASE_PAGES; // 0x00
		EraseInitStruct.Page        = FLASH_USER_PROTOCOL_R_REMOVE_START_PAGE;
		EraseInitStruct.NbPages     = FLASH_USER_PROTOCOL_R_REMOVE_END_PAGE;

		HAL_FLASH_Unlock();
		if (HAL_FLASHEx_Erase(&EraseInitStruct, &PageError) != HAL_OK)
		{
			printf("Erase Error \n");
		}
		/*
		else
		{
			printf("Success \n");
		}*/

		HAL_FLASH_Lock();
		// BANK1 Trigger Sensor~Bat Sensor Erazing...
		EraseInitStruct.Banks       = FLASH_BANK_1;
		EraseInitStruct.TypeErase   = FLASH_TYPEERASE_PAGES; // 0x00
		EraseInitStruct.Page        = FLASH_USER_PROTOCOL_TRIGGER_SENSOR_ADDR_START_PAGE;  //FLASH_USER_START_ADDR-TMP_FLASH_PAGE_SIZE;
		EraseInitStruct.NbPages     = 21; //  23(21)
		HAL_FLASH_Lock();

		HAL_FLASH_Unlock();
		if (HAL_FLASHEx_Erase(&EraseInitStruct, &PageError) != HAL_OK)
		{
			//printf("Erase Error\r\n");
		}
		HAL_FLASH_Lock();
	}
	else if(op_code == ASC_CAL)
	{
		EraseInitStruct.Banks       = FLASH_BANK_1;
		EraseInitStruct.TypeErase   = FLASH_TYPEERASE_PAGES; // 0x00
		EraseInitStruct.Page        = FLASH_USER_PROTOCOL_CAL_ADDR_START_PAGE;
		EraseInitStruct.NbPages     = 1;
		HAL_FLASH_Unlock();
		if (HAL_FLASHEx_Erase(&EraseInitStruct, &PageError) != HAL_OK)
		{
			printf("Erase Error CAL \n");
		}
		else
		{
			printf("Erase Success CAL \n");
		}
		HAL_FLASH_Lock();
	}
	else
	{
		printf("NOT SUPPORT FLASH ERASE!!\r\n");
	}

}

void Protocol_Flash_Memory_Store(uint8_t op_code,uint32_t *frame)
{
	//HAL_StatusTypeDef result;
	unsigned int  write_addr;

	if(op_code == ASC_M)
	{
		HAL_FLASH_Unlock();
		write_addr = M_FLASH_AREA;//Bank1 101
		HAL_FLASH_Program(FLASH_TYPEPROGRAM_BURST, write_addr,(unsigned int )(__IO uint32_t *)(frame));//128Byte Write
		write_addr = M_FLASH_AREA + (0x80) ;
		HAL_FLASH_Program(FLASH_TYPEPROGRAM_BURST, write_addr,(unsigned int )(__IO uint32_t *)(frame+0x20));
		//HAL_Delay(50);
		HAL_FLASH_Lock();
		//HAL_Delay(50);
		write_addr = M_FLASH_AREA + (0x80);
		HAL_FLASH_Program(FLASH_TYPEPROGRAM_BURST, write_addr,(unsigned int )(__IO uint32_t *)(frame + 0x20)); // 4bytx32(0x20)=128
		HAL_FLASH_Lock();
	}
	else if(op_code == ASC_E)
	{
		HAL_FLASH_Unlock();
		write_addr = E_FLASH_AREA;
		//result = HAL_FLASH_Program(FLASH_TYPEPROGRAM_BURST, write_addr,(unsigned int )(__IO uint32_t *)(WriteEFrame._04_byte));	// flash memory write
		HAL_FLASH_Program(FLASH_TYPEPROGRAM_BURST, write_addr,(unsigned int )(__IO uint32_t *)(frame));
		HAL_FLASH_Lock();
	}
	else if(op_code == ASC_F)
	{
		HAL_FLASH_Unlock();
		write_addr = F_FLASH_AREA;
		//result = HAL_FLASH_Program(FLASH_TYPEPROGRAM_BURST, write_addr,(unsigned int )(__IO uint32_t *)(WriteFFrame._04_byte));	// flash memory write
		HAL_FLASH_Program(FLASH_TYPEPROGRAM_BURST, write_addr,(unsigned int )(__IO uint32_t *)(frame));
		//HAL_Delay(50);
		//HAL_FLASH_Lock();
		//HAL_Delay(50);
		//HAL_FLASH_Unlock();
		write_addr = F_FLASH_AREA + (0x80) ;
		//result = HAL_FLASH_Program(FLASH_TYPEPROGRAM_BURST, write_addr,(unsigned int )(__IO uint32_t *)(WriteFFrame._04_byte + 0x20));	// flash memory write
		HAL_FLASH_Program(FLASH_TYPEPROGRAM_BURST, write_addr,(unsigned int )(__IO uint32_t *)(frame + 0x20));
		HAL_FLASH_Lock();
	}
	//HAL_Delay(50);
	//HAL_FLASH_Lock();
}

void Store_tri_bat_data(uint8_t flag, uint32_t data_count, uint32_t* frame)
{
	unsigned int  write_addr;
	if(flag)//trigger
	{
		write_addr = (ADDR_FlASH_PAGE_A_105 + (data_count*FLASH_SEGMENT)); // 0x80 = 64
	}
	else//Bat
	{
		write_addr = (ADDR_FlASH_PAGE_A_118 +(data_count*FLASH_SEGMENT));
	}
	HAL_FLASH_Unlock();
	HAL_FLASH_Program(FLASH_TYPEPROGRAM_BURST, write_addr,(unsigned int )(__IO uint32_t *)(frame));
	HAL_FLASH_Lock();
}

void MissionSUccessStore(void)
{
	Buf_Frame s_Buf_Frame;
	uint32_t rw_read_addr 	= FLASH_USER_PROTOCOL_M_ADDR_START_PAGE;
	_opcode_M *ptr_opcode_M_Buf;
	memset(s_Buf_Frame._01_byte,0,sizeof(s_Buf_Frame._01_byte));
	for(int x = 0; x < 256; x++)  //128 >> 256
	{
		s_Buf_Frame._01_byte[x]	=	*(__IO uint8_t*) ( rw_read_addr + (1 * x));
	}
	ptr_opcode_M_Buf=(_opcode_M*)s_Buf_Frame._01_byte;
	memcpy(ptr_opcode_M_Buf->mission_complete_status,"SUCCESS",7);
	Protocol_Erase_Page(ASC_M);
	Protocol_Flash_Memory_Store(ASC_M,(uint32_t*)s_Buf_Frame._04_byte);
}

void Wrtie_Hours_of_used(control_flag* flag)
{
	Buf_Frame s_Buf_Frame;
	uint8_t strbuf[20];
	uint32_t SumValue=0;
	uint32_t rw_read_addr 	= FLASH_USER_PROTOCOL_E_ADDR_START_PAGE;
	memset(s_Buf_Frame._01_byte,0,sizeof(s_Buf_Frame._01_byte));
	for(int x = 0; x < 128; x++)
	{
		s_Buf_Frame._01_byte[x]	=	*(__IO uint8_t*) ( rw_read_addr + (1 * x));
	}
	if(s_Buf_Frame._01_byte[37]!=(uint8_t)' ')
	{
		memset(strbuf,0,sizeof(strbuf));
		sprintf((char*)strbuf,"%11ld",flag->i_acquisition_time);
	}
	else
	{
		memset(strbuf,0,sizeof(strbuf));
		memcpy(strbuf,&s_Buf_Frame._01_byte[37], 11);
	  	SumValue=flag->i_acquisition_time+atoi((char*)strbuf);

	   	memset(strbuf,0,sizeof(strbuf));
	  	sprintf((char*)strbuf,"%11ld",SumValue);
	}
	memcpy(&s_Buf_Frame._01_byte[37],strbuf,11);
	Protocol_Erase_Page(ASC_E);
	Protocol_Flash_Memory_Store(ASC_E,(uint32_t*)s_Buf_Frame._04_byte);
}
