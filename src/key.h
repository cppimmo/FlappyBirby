#ifndef KEY_H
#define KEY_H

#include <SDL2/SDL.h>

static SDL_Keycode pressedKey;
static SDL_MouseButtonEvent mbState;

void onMouseDown(SDL_MouseButtonEvent event)
{
    mbState = event;
}

void onMouseUp(SDL_MouseButtonEvent event)
{
    mbState = event;
}

SDL_MouseButtonEvent getMousePressed()
{
    return mbState;
}

void onKeyDown(SDL_Keycode code)
{
    pressedKey = code;
}

void onKeyUp(SDL_Keycode code)
{
    pressedKey = code;
}

SDL_Keycode getPressedKey()
{
    return pressedKey;
}

#endif // KEY_H
