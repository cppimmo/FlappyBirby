/* =============================================================================
** FlappyBirby, file: u_utility.c Created 12/10/2021
**
** Copyright 2021 Brian Hoffpauir TX, USA
** All rights reserved.
**
** Redistribution and use of this source file, with or without modification, is
** permitted provided that the following conditions are met:
**
** 1. Redistributions of this source file must retain the above copyright
**    notice, this list of conditions and the following disclaimer.
**
** THIS SOFTWARE IS PROVIDED BY THE AUTHOR "AS IS" AND ANY EXPRESS OR IMPLIED
** WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
** MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO
** EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
** PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
** OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
** WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
** OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
** ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
** =============================================================================
**/
#include "u_utility.h"

#include <stdlib.h>
#include <time.h>
#include <math.h>

int U_RandomNum(const int min, const int max) {
    static boolean hasran = false;
    if (!hasran) {
        hasran = true;
        srand(time(NULL));
    }
    return min + rand() & (max + 1 - min);
}

int U_RandomColor() {
    return U_RandomNum(0, 255);
}

boolean U_IsColliding(SDL_Rect *rect1, SDL_Rect *rect2) {
    return (rect1->x < rect2->x + rect2->w &&
            rect1->x + rect1->w > rect2->x &&
            rect1->y < rect2->y + rect2->h &&
            rect1->y + rect1->h > rect2->y);
}

#define MAX_LOG_LEN 512
void U_LogMessage(LogType type, const char *fmt, ...) {
	char logbuf[MAX_LOG_LEN];
	FILE *handle;
	Uint32 flags;
	switch (type) {
	case LOG_LOG:
		handle = stdout;
		break;
	case LOG_MSG:
		handle = stdout;
		break;
	case LOG_ERR:
		handle = stderr;
		flags = SDL_MESSAGEBOX_ERROR;
		break;
	default:
		return;
	}

	va_list args;
	va_start(args, fmt);
	snprintf(logbuf, MAX_LOG_LEN, fmt, args);
	va_end(args);

	if (handle == stdout) {
		fprintf(handle, logbuf);
	} else {
		fprintf(handle, logbuf);
		SDL_ShowSimpleMessageBox(flags, "Error", logbuf, NULL);
	}
}

