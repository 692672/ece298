/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
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

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define BUFFER_SIZE 128
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
TIM_HandleTypeDef htim3;

UART_HandleTypeDef huart6;

/* USER CODE BEGIN PV */
char buffer[BUFFER_SIZE];
volatile int buffer_index = 0;
uint8_t rx_data;
volatile uint8_t rcv_intpt_flag = 0;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART6_UART_Init(void);
static void MX_TIM3_Init(void);
/* USER CODE BEGIN PFP */
void handle_setup_mode(void);
void prompt_and_receive(const char *prompt, int *value);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
  if (huart->Instance == USART6)
  {
    if (rx_data == '\r')
    {
      buffer[buffer_index] = '\0';
      rcv_intpt_flag = 1;
      buffer_index = 0; // Reset buffer index for next input
      HAL_UART_Transmit(&huart6, (uint8_t *)"\r\n", 2, HAL_MAX_DELAY); // Echo newline
    }
    else
    {
      buffer[buffer_index++] = rx_data;
      HAL_UART_Transmit(&huart6, &rx_data, 1, HAL_MAX_DELAY); // Echo the received character
    }
    HAL_UART_Receive_IT(&huart6, &rx_data, 1);
  }
}

void handle_setup_mode(void)
{
  char response[BUFFER_SIZE];
  int inlet_pwm, zone1_pwm, zone2_pwm, zone3_pwm;
  int current_time, inlet_start, inlet_stop, zone1_start, zone1_stop, zone2_start, zone2_stop, zone3_start, zone3_stop;

  snprintf(response, BUFFER_SIZE, "SETUP MODE\r\n");
  HAL_UART_Transmit(&huart6, (uint8_t *)response, strlen(response), HAL_MAX_DELAY);
  snprintf(response, BUFFER_SIZE, "0 - MANUAL, 1 - 60% PWM, 2 - 80% PWM, 3 - 99% PWM\r\n");
    HAL_UART_Transmit(&huart6, (uint8_t *)response, strlen(response), HAL_MAX_DELAY);

  	prompt_and_receive("Enter Inlet Motor Speed PWM (0-3): ", &inlet_pwm);
	prompt_and_receive("Enter Zone 1 Motor Speed PWM (0-3): ", &zone1_pwm);
	prompt_and_receive("Enter Zone 2 Motor Speed PWM (0-3): ", &zone2_pwm);
	prompt_and_receive("Enter Zone 3 Motor Speed PWM (0-3): ", &zone3_pwm);

	snprintf(response, BUFFER_SIZE, "\n0 - 12AM, 1 - 1AM, ... , 22 - 10PM, 23 - 11PM\r\n");
	    HAL_UART_Transmit(&huart6, (uint8_t *)response, strlen(response), HAL_MAX_DELAY);
	prompt_and_receive("Enter Current Wall Clock Time (00-23): ", &current_time);
	prompt_and_receive("Enter Inlet Wall Clock Start Time (00-23): ", &inlet_start);
	prompt_and_receive("Enter Inlet Wall Clock Stop Time (00-23): ", &inlet_stop);
	prompt_and_receive("Enter Zone 1 Wall Clock Start Time (00-23): ", &zone1_start);
	prompt_and_receive("Enter Zone 1 Wall Clock Stop Time (00-23): ", &zone1_stop);
	prompt_and_receive("Enter Zone 2 Wall Clock Start Time (00-23): ", &zone2_start);
	prompt_and_receive("Enter Zone 2 Wall Clock Stop Time (00-23): ", &zone2_stop);
	prompt_and_receive("Enter Zone 3 Wall Clock Start Time (00-23): ", &zone3_start);
	prompt_and_receive("Enter Zone 3 Wall Clock Stop Time (00-23): ", &zone3_stop);

//  snprintf(response, BUFFER_SIZE, "Configuration Completed:\r\nInlet PWM: %d\r\nZone 1 PWM: %d\r\nZone 2 PWM: %d\r\nZone 3 PWM: %d\r\nCurrent Time: %d\r\nInlet Start: %d\r\nInlet Stop: %d\r\nZone 1 Start: %d\r\nZone 1 Stop: %d\r\nZone 2 Start: %d\r\nZone 2 Stop: %d\r\nZone 3 Start: %d\r\nZone 3 Stop: %d\r\n",
//           inlet_pwm, zone1_pwm, zone2_pwm, zone3_pwm, current_time, inlet_start, inlet_stop, zone1_start, zone1_stop, zone2_start, zone2_stop, zone3_start, zone3_stop);
//  HAL_UART_Transmit(&huart6, (uint8_t *)response, strlen(response), HAL_MAX_DELAY);
}

void prompt_and_receive(const char *prompt, int *value)
{
  char response[BUFFER_SIZE];

  HAL_UART_Transmit(&huart6, (uint8_t *)prompt, strlen(prompt), HAL_MAX_DELAY);
  rcv_intpt_flag = 0;
  while (!rcv_intpt_flag); // Wait for input
  sscanf(buffer, "%d", value);
//  snprintf(response, BUFFER_SIZE, "Received: %d\r\n", *value);
//  HAL_UART_Transmit(&huart6, (uint8_t *)response, strlen(response), HAL_MAX_DELAY);
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
  MX_USART6_UART_Init();
  MX_TIM3_Init();
  /* USER CODE BEGIN 2 */

  // Turn off the green LED (PA5) at startup
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);

  // clear the terminal
  const char clear_screen[] = "\x1b[2J\x1b[H";
  HAL_UART_Transmit(&huart6, (uint8_t *)clear_screen, strlen(clear_screen), HAL_MAX_DELAY);

  // Start receiving data via interrupt
  HAL_UART_Receive_IT(&huart6, &rx_data, 1);

  // Indicate entering setup mode
  handle_setup_mode();

  // Flash led green after recieved all input

  // wait for blue button to be

//  while (!b1_pressed) {
//	  HAL_Delay(100);
//  }


  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    // Main loop does nothing, everything is handled in interrupt
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
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief TIM3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM3_Init(void)
{

  /* USER CODE BEGIN TIM3_Init 0 */

  /* USER CODE END TIM3_Init 0 */

  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM3_Init 1 */

  /* USER CODE END TIM3_Init 1 */
  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 16-1;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = 65535;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_PWM_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM3_Init 2 */

  /* USER CODE END TIM3_Init 2 */
  HAL_TIM_MspPostInit(&htim3);

}

/**
  * @brief USART6 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART6_UART_Init(void)
{

  /* USER CODE BEGIN USART6_Init 0 */

  /* USER CODE END USART6_Init 0 */

  /* USER CODE BEGIN USART6_Init 1 */

  /* USER CODE END USART6_Init 1 */
  huart6.Instance = USART6;
  huart6.Init.BaudRate = 9600;
  huart6.Init.WordLength = UART_WORDLENGTH_8B;
  huart6.Init.StopBits = UART_STOPBITS_1;
  huart6.Init.Parity = UART_PARITY_NONE;
  huart6.Init.Mode = UART_MODE_TX_RX;
  huart6.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart6.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart6) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART6_Init 2 */

  /* USER CODE END USART6_Init 2 */

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
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : B1_Pin */
  GPIO_InitStruct.Pin = B1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : PA1 */
  GPIO_InitStruct.Pin = GPIO_PIN_1;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : USART_TX_Pin USART_RX_Pin */
  GPIO_InitStruct.Pin = USART_TX_Pin|USART_RX_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF7_USART2;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : LD2_Pin */
  GPIO_InitStruct.Pin = LD2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LD2_GPIO_Port, &GPIO_InitStruct);

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
