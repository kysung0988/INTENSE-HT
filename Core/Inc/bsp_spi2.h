/*
 * bsp_spi2.h
 *
 *  Created on: Mar 4, 2026
 *      Author: kimst
 */

#ifndef INC_BSP_SPI2_H_
#define INC_BSP_SPI2_H_

#include <stdint.h>
#include <stdbool.h>
#include "stm32u5xx_hal.h"
#include "main.h"

#define SPI_ERROR_NONE                    0
#define SPI_ERROR_NO_INIT                -1
#define SPI_ERROR_WRONG_PARAM            -2
#define SPI_ERROR_BUSY                   -3
#define SPI_ERROR_PERIPH_FAILURE         -4
#define SPI_ERROR_COMPONENT_FAILURE      -5
#define SPI_ERROR_UNKNOWN_FAILURE        -6
#define SPI_ERROR_UNKNOWN_COMPONENT      -7
#define SPI_ERROR_BUS_FAILURE            -8
#define SPI_ERROR_CLOCK_FAILURE          -9
#define SPI_ERROR_MSP_FAILURE            -10
#define SPI_ERROR_FEATURE_NOT_SUPPORTED      -11

#define BUS_SPI2_POLL_TIMEOUT                   0x1000U


void RTD_CS_LOW(void);
void RTD_CS_HIGH(void);
int32_t BSP_SPI2_Send(uint8_t *pData, uint16_t Length);
int32_t BSP_SPI2_Recv(uint8_t *pData, uint16_t Length);
int32_t BSP_SPI2_SendRecv(uint8_t *pTxData, uint8_t *pRxData, uint16_t Length);



#endif /* INC_BSP_SPI2_H_ */
