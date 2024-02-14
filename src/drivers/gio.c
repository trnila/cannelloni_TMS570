#include "gio.h"

#define GIO_GCR0_RESET_SHIFT 0

void gio_init(void) {
  gioREG->GCR0 = 1U << GIO_GCR0_RESET_SHIFT;
}

void gio_mode_output(gioPORT_t *port, uint32_t pin, uint32_t level) {
  port->DIR |= 1U << pin;
  if (level) {
    port->DSET |= 1U << pin;
  } else {
    port->DCLR |= 1U << pin;
  }
}
