#pragma once

#include <stddef.h>
#include <stdint.h>

void spi_init();
void spi_transfer(uint8_t *tx, size_t len);
