/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define B1_Pin GPIO_PIN_13
#define B1_GPIO_Port GPIOC
#define B1_EXTI_IRQn EXTI15_10_IRQn
#define SERV_PWM_OUT_Pin GPIO_PIN_0
#define SERV_PWM_OUT_GPIO_Port GPIOA
#define LED_BUILTIN_Pin GPIO_PIN_5
#define LED_BUILTIN_GPIO_Port GPIOA
#define TIM3_DC_MOTOR_Pin GPIO_PIN_6
#define TIM3_DC_MOTOR_GPIO_Port GPIOA
#define TIM3_DC_MOTOR_REV_Pin GPIO_PIN_7
#define TIM3_DC_MOTOR_REV_GPIO_Port GPIOA
#define RPM_TICK_Pin GPIO_PIN_2
#define RPM_TICK_GPIO_Port GPIOB
#define RPM_TICK_EXTI_IRQn EXTI2_IRQn
#define DIGIT_B3_Pin GPIO_PIN_10
#define DIGIT_B3_GPIO_Port GPIOB
#define RED_Pin GPIO_PIN_8
#define RED_GPIO_Port GPIOA
#define HCSR04_TRIG_Pin GPIO_PIN_10
#define HCSR04_TRIG_GPIO_Port GPIOA
#define GRN_Pin GPIO_PIN_11
#define GRN_GPIO_Port GPIOA
#define BLU_Pin GPIO_PIN_12
#define BLU_GPIO_Port GPIOA
#define TMS_Pin GPIO_PIN_13
#define TMS_GPIO_Port GPIOA
#define TCK_Pin GPIO_PIN_14
#define TCK_GPIO_Port GPIOA
#define DIGIT_A0_Pin GPIO_PIN_3
#define DIGIT_A0_GPIO_Port GPIOB
#define DIGIT_A1_Pin GPIO_PIN_4
#define DIGIT_A1_GPIO_Port GPIOB
#define DIGIT_A2_Pin GPIO_PIN_5
#define DIGIT_A2_GPIO_Port GPIOB
#define DIGIT_A3_Pin GPIO_PIN_6
#define DIGIT_A3_GPIO_Port GPIOB
#define DIGIT_B0_Pin GPIO_PIN_7
#define DIGIT_B0_GPIO_Port GPIOB
#define DIGIT_B1_Pin GPIO_PIN_8
#define DIGIT_B1_GPIO_Port GPIOB
#define DIGIT_B2_Pin GPIO_PIN_9
#define DIGIT_B2_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
