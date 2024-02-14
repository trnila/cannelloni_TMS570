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


#ifndef __REG_PINMUX_H__
#define __REG_PINMUX_H__

#include <stdint.h>


/** @struct pinMuxBase
*   @brief PINMUX Register Definition
*
*   This structure is used to access the PINMUX module registers.
*/
/** @typedef pinMuxBASE_t
*   @brief PINMUX Register Frame Type Definition
*
*   This type is used to access the PINMUX Registers.
*/
typedef volatile struct pinMuxBase
{
    uint32_t REVISION_REG; /**< 0x00: Revision Register */
	uint32_t rsvd1[7];	 /**<Reserved */
	uint32_t BOOT_REG;	/**< 0x20: Boot Mode Register */
	uint32_t rsvd2[5];	/**<Reserved */
	uint32_t KICKER0;		/**< 0x38: Kicker Register 0 */
	uint32_t KICKER1;		/**< 0x3C: Kicker Register 1 */
	uint32_t rsvd3[40];	/**<Reserved */
	uint32_t ERR_RAW_STATUS_REG; 	/**< 0xE0: Error Raw Status / Set Register */
	uint32_t ERR_ENABLED_STATUS_REG;	/**< 0xE4: Error Enabled Status / Clear Register */
	uint32_t ERR_ENABLE_REG;		/**< 0xE8: Error Signaling Enable Register */
	uint32_t ERR_ENABLE_CLR_REG;	/**< 0xEC: Error Signaling Enable Clear Register*/
	uint32_t rsvd4;				/**<Reserved */
	uint32_t FAULT_ADDRESS_REG;	/**< 0xF4: Fault Address Register */
	uint32_t FAULT_STATUS_REG;	/**< 0xF8: Fault Status Register */
	uint32_t FAULT_CLEAR_REG;		/**< 0xFC: Fault Clear Register */
	uint32_t rsvd5[4];			/**< Reserved*/
	uint32_t PINMUX[180];			/**< 0x110 - 1A4 : Output Pin Multiplexing Control Registers (38 registers); 0x250 - 0x29C : Input Pin Multiplexing Control Registers (20); 0X390 - 3DC : Special Functionality Control Registers (20) */


}pinMuxBASE_t;

/** @def pinMuxReg
*       @brief Pin Muxing Control Register Frame Pointer
*
*               This pointer is used to access the PINMUX module registers.
*/
#define pinMuxReg ((pinMuxBASE_t *) 0xFFFF1C00U)


#endif
