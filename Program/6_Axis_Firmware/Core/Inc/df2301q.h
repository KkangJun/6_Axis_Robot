/*
 * df2301q.h
 *
 *  Created on: Jan 17, 2025
 *      Author: KkangJun
 */

#ifndef INC_DF2301Q_H_
#define INC_DF2301Q_H_

#include "stm32f4xx_hal.h"

#define DF2301Q_I2C_ADDR	0x64 << 1
#define DF2301Q_MUTE_OFF	0x00
#define DF2301Q_MUTE_ON	0x01

extern I2C_HandleTypeDef hi2c1;

extern uint8_t i2c1_rx;

void DF2301QGetCMDID();
void DF2301QPlayByCMDID(uint8_t cmdid);
void DF2301QGetWakeTime();
void DF2301QSetWakeTime(uint8_t waketime);
void DF2301QSetVolume(int vol);
void DF2301QSetMuteMode(uint8_t mode);

#endif /* INC_DF2301Q_H_ */
