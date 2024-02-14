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


#ifndef __REG_PCR_H__
#define __REG_PCR_H__

#include <stdint.h>


/* Pcr Register Frame Definition */
/** @struct pcrBase
*   @brief Pcr Register Frame Definition
*
*   This type is used to access the Pcr Registers.
*/
/** @typedef pcrBASE_t
*   @brief PCR Register Frame Type Definition
*
*   This type is used to access the PCR Registers.
*/
typedef volatile struct pcrBase
{
    uint32_t PMPROTSET0;    /* 0x0000 */
    uint32_t PMPROTSET1;    /* 0x0004 */
    uint32_t rsvd1[2U];     /* 0x0008 */
    uint32_t PMPROTCLR0;    /* 0x0010 */
    uint32_t PMPROTCLR1;    /* 0x0014 */
    uint32_t rsvd2[2U];     /* 0x0018 */
    uint32_t PPROTSET0;     /* 0x0020 */
    uint32_t PPROTSET1;     /* 0x0024 */
    uint32_t PPROTSET2;     /* 0x0028 */
    uint32_t PPROTSET3;     /* 0x002C */
    uint32_t rsvd3[4U];     /* 0x0030 */
    uint32_t PPROTCLR0;     /* 0x0040 */
    uint32_t PPROTCLR1;     /* 0x0044 */
    uint32_t PPROTCLR2;     /* 0x0048 */
    uint32_t PPROTCLR3;     /* 0x004C */
    uint32_t rsvd4[4U];     /* 0x0050 */
    uint32_t PCSPWRDWNSET0; /* 0x0060 */
    uint32_t PCSPWRDWNSET1; /* 0x0064 */
    uint32_t rsvd5[2U];     /* 0x0068 */
    uint32_t PCSPWRDWNCLR0; /* 0x0070 */
    uint32_t PCSPWRDWNCLR1; /* 0x0074 */
    uint32_t rsvd6[2U];     /* 0x0078 */
    uint32_t PSPWRDWNSET0;  /* 0x0080 */
    uint32_t PSPWRDWNSET1;  /* 0x0084 */
    uint32_t PSPWRDWNSET2;  /* 0x0088 */
    uint32_t PSPWRDWNSET3;  /* 0x008C */
    uint32_t rsvd7[4U];     /* 0x0090 */
    uint32_t PSPWRDWNCLR0;  /* 0x00A0 */
    uint32_t PSPWRDWNCLR1;  /* 0x00A4 */
    uint32_t PSPWRDWNCLR2;  /* 0x00A8 */
    uint32_t PSPWRDWNCLR3;  /* 0x00AC */
    uint32_t rsvd8[4U];     /* 0x00B0 */
    uint32_t PDPWRDWNSET;   /* 0x00C0 */
    uint32_t PDPWRDWNCLR;   /* 0x00C4 */
    uint32_t rsvd9[78U];    /* 0x00C8 */
    uint32_t MSTIDWRENA;    /* 0x0200 */
    uint32_t MSTIDENA;      /* 0x0204 */
    uint32_t MSTIDDIAGCTRL; /* 0x0208 */
    uint32_t rsvd10[61U];   /* 0x020C */
    struct
    {
        uint32_t PSxMSTID_L;
        uint32_t PSxMSTID_H;
    }PSxMSTID[32];            /* 0x0300 */
    struct
    {
        uint32_t PPSxMSTID_L;
        uint32_t PPSxMSTID_H;
    }PPSxMSTID[8];            /* 0x0400 */
    struct
    {
        uint32_t PPSExMSTID_L;
        uint32_t PPSExMSTID_H;
    }PPSExMSTID[32];            /* 0x0440 */
    uint32_t PCSxMSTID[32];    /* 0x0540 */
    uint32_t PPCSxMSTID[8];    /* 0x05C0 */
} pcrBASE_t;

/** @def pcrREG1
*   @brief Pcr1 Register Frame Pointer
*
*   This pointer is used by the system driver to access the Pcr1 registers.
*/
#define pcrREG1 ((pcrBASE_t *)0xFFFF1000U)

/** @def pcrREG2
*   @brief Pcr2 Register Frame Pointer
*
*   This pointer is used by the system driver to access the Pcr2 registers.
*/
#define pcrREG2 ((pcrBASE_t *)0xFCFF1000U)

/** @def pcrREG3
*   @brief Pcr3 Register Frame Pointer
*
*   This pointer is used by the system driver to access the Pcr3 registers.
*/
#define pcrREG3 ((pcrBASE_t *)0xFFF78000U)

#endif
