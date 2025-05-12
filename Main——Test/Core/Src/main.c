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
#include "can.h"
#include "spi.h"
#include "gpio.h"
#include "FSV9563.h"
#include <stdio.h>

void SystemClock_Config(void);

//#define SPI_SCK_PIN   GPIO_PIN_5
//#define SPI_MOSI_PIN  GPIO_PIN_7
//#define SPI_MISO_PIN  GPIO_PIN_6
//#define SPI_NSS_PIN   GPIO_PIN_4
//#define SPI_GPIO_PORT GPIOA

////void SPI_Soft_Init() 
////{
////		GPIO_InitTypeDef GPIO_InitStruct = {0};

////		__HAL_RCC_GPIOA_CLK_ENABLE();

////		GPIO_InitStruct.Pin = SPI_SCK_PIN | SPI_MOSI_PIN | SPI_NSS_PIN;
////		GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
////		GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
////		HAL_GPIO_Init(SPI_GPIO_PORT, &GPIO_InitStruct);

////		GPIO_InitStruct.Pin = SPI_MISO_PIN;
////		GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
////		GPIO_InitStruct.Pull = GPIO_NOPULL;
////		HAL_GPIO_Init(SPI_GPIO_PORT, &GPIO_InitStruct);
////		
////		FSV9563_NSS_1; 
////}

//void SPI_Soft_WriteByte(uint8_t data) 
//{
//    for (int i = 0; i < 8; i++) {
//		
//		 HAL_GPIO_WritePin(SPI_GPIO_PORT, SPI_SCK_PIN, GPIO_PIN_RESET); 
//		
//        if (data & 0x80) { 
//            HAL_GPIO_WritePin(SPI_GPIO_PORT, SPI_MOSI_PIN, GPIO_PIN_SET);
//        } else {
//            HAL_GPIO_WritePin(SPI_GPIO_PORT, SPI_MOSI_PIN, GPIO_PIN_RESET);
//        }
//        
//        HAL_GPIO_WritePin(SPI_GPIO_PORT, SPI_SCK_PIN, GPIO_PIN_SET); 
//		
//        data <<= 1;  
//         
//    }
//}

//uint8_t SPI_Soft_ReadByte(void) {
//    uint8_t data = 0;
//    for (int i = 0; i < 8; i++) {
//		HAL_GPIO_WritePin(SPI_GPIO_PORT, SPI_SCK_PIN, GPIO_PIN_RESET); 
//		
//		
//		HAL_GPIO_WritePin(SPI_GPIO_PORT, SPI_SCK_PIN, GPIO_PIN_SET);
//        data <<= 1;

//        if (HAL_GPIO_ReadPin(SPI_GPIO_PORT, SPI_MISO_PIN) == GPIO_PIN_SET) {
//			__NOP();
//            data |= 0x01;
//        }
//        HAL_GPIO_WritePin(SPI_GPIO_PORT, SPI_SCK_PIN, GPIO_PIN_RESET); 
//    }
//    return data;
//}

//uint8_t SPI_Soft_ReadRegister(uint8_t reg) {
//    uint8_t value;
//    FSV9563_NSS_0; 
//	
//	__NOP();
//    SPI_Soft_WriteByte((reg<<1)|0x01); 
//	
//	SPI_Soft_WriteByte(0);  
//    value = SPI_Soft_ReadByte();
//	__NOP();
//    FSV9563_NSS_1; 
//    return value;
//}

//void SPI_Soft_WriteRegister(uint8_t reg, uint8_t value) {
//    FSV9563_NSS_0; 
//    SPI_Soft_WriteByte(reg<<1); 
//    SPI_Soft_WriteByte(value);
//    FSV9563_NSS_1; 
//}


//int main(void)
//{
//	SystemClock_Config();
//	HAL_Init();
//	MX_GPIO_Init();
//	LED_1;
//    MX_CAN_Init();
//	MX_SPI1_Init();
//	//SPI_Soft_Init();
//	while(1)
//	{
//		volatile u8 reg_value;
//		SPI_Soft_WriteRegister(0x03, 0x01);
//		reg_value = SPI_Soft_ReadRegister(0x03);
//		
//		while(1);
//	}

//}


void SPI_WriteReg(uint8_t Address, uint8_t Value)
{
    uint8_t data[2];
    data[0] = (Address << 1); // 写操作：地址左移 1
    data[1] = Value;

    FSV9563_NSS_0;
    HAL_SPI_Transmit(&hspi1, data, 2, HAL_MAX_DELAY);
    FSV9563_NSS_1;
}

uint8_t SPI_ReadReg(uint8_t Address)
{
    uint8_t txData[2];  
    uint8_t rxData[2];  

    txData[0] = (Address << 1) | 0x01;  // 读指令（地址左移 1 并 OR 0x01）
    txData[1] = 0x00;  // 发送一个 Dummy 字节

    FSV9563_NSS_0;

    HAL_SPI_TransmitReceive(&hspi1, txData, rxData, 2, HAL_MAX_DELAY);  

    FSV9563_NSS_1;

    return rxData[1];  // 返回收到的数据
}



int main(void)
{

  //__HAL_RCC_WWDG_CLK_DISABLE();
  HAL_Init();

  SystemClock_Config();
  
  MX_GPIO_Init();
  MX_CAN_Init();
  MX_SPI1_Init();
  /* USER CODE BEGIN 2 */

  /* USER CODE END 2 */
	if (!(SPI1->CR1 & SPI_CR1_SPE)) 
	{
		SPI1->CR1 |= SPI_CR1_SPE;
		LED_1;
	}

  while(1)
  {
	  LED_1;
	  volatile u8 reg_value;
	  SPI_WriteReg(0x14, 0x98);
	  reg_value = SPI_ReadReg(0x14);

	  while(1);
  }

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
//  while (1)
//  {
//	volatile u8 reg_value;
//	FSV9563_WriteReg(0x11,0x56);
//		
//	reg_value = FSV9563_ReadReg(0x11);  //46  //46  //46
//    /* USER CODE END WHILE */
//	FSV9563_WriteReg(0x11,0x78);
//	reg_value = FSV9563_ReadReg(0x11);
//	LED_0;
//	FSV9563_ISO15693();
//    /* USER CODE BEGIN 3 */
//  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI48;
  RCC_OscInitStruct.HSI48State = RCC_HSI48_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI48;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
