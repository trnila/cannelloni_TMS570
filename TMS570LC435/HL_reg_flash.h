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


#ifndef __REG_FLASH_H__
#define __REG_FLASH_H__

#include <stdint.h>

/* Flash Register Frame Definition */
/** @struct flashWBase
*   @brief Flash Wrapper Register Frame Definition
*
*   This type is used to access the Flash Wrapper Registers.
*/
typedef volatile struct flashWBase
{
    uint32_t FRDCNTL;         /* 0x0000 */
    uint32_t rsvd1;           /* 0x0004 */
    uint32_t EE_FEDACCTRL1;   /* 0x0008 */
    uint32_t rsvd2;           /* 0x000C */
    uint32_t rsvd3;           /* 0x0010 */
    uint32_t FEDAC_PASTATUS;  /* 0x0014 */
    uint32_t FEDAC_PBSTATUS;  /* 0x0018 */
    uint32_t FEDAC_GBLSTATUS; /* 0x001C */
    uint32_t rsvd4;           /* 0x0020 */
    uint32_t FEDACSDIS;       /* 0x0024 */
    uint32_t FPRIM_ADD_TAG;   /* 0x0028 */
    uint32_t FDUP_ADD_TAG;    /* 0x002C */
    uint32_t FBPROT;          /* 0x0030 */
    uint32_t FBSE;            /* 0x0034 */
    uint32_t FBBUSY;          /* 0x0038 */
    uint32_t FBAC;            /* 0x003C */
    uint32_t FBPWRMODE;       /* 0x0040 */
    uint32_t FBPRDY;          /* 0x0044 */
    uint32_t FPAC1;           /* 0x0048 */
    uint32_t rsvd5;           /* 0x004C */
    uint32_t FMAC;            /* 0x0050 */
    uint32_t FMSTAT;          /* 0x0054 */
    uint32_t FEMU_DMSW;       /* 0x0058 */
    uint32_t FEMU_DLSW;       /* 0x005C */
    uint32_t FEMU_ECC;        /* 0x0060 */
    uint32_t FLOCK;           /* 0x0064 */
    uint32_t rsvd6;           /* 0x0068 */
    uint32_t FDIAGCTRL;       /* 0x006C */
    uint32_t rsvd7;           /* 0x0070 */
    uint32_t FRAW_ADDR;       /* 0x0074 */
    uint32_t rsvd8;           /* 0x0078 */
    uint32_t FPAR_OVR;        /* 0x007C */
    uint32_t rsvd9[13U];      /* 0x0080 - 0x00B0 */
    uint32_t RCR_VALID;       /* 0x00B4 */
    uint32_t ACC_THRESHOLD;   /* 0x00B8 */
    uint32_t rsvd10;          /* 0x00BC */
    uint32_t FEDACSDIS2;      /* 0x00C0 */
    uint32_t rsvd11;          /* 0x00C4 */
    uint32_t rsvd12;          /* 0x00C8 */
    uint32_t rsvd13;          /* 0x00CC */
    uint32_t RCR_VALUE0;      /* 0x00D0 */
    uint32_t RCR_VALUE1;      /* 0x00D4 */
    uint32_t rsvd14[108U];    /* 0x00D8 - 0x00284 */
    uint32_t FSM_WR_ENA;      /* 0x0288 */
    uint32_t rsvd15[11U];     /* 0x028C - 0x002B4 */
    uint32_t EEPROM_CONFIG;   /* 0x02B8 */
    uint32_t rsvd16;          /* 0x02BC */
    uint32_t FSM_SECTOR1;     /* 0x02C0 */
    uint32_t FSM_SECTOR2;     /* 0x02C4 */
    uint32_t rsvd17[78U];     /* 0x02A8 */
    uint32_t FCFG_BANK;       /* 0x02B8 */

} flashWBASE_t;

/** @def flashWREG
*   @brief Flash Wrapper Register Frame Pointer
*
*   This pointer is used by the system driver to access the flash wrapper registers.
*/
#define flashWREG ((flashWBASE_t *)(0xFFF87000U))


#endif
