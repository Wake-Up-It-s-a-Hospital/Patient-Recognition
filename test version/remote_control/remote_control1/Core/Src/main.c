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
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
// ?��?�� 버퍼 �??��
char uart_buf[8];
char re_data[8];
int x_data, y_data;
float x_calc, y_calc;
uint32_t PWM1_value, PWM2_value;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
// 블루?��?��(UART) ?��?�� ?��?��?��?��
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
	if (huart -> Instance == USART1) {
		//
		for (int i = 0; i < 8; i++) {
			if (uart_buf[(i + 7) % 8] == '.') {
				for (int j = 0; j < 8; j++) {
					re_data[j] = uart_buf[(i + j) % 8];
				}
			}
		}

		// ?��?�� �??��
		sscanf(re_data, "%d,%d.", &x_data, &y_data);
		if (y_data == 101)
			y_data = 100;

		x_data = x_data - 50;
		y_data = y_data - 50;

		// ?��?�� 차동 구동 코드
		if (x_data >= 0 && y_data >= 0) {
			x_calc = (float)x_data ;
			PWM1_value = (uint32_t)x_calc;
			y_calc = (float)x_data  * (100 - (float)y_data) / 100;
			PWM2_value = (uint32_t)y_calc;
			HAL_GPIO_WritePin(GPIOA, GPIO_PIN_11, 0);
			HAL_GPIO_WritePin(GPIOA, GPIO_PIN_12, 0);
			__HAL_TIM_SET_COMPARE(&htim3,TIM_CHANNEL_1, PWM2_value);
			__HAL_TIM_SET_COMPARE(&htim3,TIM_CHANNEL_2, PWM1_value);
		}
		else if (x_data >= 0 && y_data < 0) {
			x_calc = (float)x_data  * (100 + (float)y_data) / 100;
			PWM1_value = (uint32_t)x_calc;
			y_calc = (float)x_data ;
			PWM2_value = (uint32_t)y_calc;
			HAL_GPIO_WritePin(GPIOA, GPIO_PIN_11, 0);
			HAL_GPIO_WritePin(GPIOA, GPIO_PIN_12, 0);
			__HAL_TIM_SET_COMPARE(&htim3,TIM_CHANNEL_1, PWM2_value);
			__HAL_TIM_SET_COMPARE(&htim3,TIM_CHANNEL_2, PWM1_value);
		}
		else if (x_data < 0 && y_data >= 0) {
			x_calc = fabsf((float)x_data) ;
			PWM1_value = (uint32_t)x_calc;
			y_calc = fabsf((float)x_data)* (100 - (float)y_data) / 100;
			PWM2_value = (uint32_t)y_calc;
			HAL_GPIO_WritePin(GPIOA, GPIO_PIN_11, 1);
			HAL_GPIO_WritePin(GPIOA, GPIO_PIN_12, 1);
			__HAL_TIM_SET_COMPARE(&htim3,TIM_CHANNEL_1, PWM2_value);
			__HAL_TIM_SET_COMPARE(&htim3,TIM_CHANNEL_2, PWM1_value);
		}
		else if (x_data < 0 && y_data < 0) {
			x_calc = fabsf((float)x_data) * (100 + (float)y_data) / 100;
			PWM1_value = (uint32_t)x_calc;
			y_calc = fabsf((float)x_data);
			PWM2_value = (uint32_t)y_calc;
			HAL_GPIO_WritePin(GPIOA, GPIO_PIN_11, 1);
			HAL_GPIO_WritePin(GPIOA, GPIO_PIN_12, 1);
			__HAL_TIM_SET_COMPARE(&htim3,TIM_CHANNEL_1, PWM2_value);
			__HAL_TIM_SET_COMPARE(&htim3,TIM_CHANNEL_2, PWM1_value);
		}

		// ?��?�� ?��?�� 받을 �?�?
		HAL_UART_Receive_IT(&huart1, &uart_buf, 8);
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
  MX_TIM3_Init();
  MX_USART1_UART_Init();

  HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);
  HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_2);

  HAL_UART_Receive_IT(&huart1, &uart_buf, 8);
  /* USER CODE BEGIN 2 */

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

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_BYPASS;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
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
