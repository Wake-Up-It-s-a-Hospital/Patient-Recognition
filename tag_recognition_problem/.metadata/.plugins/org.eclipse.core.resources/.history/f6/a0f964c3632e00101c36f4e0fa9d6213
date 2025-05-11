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
#define FRAME_BUFFER_SIZE 128
#define HEADER_0_INDEX      0
#define HEADER_1_INDEX      1
#define ADDRESS_INDEX       2
#define CONTENT_SIZE_INDEX  3
#define COMMAND_INDEX       4
#define CONTENT_INDEX       5
#define PROTOCOL_SIZE       6

static uint8_t receive_buffer[FRAME_BUFFER_SIZE];
static short receive_index = 0;
static short content_current = 0;
static short content_end = 0;
static bool receive_fail = false;
static bool content_read_end = false;

#define IS_BIG_ENDIAN() (!*(uint8_t *)&(uint16_t){1})
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
uint8_t rx_byte;

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART1)
    {
        bool frame_complete = husky_lens_protocol_receive(rx_byte);

        if (frame_complete)
        {
            if (husky_lens_protocol_read_begin(0x36)) // ?òà: command ID 0x36 (Learned ID Result ?ì±)
            {
                uint8_t x = husky_lens_protocol_read_uint8();
                uint8_t y = husky_lens_protocol_read_uint8();
                husky_lens_protocol_read_end();

                printf("Received Tag: X = %d, Y = %d\r\n", x, y);
            }
        }

        // ?ã§?ùå Î∞îÏù¥?ä∏ ?àò?ã† Ï§?Îπ?
        HAL_UART_Receive_IT(&huart1, &rx_byte, 1);
    }
}


int __io_putchar(int ch)
{
    HAL_UART_Transmit(&huart2, (uint8_t *)&ch, 1, HAL_MAX_DELAY);
    return ch;
}

bool validateCheckSum(){
    uint8_t stackSumIndex=receive_buffer[CONTENT_SIZE_INDEX] + CONTENT_INDEX;
    uint8_t sum = 0;
    for (uint8_t i=0; i< stackSumIndex; i++) {
        sum+=receive_buffer[i];
    }
    return (sum == receive_buffer[stackSumIndex]);
}

bool husky_lens_protocol_receive(uint8_t data){
    switch (receive_index)
    {
    case HEADER_0_INDEX:
        if (data!=0x55) {receive_index = 0; return false;}
        receive_buffer[HEADER_0_INDEX] = 0x55;
        break;
    case HEADER_1_INDEX:
        if (data!=0xaa) {receive_index = 0; return false;}
        receive_buffer[HEADER_1_INDEX] = 0xaa;
        break;
    case ADDRESS_INDEX:
        receive_buffer[ADDRESS_INDEX] = data;
        break;
    case CONTENT_SIZE_INDEX:
        if (data >= FRAME_BUFFER_SIZE-PROTOCOL_SIZE) {receive_index = 0; return false;}
        receive_buffer[CONTENT_SIZE_INDEX] = data;
        break;
    default:
        receive_buffer[receive_index]=data;
        if (receive_index==receive_buffer[CONTENT_SIZE_INDEX]+CONTENT_INDEX) {
            content_end = receive_index;
            receive_index=0;
            return validateCheckSum();
        }
        break;
    }
    receive_index++;
    return false;
}

bool husky_lens_protocol_read_begin(uint8_t command){
    if (command == receive_buffer[COMMAND_INDEX])
    {
        content_current = CONTENT_INDEX;
        content_read_end = false;
        receive_fail = false;
        return true;
    }
    return false;
}

uint8_t husky_lens_protocol_read_uint8(){
    if (content_current >= content_end || content_read_end){receive_fail = true; return 0;}
    uint8_t result;
    memcpy(&result, receive_buffer + content_current, sizeof(result));
    content_current += sizeof(result);
    return result;
}

bool husky_lens_protocol_read_end(){
    if (receive_fail)
    {
        receive_fail = false;
        return false;
    }
    return content_current == content_end;
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
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */
  HAL_UART_Receive_IT(&huart1, &rx_byte, 1); // UART1 ?àò?ã† ?ù∏?Ñ∞?üΩ?ä∏ ?ãú?ûë
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
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI_DIV2;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL16;
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
