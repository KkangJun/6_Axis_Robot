/*
 * at24cxx.c
 *
 *  Created on: Jan 17, 2025
 *      Author: KkangJun
 */

#include "at24cxx.h"

int _write(int file, char *p, int len) {
	for (int i = 0; i < len; i++) {
		ITM_SendChar((*p++));
	}
	return len;
}

void AT24Write(uint8_t mem_addr, uint8_t *pData, uint8_t size) {
	while(HAL_I2C_GetState(&hi2c1) != HAL_I2C_STATE_READY);

	uint8_t error;
	error = HAL_I2C_Mem_Write(&hi2c1, AT24CXX_ADDR, mem_addr,
			I2C_MEMADD_SIZE_8BIT, pData, size, 10);

	printf("write: 0x%x\r\n", error);
	osDelay(10);
}

void AT24Read(uint8_t mem_addr, uint8_t *pData, uint8_t size) {
	uint8_t error;
	while(HAL_I2C_GetState(&hi2c1) != HAL_I2C_STATE_READY);

	error = HAL_I2C_Mem_Read(&hi2c1, AT24CXX_ADDR, mem_addr,
			I2C_MEMADD_SIZE_8BIT, pData, size, 10);

	printf("read: 0x%x\r\n", error);
	osDelay(10);
}
