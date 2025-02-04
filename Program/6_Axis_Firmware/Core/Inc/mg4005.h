/*
 * mg4005.h
 *
 *  Created on: Jan 11, 2025
 *      Author: KkangJun
 */

#ifndef INC_MG4005_H_
#define INC_MG4005_H_

#include "stm32f4xx_hal.h"
#include "cmsis_os2.h"

extern CAN_HandleTypeDef hcan2;

extern CAN_FilterTypeDef canFilter;
extern CAN_RxHeaderTypeDef canRxHeader;
extern CAN_TxHeaderTypeDef canTxHeader;
extern uint8_t canRx0Data[8];
extern uint32_t canTxMailbox;
extern uint8_t canTxData[8];

void InitCANFilter(void);
void MG4005Commend(uint8_t id, uint8_t cmd, uint8_t *data);
void MG4005Off(uint8_t id);
void MG4005On(uint8_t id);
void MG4005Stop(uint8_t id);
void MG4005BrakeAct(uint8_t id);
void MG4005BrakeDeact(uint8_t id);
void MG4005MultiLoop2(uint8_t id, uint16_t speed, int32_t angle);
void MG4005SingleLoop2(uint8_t id, uint8_t clockwise, uint16_t speed, uint32_t angle);
void MG4005Incremental2(uint8_t id, uint16_t speed, uint32_t angle);
void MG4005ReadEncoder(uint8_t id);
void MG4005SetZeroPosROM(uint8_t id);
void MG4005SetZeroPosRAM(uint8_t id, int mid_angle);
void MG4005ReadMultiLoop(uint8_t id);
void MG4005ReadSingleLoop(uint8_t id);
void MG4005GetEncoderVal(uint8_t *data, uint16_t *parse_data);
void MG4005GetMultiVal(uint8_t *data, int64_t *parse_data);
void MG4005GetSingleVal(uint8_t *data, uint32_t *parse_data);

#endif /* INC_MG4005_H_ */
