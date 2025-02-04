/*
 * mg4005.c
 *
 *  Created on: Jan 11, 2025
 *      Author: KkangJun
 */

#include "mg4005.h"

void InitCANFilter(void) {
	canFilter.FilterMaskIdHigh = 0x7F0 << 5;
	canFilter.FilterIdHigh = 0x140 << 5;
	canFilter.FilterMaskIdLow = 0x7F0 << 5;
	canFilter.FilterIdLow = 0x140 << 5;
	canFilter.FilterMode = CAN_FILTERMODE_IDMASK;
	canFilter.FilterScale = CAN_FILTERSCALE_16BIT;
	canFilter.FilterFIFOAssignment = CAN_FILTER_FIFO0;
	canFilter.FilterBank = 0;
	canFilter.FilterActivation = ENABLE;

	HAL_CAN_ConfigFilter(&hcan2, &canFilter);

	HAL_CAN_ActivateNotification(&hcan2, CAN_IT_RX_FIFO0_MSG_PENDING);

	HAL_CAN_Start(&hcan2);
}

void MG4005Commend(uint8_t id, uint8_t cmd, uint8_t *data) {
	canTxHeader.StdId = 0x140 + id;
	canTxHeader.RTR = CAN_RTR_DATA;
	canTxHeader.IDE = CAN_ID_STD;
	canTxHeader.DLC = 8;

	canTxData[0] = cmd;
	canTxData[1] = data[0];
	canTxData[2] = data[1];
	canTxData[3] = data[2];
	canTxData[4] = data[3];
	canTxData[5] = data[4];
	canTxData[6] = data[5];
	canTxData[7] = data[6];

	canTxMailbox = HAL_CAN_GetTxMailboxesFreeLevel(&hcan2);
	HAL_CAN_AddTxMessage(&hcan2, &canTxHeader, canTxData, &canTxMailbox);

	osDelay(15);
}

void MG4005Off(uint8_t id) {
	uint8_t data[7] = { 0x00 };
	MG4005Commend(id, 0x80, data);
}

void MG4005On(uint8_t id) {
	uint8_t data[7] = { 0x00 };
	MG4005Commend(id, 0x88, data);
}

void MG4005Stop(uint8_t id) {
	uint8_t data[7] = { 0x00 };
	MG4005Commend(id, 0x81, data);
}

void MG4005BrakeAct(uint8_t id) {
	uint8_t data[7] = { 0x00 };
	data[0] = 0x00;
	MG4005Commend(id, 0x8C, data);
}

void MG4005BrakeDeact(uint8_t id) {
	uint8_t data[7] = { 0x00 };
	data[0] = 0x01;
	MG4005Commend(id, 0x8C, data);
}

void MG4005MultiLoop2(uint8_t id, uint16_t speed, int32_t angle) {
	/* Angle Range: 0 ~ 360
	 * 1 = 1 [deg], counter clockwise
	 *
	 * angle Range(Recommended): 0 ~ 1000
	 * 1 = 1 [dps]
	 * */
	uint8_t data[7] = { 0x00 };
	angle *= 1000;

	data[0] = 0x00;
	data[1] = *((uint8_t*) (&speed));
	data[2] = *((uint8_t*) (&speed) + 1);
	data[3] = *((uint8_t*) (&angle));
	data[4] = *((uint8_t*) (&angle) + 1);
	data[5] = *((uint8_t*) (&angle) + 2);
	data[6] = *((uint8_t*) (&angle) + 3);

	MG4005Commend(id, 0xA4, data);
}

void MG4005SingleLoop2(uint8_t id, uint8_t clockwise, uint16_t speed,
		uint32_t angle) {
	uint8_t data[7] = { 0x00 };

	data[0] = clockwise;
	data[1] = *((uint8_t*) (&speed));
	data[2] = *((uint8_t*) (&speed) + 1);
	data[3] = *((uint8_t*) (&angle));
	data[4] = *((uint8_t*) (&angle) + 1);
	data[5] = *((uint8_t*) (&angle) + 2);
	data[6] = *((uint8_t*) (&angle) + 3);

	MG4005Commend(id, 0xA6, data);
}

void MG4005Incremental2(uint8_t id, uint16_t speed, uint32_t angle) {
	uint8_t data[7] = { 0x00 };

	data[0] = 0x00;
	data[1] = *((uint8_t*) (&speed));
	data[2] = *((uint8_t*) (&speed) + 1);
	data[3] = *((uint8_t*) (&angle));
	data[4] = *((uint8_t*) (&angle) + 1);
	data[5] = *((uint8_t*) (&angle) + 2);
	data[6] = *((uint8_t*) (&angle) + 3);

	MG4005Commend(id, 0xA8, data);
}

void MG4005ReadEncoder(uint8_t id) {
	uint8_t data[7] = { 0x00 };
	MG4005Commend(id, 0x90, data);
}

void MG4005SetZeroPosROM(uint8_t id) {
	uint8_t data[7] = { 0x00 };
	MG4005Commend(id, 0x19, data);
}

void MG4005SetZeroPosRAM(uint8_t id, int mid_angle) {
	// for multi loop angle
	uint8_t data[7] = { 0x00 };

	data[3] = *((uint8_t*) (&mid_angle));
	data[4] = *((uint8_t*) (&mid_angle) + 1);
	data[5] = *((uint8_t*) (&mid_angle) + 2);
	data[6] = *((uint8_t*) (&mid_angle) + 3);

	MG4005Commend(id, 0x95, data);
}

void MG4005ReadMultiLoop(uint8_t id) {
	uint8_t data[7] = { 0x00 };
	MG4005Commend(id, 0x92, data);
}

void MG4005ReadSingleLoop(uint8_t id) {
	uint8_t data[7] = { 0x00 };
	MG4005Commend(id, 0x94, data);
}

void MG4005GetEncoderVal(uint8_t *data, uint16_t *parse_data) {
	parse_data[1] = ((uint16_t) data[5] << 8) + data[4]; // encoder_original
	parse_data[2] = ((uint16_t) data[7] << 8) + data[6]; // encoder_zero

	if (parse_data[1] < parse_data[2]) {
		parse_data[0] = ~(((uint16_t) data[3] << 8) + data[2]) + 1; // encoder_position
	} else {
		parse_data[0] = ((uint16_t) data[3] << 8) + data[2]; // encoder_position
	}
}

void MG4005GetMultiVal(uint8_t *data, int64_t *parse_data) {
	*parse_data = ((int64_t) data[7] << 56) + ((int64_t) data[6] << 48)
			+ ((int64_t) data[5] << 40) + ((int64_t) data[4] << 32)
			+ ((int64_t) data[3] << 24) + ((int64_t) data[2] << 16)
			+ ((int64_t) data[1] << 8) + data[0];
}

void MG4005GetSingleVal(uint8_t *data, uint32_t *parse_data) {
	*parse_data = ((uint32_t) data[7] << 24) + ((uint32_t) data[6] << 16)
			+ ((uint32_t) data[5] << 8) + data[4];
}
