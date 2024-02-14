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


#ifndef __REG_RTI_H__
#define __REG_RTI_H__

#include <stdint.h>

/* Rti Register Frame Definition */
/** @struct rtiBase
*   @brief RTI Register Frame Definition
*
*   This type is used to access the RTI Registers.
*/
/** @typedef rtiBASE_t
*   @brief RTI Register Frame Type Definition
*
*   This type is used to access the RTI Registers.
*/
typedef volatile struct rtiBase
{
    uint32_t GCTRL;          /**< 0x0000: Global Control Register   */
    uint32_t TBCTRL;         /**< 0x0004: Timebase Control Register */
    uint32_t CAPCTRL;        /**< 0x0008: Capture Control Register  */
    uint32_t COMPCTRL;       /**< 0x000C: Compare Control Register  */
    struct
    {
        uint32_t FRCx;       /**< 0x0010,0x0030: Free Running Counter x Register         */
        uint32_t UCx;        /**< 0x0014,0x0034: Up Counter x Register                   */
        uint32_t CPUCx;      /**< 0x0018,0x0038: Compare Up Counter x Register           */
        uint32_t   rsvd1;    /**< 0x001C,0x003C: Reserved                                */
        uint32_t CAFRCx;     /**< 0x0020,0x0040: Capture Free Running Counter x Register */
        uint32_t CAUCx;      /**< 0x0024,0x0044: Capture Up Counter x Register           */
        uint32_t   rsvd2[2U]; /**< 0x0028,0x0048: Reserved                                */
    } CNT[2U];               /**< Counter x selection:
                                    - 0: Counter 0
                                    - 1: Counter 1                                       */
    struct
    {
        uint32_t COMPx;      /**< 0x0050,0x0058,0x0060,0x0068: Compare x Register        */
        uint32_t UDCPx;      /**< 0x0054,0x005C,0x0064,0x006C: Update Compare x Register */
    } CMP[4U];               /**< Compare x selection:
                                    - 0: Compare 0
                                    - 1: Compare 1
                                    - 2: Compare 2
                                    - 3: Compare 3                                       */
    uint32_t TBLCOMP;        /**< 0x0070: External Clock Timebase Low Compare Register   */
    uint32_t TBHCOMP;        /**< 0x0074: External Clock Timebase High Compare Register  */
    uint32_t   rsvd3[2U];    /**< 0x0078: Reserved                                       */
    uint32_t SETINTENA;      /**< 0x0080: Set/Status Interrupt Register                  */
    uint32_t CLEARINTENA;    /**< 0x0084: Clear/Status Interrupt Register                */
    uint32_t INTFLAG;        /**< 0x0088: Interrupt Flag Register                        */
    uint32_t   rsvd4;        /**< 0x008C: Reserved                                       */
    uint32_t DWDCTRL;        /**< 0x0090: Digital Watchdog Control Register   */
    uint32_t DWDPRLD;        /**< 0x0094: Digital Watchdog Preload Register */
    uint32_t WDSTATUS;       /**< 0x0098: Watchdog Status Register  */
    uint32_t WDKEY;          /**< 0x009C: Watchdog Key Register  */
    uint32_t DWDCNTR;        /**< 0x00A0: Digital Watchdog Down Counter   */
    uint32_t WWDRXNCTRL;     /**< 0x00A4: Digital Windowed Watchdog Reaction Control */
    uint32_t WWDSIZECTRL;    /**< 0x00A8: Digital Windowed Watchdog Window Size Control  */
    uint32_t INTCLRENABLE;   /**< 0x00AC: RTI Compare Interrupt Clear Enable Register  */
    uint32_t COMP0CLR;       /**< 0x00B0: RTI Compare 0 Clear Register   */
    uint32_t COMP1CLR;       /**< 0x00B4: RTI Compare 1 Clear Register */
    uint32_t COMP2CLR;       /**< 0x00B8: RTI Compare 2 Clear Register  */
    uint32_t COMP3CLR;       /**< 0x00BC: RTI Compare 3 Clear Register  */
} rtiBASE_t;

/** @def rtiREG1
*   @brief RTI1 Register Frame Pointer
*
*   This pointer is used by the RTI driver to access the RTI1 registers.
*/
#define rtiREG1 ((rtiBASE_t *)0xFFFFFC00U)

#endif
