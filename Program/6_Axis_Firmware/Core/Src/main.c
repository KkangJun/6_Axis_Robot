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
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "at24cxx.h"
#include "df2301q.h"
#include "mg4005.h"
#include "xl430.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define ROBOT_INIT_FLAG 0x01
#define ROBOT_INITCOMP_FLAG 0x02
#define ROBOT_STOP_FLAG 0x04
#define ROBOT_STOPCOMP_FLAG 0x08

#define EEPROM_BUSY 0x01

#define MG4005_ID1_INIT 0
#define MG4005_ID2_INIT 0
#define MG4005_ID3_INIT 0
#define MG4005_ID1_OFFSET 180
#define MG4005_SPEED 300

#define XL430_ID4_INIT 17
#define XL430_ID5_INIT 180
#define XL430_ID6_INIT 180
#define XL430_SPEED 218
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
CAN_HandleTypeDef hcan2;

I2C_HandleTypeDef hi2c1;

UART_HandleTypeDef huart2;
UART_HandleTypeDef huart3;

/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityLow,
};
/* Definitions for Task_VIT */
osThreadId_t Task_VITHandle;
const osThreadAttr_t Task_VIT_attributes = {
  .name = "Task_VIT",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal1,
};
/* Definitions for Task_Motor */
osThreadId_t Task_MotorHandle;
const osThreadAttr_t Task_Motor_attributes = {
  .name = "Task_Motor",
  .stack_size = 256 * 4,
  .priority = (osPriority_t) osPriorityNormal2,
};
/* Definitions for Task_Data */
osThreadId_t Task_DataHandle;
const osThreadAttr_t Task_Data_attributes = {
  .name = "Task_Data",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal3,
};
/* Definitions for Task_GetCMD */
osThreadId_t Task_GetCMDHandle;
const osThreadAttr_t Task_GetCMD_attributes = {
  .name = "Task_GetCMD",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for Task_BLE */
osThreadId_t Task_BLEHandle;
const osThreadAttr_t Task_BLE_attributes = {
  .name = "Task_BLE",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal4,
};
/* Definitions for ble_IT */
osMessageQueueId_t ble_ITHandle;
const osMessageQueueAttr_t ble_IT_attributes = {
  .name = "ble_IT"
};
/* Definitions for voice_IT */
osMessageQueueId_t voice_ITHandle;
const osMessageQueueAttr_t voice_IT_attributes = {
  .name = "voice_IT"
};
/* Definitions for pos_data */
osMessageQueueId_t pos_dataHandle;
const osMessageQueueAttr_t pos_data_attributes = {
  .name = "pos_data"
};
/* Definitions for voice_data */
osMessageQueueId_t voice_dataHandle;
const osMessageQueueAttr_t voice_data_attributes = {
  .name = "voice_data"
};
/* Definitions for pos_single */
osMessageQueueId_t pos_singleHandle;
const osMessageQueueAttr_t pos_single_attributes = {
  .name = "pos_single"
};
/* Definitions for bSem_BLE */
osSemaphoreId_t bSem_BLEHandle;
const osSemaphoreAttr_t bSem_BLE_attributes = {
  .name = "bSem_BLE"
};
/* Definitions for bSem_Voice */
osSemaphoreId_t bSem_VoiceHandle;
const osSemaphoreAttr_t bSem_Voice_attributes = {
  .name = "bSem_Voice"
};
/* Definitions for cmdFlag */
osEventFlagsId_t cmdFlagHandle;
const osEventFlagsAttr_t cmdFlag_attributes = {
  .name = "cmdFlag"
};
/* Definitions for i2cFlag */
osEventFlagsId_t i2cFlagHandle;
const osEventFlagsAttr_t i2cFlag_attributes = {
  .name = "i2cFlag"
};
/* USER CODE BEGIN PV */
// BLE module code
typedef struct {
	uint8_t cmd[4];
	uint8_t idx;
	uint8_t pos1[4];
	uint8_t pos2[4];
	uint8_t pos3[4];
	uint8_t pos4[4];
	uint8_t pos5[4];
	uint8_t pos6[4];
} bleData;

typedef struct {
	uint8_t idx;
	int32_t pos1;
	int32_t pos2;
	int32_t pos3;
	int32_t pos4;
	int32_t pos5;
	int32_t pos6;
} posData;

typedef struct {
	uint8_t id;
	int16_t pos;
} posSingleData;

bleData *pBle_it;
posData *pPos_data;
uint8_t uart2_rx = 0;

// Voice sensor code
uint8_t *pVoice_it;
uint8_t *pVoice_data;
uint8_t i2c1_rx = 0;

// MG4005 code
CAN_FilterTypeDef canFilter;
CAN_RxHeaderTypeDef canRxHeader;
CAN_TxHeaderTypeDef canTxHeader;
uint8_t canRx0Data[8];
uint32_t canTxMailbox;
uint8_t canTxData[8];

// XL430 code
uint8_t tx_packet[64];

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_CAN2_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_I2C1_Init(void);
static void MX_USART3_UART_Init(void);
void StartDefaultTask(void *argument);
void VoiceInterrupt(void *argument);
void Motor(void *argument);
void DataParse(void *argument);
void GetVoiceCmd(void *argument);
void BLEInterrupt(void *argument);

/* USER CODE BEGIN PFP */
void PosSetDataParsing(bleData *p, posData *data);
void posSingleDataParsing(bleData *p, posSingleData *data, uint8_t id);
void PosSaveDataParsing(bleData *p, uint8_t *save_arr);
void PosSaveDataDecode(uint8_t *save_arr, posData *data);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
//int _write(int file, char *p, int len) {
//	for (int i = 0; i < len; i++) {
//		ITM_SendChar((*p++));
//	}
//	return len;
//}
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
  MX_CAN2_Init();
  MX_USART2_UART_Init();
  MX_I2C1_Init();
  MX_USART3_UART_Init();
  /* USER CODE BEGIN 2 */
	// Boot delay
	HAL_Delay(2000);

	HAL_UART_Receive_IT(&huart2, &uart2_rx, 1);
	InitCANFilter();
  /* USER CODE END 2 */

  /* Init scheduler */
  osKernelInitialize();

  /* USER CODE BEGIN RTOS_MUTEX */
	/* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* Create the semaphores(s) */
  /* creation of bSem_BLE */
  bSem_BLEHandle = osSemaphoreNew(1, 1, &bSem_BLE_attributes);

  /* creation of bSem_Voice */
  bSem_VoiceHandle = osSemaphoreNew(1, 1, &bSem_Voice_attributes);

  /* USER CODE BEGIN RTOS_SEMAPHORES */
	/* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
	/* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* Create the queue(s) */
  /* creation of ble_IT */
  ble_ITHandle = osMessageQueueNew (5, sizeof(bleData), &ble_IT_attributes);

  /* creation of voice_IT */
  voice_ITHandle = osMessageQueueNew (5, sizeof(uint8_t), &voice_IT_attributes);

  /* creation of pos_data */
  pos_dataHandle = osMessageQueueNew (5, sizeof(posData), &pos_data_attributes);

  /* creation of voice_data */
  voice_dataHandle = osMessageQueueNew (5, sizeof(uint8_t), &voice_data_attributes);

  /* creation of pos_single */
  pos_singleHandle = osMessageQueueNew (5, sizeof(posSingleData), &pos_single_attributes);

  /* USER CODE BEGIN RTOS_QUEUES */
	/* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of defaultTask */
  defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);

  /* creation of Task_VIT */
  Task_VITHandle = osThreadNew(VoiceInterrupt, NULL, &Task_VIT_attributes);

  /* creation of Task_Motor */
  Task_MotorHandle = osThreadNew(Motor, NULL, &Task_Motor_attributes);

  /* creation of Task_Data */
  Task_DataHandle = osThreadNew(DataParse, NULL, &Task_Data_attributes);

  /* creation of Task_GetCMD */
  Task_GetCMDHandle = osThreadNew(GetVoiceCmd, NULL, &Task_GetCMD_attributes);

  /* creation of Task_BLE */
  Task_BLEHandle = osThreadNew(BLEInterrupt, NULL, &Task_BLE_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
	/* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* Create the event(s) */
  /* creation of cmdFlag */
  cmdFlagHandle = osEventFlagsNew(&cmdFlag_attributes);

  /* creation of i2cFlag */
  i2cFlagHandle = osEventFlagsNew(&i2cFlag_attributes);

  /* USER CODE BEGIN RTOS_EVENTS */
	/* add events, ... */
	osEventFlagsSet(cmdFlagHandle, ROBOT_INIT_FLAG);
  /* USER CODE END RTOS_EVENTS */

  /* Start scheduler */
  osKernelStart();

  /* We should never get here as control is now taken by the scheduler */
  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
	while (1) {
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
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 168;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
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
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief CAN2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_CAN2_Init(void)
{

  /* USER CODE BEGIN CAN2_Init 0 */

  /* USER CODE END CAN2_Init 0 */

  /* USER CODE BEGIN CAN2_Init 1 */

  /* USER CODE END CAN2_Init 1 */
  hcan2.Instance = CAN2;
  hcan2.Init.Prescaler = 2;
  hcan2.Init.Mode = CAN_MODE_NORMAL;
  hcan2.Init.SyncJumpWidth = CAN_SJW_1TQ;
  hcan2.Init.TimeSeg1 = CAN_BS1_15TQ;
  hcan2.Init.TimeSeg2 = CAN_BS2_5TQ;
  hcan2.Init.TimeTriggeredMode = DISABLE;
  hcan2.Init.AutoBusOff = DISABLE;
  hcan2.Init.AutoWakeUp = DISABLE;
  hcan2.Init.AutoRetransmission = DISABLE;
  hcan2.Init.ReceiveFifoLocked = DISABLE;
  hcan2.Init.TransmitFifoPriority = DISABLE;
  if (HAL_CAN_Init(&hcan2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN CAN2_Init 2 */

  /* USER CODE END CAN2_Init 2 */

}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.ClockSpeed = 100000;
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

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
  huart2.Init.BaudRate = 9600;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * @brief USART3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART3_UART_Init(void)
{

  /* USER CODE BEGIN USART3_Init 0 */

  /* USER CODE END USART3_Init 0 */

  /* USER CODE BEGIN USART3_Init 1 */

  /* USER CODE END USART3_Init 1 */
  huart3.Instance = USART3;
  huart3.Init.BaudRate = 115200;
  huart3.Init.WordLength = UART_WORDLENGTH_8B;
  huart3.Init.StopBits = UART_STOPBITS_1;
  huart3.Init.Parity = UART_PARITY_NONE;
  huart3.Init.Mode = UART_MODE_TX;
  huart3.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart3.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_HalfDuplex_Init(&huart3) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART3_Init 2 */

  /* USER CODE END USART3_Init 2 */

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
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOD, Green_LED_Pin|Orange_LED_Pin|Red_LED_Pin|Blue_LED_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : Green_LED_Pin Orange_LED_Pin Red_LED_Pin Blue_LED_Pin */
  GPIO_InitStruct.Pin = Green_LED_Pin|Orange_LED_Pin|Red_LED_Pin|Blue_LED_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
// Bluetooth data Receive
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
	if (huart->Instance == USART2) {
		osSemaphoreRelease(bSem_BLEHandle);
	}
}

void HAL_I2C_MemRxCpltCallback(I2C_HandleTypeDef *hi2c) {
	if (hi2c->Instance == I2C1) {
		osSemaphoreRelease(bSem_VoiceHandle);
	}
}

void PosSetDataParsing(bleData *p, posData *data) {
	data->pos1 = (int32_t) (strtoul(p->pos1, NULL, 10) - MG4005_ID1_OFFSET);
	data->pos2 = (int32_t) strtoul(p->pos2, NULL, 10);
	data->pos3 = (int32_t) strtoul(p->pos3, NULL, 10);
	data->pos4 = (int32_t) strtoul(p->pos4, NULL, 10);
	data->pos5 = (int32_t) strtoul(p->pos5, NULL, 10);
	data->pos6 = (int32_t) strtoul(p->pos6, NULL, 10);
}

void posSingleDataParsing(bleData *p, posSingleData *data, uint8_t id) {
	data->id = id;
	data->pos = (int16_t) strtoul(p->pos1, NULL, 10);
}

void PosSaveDataParsing(bleData *p, uint8_t *save_arr) {

	int16_t pos1 = (int16_t) strtoul(p->pos1, NULL, 10);
	int16_t pos2 = (int16_t) strtoul(p->pos2, NULL, 10);
	int16_t pos3 = (int16_t) strtoul(p->pos3, NULL, 10);
	int16_t pos4 = (int16_t) strtoul(p->pos4, NULL, 10);
	int16_t pos5 = (int16_t) strtoul(p->pos5, NULL, 10);
	int16_t pos6 = (int16_t) strtoul(p->pos6, NULL, 10);

	save_arr[0] = *((uint8_t*) (&pos1));
	save_arr[1] = *((uint8_t*) (&pos1) + 1);
	save_arr[2] = *((uint8_t*) (&pos2));
	save_arr[3] = *((uint8_t*) (&pos2) + 1);
	save_arr[4] = *((uint8_t*) (&pos3));
	save_arr[5] = *((uint8_t*) (&pos3) + 1);
	save_arr[6] = *((uint8_t*) (&pos4));
	save_arr[7] = *((uint8_t*) (&pos4) + 1);
	save_arr[8] = *((uint8_t*) (&pos5));
	save_arr[9] = *((uint8_t*) (&pos5) + 1);
	save_arr[10] = *((uint8_t*) (&pos6));
	save_arr[11] = *((uint8_t*) (&pos6) + 1);

	printf("idx: %d\r\n", p->idx - 48);
	printf("pos2: %d\r\n", pos2);
	printf("SaveArr2: 0x%x, 0x%x\r\n", save_arr[2], save_arr[3]);
}

void PosSaveDataDecode(uint8_t *save_arr, posData *data) {
	data->pos1 = ((int32_t) (((uint16_t) save_arr[1] << 8)
			| (uint16_t) save_arr[0])) - MG4005_ID1_OFFSET;
	data->pos2 = (int32_t) (((uint16_t) save_arr[3] << 8)
			| (uint16_t) save_arr[2]);
	data->pos3 = (int32_t) (((uint16_t) save_arr[5] << 8)
			| (uint16_t) save_arr[4]);
	data->pos4 = (int32_t) (((uint16_t) save_arr[7] << 8)
			| (uint16_t) save_arr[6]);
	data->pos5 = (int32_t) (((uint16_t) save_arr[9] << 8)
			| (uint16_t) save_arr[8]);
	data->pos6 = (int32_t) (((uint16_t) save_arr[11] << 8)
			| (uint16_t) save_arr[10]);

	printf("Decode SaveArr2: 0x%x, 0x%x\r\n", save_arr[2], save_arr[3]);
	printf("Decode pos2: %d", data->pos2);
}
/* USER CODE END 4 */

/* USER CODE BEGIN Header_StartDefaultTask */
/**
 * @brief  Function implementing the defaultTask thread.
 * @param  argument: Not used
 * @retval None
 */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void *argument)
{
  /* USER CODE BEGIN 5 */
	/* Infinite loop */
	for (;;) {
		osDelay(1);
	}
  /* USER CODE END 5 */
}

/* USER CODE BEGIN Header_VoiceInterrupt */
/**
 * @brief Function implementing the Task_VIT thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_VoiceInterrupt */
void VoiceInterrupt(void *argument)
{
  /* USER CODE BEGIN VoiceInterrupt */
	uint8_t voice_cmd = 0;
	/* Infinite loop */
	for (;;) {
		if (bSem_VoiceHandle != NULL) {
			if (osSemaphoreAcquire(bSem_VoiceHandle, osWaitForever) == osOK) {
				HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12, SET);

				if (i2c1_rx != 0) {
					voice_cmd = i2c1_rx;
					osMessageQueuePut(voice_ITHandle, &voice_cmd, 1U, 0U);

					HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12, RESET);
				}
			}
		}
//		osDelay(1);
	}
  /* USER CODE END VoiceInterrupt */
}

/* USER CODE BEGIN Header_Motor */
/**
 * @brief Function implementing the Task_Motor thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_Motor */
void Motor(void *argument)
{
  /* USER CODE BEGIN Motor */
	osStatus_t pos_data_event;
	osStatus_t pos_single_event;
	osStatus_t voice_data_event;

	posData pos_data;
	posSingleData single_data;
	uint8_t voice_data;
	uint32_t event_flags;

	posData decode_data;
	uint8_t addr = 0x00;
	uint8_t eeprom_data[12] = { 0x00 };

	/* Infinite loop */
	for (;;) {
		pos_data_event = osMessageQueueGet(pos_dataHandle, &pos_data, 2U, 10);
		pos_single_event = osMessageQueueGet(pos_singleHandle, &single_data, 2U,
				10);
		voice_data_event = osMessageQueueGet(voice_dataHandle, &voice_data, 3U,
				10);

		event_flags = osEventFlagsGet(cmdFlagHandle);

		if ((event_flags & ROBOT_INIT_FLAG)
				&& !(event_flags & ROBOT_INITCOMP_FLAG)) {
			// Robot initialize for motor zero position
			printf("Robot initialize\r\n");

			// Motor ON
//			MG4005On(0x01);
//			MG4005On(0x02);
//			MG4005On(0x03);

			xl430TorqueEnable(0x04, TORQUE_MODE_ON);
			xl430TorqueEnable(0x05, TORQUE_MODE_ON);
			xl430TorqueEnable(0x06, TORQUE_MODE_ON);

			xl430ProfVelo(0x04, XL430_SPEED);
			xl430ProfVelo(0x05, XL430_SPEED);
			xl430ProfVelo(0x06, XL430_SPEED);

			// Motor zero position init
			MG4005MultiLoop2(0x01, MG4005_SPEED, MG4005_ID1_INIT);
			MG4005MultiLoop2(0x02, MG4005_SPEED, MG4005_ID2_INIT);
			MG4005MultiLoop2(0x03, MG4005_SPEED, MG4005_ID3_INIT);

			xl430GoalPos(0x04, XL430_ID4_INIT);
			xl430GoalPos(0x05, XL430_ID5_INIT);
			xl430GoalPos(0x06, XL430_ID6_INIT);

			osEventFlagsSet(cmdFlagHandle, ROBOT_INITCOMP_FLAG);
			osEventFlagsClear(cmdFlagHandle, ROBOT_INIT_FLAG);
		}

		if ((event_flags & ROBOT_STOP_FLAG)
				&& !(event_flags & ROBOT_STOPCOMP_FLAG)) {
			// Robot motor stop
			printf("Robot stop\r\n");

			osEventFlagsSet(cmdFlagHandle, ROBOT_STOPCOMP_FLAG);
			osEventFlagsClear(cmdFlagHandle, ROBOT_STOP_FLAG);
		}

		if (pos_data_event == osOK) {
			// Robot motor commend
			printf("Robot set motor: %d. %d. %d. %d. %d. %d\r\n", pos_data.pos1,
					pos_data.pos2, pos_data.pos3, pos_data.pos4, pos_data.pos5,
					pos_data.pos6);

			// Motor angle set
			MG4005MultiLoop2(0x01, MG4005_SPEED, (int) pos_data.pos1);
			MG4005MultiLoop2(0x02, MG4005_SPEED, (int) pos_data.pos2);
			MG4005MultiLoop2(0x03, MG4005_SPEED, (int) pos_data.pos3);

			xl430GoalPos(0x04, (uint32_t) pos_data.pos4);
			xl430GoalPos(0x05, (uint32_t) pos_data.pos5);
			xl430GoalPos(0x06, (uint32_t) pos_data.pos6);
		}

		if (pos_single_event == osOK) {
			// Robot realtime motor commend
			printf("Robot S: 0x%02x, %d\r\n", single_data.id, single_data.pos);

			switch (single_data.id) {
			case 0x01:
				MG4005MultiLoop2(0x01, MG4005_SPEED,
						(int16_t) (single_data.pos - MG4005_ID1_OFFSET));
				break;
			case 0x02:
				MG4005MultiLoop2(0x02, MG4005_SPEED, (int) single_data.pos);
				break;
			case 0x03:
				MG4005MultiLoop2(0x03, MG4005_SPEED, (int) single_data.pos);
				break;
			case 0x04:
				xl430GoalPos(0x04, (uint32_t) single_data.pos);
				break;
			case 0x05:
				xl430GoalPos(0x05, (uint32_t) single_data.pos);
				break;
			case 0x06:
				xl430GoalPos(0x06, (uint32_t) single_data.pos);
				break;
			}
		}

		if (voice_data_event == osOK) {
			// Voice commend
			printf("Voice input: %02x\r\n", voice_data);

			switch (voice_data) {
			case 0x05:
			case 0x06:
			case 0x07:
			case 0x08:
			case 0x09:
				// Read save data (EEPROM)
				addr = (voice_data - 0x05) * 12;

				AT24Read(addr, eeprom_data, 12);
				PosSaveDataDecode(eeprom_data, &decode_data);

				// Motor angle set
				MG4005MultiLoop2(0x01, MG4005_SPEED,
						(int32_t) decode_data.pos1);
				MG4005MultiLoop2(0x02, MG4005_SPEED,
						(int32_t) decode_data.pos2);
				MG4005MultiLoop2(0x03, MG4005_SPEED,
						(int32_t) decode_data.pos3);

				xl430GoalPos(0x04, (uint32_t) decode_data.pos4);
				xl430GoalPos(0x05, (uint32_t) decode_data.pos5);
				xl430GoalPos(0x06, (uint32_t) decode_data.pos6);
				break;
			case 0x0A:
				// Guidance voice ON
				DF2301QSetMuteMode(DF2301Q_MUTE_OFF);
				break;
			case 0x0B:
				// Guidance voice OFF
				DF2301QSetMuteMode(DF2301Q_MUTE_ON);
				break;
			case 0x17:
				// Test code
				addr = 0x00;

				AT24Read(addr, eeprom_data, 12);
				PosSaveDataDecode(eeprom_data, &decode_data);

				printf("Robot set motor: %d. %d. %d. %d. %d. %d\r\n",
						decode_data.pos1, decode_data.pos2, decode_data.pos3,
						decode_data.pos4, decode_data.pos5, decode_data.pos6);

				// Motor angle set
				MG4005MultiLoop2(0x01, MG4005_SPEED,
						(int32_t) decode_data.pos1);
				MG4005MultiLoop2(0x02, MG4005_SPEED,
						(int32_t) decode_data.pos2);
				MG4005MultiLoop2(0x03, MG4005_SPEED,
						(int32_t) decode_data.pos3);

				xl430GoalPos(0x04, (uint32_t) decode_data.pos4);
				xl430GoalPos(0x05, (uint32_t) decode_data.pos5);
				xl430GoalPos(0x06, (uint32_t) decode_data.pos6);
				break;
			case 0x52:
				// Reset
				osEventFlagsSet(cmdFlagHandle, ROBOT_INIT_FLAG);

				osEventFlagsClear(cmdFlagHandle, ROBOT_INITCOMP_FLAG);
				osEventFlagsClear(cmdFlagHandle, ROBOT_STOPCOMP_FLAG);
				break;
			}
		}

		osDelay(1);
	}
  /* USER CODE END Motor */
}

/* USER CODE BEGIN Header_DataParse */
/**
 * @brief Function implementing the Task_Data thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_DataParse */
void DataParse(void *argument)
{
  /* USER CODE BEGIN DataParse */
	osStatus_t ble_it_event;
	osStatus_t voice_it_event;

	bleData ble_it;
	posData parse_data;
	posSingleData single_parse_data;
	uint8_t voice_it;

	/* Infinite loop */
	for (;;) {
		ble_it_event = osMessageQueueGet(ble_ITHandle, &ble_it, 0U, 10);
		voice_it_event = osMessageQueueGet(voice_ITHandle, &voice_it, 1U, 10);

		if (ble_it_event == osOK) {
			HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13, SET);

			printf("%s\r\n", ble_it.cmd);
			if (ble_it.cmd[0] == 'P') {
				switch (ble_it.cmd[1]) {
				case '1':
					osEventFlagsClear(cmdFlagHandle, ROBOT_INITCOMP_FLAG);
					osEventFlagsClear(cmdFlagHandle, ROBOT_STOPCOMP_FLAG);

					posSingleDataParsing(&ble_it, &single_parse_data, 0x01);
					osMessageQueuePut(pos_singleHandle, &single_parse_data, 2U,
							0U);
					break;
				case '2':
					osEventFlagsClear(cmdFlagHandle, ROBOT_INITCOMP_FLAG);
					osEventFlagsClear(cmdFlagHandle, ROBOT_STOPCOMP_FLAG);

					posSingleDataParsing(&ble_it, &single_parse_data, 0x02);
					osMessageQueuePut(pos_singleHandle, &single_parse_data, 2U,
							0U);
					break;
				case '3':
					osEventFlagsClear(cmdFlagHandle, ROBOT_INITCOMP_FLAG);
					osEventFlagsClear(cmdFlagHandle, ROBOT_STOPCOMP_FLAG);

					posSingleDataParsing(&ble_it, &single_parse_data, 0x03);
					osMessageQueuePut(pos_singleHandle, &single_parse_data, 2U,
							0U);
					break;
				case '4':
					osEventFlagsClear(cmdFlagHandle, ROBOT_INITCOMP_FLAG);
					osEventFlagsClear(cmdFlagHandle, ROBOT_STOPCOMP_FLAG);

					posSingleDataParsing(&ble_it, &single_parse_data, 0x04);
					osMessageQueuePut(pos_singleHandle, &single_parse_data, 2U,
							0U);
					break;
				case '5':
					osEventFlagsClear(cmdFlagHandle, ROBOT_INITCOMP_FLAG);
					osEventFlagsClear(cmdFlagHandle, ROBOT_STOPCOMP_FLAG);

					posSingleDataParsing(&ble_it, &single_parse_data, 0x05);
					osMessageQueuePut(pos_singleHandle, &single_parse_data, 2U,
							0U);
					break;
				case '6':
					osEventFlagsClear(cmdFlagHandle, ROBOT_INITCOMP_FLAG);
					osEventFlagsClear(cmdFlagHandle, ROBOT_STOPCOMP_FLAG);

					posSingleDataParsing(&ble_it, &single_parse_data, 0x06);
					osMessageQueuePut(pos_singleHandle, &single_parse_data, 2U,
							0U);
					break;
				}
			} else if (!(strcmp(ble_it.cmd, "STOP"))) {
				// stop flag on
				osEventFlagsSet(cmdFlagHandle, ROBOT_STOP_FLAG);
				osEventFlagsClear(cmdFlagHandle, ROBOT_INITCOMP_FLAG);
				osEventFlagsClear(cmdFlagHandle, ROBOT_STOPCOMP_FLAG);

			} else if (!(strcmp(ble_it.cmd, "SET"))) {
				// stop flag off & put pos data
				osEventFlagsClear(cmdFlagHandle, ROBOT_INITCOMP_FLAG);
				osEventFlagsClear(cmdFlagHandle, ROBOT_STOPCOMP_FLAG);

				PosSetDataParsing(&ble_it, &parse_data);
				osMessageQueuePut(pos_dataHandle, &parse_data, 2U, 0U);

			} else if (!(strcmp(ble_it.cmd, "SAV"))) {
				// eeprom write
				uint8_t parse_idx = ble_it.idx - 48;
				uint8_t addr = parse_idx * 12;
				uint8_t data[12] = {0x00};

				PosSaveDataParsing(&ble_it, data);
				AT24Write(addr, data, 12);

			} else if (!(strcmp(ble_it.cmd, "DEL"))) {
				// eeprom delete
				uint8_t parse_idx = ble_it.idx - 48;
				uint8_t addr;
				uint8_t next_addr;
				uint8_t data[12] = {0x00};

				for (int i = parse_idx; i < 4; i++) {
					addr = i * 12;
					next_addr = addr + 12;

					AT24Read(next_addr, data, 12);

					AT24Write(addr, data, 12);
				}

				memset(data, 0x00, 12);
				addr = 0x30; // 5th address
				AT24Write(addr, data, 12);

			} else if (!(strcmp(ble_it.cmd, "RSET"))) {
				// connect or reset flag on
				osEventFlagsSet(cmdFlagHandle, ROBOT_INIT_FLAG);

				osEventFlagsClear(cmdFlagHandle, ROBOT_INITCOMP_FLAG);
				osEventFlagsClear(cmdFlagHandle, ROBOT_STOPCOMP_FLAG);

			}

			HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13, RESET);
		}

		if (voice_it_event == osOK) {
			HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, SET);

			osEventFlagsClear(cmdFlagHandle, ROBOT_INIT_FLAG);
			osEventFlagsClear(cmdFlagHandle, ROBOT_INITCOMP_FLAG);
			osEventFlagsClear(cmdFlagHandle, ROBOT_STOP_FLAG);
			osEventFlagsClear(cmdFlagHandle, ROBOT_STOPCOMP_FLAG);

			osMessageQueuePut(voice_dataHandle, &voice_it, 3U, 0U);
			HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, RESET);
		}
		osDelay(1);
	}
  /* USER CODE END DataParse */
}

/* USER CODE BEGIN Header_GetVoiceCmd */
/**
 * @brief Function implementing the Task_GetCMD thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_GetVoiceCmd */
void GetVoiceCmd(void *argument)
{
  /* USER CODE BEGIN GetVoiceCmd */

	/* Infinite loop */
	for (;;) {
		DF2301QGetCMDID();

		osDelay(100);
	}
  /* USER CODE END GetVoiceCmd */
}

/* USER CODE BEGIN Header_BLEInterrupt */
/**
 * @brief Function implementing the Task_BLE thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_BLEInterrupt */
void BLEInterrupt(void *argument)
{
  /* USER CODE BEGIN BLEInterrupt */
	bleData ble_data;

	memset(ble_data.cmd, 0x00, 4);
	ble_data.idx = 0x00;
	memset(ble_data.pos1, 0x00, 3);
	memset(ble_data.pos2, 0x00, 3);
	memset(ble_data.pos3, 0x00, 3);
	memset(ble_data.pos4, 0x00, 3);
	memset(ble_data.pos5, 0x00, 3);
	memset(ble_data.pos6, 0x00, 3);

	uint8_t i = 0;
	uint8_t uart2_count = 0;

	/* Infinite loop */
	for (;;) {
		if (bSem_BLEHandle != NULL) {
			if (osSemaphoreAcquire(bSem_BLEHandle, osWaitForever) == osOK) {
				HAL_GPIO_WritePin(GPIOD, GPIO_PIN_15, SET);

				switch (uart2_rx) {
				case '.':
					i = 0;
					++uart2_count;
					break;
				case '\r':
					osMessageQueuePut(ble_ITHandle, &ble_data, 0U, 0U);
					break;
				case '\n':
					memset(ble_data.cmd, 0x00, 4);
					ble_data.idx = 0x00;
					memset(ble_data.pos1, 0x00, 3);
					memset(ble_data.pos2, 0x00, 3);
					memset(ble_data.pos3, 0x00, 3);
					memset(ble_data.pos4, 0x00, 3);
					memset(ble_data.pos5, 0x00, 3);
					memset(ble_data.pos6, 0x00, 3);

					i = 0;
					uart2_count = 0;

					HAL_GPIO_WritePin(GPIOD, GPIO_PIN_15, RESET);
					break;
				default:
					switch (uart2_count) {
					case 0:
						ble_data.cmd[i++] = uart2_rx;
						break;
					case 1:
						ble_data.idx = uart2_rx;
						break;
					case 2:
						ble_data.pos1[i++] = uart2_rx;
						break;
					case 3:
						ble_data.pos2[i++] = uart2_rx;
						break;
					case 4:
						ble_data.pos3[i++] = uart2_rx;
						break;
					case 5:
						ble_data.pos4[i++] = uart2_rx;
						break;
					case 6:
						ble_data.pos5[i++] = uart2_rx;
						break;
					case 7:
						ble_data.pos6[i++] = uart2_rx;
						break;
					}
					break;
				}

				HAL_UART_Receive_IT(&huart2, &uart2_rx, 1);
//    osDelay(1);
			}
		}
	}
  /* USER CODE END BLEInterrupt */
}

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM6 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM6) {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */

  /* USER CODE END Callback 1 */
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
	/* User can add his own implementation to report the HAL error return state */
	__disable_irq();
	while (1) {
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
