/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "dma.h"
#include "usart.h"
#include "gpio.h"
#include <stdio.h>

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);


/* Function processing DMA Rx data. Counts how many capital and small letters are in sentence.
 * Result is supposed to be stored in global variable of type "letter_count_" that is defined in "main.h"
 *
 * @param1 - received sign
 */
void proccesDmaData(uint8_t sign);


/* Space for your global variables. */

	// type your global variables here:

/* Parser state machine for #...$ strings */
static uint8_t  parserCollecting = 0;       /* 1 = between # and $, 0 = waiting for # */
static uint8_t  parserPos = 0;              /* number of chars collected after # */
static uint8_t  parserBuffer[40];           /* buffer for the string content (max 35 chars + null) */

/* Result of the last valid string and a flag for the main loop */
static letter_count_ lastLetterCount = {0, 0};
static uint8_t  validStringBuffer[40];
static volatile uint8_t validStringFlag = 0;


int main(void)
{
  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_SYSCFG);
  LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_PWR);
  NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4);

  /* Configure the system clock */
  SystemClock_Config();
  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_USART2_UART_Init();

  /* Space for your local variables, callback registration ...*/

  	  //type your code here:
  USART2_RegisterCallback(proccesDmaData);

  while (1)
  {

	  LL_mDelay(2000);

	  uint16_t occupied = DMA_USART2_BUFFER_SIZE - (uint16_t)LL_DMA_GetDataLength(DMA1, LL_DMA_CHANNEL_6);
	  uint16_t loadInt  = (uint16_t)(((uint32_t)occupied * 100) / DMA_USART2_BUFFER_SIZE);
	  uint16_t loadFrac = (uint16_t)(((uint32_t)occupied * 1000) / DMA_USART2_BUFFER_SIZE) % 10;

	  char txBuf[100];
	  int txLen = snprintf(txBuf, sizeof(txBuf),
		   "Buffer capacity: %d bytes, occupied memory: %d bytes, load [in %%]: %d.%d%%\r\n",
		   (int)DMA_USART2_BUFFER_SIZE, (int)occupied, (int)loadInt, (int)loadFrac);
	  if (txLen > 0)
	  {
		  while (LL_DMA_IsEnabledChannel(DMA1, LL_DMA_CHANNEL_7));
		  USART2_PutBuffer((uint8_t *)txBuf, (uint8_t)txLen);
	  }


	  if (validStringFlag)
	  {
		  validStringFlag = 0;
		  int txLen2 = snprintf(txBuf, sizeof(txBuf),
				 "Valid string: %s, lower-case: %d, upper-case: %d\r\n",
				 (char *)validStringBuffer,
				 (int)lastLetterCount.small_letter,
				 (int)lastLetterCount.capital_letter);
		  if (txLen2 > 0)
		  {
			  while (LL_DMA_IsEnabledChannel(DMA1, LL_DMA_CHANNEL_7));
			  USART2_PutBuffer((uint8_t *)txBuf, (uint8_t)txLen2);
		  }
	  }
  }
  /* USER CODE END 3 */
}


void SystemClock_Config(void)
{
  LL_FLASH_SetLatency(LL_FLASH_LATENCY_0);

  if(LL_FLASH_GetLatency() != LL_FLASH_LATENCY_0)
  {
  Error_Handler();  
  }
  LL_RCC_HSI_Enable();

  while(LL_RCC_HSI_IsReady() != 1)
  {
    
  }
  LL_RCC_HSI_SetCalibTrimming(16);
  LL_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_1);
  LL_RCC_SetAPB1Prescaler(LL_RCC_APB1_DIV_1);
  LL_RCC_SetAPB2Prescaler(LL_RCC_APB1_DIV_1);
  LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_HSI);

  while(LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_HSI)
  {
  
  }
  LL_Init1msTick(8000000);
  LL_SYSTICK_SetClkSource(LL_SYSTICK_CLKSOURCE_HCLK);
  LL_SetSystemCoreClock(8000000);
}


void proccesDmaData(uint8_t sign)
{

		if (!parserCollecting)
		{
			if (sign == '#')
			{
				parserCollecting = 1;
				parserPos = 0;
				lastLetterCount.small_letter = 0;
				lastLetterCount.capital_letter = 0;
			}

		}
		else
		{
			if (sign == '$')
			{
				parserBuffer[parserPos] = '\0';
				for (uint8_t i = 0; i <= parserPos; i++)
				{
					validStringBuffer[i] = parserBuffer[i];
				}
				validStringFlag = 1;
				parserCollecting = 0;
			}
			else if (parserPos >= 35)
			{
				parserCollecting = 0;
			}
			else
			{
				parserBuffer[parserPos] = sign;
				parserPos++;
				if (sign >= 'a' && sign <= 'z')
				{
					lastLetterCount.small_letter++;
				}
				else if (sign >= 'A' && sign <= 'Z')
				{
					lastLetterCount.capital_letter++;
				}
			}
		}
}


void Error_Handler(void)
{

}

#ifdef  USE_FULL_ASSERT

void assert_failed(char *file, uint32_t line)
{ 

}

#endif

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
