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
#define BUFFER_SIZE 512
#define SLOTS_PER_REVOLUTION 20  // Adjust this to match your slotted wheel
#define SAMPLE_PERIOD_MS 1000    // Sample over 1 second
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc1;

TIM_HandleTypeDef htim1;
TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim3;
TIM_HandleTypeDef htim4;
TIM_HandleTypeDef htim5;

UART_HandleTypeDef huart6;

/* USER CODE BEGIN PV */
char buffer[BUFFER_SIZE];
volatile int buffer_index = 0;
uint8_t rx_data;
volatile uint8_t rcv_intpt_flag = 0;
volatile int exit_while_loop = 0;

volatile int inlet_pwm, zone1_pwm, zone2_pwm, zone3_pwm;
volatile int inlet_start, inlet_stop, zone1_start, zone1_stop, zone2_start, zone2_stop, zone3_start, zone3_stop;
volatile uint32_t g_rpm_tick_count = 0;
volatile int g_water_depth = 0;
volatile int g_adc_value = 0;
volatile uint32_t g_last_rpm_calc_time = 0;
volatile uint32_t g_current_rpm = 0;

// Clock variables
volatile int hours = 0;
volatile int minutes = 0;
volatile int seconds = 0;
volatile int real_seconds = 0;
char txd_msg_buffer[256];
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART6_UART_Init(void);
static void MX_ADC1_Init(void);
static void MX_TIM3_Init(void);
static void MX_TIM2_Init(void);
static void MX_TIM5_Init(void);
static void MX_TIM1_Init(void);
static void MX_TIM4_Init(void);
/* USER CODE BEGIN PFP */
void handle_setup_mode(void);
void prompt_and_receive(const char *prompt, int *value);
void display_value_on_timer_board(int value);
uint8_t get_adc_value(void);
int update_current_water_reservoir_depth(void);


/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */


void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
  if (GPIO_Pin == B1_Pin) {
    exit_while_loop = 1;
  }
  else if (GPIO_Pin == RPM_TICK_Pin) {
    // increment the rpm value
    g_rpm_tick_count++;

    // if real_seconds is 1 second, calculate the rpm
    if (g_rpm_tick_count == UINT32_MAX) {
      g_rpm_tick_count = 0;
    }
  }
}

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
  int current_time;

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

  snprintf(response, BUFFER_SIZE, "Configuration Completed:\r\nInlet PWM: %d\r\nZone 1 PWM: %d\r\nZone 2 PWM: %d\r\nZone 3 PWM: %d\r\nCurrent Time: %d\r\nInlet Start: %d\r\nInlet Stop: %d\r\nZone 1 Start: %d\r\nZone 1 Stop: %d\r\nZone 2 Start: %d\r\nZone 2 Stop: %d\r\nZone 3 Start: %d\r\nZone 3 Stop: %d\r\n",
           inlet_pwm, zone1_pwm, zone2_pwm, zone3_pwm, current_time, inlet_start, inlet_stop, zone1_start, zone1_stop, zone2_start, zone2_stop, zone3_start, zone3_stop);
  HAL_UART_Transmit(&huart6, (uint8_t *)response, strlen(response), HAL_MAX_DELAY);
}

void prompt_and_receive(const char *prompt, int *value)
{
  char response[BUFFER_SIZE];

  HAL_UART_Transmit(&huart6, (uint8_t *)prompt, strlen(prompt), HAL_MAX_DELAY);
  rcv_intpt_flag = 0;
  while (!rcv_intpt_flag); // Wait for input
  sscanf(buffer, "%d", value); // load value with the inputted characters
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  if (htim->Instance == TIM4) {
    real_seconds++;

    //global update vars adc, rpm
    g_adc_value = get_adc_value();
    uint32_t current_time = HAL_GetTick();
    if (current_time - g_last_rpm_calc_time >= SAMPLE_PERIOD_MS)
    {
        // Calculate RPM
        g_current_rpm = (g_rpm_tick_count * 60 * 1000) / (SLOTS_PER_REVOLUTION * SAMPLE_PERIOD_MS);
        g_rpm_tick_count = 0;  // Reset the count for the next sample period
        g_last_rpm_calc_time = current_time;
    }


    seconds += 600;
    if (seconds >= 60) {
      minutes += seconds / 60;
      seconds %= 60;
      if (minutes >= 60) {
        hours += minutes / 60;
        minutes %= 60;
        if (hours >= 24) {
          hours %= 24;
        }
        // Update display every hour
        update_display();
      }
    }
    //display_value_on_timer_board(g_water_depth);
  }
}

int g_first_edge = 0;
int g_time_edge1 = 0;
int g_time_edge2 = 0;
int g_time_diff = 0;
int g_hcsr04_Rx_flag = 0;
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{

    if (htim->Instance == TIM2)
    {
        if (htim->Channel == 4)
        {
            // if (g_first_edge == 0)  // if the first value is not captured
            // {
            //     g_time_edge1 = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_4);  // read the first value
            //     g_first_edge = 1;  // set the first captured as true
            // }
            // else  // if the first is already captured
            // {
            //     g_time_edge2 = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_4);  // read second value
            //     __HAL_TIM_SET_COUNTER(htim, 0);  // reset the counter
            //     g_hcsr04_Rx_flag = 1;  // set the interrupt flag for result done

            //     g_time_diff = g_time_edge1 - g_time_edge2;

            //     /* enter below your distance calculation here that uses HCSR04 datasheet information */
            //     g_water_depth = g_time_diff/580;
            //     //g_water_depth = g_water_depth / 10;
            // }
            HAL_UART_Transmit(&huart6, "AAA", 3, 1000);

        }
    }
}

// Add this new function
void update_display(void)
{
  char active_zone[10] = "None";
  int active_pwm = 0;
  int motor_rpm = 0;

  // Determine active zone and PWM
  if (hours >= inlet_start && hours < inlet_stop) {
    strcpy(active_zone, "Inlet");
    active_pwm = inlet_pwm;
  } else if (hours >= zone1_start && hours < zone1_stop) {
    strcpy(active_zone, "Zone 1");
    active_pwm = zone1_pwm;
  } else if (hours >= zone2_start && hours < zone2_stop) {
    strcpy(active_zone, "Zone 2");
    active_pwm = zone2_pwm;
  } else if (hours >= zone3_start && hours < zone3_stop) {
    strcpy(active_zone, "Zone 3");
    active_pwm = zone3_pwm;
  }

  // Determine motor pwm%
  int pwm_percent = 0;


  switch(active_pwm) {
    case 0: pwm_percent = (g_adc_value/256.0)*100; break;
    case 1: pwm_percent = 60; break;
    case 2: pwm_percent = 80; break;
    case 3: pwm_percent = 99; break;
  }


  // motor rpm assumed to be updated

  // depth assumed to be gotten
//  update_current_water_reservoir_depth();

  // Format and send the display update
  sprintf(txd_msg_buffer, "%d:00 | %s | %d%% | %d | %d%%",
          hours, active_zone, pwm_percent, g_current_rpm, g_water_depth);
  HAL_UART_Transmit(&huart6, (uint8_t*)txd_msg_buffer, strlen(txd_msg_buffer), 1000);
  HAL_UART_Transmit(&huart6, "\n", 1, 1000);
}

uint8_t get_adc_value() {
    HAL_ADC_Start(&hadc1);
    // Poll for the end of the conversion
    HAL_ADC_PollForConversion(&hadc1, 1000);
    // Get the converted value
    uint8_t ADC_CH9 = HAL_ADC_GetValue(&hadc1);
    // Stop the ADC
    HAL_ADC_Stop(&hadc1);
    // Return the ADC value
    return ADC_CH9;
}

int update_current_water_reservoir_depth(void)
{
  // Simulate the water reservoir depth value; replace this with actual sensor reading if available
  uint32_t local_time = 0;
  uint32_t sensor_time = 0;
  uint32_t distance = 0;

  // Trigger the sensor by sending a 10us pulse
  HAL_GPIO_WritePin(HCSR04_TRIG_GPIO_Port, HCSR04_TRIG_Pin, GPIO_PIN_SET);
  for (int j = 0; j != 100; j++) {};
  HAL_GPIO_WritePin(HCSR04_TRIG_GPIO_Port, HCSR04_TRIG_Pin, GPIO_PIN_RESET);

  // Wait for the sensor to finish reading
  while (!g_hcsr04_Rx_flag) {
    //HAL_UART_Transmit(&huart6, "lol", 3, 1000);
  };

    sprintf(txd_msg_buffer, "Water Depth: %d\n", g_water_depth);
    HAL_UART_Transmit(&huart6, (uint8_t*)txd_msg_buffer, strlen(txd_msg_buffer), 1000);


  return g_water_depth;
}

void display_value_on_timer_board(int value)
{
    /* Legend
    A is tens digit
    B is ones digit

    B3 is MSb, ie writing 8 = 1000[B3,B2,B1,B0]
    A3 is MSb, ie writing 8 = 1000[A3,A2,A1,A0]
    */
    if (value < 0 || value > 99) {
        // Invalid value, do nothing
        return;
    }

    int tens = value / 10;
    int ones = value % 10;

    // Write the tens digit to the first set of digit pins (DIGIT_A)
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_3, (tens & 0x01) ? GPIO_PIN_SET : GPIO_PIN_RESET);  // DIGIT_A0 -> PB3
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_4, (tens & 0x02) ? GPIO_PIN_SET : GPIO_PIN_RESET);  // DIGIT_A1 -> PB4
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, (tens & 0x04) ? GPIO_PIN_SET : GPIO_PIN_RESET);  // DIGIT_A2 -> PB5
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, (tens & 0x08) ? GPIO_PIN_SET : GPIO_PIN_RESET);  // DIGIT_A3 -> PB6

    // Write the ones digit to the second set of digit pins (DIGIT_B)
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, (ones & 0x01) ? GPIO_PIN_SET : GPIO_PIN_RESET);  // DIGIT_B0 -> PB7
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8, (ones & 0x02) ? GPIO_PIN_SET : GPIO_PIN_RESET);  // DIGIT_B1 -> PB8
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_9, (ones & 0x04) ? GPIO_PIN_SET : GPIO_PIN_RESET);  // DIGIT_B2 -> PB9
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_10, (ones & 0x08) ? GPIO_PIN_SET : GPIO_PIN_RESET); // DIGIT_B3 -> PB10
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */
  uint8_t timer_board_ones, timer_board_tens;
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
  MX_ADC1_Init();
  MX_TIM3_Init();
  MX_TIM2_Init();
  MX_TIM5_Init();
  MX_TIM1_Init();
  MX_TIM4_Init();
  /* USER CODE BEGIN 2 */
  // Turn off the green LED (PA5) at startup
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);

  // clear the uart terminal
  const char clearScreen[] = "\033[2J\033[H";
  HAL_UART_Transmit(&huart6, (uint8_t*)clearScreen, strlen(clearScreen), HAL_MAX_DELAY);

  // Start receiving data via interrupt
  HAL_UART_Receive_IT(&huart6, &rx_data, 1);

  // Indicate entering setup mode
  handle_setup_mode();


  // Turn on the green LED after setup mode
  while (1)
  {
    // Toggle the green LED
    HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);

    // Wait for 200ms
    HAL_Delay(200);

    // Check if the external interrupt from pin C13 (the blue Nucleo push button) is triggered
    if (exit_while_loop)
    {

      // Break out of the while loop
      break;
    }
  }

  /* USER CODE BEGIN 2 */

  // Start the timer for the clock counter, depth sensor
  HAL_TIM_Base_Start_IT(&htim4);
  HAL_TIM_Base_Start_IT(&htim2);



  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
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
  * @brief ADC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC1_Init(void)
{

  /* USER CODE BEGIN ADC1_Init 0 */

  /* USER CODE END ADC1_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC1_Init 1 */

  /* USER CODE END ADC1_Init 1 */

  /** Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion)
  */
  hadc1.Instance = ADC1;
  hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV2;
  hadc1.Init.Resolution = ADC_RESOLUTION_8B;
  hadc1.Init.ScanConvMode = ENABLE;
  hadc1.Init.ContinuousConvMode = DISABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 1;
  hadc1.Init.DMAContinuousRequests = DISABLE;
  hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
  */
  sConfig.Channel = ADC_CHANNEL_1;
  sConfig.Rank = 1;
  sConfig.SamplingTime = ADC_SAMPLETIME_15CYCLES;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */

}

/**
  * @brief TIM1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM1_Init(void)
{

  /* USER CODE BEGIN TIM1_Init 0 */

  /* USER CODE END TIM1_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM1_Init 1 */

  /* USER CODE END TIM1_Init 1 */
  htim1.Instance = TIM1;
  htim1.Init.Prescaler = 16000-1;
  htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim1.Init.Period = 500;
  htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim1.Init.RepetitionCounter = 0;
  htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  if (HAL_TIM_Base_Init(&htim1) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim1, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM1_Init 2 */

  /* USER CODE END TIM1_Init 2 */

}

/**
  * @brief TIM2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM2_Init(void)
{

  /* USER CODE BEGIN TIM2_Init 0 */

  /* USER CODE END TIM2_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_IC_InitTypeDef sConfigIC = {0};

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 16-1;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 20000-1;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_IC_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigIC.ICPolarity = TIM_INPUTCHANNELPOLARITY_BOTHEDGE;
  sConfigIC.ICSelection = TIM_ICSELECTION_DIRECTTI;
  sConfigIC.ICPrescaler = TIM_ICPSC_DIV1;
  sConfigIC.ICFilter = 0;
  if (HAL_TIM_IC_ConfigChannel(&htim2, &sConfigIC, TIM_CHANNEL_4) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM2_Init 2 */

  /* USER CODE END TIM2_Init 2 */

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
  htim3.Init.Period = 2000-1;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
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
  * @brief TIM4 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM4_Init(void)
{

  /* USER CODE BEGIN TIM4_Init 0 */

  /* USER CODE END TIM4_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM4_Init 1 */

  /* USER CODE END TIM4_Init 1 */
  htim4.Instance = TIM4;
  htim4.Init.Prescaler = 16000-1;
  htim4.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim4.Init.Period = 1000-1;
  htim4.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim4.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  if (HAL_TIM_Base_Init(&htim4) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim4, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim4, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM4_Init 2 */

  /* USER CODE END TIM4_Init 2 */

}

/**
  * @brief TIM5 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM5_Init(void)
{

  /* USER CODE BEGIN TIM5_Init 0 */

  /* USER CODE END TIM5_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM5_Init 1 */

  /* USER CODE END TIM5_Init 1 */
  htim5.Instance = TIM5;
  htim5.Init.Prescaler = 16-1;
  htim5.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim5.Init.Period = 20000-1;
  htim5.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim5.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  if (HAL_TIM_Base_Init(&htim5) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim5, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim5) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim5, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 500-1;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim5, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM5_Init 2 */

  /* USER CODE END TIM5_Init 2 */
  HAL_TIM_MspPostInit(&htim5);

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
  HAL_GPIO_WritePin(GPIOA, HCSR04_TRIG_Pin|LED_BUILTIN_Pin|RED_Pin|GRN_Pin
                          |BLU_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, DIGIT_B3_Pin|DIGIT_A0_Pin|DIGIT_A1_Pin|DIGIT_A2_Pin
                          |DIGIT_A3_Pin|DIGIT_B0_Pin|DIGIT_B1_Pin|DIGIT_B2_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : B1_Pin */
  GPIO_InitStruct.Pin = B1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : HCSR04_TRIG_Pin LED_BUILTIN_Pin RED_Pin GRN_Pin
                           BLU_Pin */
  GPIO_InitStruct.Pin = HCSR04_TRIG_Pin|LED_BUILTIN_Pin|RED_Pin|GRN_Pin
                          |BLU_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : RPM_TICK_Pin */
  GPIO_InitStruct.Pin = RPM_TICK_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(RPM_TICK_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : DIGIT_B3_Pin DIGIT_A0_Pin DIGIT_A1_Pin DIGIT_A2_Pin
                           DIGIT_A3_Pin DIGIT_B0_Pin DIGIT_B1_Pin DIGIT_B2_Pin */
  GPIO_InitStruct.Pin = DIGIT_B3_Pin|DIGIT_A0_Pin|DIGIT_A1_Pin|DIGIT_A2_Pin
                          |DIGIT_A3_Pin|DIGIT_B0_Pin|DIGIT_B1_Pin|DIGIT_B2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI2_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI2_IRQn);

  HAL_NVIC_SetPriority(EXTI15_10_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

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
