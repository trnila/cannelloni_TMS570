#include "spi.h"
#include "HL_reg_mibspi.h"

#define CS 0
#define CS_DELAY 45U
#define TRANSACTION_DELAY 0xFFU

#define SPIGCR1_MASTER_SHIFT 0U
#define SPIGCR1_CLKMOD_SHIFT 1U
#define SPIGCR1_SPIEN_SHIFT 24U

#define SPIPC0_SCS0_SHIFT 0
#define SPIPC0_SCS1_SHIFT 1U
#define SPIPC0_CLK_SHIFT 9U
#define SPIPC0_SIMO_SHIFT 10U
#define SPIPC0_SOMI_SHIFT 11U

#define SPIDELAY_C2TDELAY_SHIFT 24U
#define SPIDELAY_T2CDELAY_SHIFT 16U

#define SPIFMT_CHARLEN_SHIFT 0U
#define SPIFMT_PRESCALE_SHIFT 8U
#define SPIFMT_WDELAY_SHIFT 24U

#define SPIDAT1_CSNR_SHIFT 16U
#define SPIDAT1_DFSEL_SHIFT 24U
#define SPIDAT1_WDEL_SHIFT 26U
#define SPIDAT1_CSHOLD_SHIFT 28U

#define SPIBUF_TXFULL_SHIFT 29U

void spi_init() {
  mibspiREG2->GCR0 = 1U;
  mibspiREG2->PC0 = (1U << SPIPC0_SCS0_SHIFT) | (1U << SPIPC0_SCS1_SHIFT) | (1U << SPIPC0_CLK_SHIFT) | (1U << SPIPC0_SIMO_SHIFT) | (1U << SPIPC0_SOMI_SHIFT);
  mibspiREG2->GCR1 = (1U << SPIGCR1_MASTER_SHIFT) | (1U << SPIGCR1_CLKMOD_SHIFT) | (1U << SPIGCR1_SPIEN_SHIFT);
  mibspiREG2->DELAY = (CS_DELAY << SPIDELAY_C2TDELAY_SHIFT) | (CS_DELAY << SPIDELAY_T2CDELAY_SHIFT);

  for (int i = 0; i < 128; i++) {
    asm(" NOP");
  }

  mibspiREG2->FMT0 = (TRANSACTION_DELAY << SPIFMT_WDELAY_SHIFT) | (74U << SPIFMT_PRESCALE_SHIFT) | (8U << SPIFMT_CHARLEN_SHIFT);
}

void spi_transfer(uint8_t *tx, size_t len) {
  for (size_t i = 0; i < len; i++) {
    // wait empty TX buffer
    while ((mibspiREG2->BUF & (1U << SPIBUF_TXFULL_SHIFT)) != 0)
      ;

    uint32_t flags = ((0xFFU & ~(1U << CS)) << SPIDAT1_CSNR_SHIFT) | (1U << SPIDAT1_WDEL_SHIFT);
    flags |= ((uint32_t)CS) << SPIDAT1_DFSEL_SHIFT;
    if (i + 1 != len) {
      flags |= 1U << SPIDAT1_CSHOLD_SHIFT;
    }
    mibspiREG2->DAT1 = tx[i] | flags;
  }
}
