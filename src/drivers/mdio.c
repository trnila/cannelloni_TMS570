#include <stdint.h>
#include "HL_hw_reg_access.h"
#include "HL_hw_mdio.h"
#include "HL_system.h"

#define PHY_REG_MASK 0x1FU
#define PHY_ADDR_MASK 0x1FU
#define PHY_DATA_MASK 0xFFFFU
#define PHY_REG_SHIFT 21U
#define PHY_ADDR_SHIFT 16U

// last standardly addressed register (only 5 bits, extended registers are accessed indirectly)
#define MDIO_LAST_STD_REG 31

// registers for extended addressing
#define MDIO_ADDAR 0xEU
#define MDIO_REGCR 0xDU
#define MDIO_REGCR_DEVAD_ALL 0x1FU
#define MDIO_REGCR_FN_ADDR 0U
#define MDIO_REGCR_FN_DATA_NO_INCR (1U << 14U)

#define DP8386_PHYID 3U
#define DP8386_RGMIICTL 0x32U
#define RGMII_RX_CLK_DELAY_SHIFT 0
#define RGMII_TX_CLK_DELAY_SHIFT 1
#define RGMII_TX_HALF_FULL_THR_SHIFT 3
#define RGMII_RX_HALF_FULL_THR_SHIFT 5
#define RGMII_EN_SHIFT 7

static void mdio_wait_until_rdy() {
  while ((HWREG(MDIO_BASE + MDIO_USERACCESS0) & MDIO_USERACCESS0_GO) == MDIO_USERACCESS0_GO)
    ;
}

static uint16_t mdio_reg_read(uint32_t phyAddr, uint32_t regNum) {
  mdio_wait_until_rdy();

  HWREG(MDIO_BASE + MDIO_USERACCESS0) =
      (((uint32_t)MDIO_USERACCESS0_READ) | MDIO_USERACCESS0_GO |
       ((regNum & PHY_REG_MASK) << PHY_REG_SHIFT) |
       ((phyAddr & PHY_ADDR_MASK) << PHY_ADDR_SHIFT));

  mdio_wait_until_rdy();

  // use rx data only if acknowledged
  if (((HWREG(MDIO_BASE + MDIO_USERACCESS0)) & MDIO_USERACCESS0_ACK) == MDIO_USERACCESS0_ACK) {
    return HWREG(MDIO_BASE + MDIO_USERACCESS0) & PHY_DATA_MASK;
  }
  return 0;
}

static void mdio_reg_write(uint32_t phyAddr, uint32_t regNum, uint16_t RegVal) {
  mdio_wait_until_rdy();

  HWREG(MDIO_BASE + MDIO_USERACCESS0) = (MDIO_USERACCESS0_WRITE | MDIO_USERACCESS0_GO |
                                         ((regNum & PHY_REG_MASK) << PHY_REG_SHIFT) |
                                         ((phyAddr & PHY_ADDR_MASK) << PHY_ADDR_SHIFT) | RegVal);
  mdio_wait_until_rdy();
}

void mdio_init(uint32_t clk) {
  uint32_t periph_clk = VCLK3_FREQ * 1000000U;
  uint32_t clkDiv = (periph_clk / clk) - 1U;
  HWREG(MDIO_BASE + MDIO_CONTROL) = ((clkDiv & MDIO_CONTROL_CLKDIV) | MDIO_CONTROL_ENABLE | MDIO_CONTROL_FAULTENB);
}

void mdio_disable() {
  HWREG(MDIO_BASE + MDIO_CONTROL) &= ~MDIO_CONTROL_ENABLE;
}

static uint8_t mdio_send_addr(uint32_t phyid, uint16_t addr) {
  if (addr <= MDIO_LAST_STD_REG) {
    return addr;
  }
  mdio_reg_write(phyid, MDIO_REGCR, MDIO_REGCR_FN_ADDR | MDIO_REGCR_DEVAD_ALL);
  mdio_reg_write(phyid, MDIO_ADDAR, addr);
  mdio_reg_write(phyid, MDIO_REGCR, MDIO_REGCR_FN_DATA_NO_INCR | MDIO_REGCR_DEVAD_ALL);
  return MDIO_ADDAR;
}

uint16_t mdio_read(uint32_t phyid, uint16_t addr) {
  addr = mdio_send_addr(phyid, addr);
  return mdio_reg_read(phyid, addr);
}

void mdio_write(uint32_t phyid, uint16_t addr, uint16_t value) {
  addr = mdio_send_addr(phyid, addr);
  mdio_reg_write(phyid, addr, value);
}
