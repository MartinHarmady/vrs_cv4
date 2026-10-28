/**
  ******************************************************************************
  * File Name          : USART.c
  * Description        : This file provides code for the configuration
  *                      of the USART instances.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2019 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "usart.h"

uint8_t bufferUSART2dma[DMA_USART2_BUFFER_SIZE];

static void (* USART2_ProcessData)(uint8_t data) = 0;

void USART2_RegisterCallback(void *callback)
{
	if(callback != 0)
	{
		USART2_ProcessData = callback;
	}
}




void MX_USART2_UART_Init(void)
{
  LL_USART_InitTypeDef USART_InitStruct = {0};

  LL_GPIO_InitTypeDef GPIO_InitStruct = {0};
  LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_USART2);
  
  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOA);

  GPIO_InitStruct.Pin = LL_GPIO_PIN_2|LL_GPIO_PIN_15;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_HIGH;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  GPIO_InitStruct.Alternate = LL_GPIO_AF_7;
  LL_GPIO_Init(GPIOA, &GPIO_InitStruct);


  
  /* USART2_RX Init - DMA1 Channel 6, normal mode (NOT circular) */
  LL_DMA_SetDataTransferDirection(DMA1, LL_DMA_CHANNEL_6, LL_DMA_DIRECTION_PERIPH_TO_MEMORY);
  LL_DMA_SetMode(DMA1, LL_DMA_CHANNEL_6, LL_DMA_MODE_NORMAL);
  LL_DMA_SetPeriphIncMode(DMA1, LL_DMA_CHANNEL_6, LL_DMA_PERIPH_NOINCREMENT);
  LL_DMA_SetMemoryIncMode(DMA1, LL_DMA_CHANNEL_6, LL_DMA_MEMORY_INCREMENT);
  LL_DMA_SetPeriphSize(DMA1, LL_DMA_CHANNEL_6, LL_DMA_PDATAALIGN_BYTE);
  LL_DMA_SetMemorySize(DMA1, LL_DMA_CHANNEL_6, LL_DMA_MDATAALIGN_BYTE);
  LL_DMA_SetChannelPriorityLevel(DMA1, LL_DMA_CHANNEL_6, LL_DMA_PRIORITY_MEDIUM);
  LL_DMA_SetPeriphAddress(DMA1, LL_DMA_CHANNEL_6, LL_USART_DMA_GetRegAddr(USART2, LL_USART_DMA_REG_DATA_RECEIVE));
  LL_DMA_SetMemoryAddress(DMA1, LL_DMA_CHANNEL_6, (uint32_t)bufferUSART2dma);
  LL_DMA_SetDataLength(DMA1, LL_DMA_CHANNEL_6, DMA_USART2_BUFFER_SIZE);

  LL_DMA_SetDataTransferDirection(DMA1, LL_DMA_CHANNEL_7, LL_DMA_DIRECTION_MEMORY_TO_PERIPH);
  LL_DMA_SetMode(DMA1, LL_DMA_CHANNEL_7, LL_DMA_MODE_NORMAL);
  LL_DMA_SetPeriphIncMode(DMA1, LL_DMA_CHANNEL_7, LL_DMA_PERIPH_NOINCREMENT);
  LL_DMA_SetMemoryIncMode(DMA1, LL_DMA_CHANNEL_7, LL_DMA_MEMORY_INCREMENT);
  LL_DMA_SetPeriphSize(DMA1, LL_DMA_CHANNEL_7, LL_DMA_PDATAALIGN_BYTE);
  LL_DMA_SetMemorySize(DMA1, LL_DMA_CHANNEL_7, LL_DMA_MDATAALIGN_BYTE);
  LL_DMA_SetChannelPriorityLevel(DMA1, LL_DMA_CHANNEL_7, LL_DMA_PRIORITY_MEDIUM);
  LL_DMA_SetPeriphAddress(DMA1, LL_DMA_CHANNEL_7, LL_USART_DMA_GetRegAddr(USART2, LL_USART_DMA_REG_DATA_TRANSMIT));


  NVIC_SetPriority(USART2_IRQn, 0);
  NVIC_EnableIRQ(USART2_IRQn);

  USART_InitStruct.BaudRate = 115200;
  USART_InitStruct.DataWidth = LL_USART_DATAWIDTH_8B;
  USART_InitStruct.StopBits = LL_USART_STOPBITS_1;
  USART_InitStruct.Parity = LL_USART_PARITY_NONE;
  USART_InitStruct.TransferDirection = LL_USART_DIRECTION_TX_RX;
  USART_InitStruct.HardwareFlowControl = LL_USART_HWCONTROL_NONE;
  USART_InitStruct.OverSampling = LL_USART_OVERSAMPLING_16;
  LL_USART_Init(USART2, &USART_InitStruct);
  LL_USART_ConfigAsyncMode(USART2);
  LL_USART_DisableIT_CTS(USART2);

  LL_USART_Enable(USART2);
  LL_USART_EnableIT_IDLE(USART2);
  LL_USART_EnableDMAReq_RX(USART2);
  LL_USART_EnableDMAReq_TX(USART2);

  LL_DMA_EnableIT_HT(DMA1, LL_DMA_CHANNEL_6);
  LL_DMA_EnableIT_TC(DMA1, LL_DMA_CHANNEL_6);

  LL_DMA_EnableChannel(DMA1, LL_DMA_CHANNEL_6);
}


void USART2_PutBuffer(uint8_t *buffer, uint8_t length)
{
	LL_DMA_SetMemoryAddress(DMA1, LL_DMA_CHANNEL_7, (uint32_t)buffer);

	LL_DMA_SetDataLength(DMA1, LL_DMA_CHANNEL_7, length);

	LL_DMA_EnableIT_TC(DMA1, LL_DMA_CHANNEL_7);

	LL_DMA_EnableChannel(DMA1, LL_DMA_CHANNEL_7);
}



void USART2_CheckDmaReception(void)
{
	static uint16_t lastProcessedPos = 0;
	uint16_t currentPos;
	uint16_t newBytes;
	uint16_t i;

	currentPos = DMA_USART2_BUFFER_SIZE - (uint16_t)LL_DMA_GetDataLength(DMA1, LL_DMA_CHANNEL_6);

	if (currentPos >= lastProcessedPos)
	{
		newBytes = currentPos - lastProcessedPos;
	}
	else
	{
		newBytes = DMA_USART2_BUFFER_SIZE - lastProcessedPos + currentPos;
	}

	for (i = 0; i < newBytes; i++)
	{
		if (USART2_ProcessData != 0)
		{
			USART2_ProcessData(bufferUSART2dma[lastProcessedPos]);
		}
		lastProcessedPos++;
		if (lastProcessedPos >= DMA_USART2_BUFFER_SIZE)
		{
			lastProcessedPos = 0;
		}
	}


	if (LL_DMA_IsEnabledChannel(DMA1, LL_DMA_CHANNEL_6) == 0)
	{
		LL_DMA_SetMemoryAddress(DMA1, LL_DMA_CHANNEL_6, (uint32_t)bufferUSART2dma);
		LL_DMA_SetDataLength(DMA1, LL_DMA_CHANNEL_6, DMA_USART2_BUFFER_SIZE);
		LL_DMA_EnableChannel(DMA1, LL_DMA_CHANNEL_6);
		lastProcessedPos = 0;
	}
}




/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
