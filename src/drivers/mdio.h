#pragma once
#include <stdint.h>

void mdio_init(uint32_t clk);
void mdio_disable();
uint16_t mdio_read(uint32_t phyid, uint16_t addr);
void mdio_write(uint32_t phyid, uint16_t addr, uint16_t value);
