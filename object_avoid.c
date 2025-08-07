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
#include "eth.h"
#include "tim.h"
#include "usart.h"
#include "usb_otg.h"
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
char uart_buf[8];
char re_data[8];
char uwb_buf[4];
char fe_data[5];
float uwb_data;
int x_data, y_data;
float w_L, w_R;
uint32_t PWM1_value, PWM2_value;
uint32_t spin_value;

float r = 0.0575;
float b = 0.257;
float dis, theta, V, W, W_test;
float K_v = 1.5;
float K_w = 1.5;

float disLimit = 1.2;

#define PI 3.1415926535

#define RX_BUF_SIZE 32

uint8_t rx_byte;
char rx_buf[RX_BUF_SIZE];
uint8_t rx_index = 0;
volatile uint8_t rx_flag = 0;
int rx_theta;

uint8_t rx_byte2;
char rx_buf2[RX_BUF_SIZE];
uint8_t rx_index2 = 0;
volatile uint8_t rx_flag2 = 0;
int rx_theta2;

int active_theta;

int16_t prev_cnt1 = 0, prev_cnt2 = 0;
int16_t curr_cnt1, curr_cnt2;
int16_t delta_R, delta_L;
float speed_tick_per_sec1, speed_tick_per_sec2;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM2)
    {
        curr_cnt1 = (int16_t)__HAL_TIM_GET_COUNTER(&htim2);
        curr_cnt2 = (int16_t)__HAL_TIM_GET_COUNTER(&htim4);
        delta_R = abs(curr_cnt1 - prev_cnt1);
        delta_L = abs(curr_cnt2 - prev_cnt2);
        speed_tick_per_sec1 = delta_R / 0.01;
        speed_tick_per_sec2 = delta_L / 0.01;
        prev_cnt1 = curr_cnt1;
        prev_cnt2 = curr_cnt2;
    }
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
	if (huart -> Instance == UART4) {
		for (int i = 0; i < 4; i++) {
			if (uwb_buf[(i + 1) % 4] == '.') {
				for (int j = 0; j < 4; j++) {
					fe_data[j] = uwb_buf[(i + j) % 4];
				}
			}
		}

		fe_data[4] = '\0';

		sscanf(fe_data, "%f", &dis);

		dis = roundf(dis * 100) / 100;

		HAL_UART_Receive_IT(&huart4, &uwb_buf, 4);
	}

    if (huart->Instance == USART2) {
		if (rx_byte == '/') {
			rx_buf[rx_index] = rx_byte;
			rx_buf[rx_index + 1] = '\0';
			rx_flag = 1;
			rx_index = 0;
		} else {
			if (rx_index < RX_BUF_SIZE - 1) {
				rx_buf[rx_index++] = rx_byte;
			} else {
				rx_index = 0;
			}
		}
		HAL_UART_Receive_IT(&huart2, &rx_byte, 1);
    }

	if (huart -> Instance == USART6) {
		if (rx_byte2 == '/') {
			rx_buf2[rx_index2] = rx_byte2;
			rx_buf2[rx_index2 + 1] = '\0';
			rx_flag2 = 1;
			rx_index2 = 0;
		} else {
			if (rx_index2 < RX_BUF_SIZE - 1) {
				rx_buf2[rx_index2++] = rx_byte2;
			} else {
				rx_index2 = 0;
			}
		}
		HAL_UART_Receive_IT(&huart6, &rx_byte2, 1);
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
  MX_ETH_Init();
  MX_USART3_UART_Init();
  MX_USB_OTG_FS_PCD_Init();
  MX_TIM1_Init();
  MX_TIM2_Init();
  MX_TIM3_Init();
  MX_TIM4_Init();
  MX_UART4_Init();
  MX_USART2_UART_Init();
  MX_USART6_UART_Init();
  /* USER CODE BEGIN 2 */
  HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
  HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_2);

  HAL_UART_Receive_IT(&huart2, &rx_byte, 1);
  HAL_UART_Receive_IT(&huart6, &rx_byte2, 1);
  HAL_UART_Receive_IT(&huart4, &uwb_buf, 4);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
      /*UART 통신으로 theta 값 송신신*/
	  if (rx_flag) 
      {
	      rx_flag = 0;

	      sscanf(rx_buf, "%d/", &rx_theta);
	  }

	  if (rx_flag2) 
      {
	      rx_flag2 = 0;
	      sscanf(rx_buf2, "%d/", &rx_theta2);
	  }

	  if (rx_theta != 999) {
		  active_theta = rx_theta;
	  } else if (rx_theta2 != 999) {
		  active_theta = rx_theta2;
	  } else {
		  active_theta = 0;
	  }

	  theta = active_theta;  // 왼쪽 허스키 또는 오른쪽 허스키 감지 시 해당 위치의의 theta 값 결정정 


      //////////////////////////////////////////////////////////////////////////////////////////


      if (dis > disLimit) 
      {
		  dis = disLimit;  // 거리 제한 1.2m
	  }
      

	  if (dis >= 0.7) 
      {
		  V = K_v * (dis - 0.7); // 일반 주행 선속도
	  }
	  else 
      {
		  V = 0;   // 0.7m 미만 일때 제자리 조향
	  }

	  W = K_w * theta * PI / 180;

	  w_L = (V + W * b / 2) / r;
	  w_R = (V - W * b / 2) / r;

	  PWM1_value = w_L / 26.28 * 100;
	  PWM2_value = w_R / 26.28 * 100;

      /*pwm 음수 값일 시 PWM 0으로 고정*/
	  if (PWM1_value < 0) 
      {
		  PWM1_value = 0; 
	  }

	  if (PWM2_value < 0) 
      {
		  PWM2_value = 0;
	  }
     

      /*0.7이하 일 때 spin 값*/
	  if (dis < 0.7) 
      {
		  spin_value = (PWM1_value + PWM2_value) * 3 + 24; 
	  }

	  if (dis >= 0.7 && dis <= 0.8) {
		  if (theta > 15) {
			  PWM1_value += 24;
			  PWM2_value = 0;
		  }
		  else if (theta < -15) {
			  PWM1_value = 0;
			  PWM2_value += 24;
		  }
	  }

      /* PWM 임계값 설정*/
	  if (PWM1_value > 99) {
	    	PWM1_value = 99;
	  }
	  if (PWM2_value > 99) {
		  PWM2_value = 99;
	  }

/*	  if (dis >= 0.7) {
		  if (theta > 15 || theta < -15) {
		  	  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, RESET);
	  	  	  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_9, RESET);
			  __HAL_TIM_SET_COMPARE(&htim1,TIM_CHANNEL_1, PWM1_value);
			  __HAL_TIM_SET_COMPARE(&htim1,TIM_CHANNEL_2, PWM2_value);
		  }
		  else {
			  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, RESET);
	  	  	  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_9, RESET);
			  __HAL_TIM_SET_COMPARE(&htim1,TIM_CHANNEL_1, PWM1_value);
			  __HAL_TIM_SET_COMPARE(&htim1,TIM_CHANNEL_2, PWM2_value);
		  }
	  }
	  else if (dis < 0.7) {
		  if (theta > 15) {
			  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, RESET);
	  	  	  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_9, SET);
			  __HAL_TIM_SET_COMPARE(&htim1,TIM_CHANNEL_1, spin_value);
			  __HAL_TIM_SET_COMPARE(&htim1,TIM_CHANNEL_2, spin_value);
		  }
		  else if (theta < -15) {
			  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, SET);
	  	  	  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_9, RESET);
			  __HAL_TIM_SET_COMPARE(&htim1,TIM_CHANNEL_1, spin_value);
			  __HAL_TIM_SET_COMPARE(&htim1,TIM_CHANNEL_2, spin_value);
		  }
		  else if (theta >= -15 && theta <= 15) {
		  	  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, RESET);
	  	  	  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_9, RESET);
			  __HAL_TIM_SET_COMPARE(&htim1,TIM_CHANNEL_1, 0);
			  __HAL_TIM_SET_COMPARE(&htim1,TIM_CHANNEL_2, 0);
		  }
	  }*/

	  if (theta > 15) {
		  if (dis >= 0.6) {
		  	  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, RESET);
	  	  	  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_9, RESET);
			  __HAL_TIM_SET_COMPARE(&htim1,TIM_CHANNEL_1, PWM1_value);
			  __HAL_TIM_SET_COMPARE(&htim1,TIM_CHANNEL_2, PWM2_value);
		  }
		  else if (dis < 0.7) {
			  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, RESET);
	  	  	  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_9, SET);
			  __HAL_TIM_SET_COMPARE(&htim1,TIM_CHANNEL_1, spin_value);
			  __HAL_TIM_SET_COMPARE(&htim1,TIM_CHANNEL_2, spin_value);
		  }
	  }
	  else if (theta < -15) {
		  if (dis >= 0.6) {
		  	  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, RESET);
	  	  	  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_9, RESET);
			  __HAL_TIM_SET_COMPARE(&htim1,TIM_CHANNEL_1, PWM1_value);
			  __HAL_TIM_SET_COMPARE(&htim1,TIM_CHANNEL_2, PWM2_value);
		  }
		  else if (dis < 0.7) {
			  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, SET);
	  	  	  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_9, RESET);
			  __HAL_TIM_SET_COMPARE(&htim1,TIM_CHANNEL_1, spin_value);
			  __HAL_TIM_SET_COMPARE(&htim1,TIM_CHANNEL_2, spin_value);
		  }
	  }
	  else if (theta >= -15 && theta <= 15) {
		  if (dis >= 0.7) {
			  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, RESET);
	  	  	  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_9, RESET);
			  __HAL_TIM_SET_COMPARE(&htim1,TIM_CHANNEL_1, PWM1_value);
			  __HAL_TIM_SET_COMPARE(&htim1,TIM_CHANNEL_2, PWM2_value);
		  }
		  else if (dis < 0.7) {
		  	  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, RESET);
	  	  	  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_9, RESET);
			  __HAL_TIM_SET_COMPARE(&htim1,TIM_CHANNEL_1, 0);
			  __HAL_TIM_SET_COMPARE(&htim1,TIM_CHANNEL_2, 0);
		  }
	  }

/*	  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, SET);
	  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_9, RESET);
	  __HAL_TIM_SET_COMPARE(&htim1,TIM_CHANNEL_1, 25);
	  __HAL_TIM_SET_COMPARE(&htim1,TIM_CHANNEL_2, 25);*/
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

  /** Configure LSE Drive Capability
  */
  HAL_PWR_EnableBkUpAccess();

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_BYPASS;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 72;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 3;
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
