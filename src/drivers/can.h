#pragma once
#include "HL_reg_can.h"

void can_init(canBASE_t *canreg);
uint8_t can_get_rx_ready_mbox(canBASE_t *canreg);
void can_fill_rx_mbox(canBASE_t *canreg, uint8_t mbox, uint32_t *id, uint8_t *len, uint8_t *data);
void can_send(canBASE_t *canreg, uint32_t id, uint8_t dlc, const uint8_t *data);
