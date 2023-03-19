/*=============================================================================*
 * Utility.c - Utility types and functions.
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
#include "Utility.h"

#include <stdlib.h>
#include <time.h>
#include <math.h>

int U_RandomNum(int min, int max)
{
    static boolean c_hasRan = false;
    if (!c_hasRan)
	{
        c_hasRan = true;
        srand(time(NULL));
    }
    return (min + rand()) & (max + 1 - min);
}

int U_RandomColor(void)
{
    return U_RandomNum(0, 255);
}

boolean U_IsColliding(SDL_Rect *pRectA, SDL_Rect *pRectB)
{
    return (pRectA->x < pRectB->x + pRectB->w &&
            pRectA->x + pRectA->w > pRectB->x &&
            pRectA->y < pRectB->y + pRectB->h &&
            pRectA->y + pRectA->h > pRectB->y);
}

#define kMAX_LOG_LEN 512
void U_LogMessage(LogType logType, const char *pFormat, ...)
{
	char pLogBuffer[kMAX_LOG_LEN];
	FILE *pHandle;
	Uint32 flags; // MessageBox flags
	switch (logType)
	{
	case LOG_LOG:
		pHandle = stdout;
		break;
	case LOG_MSG:
		pHandle = stdout;
		break;
	case LOG_ERR:
		pHandle = stderr;
		flags = SDL_MESSAGEBOX_ERROR;
		break;
	default:
		return;
	}

	va_list args;
	va_start(args, pFormat);
	snprintf(pLogBuffer, kMAX_LOG_LEN, pFormat, args);
	va_end(args);

	if (pHandle == stdout)
	{
		fprintf(pHandle, pLogBuffer);
	}
	else
	{
		fprintf(pHandle, pLogBuffer);
		SDL_ShowSimpleMessageBox(flags, "Error", pLogBuffer, NULL);
	}
}
