#pragma once
#include <stdint.h>
#include "HL_reg_gio.h"

void gio_init(void);
void gio_mode_output(gioPORT_t *port, uint32_t pin, uint32_t level);
