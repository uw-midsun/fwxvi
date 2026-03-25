#pragma once

/************************************************************************************************
 * @file    deprecated_definitions.h
 *
 * @brief   Deprecated Definitions
 *
 * @date    2026-03-25
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
            #include "portmacro.h"
        #include "portmacro.h"
    #include "../../../Source/portable/GCC/ColdFire_V2/portmacro.h"
    #include "../../Source/portable/CodeWarrior/ColdFire_V2/portmacro.h"
    #include "../../Source/portable/CodeWarrior/HCS12/portmacro.h"
    #include "../../Source/portable/GCC/ARM7_AT91FR40008/portmacro.h"
    #include "../../Source/portable/GCC/ARM7_AT91SAM7S/portmacro.h"
    #include "../../Source/portable/GCC/ARM7_LPC2000/portmacro.h"
    #include "../../Source/portable/GCC/ARM7_LPC23xx/portmacro.h"
    #include "../../Source/portable/GCC/ARM_CM3/portmacro.h"
    #include "../../Source/portable/GCC/ARM_CM3/portmacro.h"
    #include "../../Source/portable/GCC/H8S2329/portmacro.h"
    #include "../../Source/portable/GCC/HCS12/portmacro.h"
    #include "../../Source/portable/GCC/MCF5235/portmacro.h"
    #include "../../Source/portable/GCC/MSP430F449/portmacro.h"
    #include "../../Source/portable/GCC/MicroBlaze/portmacro.h"
    #include "../../Source/portable/GCC/PPC405_Xilinx/portmacro.h"
    #include "../../Source/portable/GCC/PPC440_Xilinx/portmacro.h"
    #include "../../Source/portable/IAR/78K0R/portmacro.h"
    #include "../../Source/portable/IAR/78K0R/portmacro.h"
    #include "../../Source/portable/IAR/ARM_CM3/portmacro.h"
    #include "../../Source/portable/IAR/ARM_CM3/portmacro.h"
    #include "../../Source/portable/IAR/V850ES/portmacro.h"
    #include "../../Source/portable/IAR/V850ES/portmacro.h"
    #include "../../Source/portable/IAR/V850ES/portmacro.h"
    #include "../../Source/portable/IAR/V850ES/portmacro.h"
    #include "../../Source/portable/IAR/V850ES/portmacro.h"
    #include "../../Source/portable/MPLAB/PIC18F/portmacro.h"
    #include "../../Source/portable/MPLAB/PIC24_dsPIC/portmacro.h"
    #include "../../Source/portable/MPLAB/PIC24_dsPIC/portmacro.h"
    #include "../../Source/portable/MPLAB/PIC32MX/portmacro.h"
    #include "../../Source/portable/RVDS/ARM_CM3/portmacro.h"
    #include "../../Source/portable/Rowley/MSP430F449/portmacro.h"
    #include "../../Source/portable/SDCC/Cygnal/portmacro.h"
    #include "../portable/GCC/ATMega323/portmacro.h"
    #include "../portable/IAR/ATMega323/portmacro.h"
    #include "frconfig.h"
    #include "frconfig.h"
    #include "libFreeRTOS/Include/portmacro.h"
    #include "portmacro.h"
    #include "portmacro.h"
    #include "portmacro.h"

/* Intra-component Headers */
    #include "..\..\Source\portable\GCC\STR75x\portmacro.h"
    #include "..\..\Source\portable\IAR\AtmelSAM7S64\portmacro.h"
    #include "..\..\Source\portable\IAR\AtmelSAM9XE\portmacro.h"
    #include "..\..\Source\portable\IAR\LPC2000\portmacro.h"
    #include "..\..\Source\portable\IAR\MSP430\portmacro.h"
    #include "..\..\Source\portable\IAR\STR71x\portmacro.h"
    #include "..\..\Source\portable\IAR\STR75x\portmacro.h"
    #include "..\..\Source\portable\IAR\STR91x\portmacro.h"
    #include "..\..\Source\portable\Paradigm\Tern_EE\small\portmacro.h"
    #include "..\..\Source\portable\RVDS\ARM7_LPC21xx\portmacro.h"
    #include "..\..\Source\portable\Softune\MB96340\portmacro.h"
    #include "..\..\Source\portable\owatcom\16bitdos\flsh186\portmacro.h"
    #include "..\..\Source\portable\owatcom\16bitdos\pc\portmacro.h"
    #include "..\portable\BCC\16BitDOS\PC\prtmacro.h"
    #include "..\portable\BCC\16BitDOS\flsh186\prtmacro.h"

/**
 * @defgroup deprecated_definitions
 * @brief    deprecated_definitions Firmware
 * @{
 */

/*
 * FreeRTOS Kernel V11.1.0
 * Copyright (C) 2021 Amazon.com, Inc. or its affiliates. All Rights Reserved.
 *
 * SPDX-License-Identifier: MIT
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * https://www.FreeRTOS.org
 * https://github.com/FreeRTOS
 *
 */

#ifndef DEPRECATED_DEFINITIONS_H
#define DEPRECATED_DEFINITIONS_H

/* Each FreeRTOS port has a unique portmacro.h header file.  Originally a
 * pre-processor definition was used to ensure the pre-processor found the correct
 * portmacro.h file for the port being used.  That scheme was deprecated in favour
 * of setting the compiler's include path such that it found the correct
 * portmacro.h file - removing the need for the constant and allowing the
 * portmacro.h file to be located anywhere in relation to the port being used.  The
 * definitions below remain in the code for backward compatibility only.  New
 * projects should not use them. */

#ifdef OPEN_WATCOM_INDUSTRIAL_PC_PORT
    typedef void ( __interrupt __far * pxISR )();
#endif

#ifdef OPEN_WATCOM_FLASH_LITE_186_PORT
    typedef void ( __interrupt __far * pxISR )();
#endif

#ifdef GCC_MEGA_AVR
#endif

#ifdef IAR_MEGA_AVR
#endif

#ifdef MPLAB_PIC24_PORT
#endif

#ifdef MPLAB_DSPIC_PORT
#endif

#ifdef MPLAB_PIC18F_PORT
#endif

#ifdef MPLAB_PIC32MX_PORT
#endif

#ifdef _FEDPICC
#endif

#ifdef SDCC_CYGNAL
#endif

#ifdef GCC_ARM7
#endif

#ifdef GCC_ARM7_ECLIPSE
#endif

#ifdef ROWLEY_LPC23xx
#endif

#ifdef IAR_MSP430
#endif

#ifdef GCC_MSP430
#endif

#ifdef ROWLEY_MSP430
#endif

#ifdef ARM7_LPC21xx_KEIL_RVDS
#endif

#ifdef SAM7_GCC
#endif

#ifdef SAM7_IAR
#endif

#ifdef SAM9XE_IAR
#endif

#ifdef LPC2000_IAR
#endif

#ifdef STR71X_IAR
#endif

#ifdef STR75X_IAR
#endif

#ifdef STR75X_GCC
#endif

#ifdef STR91X_IAR
#endif

#ifdef GCC_H8S
#endif

#ifdef GCC_AT91FR40008
#endif

#ifdef RVDS_ARMCM3_LM3S102
#endif

#ifdef GCC_ARMCM3_LM3S102
#endif

#ifdef GCC_ARMCM3
#endif

#ifdef IAR_ARM_CM3
#endif

#ifdef IAR_ARMCM3_LM
#endif

#ifdef HCS12_CODE_WARRIOR
#endif

#ifdef MICROBLAZE_GCC
#endif

#ifdef TERN_EE
#endif

#ifdef GCC_HCS12
#endif

#ifdef GCC_MCF5235
#endif

#ifdef COLDFIRE_V2_GCC
#endif

#ifdef COLDFIRE_V2_CODEWARRIOR
#endif

#ifdef GCC_PPC405
#endif

#ifdef GCC_PPC440
#endif

#ifdef _16FX_SOFTUNE
#endif

#ifdef BCC_INDUSTRIAL_PC_PORT

/* A short file name has to be used in place of the normal
 * FreeRTOSConfig.h when using the Borland compiler. */
    typedef void ( __interrupt __far * pxISR )();
#endif

#ifdef BCC_FLASH_LITE_186_PORT

/* A short file name has to be used in place of the normal
 * FreeRTOSConfig.h when using the Borland compiler. */
    typedef void ( __interrupt __far * pxISR )();
#endif

#ifdef __GNUC__
    #ifdef __AVR32_AVR32A__
    #endif
#endif

#ifdef __ICCAVR32__
    #ifdef __CORE__
        #if __CORE__ == __AVR32A__
        #endif
    #endif
#endif

#ifdef __91467D
#endif

#ifdef __96340
#endif

#ifdef __IAR_V850ES_Fx3__
#endif

#ifdef __IAR_V850ES_Jx3__
#endif

#ifdef __IAR_V850ES_Jx3_L__
#endif

#ifdef __IAR_V850ES_Jx2__
#endif

#ifdef __IAR_V850ES_Hx2__
#endif

#ifdef __IAR_78K0R_Kx3__
#endif

#ifdef __IAR_78K0R_Kx3L__
#endif

#endif /* DEPRECATED_DEFINITIONS_H */

/** @} */
