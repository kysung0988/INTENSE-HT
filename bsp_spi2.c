/*
 * bsp_spi2.c
 *
 *  Created on: Mar 4, 2026
 *      Author: kimst
 */

#include "bsp_spi2.h"

extern SPI_HandleTypeDef hspi2;

//#define RTD_CS_LOW()                        WRITE_REG(GPIOC->BRR, GPIO_PIN_0)
//#define RTD_CS_HIGH()                       WRITE_REG(GPIOC->BSRR, GPIO_PIN_0)

void RTD_CS_LOW(void)
{
	 WRITE_REG(GPIOC->BRR, GPIO_PIN_0);
}

void RTD_CS_HIGH(void)
{
	WRITE_REG(GPIOC->BSRR, GPIO_PIN_0);
}

int32_t BSP_SPI2_Send(uint8_t *pData, uint16_t Length)
{
  int32_t ret = SPI_ERROR_NONE;

  if(HAL_SPI_Transmit(&hspi2, pData, Length, BUS_SPI2_POLL_TIMEOUT) != HAL_OK)
  {
      ret = SPI_ERROR_UNKNOWN_FAILURE;
  }
  return ret;
}

int32_t BSP_SPI2_Recv(uint8_t *pData, uint16_t Length)
{
  int32_t ret = SPI_ERROR_NONE;

  if(HAL_SPI_Receive(&hspi2, pData, Length, BUS_SPI2_POLL_TIMEOUT) != HAL_OK)
  {
      ret = SPI_ERROR_UNKNOWN_FAILURE;
  }
  return ret;
}

int32_t BSP_SPI2_SendRecv(uint8_t *pTxData, uint8_t *pRxData, uint16_t Length)
{
  int32_t ret = SPI_ERROR_NONE;

  if(HAL_SPI_TransmitReceive(&hspi2, pTxData, pRxData, Length, BUS_SPI2_POLL_TIMEOUT) != HAL_OK)
  {
      ret = BSP_ERROR_UNKNOWN_FAILURE;
  }
  return ret;
}
