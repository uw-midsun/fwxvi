#pragma once

/************************************************************************************************
 * @file    tvgCompressor.h
 *
 * @brief   Tvgcompressor
 *
 * @date    2026-03-15
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */
#include <cstdint>

/* Inter-component Headers */
#include "../../lv_conf_internal.h"

/* Intra-component Headers */

/**
 * @defgroup tvgCompressor
 * @brief    tvgCompressor Firmware
 * @{
 */

/*
 * Copyright (c) 2020 - 2024 the ThorVG project. All rights reserved.

 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#if LV_USE_THORVG_INTERNAL

#ifndef _TVG_COMPRESSOR_H_
#define _TVG_COMPRESSOR_H_

namespace tvg
{
    uint8_t* lzwEncode(const uint8_t* uncompressed, uint32_t uncompressedSizeBytes, uint32_t* compressedSizeBytes, uint32_t* compressedSizeBits);
    uint8_t* lzwDecode(const uint8_t* compressed, uint32_t compressedSizeBytes, uint32_t compressedSizeBits, uint32_t uncompressedSizeBytes);
    size_t b64Decode(const char* encoded, const size_t len, char** decoded);
    unsigned long djb2Encode(const char* str);
}

#endif  //_TVG_COMPRESSOR_H_

#endif /* LV_USE_THORVG_INTERNAL */


/** @} */
