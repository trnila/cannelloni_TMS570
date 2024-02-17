#include "can.h"

#define DCAN_IFARB_MSGVAL_SHIFT 31
#define DCAN_IFARB_XTD_SHIFT 30
#define DCAN_IFARB_DIR_SHIFT 29

#define DCAN_IFMCTL_NEWDAT_SHIFT 15
#define DCAN_IFMCTL_MSGLST_SHIFT 14
#define DCAN_IFMCTL_INTPND_SHIFT 13
#define DCAN_IFMCTL_UMASK_SHIFT 12
#define DCAN_IFMCTL_TXIE_SHIFT 11
#define DCAN_IFMCTL_RXIE_SHIFT 10
#define DCAN_IFMCTL_RMTEN_SHIFT 9
#define DCAN_IFMCTL_TXRQST_SHIFT 8
#define DCAN_IFMCTL_EOB_SHIFT 7
#define DCAN_IFMCTL_DLC_SHIFT 0
#define DCAN_IFMCTL_DLC_WIDTH 4

#define DCAN_IFCMD_WRRD_SHIFT 7
#define DCAN_IFCMD_MASK_SHIFT 6
#define DCAN_IFCMD_ARB_SHIFT 5
#define DCAN_IFCMD_CONTROL_SHIFT 4
#define DCAN_IFCMD_CLRINTPND_SHIFT 3
#define DCAN_IFCMD_TXRQST_SHIFT 2
#define DCAN_IFCMD_DATAA_SHIFT 1
#define DCAN_IFCMD_DATAB_SHIFT 0

#define DCAN_IFMSK_MDIR_SHIFT 30

#define DCAN_CTL_SECDED_DISABLE 0x5U
#define DCAN_CTL_SECDED_SHIFT 10
#define DCAN_CTL_INIT_SHIFT 0
#define DCAN_CTL_CCE_SHIFT 6

#define DCAN_BTR_BRP_SHIFT 0
#define DCAN_BTR_SJW_SHIFT 6
#define DCAN_BTR_TSEG1_SHIFT 8
#define DCAN_BTR_TSEG2_SHIFT 12
#define DCAN_BTR_BRPE_SHIFT 16

#define DCAN_IOC_PU_SHIFT 17
#define DCAN_IOC_FUNC_SHIFT 3

static const uint32_t data_byte_order[8U] = {3U, 2U, 1U, 0U, 7U, 6U, 5U, 4U};

static void can_if_wait_ready(canBASE_t *canreg) {
  while ((canreg->IF1STAT & 0x80U) == 0x80U) {
  }
}

void can_init(canBASE_t *canreg) {
  canreg->CTL = (DCAN_CTL_SECDED_DISABLE << DCAN_CTL_SECDED_SHIFT) | (1U << DCAN_CTL_INIT_SHIFT) | (1U << DCAN_CTL_CCE_SHIFT);

  // 1 mbox for TX, 2-64 mboxes reserved for RX
  for (int mbox = CAN_RX_QUEUE_FIRST_MBOX; mbox <= CAN_MBOX_LAST; mbox++) {
    can_if_wait_ready(canreg);
    canreg->IF1MSK = 1U << DCAN_IFMSK_MDIR_SHIFT;
    canreg->IF1ARB = (1U << DCAN_IFARB_MSGVAL_SHIFT) | (1U << DCAN_IFARB_XTD_SHIFT);
    canreg->IF1MCTL = (1U << DCAN_IFMCTL_UMASK_SHIFT) | ((mbox == 64) << DCAN_IFMCTL_EOB_SHIFT) | (1U << DCAN_IFMCTL_RXIE_SHIFT);
    canreg->IF1CMD = (1U << DCAN_IFCMD_WRRD_SHIFT) | (1U << DCAN_IFCMD_MASK_SHIFT) | (1U << DCAN_IFCMD_ARB_SHIFT) | (1U << DCAN_IFCMD_CONTROL_SHIFT) | (1U << DCAN_IFCMD_CLRINTPND_SHIFT);
    canreg->IF1NO = mbox;
  }

  canreg->BTR = (0 << DCAN_BTR_BRPE_SHIFT) | ((4U - 1U) << DCAN_BTR_TSEG2_SHIFT) | ((6U + 4U - 1U) << DCAN_BTR_TSEG1_SHIFT) | ((4U - 1U) << DCAN_BTR_SJW_SHIFT) | (9U << DCAN_BTR_BRP_SHIFT);

  // set rx/tx to CAN func
  canreg->RIOC = canreg->TIOC = (1U << DCAN_IOC_PU_SHIFT) | (1U << DCAN_IOC_FUNC_SHIFT);

  // go to normal operation
  canreg->CTL &= ~((1U << DCAN_CTL_INIT_SHIFT) | (1U << DCAN_CTL_CCE_SHIFT));
}

bool can_mbox_has_data(canBASE_t *canreg, uint8_t mbox) {
  if (mbox > CAN_MBOX_LAST) {
    return false;
  }

  can_if_wait_ready(canreg);
  canreg->IF1CMD =
      (1U << DCAN_IFCMD_ARB_SHIFT) | (1U << DCAN_IFCMD_CONTROL_SHIFT) |
      (1U << DCAN_IFCMD_CLRINTPND_SHIFT) | (1U << DCAN_IFCMD_TXRQST_SHIFT) |
      (1U << DCAN_IFCMD_DATAA_SHIFT) | (1U << DCAN_IFCMD_DATAB_SHIFT);
  canreg->IF1NO = mbox;
  can_if_wait_ready(canreg);
  return canreg->IF1MCTL & (1U << DCAN_IFMCTL_NEWDAT_SHIFT);
}

void can_fill_rx_mbox(canBASE_t *canreg, uint8_t mbox, uint32_t *id,
                      uint8_t *len, uint8_t *data) {
  *id = canreg->IF1ARB & 0x1FFFFFFFU;
  if (!(canreg->IF1ARB & (1U << DCAN_IFARB_XTD_SHIFT))) {
    *id >>= 18;
  }

  *len = canreg->IF1MCTL & 0b1111;
  for (uint8_t i = 0; i < *len; i++) {
    data[i] = canreg->IF1DATx[data_byte_order[i]];
  }
}

void can_send(canBASE_t *canreg, uint32_t id, uint8_t dlc,
              const uint8_t *data) {
  // TODO: find unused mailbox
  can_if_wait_ready(canreg);

  canreg->IF1ARB = (1U << DCAN_IFARB_MSGVAL_SHIFT) |
                   (1U << DCAN_IFARB_XTD_SHIFT) | (1U << DCAN_IFARB_DIR_SHIFT) |
                   (id & 0x1FFFFFFFU);

  canreg->IF1MCTL = (1U << DCAN_IFMCTL_NEWDAT_SHIFT) |
                    (1U << DCAN_IFMCTL_TXRQST_SHIFT) |
                    (dlc << DCAN_IFMCTL_DLC_SHIFT);

  for (int i = 0; i < dlc; i++) {
    canreg->IF1DATx[data_byte_order[i]] = data[i];
  }
  canreg->IF1CMD = (uint8_t)0xFFU;
  canreg->IF1NO = 1;
}
