#pragma once

/************************************************************************************************
 * @file    lv_uefi_edk2.h
 *
 * @brief   Lv Uefi Edk2
 *
 * @date    2026-03-15
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
    #include <Base.h>
    #include <Guid/FileInfo.h>
    #include <Protocol/AbsolutePointer.h>
    #include <Protocol/EdidActive.h>
    #include <Protocol/GraphicsOutput.h>
    #include <Protocol/LoadedImage.h>
    #include <Protocol/SimpleFileSystem.h>
    #include <Protocol/SimplePointer.h>
    #include <Protocol/SimpleTextIn.h>
    #include <Protocol/SimpleTextOut.h>
    #include <Protocol/Timestamp.h>
    #include <Uefi.h>

/* Inter-component Headers */

/* Intra-component Headers */

/**
 * @defgroup lv_uefi_edk2
 * @brief    lv_uefi_edk2 Firmware
 * @{
 */

#ifndef LV_UEFI_EDK2_H
#define LV_UEFI_EDK2_H

#if LV_USE_UEFI

    #define LV_UEFI_EDK2_HEADERS    1

#endif

#endif
/** @} */
