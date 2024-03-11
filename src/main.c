#include "lwip/init.h"
#include "lwip/ip.h"
#include "lwip/timeouts.h"
#include "HL_reg_het.h"
#include "HL_system.h"
#include "netif/hdkif.h"
#include "drivers/spi.h"
#include "drivers/can.h"
#include "drivers/gio.h"
#include "drivers/timer.h"
#include "drivers/vim.h"
#include "cannelloni.h"

#define CAN_IFACES 4
#define CNL_BUF_SIZE 128

extern struct netif netif;
int instNum = 0;

void DP8386_init();
void SJA1105_init();

struct CANInterface {
  cannelloni_handle_t cannelloni;
  struct canfd_frame tx_buf[CNL_BUF_SIZE];
  struct canfd_frame rx_buf[CNL_BUF_SIZE];
  canBASE_t *canreg;
};

struct CANInterface can_interfaces[CAN_IFACES];

uint8_t node_id() {
  switch (systemREG2->DIEIDL_REG0) {
    case 0x1600600D:
      return 1;
    case 0x16013009:
      return 2;
    default:
      return 0;
  }
}

bool on_can_transmit(cannelloni_handle_t *cannelloni, struct canfd_frame *frame) {
  struct CANInterface *iface = cannelloni;
  return can_send(iface->canreg, frame->can_id, frame->len, frame->data);
}

void on_can_receive(cannelloni_handle_t *cannelloni) {
  struct CANInterface *iface = cannelloni;
  canBASE_t *canreg = iface->canreg;

  uint8_t mbox = CAN_RX_QUEUE_FIRST_MBOX;
  while (can_mbox_has_data(canreg, mbox)) {
    struct canfd_frame *frame = get_can_rx_frame(cannelloni);
    if (!frame) {
      return;
    }

    can_fill_rx_mbox(canreg, mbox, &frame->can_id, &frame->len, frame->data);
    mbox++;
  }
}

int main(void) {
  systemInit();
  vim_init();
  timer_init();

  // set transceiver STBY to 1
  hetREG1->DOUT = 1U;
  hetREG1->DIR = 1U;

  gio_init();

  // Setting the outputs for the CAN transceivers
  gio_mode_output(gioPORTA, 0, 1);  // CAN1_EN
  gio_mode_output(gioPORTA, 1, 1);  // CAN2_EN
  gio_mode_output(gioPORTA, 2, 1);  // CAN3_EN
  gio_mode_output(gioPORTA, 3, 1);  // CAN4_EN

  lwip_init();
  netif_add(&netif, &instNum, hdkif_init, ip_input);
  netif_create_ip6_linklocal_address(&netif, 0);
  netif_set_default(&netif);
  netif_set_up(&netif);

  for (int i = 0; i < CAN_IFACES; i++) {
    struct CANInterface *can_iface = &can_interfaces[i];
    cannelloni_handle_t *cannelloni = &can_iface->cannelloni;
    IP_ADDR6(&cannelloni->Init.addr, 0xfe800000, 0x00000000, 0x5eb003c7, 0x958bfc58);

    cannelloni->Init.can_buf_size = CNL_BUF_SIZE;
    cannelloni->Init.can_rx_buf = can_interfaces->rx_buf;
    cannelloni->Init.can_rx_fn = on_can_receive;
    cannelloni->Init.can_tx_buf = can_interfaces->tx_buf;
    cannelloni->Init.can_tx_fn = on_can_transmit;
    cannelloni->Init.port = 20000 + node_id() * 10 + i;
    cannelloni->Init.remote_port = cannelloni->Init.port;

    canBASE_t *regs[] = {canREG1, canREG2, canREG3, canREG4};
    can_iface->canreg = regs[i];
    can_init(regs[i]);
    init_cannelloni(cannelloni);
  }

  if (node_id() == 2) {
    DP8386_init();
    SJA1105_init();
  }

  _enable_IRQ();
  _enable_FIQ();

  for (;;) {
    sys_check_timeouts();
    for (int i = 0; i < CAN_IFACES; i++) {
      run_cannelloni(&can_interfaces[i].cannelloni);
    }
  }
}

void IntMasterIRQEnable(void) { _enable_IRQ(); }

void IntMasterIRQDisable(void) { _disable_IRQ(); }

unsigned int IntMasterStatusGet(void) { return (0xC0 & _get_CPSR()); }
