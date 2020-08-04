#ifndef UTILITY_H
#define UTILITY_H

#include <SDL2/SDL.h>
#include <stdbool.h>

#define SIZEOF_ARRAY(X) sizeof(X)/sizeof(X[0])
//type alias for better reading
typedef unsigned int uint;

typedef enum
{
    STATE_START = 0,
    STATE_PLAY,
    STATE_OVER,
} GAME_STATE;

int random_num(const int min, const int max);
int random_color();
bool isColliding(SDL_Rect* rect1, SDL_Rect* rect2);

#endif // UTILITY_H
