/*=============================================================================*
 * Utility.h - Utility types and functions.
 *
 * Copyright (c) 2023, Brian Hoffpauir All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *============================================================================*/
#ifndef _FB_UTILITY_H_
#define _FB_UTILITY_H_

#include <stdarg.h>

#include <SDL2/SDL.h>

#undef ARRAY_SIZE
#undef bool
#undef true
#undef false

#define ARRAY_SIZE(X) sizeof(X) / sizeof(X[0])
#define FB_LOG(fmt, ...) U_LogMessage(LOG_LOG, fmt, ##__VA_ARGS__)
#define FB_MSG(fmt, ...) U_LogMessage(LOG_MSG, fmt, ##__VA_ARGS__)
#define FB_ERR(fmt, ...) U_LogMessage(LOG_ERR, fmt, ##__VA_ARGS__)

enum boolean_t
{
    false,
    true,
};
typedef enum boolean_t boolean;

// Type aliases for better reading
typedef unsigned int uint;

typedef enum
{
	LOG_LOG,
	LOG_MSG,
	LOG_ERR,
} LogType;

int U_RandomNum(int min, int max);
int U_RandomColor(void);
boolean U_IsColliding(SDL_Rect *pRectA, SDL_Rect *pRectB);
void U_LogMessage(LogType logType, const char *pFormat, ...);

#endif /* !_FB_UTILITY_H_ */
