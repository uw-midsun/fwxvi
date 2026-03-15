#pragma once

/************************************************************************************************
 * @file    tvgLoader.h
 *
 * @brief   Tvgloader
 *
 * @date    2026-03-15
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "../../lv_conf_internal.h"
#include "tvgLoadModule.h"

/* Intra-component Headers */

/**
 * @defgroup tvgLoader
 * @brief    tvgLoader Firmware
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

#ifndef _TVG_LOADER_H_
#define _TVG_LOADER_H_

struct LoaderMgr
{
    static bool init();
    static bool term();
    static LoadModule* loader(const string& path, bool* invalid);
    static LoadModule* loader(const char* data, uint32_t size, const string& mimeType, bool copy);
    static LoadModule* loader(const uint32_t* data, uint32_t w, uint32_t h, bool copy);
    static LoadModule* loader(const char* name, const char* data, uint32_t size, const string& mimeType, bool copy);
    static LoadModule* loader(const char* key);
    static bool retrieve(const string& path);
    static bool retrieve(LoadModule* loader);
};

#endif //_TVG_LOADER_H_

#endif /* LV_USE_THORVG_INTERNAL */


/** @} */
