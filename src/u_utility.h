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
#ifndef __U_UTILITY_H__
#define __U_UTILITY_H__

#include <SDL2/SDL.h>

#include <stdarg.h>

#define SIZEOF_ARRAY(X) sizeof(X)/sizeof(X[0])
#define LOG_MESSAGE(fmt, ...) U_LogMessage(LOG_MSG, fmt, ##__VA_ARGS__)
#define LOG_ERROR(fmt, ...) U_LogMessage(LOG_ERR, fmt, ##__VA_ARGS__)

#undef bool
#undef true
#undef false

enum boolean_t {
    false,
    true,
};
typedef enum boolean_t boolean;

//type alias for better reading
typedef unsigned int uint;
enum gamestate_t {
    STATE_START = 0,
    STATE_PLAY,
    STATE_OVER,
};
typedef enum gamestate_t GameState;

typedef enum {
	LOG_LOG,
	LOG_MSG,
	LOG_ERR,
} LogType;

int U_RandomNum(const int min, const int max);
int U_RandomColor();
boolean U_IsColliding(SDL_Rect *rect1, SDL_Rect *rect2);
void U_LogMessage(LogType type, const char *fmt, ...);

#endif
