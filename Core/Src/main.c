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

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <string.h>
#include "stm32g0xx_hal_flash.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
#define APP_ADDRESS  0x0800C000
#define CHUNK_SIZE               512
#define FLASH_APP_START_ADDRESS   0x0800C000U   // New app start address
//#define FLASH_PAGE_SIZE           0x800U        // 2KB per page on STM32G071
#define NUM_PAGES_TO_ERASE        10            // Erase 10 pages (20 KB)

uint32_t chunk_index = 0;
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */
uint8_t Ota_Buffer[1500];
uint32_t Ota_Length = 0;
typedef void (*pFunction)(void);
volatile uint8_t jump_to_app=0;
void jump_to_application(void);
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);
/* USER CODE BEGIN PFP */

// ---------- Send ACK ----------
void bootloader_send_ack()
{
	uint8_t Ack[]={0xF7,0xFE,0xF7,0xFE};
	HAL_UART_Transmit(&huart2, Ack, 4, 1000);
	HAL_Delay(100);
}
// ---------- Send NACK ----------
void bootloader_send_Nack()
{
	uint8_t Nack[]={0xFF,0xFF,0xFF,0xFF};
	HAL_UART_Transmit(&huart2, Nack, 4, 1000);
	HAL_Delay(100);
}
// ---------- Flash Write One 512-Byte Chunk ----------
HAL_StatusTypeDef write_flash_chunk(uint32_t address, uint8_t* data)
{
    HAL_FLASH_Unlock();

    for (uint32_t i = 0; i < CHUNK_SIZE; i += 8)
    {
        uint64_t doubleWord;
        memcpy(&doubleWord, &data[i], 8);

        if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, address + i, doubleWord) != HAL_OK)
        {
 //       	bootloader_send_Nack();
            HAL_FLASH_Lock();
            return HAL_ERROR;
        }
    }
//    bootloader_send_ack();
    HAL_FLASH_Lock();
    return HAL_OK;
}

// ---------- Flash All Received Chunks ----------
HAL_StatusTypeDef write_ota_data_to_flash(void)
{
    HAL_StatusTypeDef status;
    uint32_t flash_addr = FLASH_APP_START_ADDRESS;
    uint32_t offset = 0;
    uint8_t chunk[CHUNK_SIZE];

    while (offset < Ota_Length)
    {
        uint32_t copy_len = (Ota_Length - offset >= CHUNK_SIZE) ? CHUNK_SIZE : (Ota_Length - offset);
        memset(chunk, 0xFF, CHUNK_SIZE);
        memcpy(chunk, &Ota_Buffer[offset], copy_len);

        status = write_flash_chunk(flash_addr, chunk);
        if (status != HAL_OK)
        {
        	return status;
        }

        flash_addr += CHUNK_SIZE;
        offset += CHUNK_SIZE;
    }

    return HAL_OK;
}
HAL_StatusTypeDef erase_application_flash(void)
{
    HAL_StatusTypeDef status;
    FLASH_EraseInitTypeDef eraseInit;
    uint32_t pageError = 0;

    uint32_t page = (FLASH_APP_START_ADDRESS - FLASH_BASE) / FLASH_PAGE_SIZE;

    HAL_FLASH_Unlock();

    eraseInit.TypeErase = FLASH_TYPEERASE_PAGES;
    eraseInit.Page      = page;
    eraseInit.NbPages   = NUM_PAGES_TO_ERASE;

    status = HAL_FLASHEx_Erase(&eraseInit, &pageError);

    HAL_FLASH_Lock();

    return status;
}
void jump_to_application(void)
{

	__disable_irq();
//	for (int i = 0; i < 8; i++) {
//		NVIC->ICER[i] = 0xFFFFFFFF;
//		NVIC->ICPR[i] = 0xFFFFFFFF;
//	}

	SysTick->CTRL = 0;
	SysTick->LOAD = 0;
	SysTick->VAL  = 0;

	HAL_UART_DeInit(&huart2);
	HAL_DeInit();
	HAL_RCC_DeInit();

	uint32_t appStack = *(__IO uint32_t*)APP_ADDRESS;
	uint32_t appResetHandler = *(__IO uint32_t*)(APP_ADDRESS + 4);
	pFunction appEntry = (pFunction)appResetHandler;

	__set_MSP(appStack);
	appEntry();
}
void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{
    if (huart->Instance == USART2)
    {
	    if(Ota_Buffer[0]==0xFF && Ota_Buffer[1]==0xF4 && Ota_Buffer[2]==0xF3)
	    {
	    	if (erase_application_flash() == HAL_OK)
	    	{
	    	    HAL_UART_Transmit(&huart2, (uint8_t*)"Flash erase success\r\n", 25, 100);
	    	}
	    	else
	    	{
	    	    HAL_UART_Transmit(&huart2, (uint8_t*)"Flash erase failed\r\n", 24, 100);
	    	}
	    }
	    else
	    {
			if (Size < CHUNK_SIZE)
			{
				memset(&Ota_Buffer[Size], 0xFF, CHUNK_SIZE - Size);
			}

			uint32_t flash_addr = FLASH_APP_START_ADDRESS + (chunk_index * CHUNK_SIZE);

			if (write_flash_chunk(flash_addr, Ota_Buffer) == HAL_OK)
			{
				//HAL_UART_Transmit(&huart2, (uint8_t*)" Chunk written\r\n", 18, 100);
				chunk_index++;
			}
			else
			{
				//HAL_UART_Transmit(&huart2, (uint8_t*)"Flash write failed\r\n", 24, 100);
			}
	    }

        HAL_UARTEx_ReceiveToIdle_IT(&huart2, Ota_Buffer, sizeof(Ota_Buffer));
    }
}

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void HAL_GPIO_EXTI_Rising_Callback(uint16_t GPIO_Pin)
{

	if(HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_13))
	{
		jump_to_app=1;
	}

}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

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
  MX_USART2_UART_Init();
  /* USER CODE BEGIN 2 */
  HAL_UARTEx_ReceiveToIdle_IT(&huart2,(uint8_t*)Ota_Buffer,sizeof(Ota_Buffer));
  HAL_UART_Transmit(&huart2,(uint8_t*)"Bootloader\r\n", strlen("Bootloader\r\n"), 1000);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	 // HAL_UART_Transmit(&huart2,(uint8_t*)"Bootloader\r\n", strlen("Bootloader\r\n"), 1000);
	    if (jump_to_app)
	    {
	        HAL_UART_Transmit(&huart2, (uint8_t*)"Jumping To App\r\n", strlen("Jumping To App\r\n"), 1000);
	        HAL_Delay(100);
	        jump_to_app = 0;

	        jump_to_application();
	    }

	  HAL_Delay(1000);
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
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

  /** Configure the main internal regulator output voltage
  */
  HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSIDiv = RCC_HSI_DIV1;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = RCC_PLLM_DIV1;
  RCC_OscInitStruct.PLL.PLLN = 8;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  huart2.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart2.Init.ClockPrescaler = UART_PRESCALER_DIV1;
  huart2.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetTxFifoThreshold(&huart2, UART_TXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetRxFifoThreshold(&huart2, UART_RXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_DisableFifoMode(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();

  /*Configure GPIO pin : PC13 */
  GPIO_InitStruct.Pin = GPIO_PIN_13;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI4_15_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI4_15_IRQn);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
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
