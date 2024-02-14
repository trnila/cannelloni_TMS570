/*
 * Copyright (C) 2009-2016 Texas Instruments Incorporated - www.ti.com
 *
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *
 *    Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 *    Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the
 *    distribution.
 *
 *    Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

/* Include Files */

#include "HL_system.h"
#include "HL_reg_pcr.h"
#include "HL_pinmux.h"

void setupPLL(void) {
  /** - Configure PLL control registers */
  /** @b Initialize @b Pll1: */

  /**   - Setup pll control register 1:
   *     - Setup reset on oscillator slip
   *     - Setup bypass on pll slip
   *     - setup Pll output clock divider to max before Lock
   *     - Setup reset on oscillator fail
   *     - Setup reference clock divider
   *     - Setup Pll multiplier
   */
  systemREG1->PLLCTL1 = (uint32_t)0x20000000U | (uint32_t)((uint32_t)0x1FU << 24U) |
                        (uint32_t)((uint32_t)(8U - 1U) << 16U) | (uint32_t)(0x9500U);

  /**   - Setup pll control register 2
   *     - Setup spreading rate
   *     - Setup bandwidth adjustment
   *     - Setup internal Pll output divider
   *     - Setup spreading amount
   */
  systemREG1->PLLCTL2 =
      (uint32_t)((uint32_t)255U << 22U) | (uint32_t)((uint32_t)7U << 12U) | (uint32_t)61U;

  /** @b Initialize @b Pll2: */

  /**   - Setup pll2 control register :
   *     - setup Pll output clock divider to max before Lock
   *     - Setup reference clock divider
   *     - Setup internal Pll output divider
   *     - Setup Pll multiplier
   */
  systemREG2->PLLCTL3 = (uint32_t)((uint32_t)0x1FU << 24U) |
                        (uint32_t)((uint32_t)(8U - 1U) << 16U) | (uint32_t)(0x2700U);

  /** - Enable PLL(s) to start up or Lock */
  systemREG1->CSDIS = 0x00000008U | 0x00000080U | 0x00000004U;
}

void trimLPO(void) {
  /** @b Initialize Lpo: */
  /** Load TRIM values from OTP if present else load user defined values */
  /*SAFETYMCUSW 139 S MR:13.7 <APPROVED> "Hardware status bit read check" */
  if (LPO_TRIM_VALUE != 0xFFFFU) {
    systemREG1->LPOMONCTL = (uint32_t)((uint32_t)1U << 24U) | LPO_TRIM_VALUE;
  } else {
    systemREG1->LPOMONCTL =
        (uint32_t)((uint32_t)1U << 24U) | (uint32_t)((uint32_t)16U << 8U) | 16U;
  }
}

void setupFlash(void) {
  /** - Setup flash read mode, address wait states and data wait states */
  flashWREG->FRDCNTL = (uint32_t)((uint32_t)3U << 8U) | 3U;

  /** - Setup flash access wait states for bank 7 */
  FSM_WR_ENA_HL = 0x5U;
  EEPROM_CONFIG_HL = 0x00000002U | (uint32_t)((uint32_t)9U << 16U);

  /** - Disable write access to flash state machine registers */
  FSM_WR_ENA_HL = 0x2U;

  /** - Setup flash bank power modes */
  flashWREG->FBPWRMODE = (uint32_t)((uint32_t)SYS_ACTIVE << 14U)   /* BANK 7 */
                         | (uint32_t)((uint32_t)SYS_ACTIVE << 2U)  /* BANK 1 */
                         | (uint32_t)((uint32_t)SYS_ACTIVE << 0U); /* BANK 0 */
}

void periphInit(void) {
  /** - Disable Peripherals before peripheral powerup*/
  systemREG1->CLKCNTL &= 0xFFFFFEFFU;

  /** - Release peripherals from reset and enable clocks to all peripherals */
  /** - Power-up all peripherals */
  pcrREG1->PSPWRDWNCLR0 = 0xFFFFFFFFU;
  pcrREG1->PSPWRDWNCLR1 = 0xFFFFFFFFU;
  pcrREG1->PSPWRDWNCLR2 = 0xFFFFFFFFU;
  pcrREG1->PSPWRDWNCLR3 = 0xFFFFFFFFU;

  pcrREG2->PSPWRDWNCLR0 = 0xFFFFFFFFU;
  pcrREG2->PSPWRDWNCLR1 = 0xFFFFFFFFU;
  pcrREG2->PSPWRDWNCLR2 = 0xFFFFFFFFU;
  pcrREG2->PSPWRDWNCLR3 = 0xFFFFFFFFU;

  pcrREG3->PSPWRDWNCLR0 = 0xFFFFFFFFU;
  pcrREG3->PSPWRDWNCLR1 = 0xFFFFFFFFU;
  pcrREG3->PSPWRDWNCLR2 = 0xFFFFFFFFU;
  pcrREG3->PSPWRDWNCLR3 = 0xFFFFFFFFU;

  /** - Enable Peripherals */
  systemREG1->CLKCNTL |= 0x00000100U;
}

void mapClocks(void) {
  uint32_t SYS_CSVSTAT, SYS_CSDIS;

  /** @b Initialize @b Clock @b Tree: */
  /** - Setup system clock divider for HCLK */
  systemREG2->HCLKCNTL = 1U;

  /* Always check the CSDIS register to make sure the clock source is turned on
   * and check the CSVSTAT register to make sure the clock source is valid. Then
   * write to GHVSRC to switch the clock.
   */
  /** - Wait for until clocks are locked */
  SYS_CSVSTAT = systemREG1->CSVSTAT;
  SYS_CSDIS = systemREG1->CSDIS;
  while ((SYS_CSVSTAT & ((SYS_CSDIS ^ 0xFFU) & 0xFFU)) !=
         ((SYS_CSDIS ^ 0xFFU) & 0xFFU)) {
    SYS_CSVSTAT = systemREG1->CSVSTAT;
    SYS_CSDIS = systemREG1->CSDIS;
  } /* Wait */

  /** - Map device clock domains to desired sources and configure top-level
   * dividers */
  /** - All clock domains are working off the default clock sources until now */
  /** - The below assignments can be easily modified using the HALCoGen GUI */

  /** - Setup GCLK, HCLK and VCLK clock source for normal operation, power down
   * mode and after wakeup */
  systemREG1->GHVSRC = (uint32_t)((uint32_t)SYS_PLL1 << 24U) |
                       (uint32_t)((uint32_t)SYS_PLL1 << 16U) |
                       (uint32_t)((uint32_t)SYS_PLL1 << 0U);

  /** - Setup RTICLK1 and RTICLK2 clocks */
  systemREG1->RCLKSRC =
      (uint32_t)((uint32_t)1U
                 << 24U)                      /* RTI2 divider (Not applicable for lock-step device)  */
      | (uint32_t)((uint32_t)SYS_VCLK << 16U) /* RTI2 clock source (Not applicable
                                             for lock-step device) */
      | (uint32_t)((uint32_t)1U << 8U)        /* RTI1 divider */
      | (uint32_t)((uint32_t)SYS_VCLK << 0U); /* RTI1 clock source */

  /** - Setup asynchronous peripheral clock sources for AVCLK1 and AVCLK2 */
  systemREG1->VCLKASRC =
      (uint32_t)((uint32_t)SYS_VCLK << 8U) | (uint32_t)((uint32_t)SYS_VCLK << 0U);

  /** - Setup synchronous peripheral clock dividers for VCLK1, VCLK2, VCLK3 */
  systemREG1->CLKCNTL =
      (systemREG1->CLKCNTL & 0xF0FFFFFFU) | (uint32_t)((uint32_t)1U << 24U);
  systemREG1->CLKCNTL =
      (systemREG1->CLKCNTL & 0xFFF0FFFFU) | (uint32_t)((uint32_t)1U << 16U);

  systemREG2->CLK2CNTRL =
      (systemREG2->CLK2CNTRL & 0xFFFFFFF0U) | (uint32_t)((uint32_t)1U << 0U);

  systemREG2->VCLKACON1 = (uint32_t)((uint32_t)(2U - 1U) << 24U) |
                          (uint32_t)((uint32_t)SYS_VCLK << 16U) |
                          (uint32_t)((uint32_t)SYS_VCLK << 0U);

  /* Now the PLLs are locked and the PLL outputs can be sped up */
  /* The R-divider was programmed to be 0xF. Now this divider is changed to
   * programmed value */
  systemREG1->PLLCTL1 &= 0xE0FFFFFFU;
  systemREG2->PLLCTL3 &= 0xE0FFFFFFU;
}

void systemInit(void) {
  setupPLL();
  periphInit();
  pinmux_init();
  setupFlash();
  trimLPO();
  mapClocks();
}
