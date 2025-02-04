/*
 * df2301q.c
 *
 *  Created on: Jan 17, 2025
 *      Author: KkangJun
 */

#include "df2301q.h"

void DF2301QGetCMDID() {
	if (HAL_I2C_GetState(&hi2c1) == HAL_I2C_STATE_READY) {
		HAL_I2C_Mem_Read_IT(&hi2c1, DF2301Q_I2C_ADDR, 0x02,
				I2C_MEMADD_SIZE_8BIT, &i2c1_rx, 1);
	}
}

void DF2301QPlayByCMDID(uint8_t cmdid) {
	uint8_t data = cmdid;
	HAL_I2C_Mem_Write(&hi2c1, DF2301Q_I2C_ADDR, 0x03, I2C_MEMADD_SIZE_8BIT,
			&data, 1, 10);
	osDelay(2);
}

void DF2301QGetWakeTime() {
	HAL_I2C_Mem_Read_IT(&hi2c1, DF2301Q_I2C_ADDR, 0x06, I2C_MEMADD_SIZE_8BIT,
			&i2c1_rx, 1);
}

void DF2301QSetWakeTime(uint8_t waketime) {
	uint8_t data = waketime;
	HAL_I2C_Mem_Write(&hi2c1, DF2301Q_I2C_ADDR, 0x06, I2C_MEMADD_SIZE_8BIT,
			&data, 1, 10);
	osDelay(2);
}

void DF2301QSetVolume(int vol) {
	if (vol < 0) {
		vol = 0;
	} else if (vol > 20) {
		vol = 20;
	}

	uint8_t data = (uint8_t) vol;
	HAL_I2C_Mem_Write(&hi2c1, DF2301Q_I2C_ADDR, 0x05, I2C_MEMADD_SIZE_8BIT,
			&data, 1, 10);
	osDelay(2);
}

void DF2301QSetMuteMode(uint8_t mode) {
	/* Mute ON: mode = 1, Mute OFF: mode 0*/
	if (mode != 0) {
		mode = 1;
	}

	uint8_t data = mode;
	HAL_I2C_Mem_Write(&hi2c1, DF2301Q_I2C_ADDR, 0x04, I2C_MEMADD_SIZE_8BIT,
			&data, 1, 10);
	osDelay(2);
}
