/*
 * at24cxx.h
 *
 *  Created on: Jan 17, 2025
 *      Author: KkangJun
 */

#ifndef INC_AT24CXX_H_
#define INC_AT24CXX_H_

#include "stm32f4xx_hal.h"

#define AT24CXX_ADDR 0xA0
#define EEPROM_BUSY 0x01

extern I2C_HandleTypeDef hi2c1;

void AT24Write(uint8_t mem_addr, uint8_t *pData, uint8_t size);
void AT24Read(uint8_t mem_addr, uint8_t *pData, uint8_t size);

#endif /* INC_AT24CXX_H_ */
