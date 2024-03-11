/**
 * Copyright (c) 2001-2004 Swedish Institute of Computer Science.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
 * EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * This file is part of the lwIP TCP/IP stack.
 *
 * Author: Adam Dunkels <adam@sics.se>
 *
 */

/**
 * Copyright (c) 2010 Texas Instruments Incorporated
 *
 * This file is dervied from the "ethernetif.c" skeleton Ethernet network
 * interface driver for lwIP.
 *
 */
#include <stdint.h>
#include "lwip/opt.h"
#include "lwip/def.h"
#include "lwip/mem.h"
#include "lwip/pbuf.h"
#include "lwip/sys.h"
#include "lwip/stats.h"
#include "lwip/snmp.h"
#include "lwip/ethip6.h"
#include "lwip/err.h"
#include "netif/hdkif.h"
#include "arch/cc.h"
#include "HL_hw_reg_access.h"
#include "HL_hw_emac.h"
#include "HL_hw_emac_ctrl.h"
#include "HL_reg_system.h"
#include "drivers/vim.h"

#define ETHARP_HWADDR_LEN 6
#define MAX_TRANSFER_UNIT 1514U
#define MAX_RX_PBUF_ALLOC 10U
#define MIN_PKT_LEN 60U
#define PBUF_LEN_MAX MAX_TRANSFER_UNIT
/* EMAC Control RAM size in bytes */
#define SIZE_EMAC_CTRL_RAM 0x2000

#define EMAC_CTRL_RAM_0_BASE 0xFC520000U
#define EMAC_0_BASE 0xFCF78000U
#define EMAC_CTRL_0_BASE 0xFCF78800U

#define EMAC_BUF_DESC_OWNER 0x20000000U
#define EMAC_BUF_DESC_SOP 0x80000000U
#define EMAC_BUF_DESC_EOP 0x40000000U
#define EMAC_BUF_DESC_EOQ 0x10000000U

#define EMAC_DUPLEX_FULL (0x00000001U)
#define EMAC_DUPLEX_HALF (0x00000000U)

#define EMAC_RMIISPEED_10MBPS (0x00000000U)
#define EMAC_RMIISPEED_100MBPS (0x00008000U)

#define EMAC_INT_CORE0_RX (0x1U)
#define EMAC_INT_CORE0_TX (0x2U)

#define EMAC_CONTROL_RESET (0x01U)
#define EMAC_SOFT_RESET (0x01U)
#define EMAC_MAX_HEADER_DESC (8U)

/* EMAC TX Buffer descriptor data structure */
struct emac_tx_bdp {
  volatile struct emac_tx_bdp *next;
  volatile uint32_t bufptr;
  volatile uint32_t bufoff_len;
  volatile uint32_t flags_pktlen;

  /* helper to know which pbuf this tx bd corresponds to */
  volatile struct pbuf *pbuf;
} emac_tx_bdp;

/* EMAC RX Buffer descriptor data structure */
struct emac_rx_bdp {
  volatile struct emac_rx_bdp *next;
  volatile uint32_t bufptr;
  volatile uint32_t bufoff_len;
  volatile uint32_t flags_pktlen;

  /* helper to know which pbuf this rx bd corresponds to */
  volatile struct pbuf *pbuf;
} emac_rx_bdp;

/**
 * Helper struct to hold the data used to operate on a particular
 * receive channel
 */
struct rxch {
  volatile struct emac_rx_bdp *free_head;
  volatile struct emac_rx_bdp *active_head;
  volatile struct emac_rx_bdp *active_tail;
  uint32_t freed_pbuf_len;
} rxch;

/**
 * Helper struct to hold the data used to operate on a particular
 * transmit channel
 */
struct txch {
  volatile struct emac_tx_bdp *free_head;
  volatile struct emac_tx_bdp *active_tail;
  volatile struct emac_tx_bdp *next_bd_to_process;
} txch;

/**
 * Helper struct to hold private data used to operate the ethernet interface.
 */
struct hdkif {
  /* emac instance number */
  uint32_t inst_num;

  /* emac base address */
  uint32_t emac_base;

  /* emac controller base address */
  volatile uint32_t emac_ctrl_base;
  volatile uint32_t emac_ctrl_ram;

  /* The tx/rx channels for the interface */
  struct txch txch;
  struct rxch rxch;
} hdkif;

/* Defining interface for all the emac instances */
static struct hdkif hdkif_data[1];
struct netif netif;

void EMACCore0TxIsr(void);
void EMACCore0RxIsr(void);

/**
 * \brief   Enables the TXPULSE Interrupt Generation.
 * \param   ctrlCore      Channel number for which the interrupt to be enabled in EMAC Control module
 * \param   channel       Channel number for which interrupt to be enabled
 **/
void EMACTxIntPulseEnable(uint32_t emacBase, uint32_t emacCtrlBase, uint32_t ctrlCore,
                          uint32_t channel) {
  HWREG(emacBase + EMAC_TXINTMASKSET) |= ((uint32_t)1U << channel);

  HWREG(emacCtrlBase + EMAC_CTRL_CnTXEN(ctrlCore)) |= ((uint32_t)1U << channel);
}

/**
 * \brief   Enables the RXPULSE Interrupt Generation.
 * \param   ctrlCore      Control core for which the interrupt to be enabled.
 * \param   channel       Channel number for which interrupt to be enabled
 **/
void EMACRxIntPulseEnable(uint32_t emacBase, uint32_t emacCtrlBase, uint32_t ctrlCore,
                          uint32_t channel)

{
  HWREG(emacBase + EMAC_RXINTMASKSET) |= ((uint32_t)1U << channel);

  HWREG(emacCtrlBase + EMAC_CTRL_CnRXEN(ctrlCore)) |= ((uint32_t)1U << channel);
}

/**
 * \brief   This API sets the RMII speed. The RMII Speed can be 10 Mbps or
 *          100 Mbps
 * \param   speed        speed for setting.
 *          speed can take the following values. \n
 *                EMAC_RMIISPEED_10MBPS - 10 Mbps \n
 *                EMAC_RMIISPEED_100MBPS - 100 Mbps.
 **/
void EMACRMIISpeedSet(uint32_t emacBase, uint32_t speed) {
  HWREG(emacBase + EMAC_MACCONTROL) &= (~(uint32_t)EMAC_MACCONTROL_RMIISPEED);
  HWREG(emacBase + EMAC_MACCONTROL) |= speed;
}

/**
 * \brief   This API set the GMII bit, RX and TX are enabled for receive and
 *          transmit. Note: This is not the API to enable MII.
 **/
void EMACMIIEnable(uint32_t emacBase) {
  HWREG(emacBase + EMAC_MACCONTROL) |= EMAC_MACCONTROL_GMIIEN;
}

/**
 * \brief   This API sets the duplex mode of operation(full/half) for MAC.
 *

 * \param   duplexMode   duplex mode of operation.
 *          duplexMode can take the following values. \n
 *                EMAC_DUPLEX_FULL - Full Duplex  \n
 *                EMAC_DUPLEX_HALF - Half Duplex.
 **/
void EMACDuplexSet(uint32_t emacBase, uint32_t duplexMode) {
  HWREG(emacBase + EMAC_MACCONTROL) &= (~(uint32_t)EMAC_MACCONTROL_FULLDUPLEX);
  HWREG(emacBase + EMAC_MACCONTROL) |= duplexMode;
}

/**
 * \brief   API to enable the transmit in the TX Control Register
 *          After the transmit is enabled, any write to TXHDP of
 *          a channel will start transmission
 **/
void EMACTxEnable(uint32_t emacBase) {
  HWREG(emacBase + EMAC_TXCONTROL) = EMAC_TXCONTROL_TXEN;
}

/**
 * \brief   API to enable the receive in the RX Control Register
 *          After the receive is enabled, and write to RXHDP of
 *          a channel, the data can be received in the destination
 *          specified by the corresponding RX buffer descriptor.
 **/
void EMACRxEnable(uint32_t emacBase) {
  HWREG(emacBase + EMAC_RXCONTROL) = EMAC_RXCONTROL_RXEN;
}

/**
 * \brief   API to write the TX HDP register. If transmit is enabled,
 *          write to the TX HDP will immediately start transmission.
 *          The data will be taken from the buffer pointer of the TX buffer
 *          descriptor written to the TX HDP
 * \param   descHdr       Address of the TX buffer descriptor
 * \param   channel       Channel Number
 **/
void EMACTxHdrDescPtrWrite(uint32_t emacBase, uint32_t descHdr, uint32_t channel) {
  HWREG(emacBase + EMAC_TXHDP(channel)) = descHdr;
}

/**
 * \brief   API to write the RX HDP register. If receive is enabled,
 *          write to the RX HDP will enable data reception to point to
 *          the corresponding RX buffer descriptor's buffer pointer.
 * \param   descHdr       Address of the RX buffer descriptor
 * \param   channel       Channel Number
 **/
void EMACRxHdrDescPtrWrite(uint32_t emacBase, uint32_t descHdr, uint32_t channel) {
  HWREG(emacBase + EMAC_RXHDP(channel)) = descHdr;
}

/**
 * \brief   This API Initializes the EMAC and EMAC Control modules. The
 *          EMAC Control module is reset, the CPPI RAM is cleared. also,
 *          all the interrupts are disabled. This API does not enable any
 *          interrupt or operation of the EMAC.
 * \param   emacBase          Base address of the EMAC module registers
 **/
void EMACInit(uint32_t emacCtrlBase, uint32_t emacBase) {
  uint32_t cnt;

  /* Reset the EMAC Control Module. This clears the CPPI RAM also */
  HWREG(emacCtrlBase + EMAC_CTRL_SOFTRESET) = EMAC_CONTROL_RESET;

  while ((HWREG(emacCtrlBase + EMAC_CTRL_SOFTRESET) & EMAC_CONTROL_RESET) == EMAC_CONTROL_RESET) {
  }

  /* Reset the EMAC Module. This clears the CPPI RAM also */
  HWREG(emacBase + EMAC_SOFTRESET) = EMAC_SOFT_RESET;

  while ((HWREG(emacBase + EMAC_SOFTRESET) & EMAC_SOFT_RESET) == EMAC_SOFT_RESET) {
  }

  HWREG(emacBase + EMAC_MACCONTROL) = 0U;
  HWREG(emacBase + EMAC_RXCONTROL) = 0U;
  HWREG(emacBase + EMAC_TXCONTROL) = 0U;

  /* Initialize all the header descriptor pointer registers */
  for (cnt = 0U; cnt < EMAC_MAX_HEADER_DESC; cnt++) {
    HWREG(emacBase + EMAC_RXHDP(cnt)) = 0U;
    HWREG(emacBase + EMAC_TXHDP(cnt)) = 0U;
    HWREG(emacBase + EMAC_RXCP(cnt)) = 0U;
    HWREG(emacBase + EMAC_TXCP(cnt)) = 0U;
    HWREG(emacBase + EMAC_RXFREEBUFFER(cnt)) = 0xFFU;
  }
  /* Clear the interrupt enable for all the channels */
  HWREG(emacBase + EMAC_TXINTMASKCLEAR) = 0xFFU;
  HWREG(emacBase + EMAC_RXINTMASKCLEAR) = 0xFFU;

  HWREG(emacBase + EMAC_MACHASH1) = 0U;
  HWREG(emacBase + EMAC_MACHASH2) = 0U;

  HWREG(emacBase + EMAC_RXBUFFEROFFSET) = 0U;
}

/**
 * \brief   Sets the MAC Address in MACSRCADDR registers.
 * \param   macAddr       Start address of a MAC address array.
 *                        The array[0] shall be the MSB of the MAC address
 **/
void EMACMACSrcAddrSet(uint32_t emacBase, uint8_t macAddr[6]) {
  HWREG(emacBase + EMAC_MACSRCADDRHI) =
      ((uint32_t)macAddr[0U] | ((uint32_t)macAddr[1U] << 8U) |
       ((uint32_t)macAddr[2U] << 16U) | ((uint32_t)macAddr[3U] << 24U));
  HWREG(emacBase + EMAC_MACSRCADDRLO) =
      ((uint32_t)macAddr[4U] | ((uint32_t)macAddr[5U] << 8U));
}

/**
 * \brief   Acknowledges an interrupt processed to the EMAC Control Core.
 * \param   eoiFlag       Type of interrupt to acknowledge to the EMAC Control
 *                         module.
 *          eoiFlag can take the following values \n
 *             EMAC_INT_CORE0_TX - Core 0 TX Interrupt
 *             EMAC_INT_CORE1_TX - Core 1 TX Interrupt
 *             EMAC_INT_CORE2_TX - Core 2 TX Interrupt
 *             EMAC_INT_CORE0_RX - Core 0 RX Interrupt
 *             EMAC_INT_CORE1_RX - Core 1 RX Interrupt
 *             EMAC_INT_CORE2_RX - Core 2 RX Interrupt
 **/
void EMACCoreIntAck(uint32_t emacBase, uint32_t eoiFlag) {
  /* Acknowledge the EMAC Control Core */
  HWREG(emacBase + EMAC_MACEOIVECTOR) = eoiFlag;
}

/**
 * \brief   Writes the the TX Completion Pointer for a specific channel
 * \param   channel       Channel Number.
 * \param   comPtr        Completion Pointer Value to be written
 **/
void EMACTxCPWrite(uint32_t emacBase, uint32_t channel, uint32_t comPtr) {
  HWREG(emacBase + EMAC_TXCP(channel)) = comPtr;
}

/**
 * \brief   Writes the the RX Completion Pointer for a specific channel
 * \param   channel       Channel Number.
 * \param   comPtr        Completion Pointer Value to be written
 **/
void EMACRxCPWrite(uint32_t emacBase, uint32_t channel, uint32_t comPtr) {
  HWREG(emacBase + EMAC_RXCP(channel)) = comPtr;
}

/**
 * \brief   Set the free buffers for a specific channel
 * \param   channel       Channel Number.
 * \param   nBuf          Number of free buffers
 **/

void EMACNumFreeBufSet(uint32_t emacBase, uint32_t channel, uint32_t nBuf) {
  HWREG(emacBase + EMAC_RXFREEBUFFER(channel)) = nBuf;
}

uint32_t hdkif_swizzle_data(uint32_t word) {
#if defined(_TMS570LC43x_)
  return (((word << 24) & 0xFF000000) | ((word << 8) & 0x00FF0000) |
          ((word >> 8) & 0x0000FF00) | ((word >> 24) & 0x000000FF));
#else
  return (((word << 24) & 0xFF000000) | ((word << 8) & 0x00FF0000) |
          ((word >> 8) & 0x0000FF00) | ((word >> 24) & 0x000000FF));
#endif
}

struct emac_tx_bdp *hdkif_swizzle_txp(volatile struct emac_tx_bdp *p) {
  return (struct emac_tx_bdp *)hdkif_swizzle_data((uint32_t)p);
}

struct emac_rx_bdp *hdkif_swizzle_rxp(volatile struct emac_rx_bdp *p) {
  return (struct emac_rx_bdp *)hdkif_swizzle_data((uint32_t)p);
}

/**
 * Function to setup the instance parameters inside the interface
 * @param   hdkif
 * @return  none.
 */
static void hdkif_inst_config(struct hdkif *hdkif) {
  if (hdkif->inst_num == 0) {
    hdkif->emac_base = EMAC_0_BASE;
    hdkif->emac_ctrl_base = EMAC_CTRL_0_BASE;
    hdkif->emac_ctrl_ram = EMAC_CTRL_RAM_0_BASE;
  }
}

/**
 * This function should do the actual transmission of the packet. The packet is
 * contained in the pbuf that is passed to the function. This pbuf might be
 * chained. That is, one pbuf can span more than one tx buffer descriptors
 *
 * @param hdkif the network interface state for this ethernetif
 * @param pbuf  the pbuf which is to be sent over EMAC
 * @return None
 */
static void hdkif_transmit(struct hdkif *hdkif, struct pbuf *pbuf) {
  struct pbuf *q;
  struct txch *txch;
  volatile struct emac_tx_bdp *curr_bd, *active_head, *bd_end;

  txch = &(hdkif->txch);

  /* Get the buffer descriptor which is free to transmit */
  curr_bd = txch->free_head;

  active_head = curr_bd;

  /* Update the total packet length */
  uint32_t flags_pktlen = pbuf->tot_len;
  flags_pktlen |= (EMAC_BUF_DESC_SOP | EMAC_BUF_DESC_OWNER);
  curr_bd->flags_pktlen = hdkif_swizzle_data(flags_pktlen);

  /* Copy pbuf information into TX buffer descriptors */
  for (q = pbuf; q != NULL; q = q->next) {
    /* Intialize the buffer pointer and length */
    curr_bd->bufptr = hdkif_swizzle_data((uint32_t)(q->payload));
    curr_bd->bufoff_len = hdkif_swizzle_data((q->len) & 0xFFFF);
    bd_end = curr_bd;
    curr_bd->pbuf = pbuf;
    curr_bd = hdkif_swizzle_txp(curr_bd->next);
    // if (curr_bd) curr_bd->flags_pktlen = 0;
  }

  /* Indicate the end of the packet */
  bd_end->next = NULL;
  bd_end->flags_pktlen |= hdkif_swizzle_data(EMAC_BUF_DESC_EOP);

  txch->free_head = curr_bd;

  /* For the first time, write the HDP with the filled bd */
  if (txch->active_tail == NULL) {
    EMACTxHdrDescPtrWrite(hdkif->emac_base, (unsigned int)(active_head), 0);
  }

  /*
   * Chain the bd's. If the DMA engine, already reached the end of the chain,
   * the EOQ will be set. In that case, the HDP shall be written again.
   */
  else {
    curr_bd = txch->active_tail;
    /* TODO: (This is a workaround) Wait for the EOQ bit is set */
    while (EMAC_BUF_DESC_EOQ !=
           (hdkif_swizzle_data(curr_bd->flags_pktlen) & EMAC_BUF_DESC_EOQ))
      ;
    /* TODO: (This is a workaround) Don't write to TXHDP0 until it turns to zero
     */
    while (0 != *((uint32_t *)0xFCF78600))
      ;
    curr_bd->next = hdkif_swizzle_txp(active_head);
    if (EMAC_BUF_DESC_EOQ ==
        (hdkif_swizzle_data(curr_bd->flags_pktlen) & EMAC_BUF_DESC_EOQ)) {
      /* Write the Header Descriptor Pointer and start DMA */
      EMACTxHdrDescPtrWrite(hdkif->emac_base, (unsigned int)(active_head), 0);
    }
  }

  txch->active_tail = bd_end;
}

/**
 * This function will send a packet through the emac if the channel is
 * available. Otherwise, the packet will be queued in a pbuf queue.
 *
 * @param netif the lwip network interface structure for this ethernetif
 * @param p the MAC packet to send (e.g. IP packet including MAC addresses and
 * type)
 * @return ERR_OK if the packet could be sent
 *         an err_t value if the packet couldn't be sent
 *
 */
static err_t hdkif_output(struct netif *netif, struct pbuf *p) {
  SYS_ARCH_DECL_PROTECT(lev);

  /**
   * This entire function must run within a "critical section" to preserve
   * the integrity of the transmit pbuf queue.
   *
   */
  SYS_ARCH_PROTECT(lev);

  /* adjust the packet length if less than minimum required */
  if (p->tot_len < MIN_PKT_LEN) {
    p->tot_len = MIN_PKT_LEN;
    p->len = MIN_PKT_LEN;
  }

  /**
   * Bump the reference count on the pbuf to prevent it from being
   * freed till we are done with it.
   *
   */
  pbuf_ref(p);

  /* call the actual transmit function */
  hdkif_transmit(netif->state, p);

  /* Return to prior interrupt state and return. */
  SYS_ARCH_UNPROTECT(lev);

  return ERR_OK;
}

/**
 * In this function, the hardware should be initialized.
 * Called from hdkif_init().
 *
 * @param netif the already initialized lwip network interface structure
 *        for this ethernetif
 */
static err_t hdkif_hw_init(struct netif *netif) {
  uint32_t channel;
  uint32_t num_bd, pbuf_cnt = 0;
  volatile uint32_t phyID = 0;
  volatile uint32_t delay = 0xfff;
  volatile uint32_t phyIdReadCount = 0xFFFF;
  volatile struct emac_tx_bdp *curr_txbd, *last_txbd;
  volatile struct emac_rx_bdp *curr_bd, *last_bd;
  struct hdkif *hdkif;
  struct txch *txch;
  struct rxch *rxch;
  struct pbuf *p, *q;

  hdkif = netif->state;

  /* set MAC hardware address length */
  netif->hwaddr_len = ETHARP_HWADDR_LEN;

  /* set MAC hardware address */
  netif->hwaddr[0] = 0x02;  // unicast + locally administered MAC
  netif->hwaddr[1] = 0x00;
  netif->hwaddr[2] = systemREG2->DIEIDL_REG0 >> 24;
  netif->hwaddr[3] = systemREG2->DIEIDL_REG0 >> 16;
  netif->hwaddr[4] = systemREG2->DIEIDL_REG0 >> 8;
  netif->hwaddr[5] = systemREG2->DIEIDL_REG0;

  /* maximum transfer unit */
  netif->mtu = MAX_TRANSFER_UNIT;

  /* device capabilities */
  netif->flags = NETIF_FLAG_BROADCAST | NETIF_FLAG_LINK_UP | NETIF_FLAG_MLD6;

  EMACInit(hdkif->emac_ctrl_base, hdkif->emac_base);
  EMACMACSrcAddrSet(hdkif->emac_base, netif->hwaddr);
  EMACDuplexSet(hdkif->emac_base, EMAC_DUPLEX_FULL);
  EMACRMIISpeedSet(hdkif->emac_base, EMAC_RMIISPEED_100MBPS);

  /** receive all frames without HW MAC address filtering */
  HWREG(hdkif->emac_base + EMAC_RXMBPENABLE) |= EMAC_RXMBPENABLE_RXCAFEN;

  txch = &(hdkif->txch);

  /**
   * Initialize the Descriptor Memory For TX and RX
   * Only Channel 0 is supported for both TX and RX
   */
  txch->free_head = (volatile struct emac_tx_bdp *)(hdkif->emac_ctrl_ram);
  txch->next_bd_to_process = txch->free_head;
  txch->active_tail = NULL;

  /* Set the number of descriptors for the channel */
  num_bd = (SIZE_EMAC_CTRL_RAM >> 1) / sizeof(emac_tx_bdp);

  curr_txbd = txch->free_head;

  /* Initialize all the TX buffer Descriptors */
  while (num_bd--) {
    curr_txbd->next = hdkif_swizzle_txp(curr_txbd + 1);
    curr_txbd->flags_pktlen = 0;
    last_txbd = curr_txbd;
    curr_txbd = hdkif_swizzle_txp(curr_txbd->next);
  }
  last_txbd->next = hdkif_swizzle_txp(txch->free_head);

  /* Initialize the descriptors for the RX channel */
  rxch = &(hdkif->rxch);
  rxch->active_head = (volatile struct emac_rx_bdp *)(curr_txbd + 1);

  rxch->free_head = NULL;
  rxch->freed_pbuf_len = 0;
  num_bd = ((SIZE_EMAC_CTRL_RAM >> 1) / sizeof(emac_rx_bdp) - 1);
  curr_bd = rxch->active_head;
  last_bd = curr_bd;

  /*
  ** Allocate the pbufs for the maximum count permitted or till the
  ** number of buffer desceriptors expire, which ever is earlier.
  */
  while (pbuf_cnt < MAX_RX_PBUF_ALLOC) {
    p = pbuf_alloc(PBUF_RAW, PBUF_LEN_MAX, PBUF_POOL);
    pbuf_cnt++;

    if (p != NULL) {
      /* write the descriptors if there are enough numbers to hold the pbuf*/
      if (((uint32_t)pbuf_clen(p)) <= num_bd) {
        for (q = p; q != NULL; q = q->next) {
          curr_bd->bufptr = hdkif_swizzle_data((uint32_t)(q->payload));
          curr_bd->bufoff_len = hdkif_swizzle_data(q->len);
          curr_bd->next = hdkif_swizzle_rxp(curr_bd + 1);
          curr_bd->flags_pktlen = hdkif_swizzle_data(EMAC_BUF_DESC_OWNER);

          /* Save the pbuf */
          curr_bd->pbuf = q;
          last_bd = curr_bd;
          curr_bd = hdkif_swizzle_rxp(curr_bd->next);
          num_bd--;
        }
      }

      /* free the allocated pbuf if no free descriptors are left */
      else {
        pbuf_free(p);
        break;
      }
    } else {
      break;
    }
  }

  last_bd->next = NULL;
  rxch->active_tail = last_bd;

  /* Acknowledge receive and transmit interrupts for proper interrupt pulsing*/
  EMACCoreIntAck(hdkif->emac_base, EMAC_INT_CORE0_RX);
  EMACCoreIntAck(hdkif->emac_base, EMAC_INT_CORE0_TX);

  EMACNumFreeBufSet(hdkif->emac_base, 0, 10);
  EMACTxEnable(hdkif->emac_base);
  EMACRxEnable(hdkif->emac_base);

  /* Write the RX HDP for channel 0 */
  EMACRxHdrDescPtrWrite(hdkif->emac_base, (uint32_t)rxch->active_head, 0);

  EMACMIIEnable(hdkif->emac_base);

  /**
   * Enable the Transmission and reception, enable the interrupts for
   * channel 0 and for control core 0
   */
  EMACTxIntPulseEnable(hdkif->emac_base, hdkif->emac_ctrl_base, 0, 0);
  EMACRxIntPulseEnable(hdkif->emac_base, hdkif->emac_ctrl_base, 0, 0);

  vim_register_irq(77, EMACCore0TxIsr);
  vim_register_irq(79, EMACCore0RxIsr);

  return ERR_OK;
}

/**
 * Should be called at the beginning of the program to set up the
 * network interface. It calls the function hdkif_hw_init() to do the
 * low level initializations.
 *
 * @param netif the lwip network interface structure for this ethernetif
 * @return ERR_OK if the loopif is initialized
 *         ERR_MEM if private data couldn't be allocated
 *         any other err_t on error
 */
err_t hdkif_init(struct netif *netif) {
  /* Get the instance number first */
  unsigned int inst_num = *(unsigned int *)(netif->state);
  struct hdkif *hdkif;

#if LWIP_NETIF_HOSTNAME
  /* Initialize interface hostname */
  netif->hostname = "lwip";
#endif /* LWIP_NETIF_HOSTNAME */

  hdkif = &hdkif_data[inst_num];

  netif->state = hdkif;

  /*
   * Initialize the snmp variables and counters inside the struct netif.
   * The last argument should be replaced with your link speed, in units
   * of bits per second.
   */
  NETIF_INIT_SNMP(netif, snmp_ifType_ethernet_csmacd, 10000000);

  hdkif->inst_num = inst_num;

  netif->name[0] = 'e';
  netif->name[1] = 'n';

  netif->num = (uint8_t)inst_num;
  netif->linkoutput = hdkif_output;
  netif->output_ip6 = ethip6_output;

  /* initialize the hardware */
  hdkif_inst_config(hdkif);

  return (hdkif_hw_init(netif));
}

/**
 * Handler for Receive interrupt. Packet processing is done in this
 * interrupt handler itself.
 *
 * @param netif the lwip network interface structure for this ethernetif
 * @return none
 */
void hdkif_rx_inthandler(struct netif *netif) {
  struct hdkif *hdkif;
  struct rxch *rxch;
  volatile struct emac_rx_bdp *curr_bd, *processed_bd, *curr_tail, *last_bd;
  volatile struct pbuf *pbuf, *q, *new_pbuf;
  uint32_t ex_len = 0, len_to_alloc = 0;
  uint16_t tot_len;

  hdkif = netif->state;
  rxch = &(hdkif->rxch);

  /* Get the bd which contains the earliest filled data */
  curr_bd = rxch->active_head;
  last_bd = rxch->active_tail;

  /**
   * Process the descriptors as long as data is available
   * when the DMA is receiving data, SOP flag will be set
   */
  while (hdkif_swizzle_data(curr_bd->flags_pktlen) & EMAC_BUF_DESC_SOP) {
    ex_len = 0;
    len_to_alloc = 0;

    /* Start processing once the packet is loaded */
    if ((hdkif_swizzle_data(curr_bd->flags_pktlen) & EMAC_BUF_DESC_OWNER) !=
        EMAC_BUF_DESC_OWNER) {
      if (rxch->free_head == NULL) {
        /* this bd chain will be freed after processing */
        rxch->free_head = curr_bd;
      }

      /* Get the total length of the packet. curr_bd points to the start
       * of the packet.
       */
      tot_len = hdkif_swizzle_data(curr_bd->flags_pktlen) & 0xFFFF;

      /* Get the start of the pbuf queue */
      q = curr_bd->pbuf;

      do {
        /* Get the pbuf pointer which is associated with the current bd */
        pbuf = curr_bd->pbuf;

        /* If the earlier pbuf ended, update the chain */
        if (pbuf->next == NULL) {
          pbuf->next = (struct pbuf *)hdkif_swizzle_rxp(curr_bd->next)->pbuf;
        }

        len_to_alloc += pbuf->len;
        /* Update the len and tot_len fields for the pbuf in the chain*/
        pbuf->len = hdkif_swizzle_data(curr_bd->bufoff_len) & 0xFFFF;
        pbuf->tot_len = tot_len - ex_len;
        processed_bd = curr_bd;
        ex_len += pbuf->len;
        curr_bd = hdkif_swizzle_rxp(curr_bd->next);
      } while ((hdkif_swizzle_data(processed_bd->flags_pktlen) &
                EMAC_BUF_DESC_EOP) != EMAC_BUF_DESC_EOP);

      /**
       * Close the chain for this pbuf. A full packet is received in
       * this pbuf chain. Now this pbuf can be given to upper layers for
       * processing. The start of the pbuf chain is now 'q'.
       */
      pbuf->next = NULL;

      /* Adjust the link statistics */
      LINK_STATS_INC(link.recv);

      /* Process the packet */
      if (ethernet_input((struct pbuf *)q, netif) != ERR_OK) {
        /* Adjust the link statistics */
        LINK_STATS_INC(link.memerr);
        LINK_STATS_INC(link.drop);
      }

      /* Acknowledge that this packet is processed */
      EMACRxCPWrite(hdkif->emac_base, 0, (unsigned int)processed_bd);

      rxch->active_head = curr_bd;

      /**
       * The earlier pbuf chain is freed from the upper layer. So, we need to
       * allocate a new pbuf chain and update the descriptors with the pbuf
       * info. To support chaining, the total length freed by the upper layer is
       * tracked. Care should be taken even if the allocation fails.
       */
      /**
       * now len_to_alloc will contain the length of the pbuf which was freed
       * from the upper layer
       */
      rxch->freed_pbuf_len += len_to_alloc;
      new_pbuf = pbuf_alloc(PBUF_RAW, (rxch->freed_pbuf_len), PBUF_POOL);

      /* Write the descriptors with the pbuf info till either of them expires */
      if (new_pbuf != NULL) {
        curr_bd = rxch->free_head;

        for (q = new_pbuf; (q != NULL) && (curr_bd != rxch->active_head);
             q = q->next) {
          curr_bd->bufptr = hdkif_swizzle_data((uint32_t)(q->payload));

          /* no support for buf_offset. RXBUFFEROFFEST register is 0 */
          curr_bd->bufoff_len = hdkif_swizzle_data((q->len) & 0xFFFF);
          curr_bd->flags_pktlen = hdkif_swizzle_data(EMAC_BUF_DESC_OWNER);

          rxch->freed_pbuf_len -= q->len;

          /* Save the pbuf */
          curr_bd->pbuf = q;
          last_bd = curr_bd;
          curr_bd = hdkif_swizzle_rxp(curr_bd->next);
        }

        /**
         * At this point either pbuf expired or no rxbd to allocate. If
         * there are no, enough rx bds to allocate all pbufs in the chain,
         * free the rest of the pbuf
         */
        if (q != NULL) {
          pbuf_free((struct pbuf *)q);
        }

        curr_tail = rxch->active_tail;
        last_bd->next = NULL;

        curr_tail->next = hdkif_swizzle_rxp(rxch->free_head);

        /**
         * Check if the reception has ended. If the EOQ flag is set, the NULL
         * Pointer is taken by the DMA engine. So we need to write the RX HDP
         * with the next descriptor.
         */
        if (hdkif_swizzle_data(curr_tail->flags_pktlen) & EMAC_BUF_DESC_EOQ) {
          EMACRxHdrDescPtrWrite(hdkif->emac_base, (uint32_t)(rxch->free_head), 0);
        }

        rxch->free_head = curr_bd;
        rxch->active_tail = last_bd;
      }
    }
    curr_bd = rxch->active_head;
  }

  EMACCoreIntAck(hdkif->emac_base, EMAC_INT_CORE0_RX);
  EMACCoreIntAck(hdkif->emac_base, EMAC_INT_CORE0_TX);
}

/**
 * Handler for EMAC Transmit interrupt
 *
 * @param netif the lwip network interface structure for this ethernetif
 * @return none
 */
void hdkif_tx_inthandler(struct netif *netif) {
  struct txch *txch;
  struct hdkif *hdkif;
  volatile struct emac_tx_bdp *curr_bd, *next_bd_to_process;

  hdkif = netif->state;
  txch = &(hdkif->txch);

  next_bd_to_process = txch->next_bd_to_process;

  curr_bd = next_bd_to_process;

  /* Check for correct start of packet */
  while (hdkif_swizzle_data(curr_bd->flags_pktlen) & EMAC_BUF_DESC_SOP) {
    /* Make sure that the transmission is over */
    while ((hdkif_swizzle_data(curr_bd->flags_pktlen) & EMAC_BUF_DESC_OWNER) ==
           EMAC_BUF_DESC_OWNER)
      ;

    /* Traverse till the end of packet is reached */
    while ((hdkif_swizzle_data(curr_bd->flags_pktlen) & EMAC_BUF_DESC_EOP) !=
           EMAC_BUF_DESC_EOP) {
      curr_bd = hdkif_swizzle_txp(curr_bd->next);
    }

    next_bd_to_process->flags_pktlen &=
        hdkif_swizzle_data(~(EMAC_BUF_DESC_SOP));
    curr_bd->flags_pktlen &= hdkif_swizzle_data(~(EMAC_BUF_DESC_EOP));

    /**
     * If there are no more data transmitted, the next interrupt
     * shall happen with the pbuf associated with the free_head
     */
    if (curr_bd->next == NULL) {
      txch->next_bd_to_process = txch->free_head;
    }

    else {
      txch->next_bd_to_process = hdkif_swizzle_txp(curr_bd->next);
    }

    /* Acknowledge the EMAC and free the corresponding pbuf */
    EMACTxCPWrite(hdkif->emac_base, 0, (uint32_t)curr_bd);

    pbuf_free((struct pbuf *)curr_bd->pbuf);

    LINK_STATS_INC(link.xmit);

    next_bd_to_process = txch->next_bd_to_process;
    curr_bd = next_bd_to_process;
  }

  EMACCoreIntAck(hdkif->emac_base, EMAC_INT_CORE0_RX);
  EMACCoreIntAck(hdkif->emac_base, EMAC_INT_CORE0_TX);
}

#pragma INTERRUPT(EMACCore0RxIsr, IRQ)
void EMACCore0RxIsr(void) { hdkif_rx_inthandler(&netif); }

#pragma INTERRUPT(EMACCore0TxIsr, IRQ)
void EMACCore0TxIsr(void) { hdkif_tx_inthandler(&netif); }
