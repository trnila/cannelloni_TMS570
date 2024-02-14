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


#ifndef __REG_VIM_H__
#define __REG_VIM_H__

#include <stdint.h>


/* Vim Register Frame Definition */
/** @struct vimBase
*   @brief Vim Register Frame Definition
*
*   This type is used to access the Vim Registers.
*/
/** @typedef vimBASE_t
*   @brief VIM Register Frame Type Definition
*
*   This type is used to access the VIM Registers.
*/
typedef volatile struct vimBase
{
    uint32_t      rsvd1[59U];       /* 0x0000 - 0x00E8 Reserved */
    uint32_t      ECCSTAT;          /* 0x00EC        */
    uint32_t      ECCCTL;           /* 0x00F0        */
    uint32_t      UERRADDR;         /* 0x00F4        */
    uint32_t      FBVECADDR;        /* 0x00F8        */
    uint32_t      SBERRADDR;        /* 0x00FC        */
    uint32_t      IRQINDEX;         /* 0x0100        */
    uint32_t      FIQINDEX;         /* 0x0104        */
    uint32_t      rsvd2;            /* 0x0108        */
    uint32_t      rsvd3;            /* 0x010C        */
    uint32_t      FIRQPR0;          /* 0x0110        */
    uint32_t      FIRQPR1;          /* 0x0114        */
    uint32_t      FIRQPR2;          /* 0x0118        */
    uint32_t      FIRQPR3;          /* 0x011C        */
    uint32_t      INTREQ0;          /* 0x0120        */
    uint32_t      INTREQ1;          /* 0x0124        */
    uint32_t      INTREQ2;          /* 0x0128        */
    uint32_t      INTREQ3;          /* 0x012C        */
    uint32_t      REQMASKSET0;      /* 0x0130        */
    uint32_t      REQMASKSET1;      /* 0x0134        */
    uint32_t      REQMASKSET2;      /* 0x0138        */
    uint32_t      REQMASKSET3;      /* 0x013C        */
    uint32_t      REQMASKCLR0;      /* 0x0140        */
    uint32_t      REQMASKCLR1;      /* 0x0144        */
    uint32_t      REQMASKCLR2;      /* 0x0148        */
    uint32_t      REQMASKCLR3;      /* 0x014C        */
    uint32_t      WAKEMASKSET0;     /* 0x0150        */
    uint32_t      WAKEMASKSET1;     /* 0x0154        */
    uint32_t      WAKEMASKSET2;     /* 0x0158        */
    uint32_t      WAKEMASKSET3;     /* 0x015C        */
    uint32_t      WAKEMASKCLR0;     /* 0x0160        */
    uint32_t      WAKEMASKCLR1;     /* 0x0164        */
    uint32_t      WAKEMASKCLR2;     /* 0x0168        */
    uint32_t      WAKEMASKCLR3;     /* 0x016C        */
    uint32_t      IRQVECREG;        /* 0x0170        */
    uint32_t      FIQVECREG;        /* 0x0174        */
    uint32_t      CAPEVT;           /* 0x0178        */
    uint32_t      rsvd4;            /* 0x017C        */
    uint32_t      CHANCTRL[32U];    /* 0x0180-0x02FC */
} vimBASE_t;

#define vimREG ((vimBASE_t *)0xFFFFFD00U)

#endif
