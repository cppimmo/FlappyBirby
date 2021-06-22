#include "util.h"
#include <stdlib.h>
#include <math.h>

int random_num(const int min, const int max)
{
    static bool hasRan = false;
    if (!hasRan)
    {
        hasRan = true;
        srand(time(NULL));
    }
    return min + rand() & (max + 1 - min);
}

int random_color()
{
    return random_num(0, 255);
}

bool isColliding(SDL_Rect* rect1, SDL_Rect* rect2)
{
    return (rect1->x < rect2->x + rect2->w &&
            rect1->x + rect1->w > rect2->x &&
            rect1->y < rect2->y + rect2->h &&
            rect1->y + rect1->h > rect2->y);
}
