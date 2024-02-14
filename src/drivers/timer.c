#include "timer.h"
#include "HL_reg_rti.h"
#include "drivers/vim.h"

#define RTI_INT_CMP0 1U

#define RTI_GCTRL_NTUSEL_NTU1 0x5U
#define RTI_GCTRL_NTUSEL_SHIFT 16

volatile uint32_t tick_ms = 0;

#pragma CODE_STATE(rtiCompare0Interrupt, 32)
#pragma INTERRUPT(rtiCompare0Interrupt, IRQ)
void rtiCompare0Interrupt(void) {
  rtiREG1->INTFLAG = RTI_INT_CMP0;
  tick_ms++;
}

void timer_init(void) {
  vim_register_irq(2, rtiCompare0Interrupt);

  // select NTU1 as a source
  rtiREG1->GCTRL = RTI_GCTRL_NTUSEL_NTU1 << RTI_GCTRL_NTUSEL_SHIFT;

  // set counter value
  rtiREG1->CNT[0U].CPUCx = 7U;
  rtiREG1->CMP[0U].COMPx = 9375U;
  rtiREG1->CMP[0U].UDCPx = 9375U;

  // enable interrupt
  rtiREG1->INTFLAG = RTI_INT_CMP0;
  rtiREG1->SETINTENA = RTI_INT_CMP0;

  // start the counter
  rtiREG1->GCTRL |= ((uint32_t)1U << (0 & 3U));
}
