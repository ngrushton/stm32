/**
  ****************************************************************************** 
  * 
  * SOUNDSENSING BOOTLOADER
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "system_jump.h"
#include <stdio.h>

/* Private variables ---------------------------------------------------------*/
UART_HandleTypeDef huart2;
uint32_t ActiveApp;

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_USART2_UART_Init(void);

uint64_t ReadActiveAppAddress();
void SaveActiveAppAddress(uint64_t activeAppAddress);
static void Jump_To_App();

char print_out[64];

#define APP_ADDRESS 0x080FEFE0

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  HAL_Init();

  // SCB->VTOR = 0x08000000;
  // __enable_irq();

  SystemClock_Config();
  MX_USART2_UART_Init();

  // SaveActiveAppAddress(0x08008000);
  // SaveActiveAppAddress(0x08080000);
  HAL_UART_Transmit(&huart2, (uint8_t*)"In bootloader...\r\n", 18, 1000);

  sprintf(print_out, "%ld\r\n", *((unsigned long *)0x20017FF0));

  HAL_UART_Transmit(&huart2, (uint8_t*)print_out, 10, 1000);

  SaveActiveAppAddress(0x08008000);

  // uint64_t activeAppAddress = ReadActiveAppAddress();
  // Jump_To_App();
}


/**
  * @brief  Jump_To_USB_DFU()
  *         On reset, the system jumps to the SystemInit(void) function in system_stm32l4xx.c.
	*         If the trigger is set to 0xFFFFFFFF the system jumps to the the DFU bootloader.
	*         If not, then the system starts up normally.
  * @param  None
  * @retval None
  */
static void Jump_To_App() {
	*((unsigned long *)0x20017FF0) = 0x08008000; // Set SRAM location to firmware trigger - 0xFFFFFFFF
  NVIC_SystemReset(); // Reset the system - go to system_stm32l4xx.c SystemInit()
}


/**
  * @brief  SaveActiveAppAddress
  *         Save the app address to jump to after system reset
  * @param  activeAppAddress
  * @retval None
  */
void SaveActiveAppAddress(uint64_t activeAppAddress) {

    volatile HAL_StatusTypeDef flashStatus = HAL_OK;

    FLASH_EraseInitTypeDef EraseInitStruct;
    EraseInitStruct.TypeErase   = FLASH_TYPEERASE_PAGES;
    EraseInitStruct.Banks       = FLASH_BANK_2;
    EraseInitStruct.Page        = 253;
    EraseInitStruct.NbPages     = 1;

    /* Unlock the Flash to enable the flash control register access *************/
    HAL_FLASH_Unlock();
    uint32_t PageError;
    flashStatus = HAL_FLASHEx_Erase(&EraseInitStruct, &PageError);
    HAL_UART_Transmit(&huart2, (uint8_t*)"App Address Erased\r\n", 20, 100);

    HAL_UART_Transmit(&huart2, (uint8_t*)"Flashing\r\n", 10, 100);

    // uint64_t flashData = (((uint64_t) 0x00000000) << 32) + ((uint64_t) activeAppAddress);

    if (flashStatus == HAL_OK) {
        flashStatus = HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, APP_ADDRESS, activeAppAddress);
    }
    HAL_FLASH_Lock();

    // return flashStatus;
}


/**
  * @brief  ReadActiveAppAddress
  *         Read the app address to jump to after system reset
  * @retval Return uint64_t app address value
  */
uint64_t ReadActiveAppAddress()
{
	volatile uint32_t read_data;
	uint32_t buffered[2];
	volatile uint32_t read_cnt = 0;
	int index = 0;
	do
	{
		read_data = *(uint32_t*)(APP_ADDRESS + read_cnt);

		if(read_data != 0xFFFFFFFF)
		{
			buffered[index] = read_data;
            
      char buffer[16];
      sprintf(buffer, "Add: %ld ", buffered[index]);
			HAL_UART_Transmit(&huart2, (uint8_t*)buffer, 10, 100);

			read_cnt += 4;
		}
		index++;
	} while(read_data != 0xFFFFFFFF);

  HAL_UART_Transmit(&huart2, (uint8_t*)"\n", 1, 100);

	uint64_t jammed = ((uint64_t)buffered[1]<<32) + (uint64_t)buffered[0];
	return jammed;
}


/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 1;
  RCC_OscInitStruct.PLL.PLLN = 10;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV7;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART2;
  PeriphClkInit.Usart2ClockSelection = RCC_USART2CLKSOURCE_PCLK1;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure the main internal regulator output voltage 
  */
  if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1) != HAL_OK)
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
  huart2.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

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
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
