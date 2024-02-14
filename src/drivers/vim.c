#include "HL_reg_vim.h"
#include "HL_system.h"
#include "HL_reg_esm.h"

typedef void (*isr_handler)();

#define vimRAM ((isr_handler*)0xFFF82000U)
#define VIM_CHANNELS 128U

#pragma CODE_STATE(esmHighInterrupt, 32)
#pragma INTERRUPT(esmHighInterrupt, FIQ)
void esmHighInterrupt(void) {
  esmREG->SR7[0U] = 0xFFFFFFFFU;
  esmREG->SR4[0U] = 0xFFFFFFFFU;
  esmREG->SR1[1U] = 0xFFFFFFFFU;
  esmREG->SR1[0U] = 0xFFFFFFFFU;
}

#pragma CODE_STATE(phantomInterrupt, 32)
#pragma INTERRUPT(phantomInterrupt, IRQ)
void phantomInterrupt(void) {}

void vim_register_irq(int channel, isr_handler handler) {
  vimRAM[channel + 1] = handler;

  volatile uint32_t* reg = &vimREG->REQMASKSET0;
  reg[channel / 32] |= 1U << (channel % 32);
}

void vim_init(void) {
  for (uint32_t i = 0U; i < VIM_CHANNELS; i++) {
    vimRAM[i] = &phantomInterrupt;
  }

  vim_register_irq(0, esmHighInterrupt);
}
