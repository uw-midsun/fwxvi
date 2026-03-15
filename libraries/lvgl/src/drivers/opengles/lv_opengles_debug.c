/************************************************************************************************
 * @file    lv_opengles_debug.c
 *
 * @brief   Lv Opengles Debug
 *
 * @date    2026-03-15
 * @author  Midnight Sun Team #24 - MSXVI
 ************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "../../misc/lv_log.h"
#include "lv_opengles_debug.h"
#include "lv_opengles_private.h"

/* Intra-component Headers */

/*********************
 *      INCLUDES
 *********************/

#if LV_USE_OPENGLES

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/

/**********************
 *  STATIC VARIABLES
 **********************/

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

#if LV_USE_OPENGLES_DEBUG
void GLClearError(void)
{
    while(glGetError() != GL_NO_ERROR);
}

void GLLogCall(const char * function, const char * file, int line)
{
    GLenum error;
    while((error = glGetError()) != GL_NO_ERROR) {
        LV_LOG_ERROR("[OpenGL Error] (%d) %s %s:%d", error, function, file, line);
    }
}
#endif

/**********************
 *   STATIC FUNCTIONS
 **********************/

#endif /* LV_USE_OPENGLES */
