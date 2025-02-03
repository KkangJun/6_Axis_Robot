/*
 * XL430.h
 *
 *  Created on: Jan 14, 2025
 *      Author: KkangJun
 */

#ifndef INC_XL430_H_
#define INC_XL430_H_

#include "stm32f4xx_hal.h"
#include "cmsis_os2.h"

#define TORQUE_MODE_OFF 0x00
#define TORQUE_MODE_ON 0x01

extern UART_HandleTypeDef huart3;

extern uint8_t tx_packet[64];

void update_crc(uint16_t crc_accum, uint8_t *data_blk_ptr,
		uint16_t data_blk_size);
void xl430Cmd(uint8_t id, uint8_t instruction, uint8_t *param,
		uint16_t param_len);
void xl430SetID(uint8_t id, uint8_t set_id);
void xl430SetBaudRate(uint8_t id, uint8_t baudrate_mode);
void xl430TorqueEnable(uint8_t id, uint8_t Torque_mode);
void xl430GoalPos(uint8_t id, uint32_t pos);
void xl430GoalPosReg(uint8_t id, uint32_t pos);
void xl430RegAction();
void xl430ProfVelo(uint8_t id, uint32_t value);

#endif /* INC_XL430_H_ */
