#include "drivers/mdio.h"

#define DP8386_PHYID 3U
#define DP8386_RGMIICTL 0x32U
#define RGMII_RX_CLK_DELAY_SHIFT 0
#define RGMII_TX_CLK_DELAY_SHIFT 1
#define RGMII_TX_HALF_FULL_THR_SHIFT 3
#define RGMII_RX_HALF_FULL_THR_SHIFT 5
#define RGMII_EN_SHIFT 7

static void delay() {
  for (int i = 0; i < 128; i++) {
    asm(" NOP");
  }
}

void DP8386_init() {
  mdio_init(100000U);

  uint16_t RGMIICTL = (1U << RGMII_EN_SHIFT) | (0b10 << RGMII_RX_HALF_FULL_THR_SHIFT) | (0b10 << RGMII_TX_HALF_FULL_THR_SHIFT) | (1U << RGMII_TX_CLK_DELAY_SHIFT) | (1U << RGMII_RX_CLK_DELAY_SHIFT);
  for (;;) {
    mdio_write(DP8386_PHYID, DP8386_RGMIICTL, RGMIICTL);

    delay();
    if (mdio_read(DP8386_PHYID, DP8386_RGMIICTL) == RGMIICTL) {
      break;
    }

    delay();
  }

  mdio_disable();
}
