#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "HL_reg_can.h"

#define CAN_RX_QUEUE_FIRST_MBOX 2
#define CAN_MBOX_LAST 64

void can_init(canBASE_t *canreg);
bool can_mbox_has_data(canBASE_t *canreg, uint8_t mbox);
void can_fill_rx_mbox(canBASE_t *canreg, uint8_t mbox, uint32_t *id, uint8_t *len, uint8_t *data);
bool can_send(canBASE_t *canreg, uint32_t id, uint8_t dlc, const uint8_t *data);
