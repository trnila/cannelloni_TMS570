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


#ifndef __REG_SYSTEM_H__
#define __REG_SYSTEM_H__

#include <stdint.h>
#include "HL_reg_gio.h"


/* System Register Frame 1 Definition */
/** @struct systemBase1
*   @brief System Register Frame 1 Definition
*
*   This type is used to access the System 1 Registers.
*/
/** @typedef systemBASE1_t
*   @brief System Register Frame 1 Type Definition
*
*   This type is used to access the System 1 Registers.
*/
typedef volatile struct systemBase1
{
    uint32_t SYSPC1;                 /* 0x0000 */
    uint32_t SYSPC2;                 /* 0x0004 */
    uint32_t SYSPC3;                 /* 0x0008 */
    uint32_t SYSPC4;                 /* 0x000C */
    uint32_t SYSPC5;                 /* 0x0010 */
    uint32_t SYSPC6;                 /* 0x0014 */
    uint32_t SYSPC7;                 /* 0x0018 */
    uint32_t SYSPC8;                 /* 0x001C */
    uint32_t SYSPC9;                 /* 0x0020 */
    uint32_t rsvd1;                  /* 0x0024 */
    uint32_t rsvd2;                  /* 0x0028 */
    uint32_t rsvd3;                  /* 0x002C */
    uint32_t CSDIS;                  /* 0x0030 */
    uint32_t CSDISSET;               /* 0x0034 */
    uint32_t CSDISCLR;               /* 0x0038 */
    uint32_t CDDIS;                  /* 0x003C */
    uint32_t CDDISSET;               /* 0x0040 */
    uint32_t CDDISCLR;               /* 0x0044 */
    uint32_t GHVSRC;                 /* 0x0048 */
    uint32_t VCLKASRC;               /* 0x004C */
    uint32_t RCLKSRC;                /* 0x0050 */
    uint32_t CSVSTAT;                /* 0x0054 */
    uint32_t MSTGCR;                 /* 0x0058 */
    uint32_t MINITGCR;               /* 0x005C */
    uint32_t MSINENA;                /* 0x0060 */
    uint32_t MSTFAIL;                /* 0x0064 */
    uint32_t MSTCGSTAT;              /* 0x0068 */
    uint32_t MINISTAT;               /* 0x006C */
    uint32_t PLLCTL1;                /* 0x0070 */
    uint32_t PLLCTL2;                /* 0x0074 */
    uint32_t SYSPC10;                /* 0x0078 */
    uint32_t DIEIDL;                 /* 0x007C */
    uint32_t DIEIDH;                 /* 0x0080 */
    uint32_t rsvd4;                  /* 0x0084 */
    uint32_t LPOMONCTL;              /* 0x0088 */
    uint32_t CLKTEST;                /* 0x008C */
    uint32_t DFTCTRLREG1;            /* 0x0090 */
    uint32_t DFTCTRLREG2;            /* 0x0094 */
    uint32_t rsvd5;                  /* 0x0098 */
    uint32_t rsvd6;                  /* 0x009C */
    uint32_t GPREG1;                 /* 0x00A0 */
    uint32_t rsvd7;                  /* 0x00A4 */
    uint32_t rsvd8;                  /* 0x00A8 */
    uint32_t rsvd9;                  /* 0x00AC */
    uint32_t SSIR1;                  /* 0x00B0 */
    uint32_t SSIR2;                  /* 0x00B4 */
    uint32_t SSIR3;                  /* 0x00B8 */
    uint32_t SSIR4;                  /* 0x00BC */
    uint32_t RAMGCR;                 /* 0x00C0 */
    uint32_t BMMCR1;                 /* 0x00C4 */
    uint32_t rsvd10;                 /* 0x00C8 */
    uint32_t CPURSTCR;               /* 0x00CC */
    uint32_t CLKCNTL;                /* 0x00D0 */
    uint32_t ECPCNTL;                /* 0x00D4 */
    uint32_t rsvd11;                 /* 0x00D8 */
    uint32_t DEVCR1;                 /* 0x00DC */
    uint32_t SYSECR;                 /* 0x00E0 */
    uint32_t SYSESR;                 /* 0x00E4 */
    uint32_t SYSTASR;                /* 0x00E8 */
    uint32_t GBLSTAT;                /* 0x00EC */
    uint32_t DEVID;                  /* 0x00F0 */
    uint32_t SSIVEC;                 /* 0x00F4 */
    uint32_t SSIF;                   /* 0x00F8 */
} systemBASE1_t;


/** @def systemREG1
*   @brief System Register Frame 1 Pointer
*
*   This pointer is used by the system driver to access the system frame 1 registers.
*/
#define systemREG1 ((systemBASE1_t *)0xFFFFFF00U)

/** @def systemPORT
*   @brief ECLK GIO Port Register Pointer
*
*   Pointer used by the GIO driver to access I/O PORT of System/Eclk
*   (use the GIO drivers to access the port pins).
*/
#define systemPORT ((gioPORT_t *)0xFFFFFF04U)


/* System Register Frame 2 Definition */
/** @struct systemBase2
*   @brief System Register Frame 2 Definition
*
*   This type is used to access the System 2 Registers.
*/
/** @typedef systemBASE2_t
*   @brief System Register Frame 2 Type Definition
*
*   This type is used to access the System 2 Registers.
*/
typedef volatile struct systemBase2
{
    uint32_t PLLCTL3;        /* 0x0000 */
    uint32_t rsvd1;          /* 0x0004 */
    uint32_t STCCLKDIV;      /* 0x0008 */
    uint32_t rsvd2[6U];      /* 0x000C */
    uint32_t ECPCNTL;        /* 0x0024 */
    uint32_t ECPCNTL1;       /* 0x0028 */
    uint32_t rsvd3[4U];      /* 0x002C */
    uint32_t CLK2CNTRL;      /* 0x003C */
    uint32_t VCLKACON1;      /* 0x0040 */
    uint32_t rsvd4[4U];      /* 0x0044 */
    uint32_t HCLKCNTL;       /* 0x0054 */
    uint32_t rsvd5[6U];      /* 0x0058 */
    uint32_t CLKSLIP;        /* 0x0070 */
    uint32_t rsvd6;          /* 0x0074 */
	uint32_t IP1ECCERREN;	   /* 0x0078 */
	uint32_t rsvd7[28U];     /* 0x007C */
    uint32_t EFC_CTLEN;      /* 0x00EC */
    uint32_t DIEIDL_REG0;    /* 0x00F0 */
    uint32_t DIEIDH_REG1;    /* 0x00F4 */
    uint32_t DIEIDL_REG2;    /* 0x00F8 */
    uint32_t DIEIDH_REG3;    /* 0x00FC */
} systemBASE2_t;

/** @def systemREG2
*   @brief System Register Frame 2 Pointer
*
*   This pointer is used by the system driver to access the system frame 2 registers.
*/
#define systemREG2 ((systemBASE2_t *)0xFFFFE100U)


#endif
