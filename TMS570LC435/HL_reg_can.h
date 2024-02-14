/*
* Copyright (C) 2009-2018 Texas Instruments Incorporated - www.ti.com
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


#ifndef __REG_CAN_H__
#define __REG_CAN_H__

#include <stdint.h>


/* Can Register Frame Definition */
/** @struct canBase
*   @brief CAN Register Frame Definition
*
*   This type is used to access the CAN Registers.
*/
/** @typedef canBASE_t
*   @brief CAN Register Frame Type Definition
*
*   This type is used to access the CAN Registers.
*/
typedef volatile struct canBase
{
    uint32_t      CTL;          /**< 0x0000: Control Register                       */
    uint32_t      ES;           /**< 0x0004: Error and Status Register              */
    uint32_t      EERC;         /**< 0x0008: Error Counter Register                 */
    uint32_t      BTR;          /**< 0x000C: Bit Timing Register                    */
    uint32_t      INT;          /**< 0x0010: Interrupt Register                     */
    uint32_t      TEST;         /**< 0x0014: Test Register                          */
    uint32_t      rsvd1;        /**< 0x0018: Reserved                               */
    uint32_t      PERR;         /**< 0x001C: Parity/SECDED Error Code Register      */
    uint32_t		rsvd11;		  /**< 0x0020: Reserved								  */
	uint32_t		ECCDIAG;	  /**< 0x0024: ECC Diagnostic Register			      */
	uint32_t		ECCDIAG_STAT; /**< 0x0028: ECC Diagnostic Status Register	      */
	uint32_t		ECC_CS;		  /**< 0x002C: ECC Control and Status Register        */
	uint32_t		ECC_SERR;	  /**< 0x0030: ECC Single Bit Error code register      */
	uint32_t      rsvd2[19];    /**< 0x002C - 0x7C: Reserved                        */
    uint32_t      ABOTR;        /**< 0x0080: Auto Bus On Time Register              */
    uint32_t      TXRQX;        /**< 0x0084: Transmission Request X Register        */
    uint32_t      TXRQx[4U];    /**< 0x0088-0x0094: Transmission Request Registers  */
    uint32_t      NWDATX;       /**< 0x0098: New Data X Register                    */
    uint32_t      NWDATx[4U];   /**< 0x009C-0x00A8: New Data Registers              */
    uint32_t      INTPNDX;      /**< 0x00AC: Interrupt Pending X Register           */
    uint32_t      INTPNDx[4U];  /**< 0x00B0-0x00BC: Interrupt Pending Registers     */
    uint32_t      MSGVALX;      /**< 0x00C0: Message Valid X Register               */
    uint32_t      MSGVALx[4U];  /**< 0x00C4-0x00D0: Message Valid Registers         */
    uint32_t        rsvd3;      /**< 0x00D4: Reserved                               */
    uint32_t      INTMUXx[4U];  /**< 0x00D8-0x00E4: Interrupt Multiplexer Registers */
    uint32_t        rsvd4[6];   /**< 0x00E8: Reserved                               */
#if ((__little_endian__ == 1) || (__LITTLE_ENDIAN__ == 1))
    uint8_t IF1NO;              /**< 0x0100: IF1 Command Register, Msg Number       */
    uint8_t IF1STAT;            /**< 0x0100: IF1 Command Register, Status           */
    uint8_t IF1CMD;             /**< 0x0100: IF1 Command Register, Command          */
    uint8_t   rsvd9;            /**< 0x0100: IF1 Command Register, Reserved         */
#else
    uint8_t   rsvd9;            /**< 0x0100: IF1 Command Register, Reserved         */
    uint8_t IF1CMD;             /**< 0x0100: IF1 Command Register, Command          */
    uint8_t IF1STAT;            /**< 0x0100: IF1 Command Register, Status           */
    uint8_t IF1NO;              /**< 0x0100: IF1 Command Register, Msg Number       */
#endif
    uint32_t      IF1MSK;       /**< 0x0104: IF1 Mask Register                      */
    uint32_t      IF1ARB;       /**< 0x0108: IF1 Arbitration Register               */
    uint32_t      IF1MCTL;      /**< 0x010C: IF1 Message Control Register           */
    uint8_t IF1DATx[8U];        /**< 0x0110-0x0114: IF1 Data A and B Registers      */
    uint32_t        rsvd5[2];   /**< 0x0118: Reserved                               */
#if ((__little_endian__ == 1) || (__LITTLE_ENDIAN__ == 1))
    uint8_t IF2NO;              /**< 0x0120: IF2 Command Register, Msg No           */
    uint8_t IF2STAT;            /**< 0x0120: IF2 Command Register, Status           */
    uint8_t IF2CMD;             /**< 0x0120: IF2 Command Register, Command          */
    uint8_t   rsvd10;           /**< 0x0120: IF2 Command Register, Reserved         */
#else
    uint8_t   rsvd10;           /**< 0x0120: IF2 Command Register, Reserved         */
    uint8_t IF2CMD;             /**< 0x0120: IF2 Command Register, Command          */
    uint8_t IF2STAT;            /**< 0x0120: IF2 Command Register, Status           */
    uint8_t IF2NO;              /**< 0x0120: IF2 Command Register, Msg Number       */
#endif
    uint32_t      IF2MSK;       /**< 0x0124: IF2 Mask Register                      */
    uint32_t      IF2ARB;       /**< 0x0128: IF2 Arbitration Register               */
    uint32_t      IF2MCTL;      /**< 0x012C: IF2 Message Control Register           */
    uint8_t IF2DATx[8U];        /**< 0x0130-0x0134: IF2 Data A and B Registers      */
    uint32_t        rsvd6[2];   /**< 0x0138: Reserved                               */
    uint32_t      IF3OBS;       /**< 0x0140: IF3 Observation Register               */
    uint32_t      IF3MSK;       /**< 0x0144: IF3 Mask Register                      */
    uint32_t      IF3ARB;       /**< 0x0148: IF3 Arbitration Register               */
    uint32_t      IF3MCTL;      /**< 0x014C: IF3 Message Control Register           */
    uint8_t IF3DATx[8U];        /**< 0x0150-0x0154: IF3 Data A and B Registers      */
    uint32_t        rsvd7[2];   /**< 0x0158: Reserved                               */
    uint32_t      IF3UEy[4U];   /**< 0x0160-0x016C: IF3 Update Enable Registers     */
    uint32_t        rsvd8[28];  /**< 0x0170: Reserved                               */
    uint32_t      TIOC;         /**< 0x01E0: TX IO Control Register                 */
    uint32_t      RIOC;         /**< 0x01E4: RX IO Control Register                 */
} canBASE_t;


/** @def canREG1
*   @brief CAN1 Register Frame Pointer
*
*   This pointer is used by the CAN driver to access the CAN1 registers.
*/
#define canREG1 ((canBASE_t *)0xFFF7DC00U)

/** @def canREG2
*   @brief CAN2 Register Frame Pointer
*
*   This pointer is used by the CAN driver to access the CAN2 registers.
*/
#define canREG2 ((canBASE_t *)0xFFF7DE00U)

/** @def canREG3
*   @brief CAN3 Register Frame Pointer
*
*   This pointer is used by the CAN driver to access the CAN3 registers.
*/
#define canREG3 ((canBASE_t *)0xFFF7E000U)

/** @def canREG4
*   @brief CAN4 Register Frame Pointer
*
*   This pointer is used by the CAN driver to access the CAN4 registers.
*/
#define canREG4 ((canBASE_t *)0xFFF7E200U)

/** @def canRAM1
*   @brief CAN1 Mailbox RAM Pointer
*
*   This pointer is used by the CAN driver to access the CAN1 RAM.
*/
#define canRAM1 (*(volatile uint32_t *)0xFF1E0000U)

/** @def canRAM2
*   @brief CAN2 Mailbox RAM Pointer
*
*   This pointer is used by the CAN driver to access the CAN2 RAM.
*/
#define canRAM2 (*(volatile uint32_t *)0xFF1C0000U)

/** @def canRAM3
*   @brief CAN3 Mailbox RAM Pointer
*
*   This pointer is used by the CAN driver to access the CAN3 RAM.
*/
#define canRAM3 (*(volatile uint32_t *)0xFF1A0000U)

/** @def canRAM4
*   @brief CAN4 Mailbox RAM Pointer
*
*   This pointer is used by the CAN driver to access the CAN4 RAM.
*/
#define canRAM4 (*(volatile uint32_t *)0xFF180000U)


/** @def canPARRAM1
*   @brief CAN1 Mailbox Parity RAM Pointer
*
*   This pointer is used by the CAN driver to access the CAN1 Parity RAM
*   for testing RAM parity error detect logic.
*/
#define canPARRAM1 (*(volatile uint32_t *)(0xFF1E0000U + 0x10U))

/** @def canPARRAM2
*   @brief CAN2 Mailbox Parity RAM Pointer
*
*   This pointer is used by the CAN driver to access the CAN2 Parity RAM
*   for testing RAM parity error detect logic.
*/
#define canPARRAM2 (*(volatile uint32_t *)(0xFF1C0000U + 0x10U))

/** @def canPARRAM3
*   @brief CAN3 Mailbox Parity RAM Pointer
*
*   This pointer is used by the CAN driver to access the CAN3 Parity RAM
*   for testing RAM parity error detect logic.
*/
#define canPARRAM3 (*(volatile uint32_t *)(0xFF1A0000U + 0x10U))

/** @def canPARRAM4
*   @brief CAN4 Mailbox Parity RAM Pointer
*
*   This pointer is used by the CAN driver to access the CAN4 Parity RAM
*   for testing RAM parity error detect logic.
*/
#define canPARRAM4 (*(volatile uint32_t *)(0xFF180000U + 0x10U))


#endif
