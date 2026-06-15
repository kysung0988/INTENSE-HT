/*
 * i2c_gpio.h
 *
 *  Created on: Jan 3, 2025
 *      Author: kimst
 */

#ifndef INC_I2C_GPIO_H_
#define INC_I2C_GPIO_H_

#include "stm32u5xx_hal.h"
#include "uart.h"
#include "main.h"

#define MULRSCNT  		14
#ifdef NOTCH
#define SENSOR_TOTAL_CNT	53
#else
#define SENSOR_TOTAL_CNT	52
#endif

typedef struct
{
	uint8_t 		addr;
	GPIO_TypeDef 	*sda_port;
	uint16_t 		sda_pin;
	GPIO_TypeDef 	*scl_port;
	uint16_t 		scl_pin;
}Temp_Port;

void i2c_delay(uint32_t delay);
void sda_in(uint8_t ch);
void m_sda_in(uint8_t row, uint8_t col_cnt);
void sda_out(uint8_t ch);
void m_sda_out(uint8_t row,uint8_t col_cnt);
void m_sda_high(uint8_t row,uint8_t col_cnt);
void m_sda_low(uint8_t row,uint8_t col_cnt);
void m_scl_high(uint8_t row,uint8_t col_cnt);
void m_scl_low(uint8_t row,uint8_t col_cnt);
uint32_t m_gpio_readpin(uint8_t row,uint8_t col_cnt);
void i2c_start(uint8_t ch);
void m_i2c_start(uint8_t row,uint8_t col_cnt);
void i2c_stop(uint8_t ch);
void m_i2c_stop(uint8_t row,uint8_t col_cnt);
uint8_t i2c_wait_ack(uint8_t ch,uint32_t wait_value);
uint32_t m_i2c_wait_ack(uint8_t row,uint8_t col_cnt,uint32_t wait_value);
void i2c_ack(uint8_t ch);
void m_i2c_ack(uint8_t row,uint8_t col_cnt);
void i2c_nack(uint8_t ch);
void m_i2c_nack(uint8_t row,uint8_t col_cnt);
void i2c_wr_byte(uint8_t ch, uint8_t data);
void m_i2c_wr_byte(uint8_t row,uint8_t col_cnt,uint8_t addr,uint8_t data,uint8_t rw);
uint8_t i2c_rd_byte(uint8_t ch, uint8_t ack);
void m_i2c_rd_byte(uint8_t row,uint8_t col_cnt,uint8_t ack, uint8_t* buf);
uint8_t i2c_wr_reg(uint8_t ch,uint8_t reg, uint16_t value);
uint32_t m_i2c_wr_reg(uint8_t row,uint8_t col_cnt,uint8_t reg, uint16_t value);
uint8_t i2c_rd_reg(uint8_t ch,uint8_t reg, uint16_t* value);
uint32_t m_i2c_rd_reg(uint8_t row,uint8_t col_cnt,uint8_t reg, uint16_t* value);
uint8_t tmp_sensor_config(uint16_t config,uint8_t *buff);
uint16_t test_sensor_id_read(uint8_t sel);
void tmp_all_read(void);
float tri_sensor_temp(void);
void ReadRealSensorAndSaveBuf(_opcode_r *opcode_r_rt);
void ReadSensor_temp(Temperature_Frame* frame,int16_t* tri_temp,uint16_t* bat_value, uint32_t* ack_data);
void i2c_io_deinit(void);
uint8_t multi_tmp_sensor_config(uint16_t config);
float sel_sensor_temp(uint8_t ch);


#endif /* INC_I2C_GPIO_H_ */
