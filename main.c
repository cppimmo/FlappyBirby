#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_ttf.h>

#ifdef WIN32
    #define WIN32_LEAN_AND_MEAN
    #include <windows.h>
#endif // WIN32

#include "util.h"

#define NUM_OF_PIPES 16
#define NUM_OF_FRAMES 2
//800 / 2 + 100 = 525
//800 / 2 - 100 = 300

char title[24] = "Flappy Birb, Score: ";
const uint width = 800;
const uint height = 600;
//pipe info
const uint pipeWidth = 150;
const int lengthInBetween = 100;
const uint pipeHeight = 400;
SDL_Rect pipes[NUM_OF_PIPES];
SDL_Texture* pipeTexture = NULL;
//bird info
const uint birdSize = 70;
bool birdHasCollided = false;
int curFrame = 0;
int speed = 250;
bool hasScored = false;
SDL_Rect bird;
SDL_Rect birdAnim;
SDL_Texture* birdTexture = NULL;

SDL_Texture* bgTexture = NULL;
SDL_Rect bgRect;
SDL_Texture* groundTexture = NULL;
SDL_Rect groundRect;

int score = 0;
TTF_Font* font = NULL;
SDL_Surface* textSurface = NULL;
SDL_Texture* textTexture = NULL;
SDL_Texture* textOverTexture = NULL;
SDL_Rect textRect;
SDL_Rect textOverRect;
//SDL_Color textColor;

Mix_Chunk* flapSound = NULL;
Mix_Chunk* scoreSound = NULL;
Mix_Chunk* loseSound = NULL;
bool losePlayedOnce = false;

bool spaceDown = false;

bool loadAssets(SDL_Renderer* renderer);
void freeAssets();
void start(SDL_Renderer* renderer);
void play(SDL_Window* window, SDL_Renderer* renderer, double dt);
void over(SDL_Renderer* renderer);
void resetPlayerPos();
void draw_background(SDL_Rect* rect, SDL_Renderer* renderer);
void draw_bird(SDL_Rect* rectBird, SDL_Rect* rectAnim, SDL_Renderer* renderer);
void draw_ground(SDL_Rect* rect, SDL_Renderer* renderer, double dt);
void draw_pipe(SDL_Rect* rect, SDL_Rect* birdRect, SDL_Renderer* renderer);
void generate_pipes();

GAME_STATE state;

int main(int argc, char** argv)
{
    //init SDL video and audio
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0) {
        fprintf(stderr, "Could not initialize sdl2: %s\n", SDL_GetError());
    #ifdef WIN32
        MessageBox(NULL, "Could not initialize SDL2", "Error", MB_ICONERROR);
    #endif
        return EXIT_FAILURE;
    }

    //init SDL_mixer
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048);
    Mix_Volume(-1, 5);

    SDL_Window* window = SDL_CreateWindow(title, SDL_WINDOWPOS_UNDEFINED,
                                SDL_WINDOWPOS_UNDEFINED, width, height, SDL_WINDOW_OPENGL);
    if (window == NULL) {
        fprintf(stderr, "SDL Window Creation failed: %s\n", SDL_GetError());
    #ifdef WIN32
        MessageBox(NULL, "Could not create SDL2 Window", "Error", MB_ICONERROR);
    #endif
        return EXIT_FAILURE;
    }
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED
                                                | SDL_RENDERER_PRESENTVSYNC);
    if (renderer == NULL) {
        fprintf(stderr, "SDL Renderer creation failed: %s\n", SDL_GetError());
    #ifdef WIN32
        MessageBox(NULL, "Could not create SDL2 Renderer", "Error", MB_ICONERROR);
    #endif
        return EXIT_FAILURE;
    }

    if (IMG_Init(IMG_INIT_WEBP) != IMG_INIT_WEBP) {
        fprintf(stderr, "Image initialization failed : %s\n", SDL_GetError());
    #ifdef WIN32
        MessageBox(NULL, "Could not intialize SDL2 Image", "Error", MB_ICONERROR);
    #endif
        return EXIT_FAILURE;
    }

    if (TTF_Init() == -1) {
        fprintf(stderr, "TrueTypeFont init failed: %s\n", SDL_GetError());
    #ifdef WIN32
        MessageBox(NULL, "TrueTypeFont init failed.", "Error", MB_ICONERROR);
    #endif
        return EXIT_FAILURE;
    }

    SDL_Surface* icon = IMG_Load("res/birb0.webp");
    if (icon == NULL) {
        fprintf(stderr, "SDL Icon failed to load: %s\n", SDL_GetError());
    #ifdef WIN32
        MessageBox(NULL, "Could not create window icon", "Error", MB_ICONERROR);
    #endif
        return EXIT_FAILURE;
    }
    SDL_SetWindowIcon(window, icon);
    SDL_FreeSurface(icon);


    TTF_Font* font = TTF_OpenFont("res/FlappyBirdy.ttf", 22);
    if (font == NULL) {
        fprintf(stderr, "Font failed to load: %s\n", SDL_GetError());
    #ifdef WIN32
        MessageBox(NULL, "Could not create window icon", "Error", MB_ICONERROR);
    #endif
        return EXIT_FAILURE;
    }
    SDL_Color textColor = { 0, 0, 0, 255 };
    SDL_Color textColor2 = { 255, 255, 255, 255 };
    textSurface = TTF_RenderText_Blended_Wrapped(font, "press space to start", textColor, 60);
    textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
    SDL_FreeSurface(textSurface);
    SDL_QueryTexture(textTexture, NULL, NULL, &textRect.w, &textRect.h);
    textRect.w *= 4;
    textRect.h *= 4;
    textRect.x = 200 - (textRect.w / 2);
    textRect.y = 125 - (textRect.h /2);
    //printf("Text 1, Width: %d, Height: %d\n", textRect.w, textRect.h);
    textSurface = TTF_RenderText_Blended_Wrapped(font, "Press space to restart", textColor2, 60);
    textOverTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
    SDL_FreeSurface(textSurface);
    SDL_QueryTexture(textOverTexture, NULL, NULL, &textOverRect.w, &textOverRect.h);
    textOverRect.w *= 4;
    textOverRect.h *= 4;
    textOverRect.x = width / 2 - (textOverRect.w / 2);
    textOverRect.y = height / 2 - (textOverRect.h / 2);
    //printf("Text 2, Width: %d, Height: %d\n", textRect.w, textRect.h);

    if (!loadAssets(renderer)) {
        fprintf(stderr, "COuld not load crucial game assets");
    #ifdef WIN32
        MessageBox(NULL, "Could not load crucial game assets. Please reinstall the application.", "Error", MB_ICONERROR);
    #endif
        return EXIT_FAILURE;
    }

    bird.x = 100;
    bird.y = 250;
    bird.w = birdSize;
    bird.h = birdSize;
    //important piece of initialization here
    state = STATE_START;
    generate_pipes();

    Uint64 NOW = SDL_GetPerformanceCounter();
    Uint64 LAST = 0;
    double deltaTime = 0;
    bool running = true;
    SDL_Event event;
    while (running)
    {
        LAST = NOW;
        NOW = SDL_GetPerformanceCounter();
        deltaTime = (double)((NOW - LAST) * 1000 / (double)SDL_GetPerformanceFrequency());

        while(SDL_PollEvent(&event))
        {
            switch (event.type)
            {
            case SDL_QUIT:
                running = false;
                break;
            case SDL_KEYDOWN:
                if (event.key.repeat == 0) {
                    if (event.key.keysym.sym == SDLK_SPACE) {
                        spaceDown = true;
                    } else if (event.key.keysym.sym == SDLK_ESCAPE) {
                        running = false;
                    }
                }
                else {
                    spaceDown = false;
                }
                break;
            }
        }
        //fps++;
        switch (state)
        {
        case STATE_START:
            start(renderer);
            break;
        case STATE_PLAY:
            play(window, renderer, deltaTime);
            break;
        case STATE_OVER:
            over(renderer);
            break;
        }
        SDL_RenderPresent(renderer);
    }
    SDL_DestroyTexture(textTexture);
    SDL_DestroyTexture(textOverTexture);
    TTF_CloseFont(font);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    freeAssets();
    SDL_Quit();
    return 0;
}

bool loadAssets(SDL_Renderer* renderer)
{
    pipeTexture = IMG_LoadTexture(renderer, "res/pipe.webp");
    if (pipeTexture == NULL) {
        fprintf(stderr, "Image could not be found: %s\n", SDL_GetError());
        return false;
    }
    birdTexture = IMG_LoadTexture(renderer, "res/birbTile.webp");
    if (birdTexture == NULL) {
        fprintf(stderr, "Image could not be found: %s\n", SDL_GetError());
        return false;
    }
    SDL_QueryTexture(birdTexture, NULL, NULL, &birdAnim.w, &birdAnim.h);

    bgTexture = IMG_LoadTexture(renderer, "res/bgTex.webp");
    if (bgTexture == NULL) {
        fprintf(stderr, "Image could not be found: %s\n", SDL_GetError());
        return false;
    }
    groundTexture = IMG_LoadTexture(renderer, "res/ground.webp");
    if (groundTexture == NULL) {
        fprintf(stderr, "Image could not be found: %s\n", SDL_GetError());
        return false;
    }
    flapSound = Mix_LoadWAV("res/flap.WAV");
    if (flapSound == NULL) return false;
    scoreSound = Mix_LoadWAV("res/score.WAV");
    if (scoreSound == NULL) return false;
    loseSound = Mix_LoadWAV("res/lose.WAV");
    if (loseSound == NULL) return false;
    return true;
}

void freeAssets()
{
    SDL_DestroyTexture(pipeTexture);
    SDL_DestroyTexture(birdTexture);
    SDL_DestroyTexture(bgTexture);
    SDL_DestroyTexture(groundTexture);
    IMG_Quit();
    TTF_Quit();
    Mix_FreeChunk(flapSound);
    Mix_FreeChunk(scoreSound);
    Mix_FreeChunk(loseSound);
    Mix_CloseAudio();
    Mix_Quit();
}

void start(SDL_Renderer* renderer)
{
    if (spaceDown) {
        if (birdHasCollided) {
            losePlayedOnce = false;
            state = STATE_PLAY;
            birdHasCollided = false;
            score = 0;
        } else {
            state++;
        }
        spaceDown = false;
    }

    SDL_SetRenderDrawColor(renderer, 0, 255/2, 255, 255);
    SDL_RenderClear(renderer);
    draw_background(&bgRect, renderer);
    draw_bird(&bird, &birdAnim, renderer);
    draw_ground(&groundRect, renderer, 0);
    SDL_RenderCopy(renderer, textTexture, NULL, &textRect);
    for (uint i = 0; i < NUM_OF_PIPES; i++)
    {
        draw_pipe(&pipes[i], &bird, renderer);
    }
    if (bird.x > pipes[NUM_OF_PIPES - 2].x)
    {
        generate_pipes();
    }
}

void play(SDL_Window* window, SDL_Renderer* renderer, double dt)
{
    if (spaceDown) {
        if (!birdHasCollided) {
            Mix_PlayChannel(-1, flapSound, 0);
            bird.y -= 50;
        }
        spaceDown = false;
    }
    SDL_SetRenderDrawColor(renderer, 0, 255/2, 255, 255);
    SDL_RenderClear(renderer);
    draw_background(&bgRect, renderer);
    bird.y += 1 * (int)dt / 6;
    draw_bird(&bird, &birdAnim, renderer);
    draw_ground(&groundRect, renderer, dt);
    for (uint i = 0; i < NUM_OF_PIPES; i++)
    {
        pipes[i].x -= 1 * (int)dt / 6;
        draw_pipe(&pipes[i], &bird, renderer);
    }
    if (bird.x > pipes[NUM_OF_PIPES - 2].x)
    {
        generate_pipes();
    }
    char scoreChar[3];
    snprintf(scoreChar, sizeof(scoreChar), "%d", score);
    strcpy(title, "Flappy Bird, Score: ");
    strcat(title, scoreChar);
    SDL_SetWindowTitle(window, title);
}

void over(SDL_Renderer* renderer)
{
    if (spaceDown) {
        resetPlayerPos();
        generate_pipes();
        state = STATE_START;
        spaceDown = false;
    }
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, textOverTexture, NULL, &textOverRect);
    if (!losePlayedOnce) {
        Mix_PlayChannel(-1, loseSound, 0);
        losePlayedOnce = true;
    }

}

void draw_background(SDL_Rect* rect, SDL_Renderer* renderer)
{
    rect->x = 0;
    rect->y = 0;
    rect->w = width;
    rect->h = height;
    SDL_RenderCopy(renderer, bgTexture, NULL, rect);
}

void draw_bird(SDL_Rect* rectBird, SDL_Rect* rectAnim, SDL_Renderer* renderer)
{
    if (rectBird->y >= height - (birdSize / 2)) {
        rectBird->y = height - (birdSize / 2);
    }
//    animate(rectAnim, 100, 3);
    /*if (fps >= 30) {
        curFrame++;
        fps = 0;
    }*/
    curFrame = (SDL_GetTicks() / speed) & NUM_OF_FRAMES + 1;
    switch(curFrame)
    {
    case 0:
        rectAnim->x = 0;
        rectAnim->y = 0;
        rectAnim->w = 65;
        rectAnim->h = 68;
        break;
    case 1:
        rectAnim->x = 65;
        rectAnim->y = 0;
        rectAnim->w = 65;
        rectAnim->h = 68;
        break;
    case 2:
        rectAnim->x = 65 + 65;
        rectAnim->y = 0;
        rectAnim->w = 65;
        rectAnim->h = 68;
        break;
    case 3:
        //curFrame = 0;
        break;
    }
    SDL_RenderCopy(renderer, birdTexture, rectAnim, rectBird);
}

void draw_ground(SDL_Rect* rect, SDL_Renderer* renderer, double dt)
{
    const int minX = width - (width * 2);
    if (rect->x <= minX) {
        rect->x = 0;
    }
    else {
        rect->x -= 1 * (int)dt / 6;
    }
    rect->y = 0;
    rect->w = width * 2;
    rect->h = height;
    SDL_RenderCopy(renderer, groundTexture, NULL, rect);
}

void draw_pipe(SDL_Rect* rect, SDL_Rect* birdRect, SDL_Renderer* renderer)
{
    SDL_Rect topRect;
    SDL_Rect middleRect;
    SDL_Rect bottomRect;

    topRect.x = rect->x;
    topRect.y = rect->y - pipeHeight - lengthInBetween;
    topRect.w = pipeWidth;
    topRect.h = pipeHeight;

    bottomRect.x = rect->x;
    bottomRect.y = rect->y + lengthInBetween;
    bottomRect.w = pipeWidth;
    bottomRect.h = pipeHeight;

    middleRect.x = rect->x + (pipeWidth/2);
    middleRect.y = bottomRect.y - (lengthInBetween * 2);
    middleRect.w = 3;
    middleRect.h = lengthInBetween * 2;

    if (isColliding(birdRect, &topRect) || isColliding(birdRect, &bottomRect)) {
        state = STATE_OVER;
        birdHasCollided = true;
    }
    if (isColliding(birdRect, &middleRect) && !hasScored && birdRect->x > middleRect.x) {
        //i dunno why this works but it does
        Mix_PlayChannel(-1, scoreSound, 0);
        score += 1;
        hasScored = true;
    } else {
        hasScored = false;
    }
    const SDL_RendererFlip flip = SDL_FLIP_VERTICAL;
    SDL_RenderCopyEx(renderer, pipeTexture, NULL, &topRect, 0, NULL, flip);
    SDL_RenderCopy(renderer, pipeTexture, NULL, &bottomRect);
}

void resetPlayerPos()
{
    bird.x = 100;
    bird.y = 250;
    bird.w = birdSize;
    bird.h = birdSize;
}

void generate_pipes()
{
    for (uint i = 0; i < NUM_OF_PIPES; i++)
    {
        if (i > 0) {
            pipes[i].x = pipes[i-1].x + 350;
        }
        else {
            pipes[0].x = 350;
        }
        pipes[i].y = random_num(height / 2 - lengthInBetween, height / 2 + lengthInBetween) + 200;
        //printf("Pipe at %d, Y: %d\n", i + 1, pipes[i].y);
    }
}
