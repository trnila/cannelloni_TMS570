#include <string.h>
#include "lwip/init.h"
#include "lwip/ip.h"
#include "lwip/timeouts.h"
#include "lwip/apps/mdns.h"
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

  mdns_resp_init();
  char name[16];
  snprintf(name, sizeof(name), "cangw%d", node_id());
  mdns_resp_add_netif(&netif, name);

  for (int i = 0; i < CAN_IFACES; i++) {
    struct CANInterface *can_iface = &can_interfaces[i];
    cannelloni_handle_t *cannelloni = &can_iface->cannelloni;
    ip6_addr_copy(cannelloni->Init.addr, netif.ip6_addr[0]);

    cannelloni->Init.addr.addr[0] = lwip_htonl(
        ((0xFF00U | IP6_MULTICAST_SCOPE_LINK_LOCAL) << 16) | (IP6_ADDR_BLOCK2(&cannelloni->Init.addr)));

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

    char srv_name[16];
    snprintf(srv_name, sizeof(srv_name), "can-%d-%d", node_id(), i);
    mdns_resp_add_service(&netif, srv_name, "_cannelloni", DNSSD_PROTO_UDP, cannelloni->Init.port, NULL, NULL);
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
