/* =============================================================================
** FlappyBirby, file: main.c Created 12/10/2021
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_ttf.h>

#include "u_utility.h"

#define ASSETS_DIR "assets/"
#define NUM_PIPES 16
#define NUM_FRAMES 2
// 800 / 2 + 100 = 525
// 800 / 2 - 100 = 300

static void G_AppendAssetPath(char *path);
static boolean G_LoadAssets(SDL_Renderer *renderer);
static boolean G_SetupFont(SDL_Renderer *renderer);
static void G_FreeAssets(void);
static void P_Start(SDL_Renderer *renderer);
static void P_Play(SDL_Window *window, SDL_Renderer *renderer, double dt);
static void P_Over(SDL_Renderer *renderer);
static void P_ResetPlayerPos(void);
static void R_DrawBackground(SDL_Rect *rect, SDL_Renderer *renderer);
static void R_DrawBird(SDL_Rect *rectbird, SDL_Rect *rectanim,
					   SDL_Renderer *renderer);
static void R_DrawGround(SDL_Rect *rect, SDL_Renderer *renderer, double dt);
static void R_DrawPipe(SDL_Rect *rect, SDL_Rect *rectbird,
					   SDL_Renderer *renderer);
static void P_GeneratePipes(void);
static void P_UpdateScore(SDL_Window *window);

static char title[75] = "Flappy Birby, Score: ";
static const uint width = 800;
static const uint height = 600;
static const uint pipewidth = 150;
static const int lengthinbetween = 100;
static const uint pipeheight = 400;
static SDL_Rect pipes[NUM_PIPES];
static SDL_Texture *pipetexture = NULL;
static const uint birdsize = 70;
static boolean birdhascollided = false;
static int curframe = 0;
static int speed = 250;
static boolean hasscored = false;
static SDL_Rect bird;
static SDL_Rect birdanim;
static SDL_Texture *birdtexture = NULL;
static SDL_Texture *bgtexture = NULL;
static SDL_Rect bgrect;
static SDL_Texture *groundtexture = NULL;
static SDL_Rect groundrect;
static int score = 0;
static TTF_Font *font = NULL;
static SDL_Surface *textsurface = NULL;
static SDL_Texture *texttexture = NULL;
static SDL_Texture *textovertexture = NULL;
static SDL_Rect textrect;
static SDL_Rect textoverrect;
static Mix_Chunk *flapsound = NULL;
static Mix_Chunk *scoresound = NULL;
static Mix_Chunk *losesound = NULL;
static boolean loseplayedonce = false;
static boolean spacedown = false;
GameState state;

int main(int argc, char *argv[]) {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0) {
        LOG_ERROR("Could not initialize SDL: %s\n", SDL_GetError());
        return EXIT_FAILURE;
    }
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048); // SDL_mixer
    Mix_Volume(-1, 5);

    SDL_Window *window = SDL_CreateWindow(title, SDL_WINDOWPOS_UNDEFINED,
                                          SDL_WINDOWPOS_UNDEFINED, width,
                                          height, SDL_WINDOW_SHOWN);
    if (window == NULL) {
        LOG_ERROR("SDL Window Creation failed: %s\n", SDL_GetError());
        return EXIT_FAILURE;
    }
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1,
                                                SDL_RENDERER_ACCELERATED |
                                                SDL_RENDERER_PRESENTVSYNC);
    if (renderer == NULL) {
        LOG_ERROR("SDL Renderer creation failed: %s\n", SDL_GetError());
        return EXIT_FAILURE;
    }

    if (IMG_Init(IMG_INIT_WEBP) != IMG_INIT_WEBP) {
        LOG_ERROR("Image initialization failed : %s\n", SDL_GetError());
        return EXIT_FAILURE;
    }

    if (TTF_Init() == -1) {
        LOG_ERROR("TrueTypeFont init failed: %s\n", SDL_GetError());
        return EXIT_FAILURE;
    }

    if (!G_LoadAssets(renderer)) {
        LOG_ERROR("Could not load crucial game assets.");
        return EXIT_FAILURE;
    }

    SDL_Surface* icon = IMG_Load(ASSETS_DIR"birb0.webp");
    if (icon == NULL) {
        LOG_ERROR("SDL Icon failed to load: %s\n", SDL_GetError());
        return EXIT_FAILURE;
    }
    SDL_SetWindowIcon(window, icon); SDL_FreeSurface(icon);

    bird.x = 100;
    bird.y = 250;
    bird.w = birdsize;
    bird.h = birdsize;

    state = STATE_START; // important piece of initialization here
    P_GeneratePipes();

    Uint64 start = SDL_GetPerformanceCounter(), end = 0;
    double deltatime = 0;
    boolean running = true;
    while (running) {
        end = start;
        start = SDL_GetPerformanceCounter();
        deltatime = (double)((start - end) * 1000 /
					 (double)SDL_GetPerformanceFrequency());

		SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
            case SDL_QUIT:
                running = false;
                break;
            case SDL_KEYDOWN: {
                if (event.key.repeat == 0) {
                    if (event.key.keysym.sym == SDLK_SPACE) {
                        spacedown = true;
                    } else if (event.key.keysym.sym == SDLK_ESCAPE) {
                        running = false;
                    }
                } else {
                    spacedown = false;
                }
                break;
			}
            }
        }
        //fps++;
        switch (state) {
        case STATE_START:
            P_Start(renderer);
            break;
        case STATE_PLAY:
            P_Play(window, renderer, deltatime);
            break;
        case STATE_OVER:
            P_Over(renderer);
            break;
        }
        SDL_RenderPresent(renderer);
    }
    SDL_DestroyTexture(texttexture);
    SDL_DestroyTexture(textovertexture);
    TTF_CloseFont(font);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    G_FreeAssets();
    SDL_Quit();
    return 0;
}

static void G_AppendAssetPath(char *path) {
	char cpystr[256];
	strcpy(cpystr, path);
	strcat(path, ASSETS_DIR);
	strcat(path, cpystr);
}

static boolean G_LoadAssets(SDL_Renderer *renderer) {
    pipetexture = IMG_LoadTexture(renderer, ASSETS_DIR"pipe.webp");
    if (pipetexture == NULL) {
        LOG_ERROR("Image could not be found: %s\n", SDL_GetError());
        return false;
    }
    birdtexture = IMG_LoadTexture(renderer, ASSETS_DIR"birbTile.webp");
    if (birdtexture == NULL) {
        LOG_ERROR("Image could not be found: %s\n", SDL_GetError());
        return false;
    }
    SDL_QueryTexture(birdtexture, NULL, NULL, &birdanim.w, &birdanim.h);

    bgtexture = IMG_LoadTexture(renderer, ASSETS_DIR"bgTex.webp");
    if (bgtexture == NULL) {
        LOG_ERROR("Image could not be found: %s\n", SDL_GetError());
        return false;
    }
    groundtexture = IMG_LoadTexture(renderer, ASSETS_DIR"ground.webp");
    if (groundtexture == NULL) {
        LOG_ERROR("Image could not be found: %s\n", SDL_GetError());
        return false;
    }
	font = TTF_OpenFont(ASSETS_DIR"FlappyBirdy.ttf", 22);
    if (font == NULL) {
        LOG_ERROR("Font failed to load: %s\n", SDL_GetError());
        return false;
    }
	if (!G_SetupFont(renderer)) {
		LOG_ERROR("G_SetupFont(): failed: %s!\n", TTF_GetError());
	}
    flapsound = Mix_LoadWAV(ASSETS_DIR"flap.wav");
    if (flapsound == NULL) return false;
    scoresound = Mix_LoadWAV(ASSETS_DIR"score.wav");
    if (scoresound == NULL) return false;
    losesound = Mix_LoadWAV(ASSETS_DIR"lose.wav");
    if (losesound == NULL) return false;
    return true;
}

static boolean G_SetupFont(SDL_Renderer *renderer) {
    const SDL_Color textcolor = { 0, 0, 0, 255 };
    const SDL_Color textcolor2 = { 255, 255, 255, 255 };
    textsurface = TTF_RenderText_Blended_Wrapped(font, "Press Space to Start",
												 textcolor, 60);
    texttexture = SDL_CreateTextureFromSurface(renderer, textsurface);
    SDL_FreeSurface(textsurface);
    SDL_QueryTexture(texttexture, NULL, NULL, &textrect.w, &textrect.h);
    textrect.w *= 4;
    textrect.h *= 4;
    textrect.x = 200 - (textrect.w / 2);
    textrect.y = 125 - (textrect.h /2);
    // printf("Text 1, Width: %d, Height: %d\n", textrect.w, textrect.h);
    textsurface = TTF_RenderText_Blended_Wrapped(font, "Press Space to Restart",
												 textcolor2, 60);
    textovertexture = SDL_CreateTextureFromSurface(renderer, textsurface);
    SDL_FreeSurface(textsurface);
    SDL_QueryTexture(textovertexture, NULL, NULL, &textoverrect.w,
					 &textoverrect.h);
    textoverrect.w *= 4;
    textoverrect.h *= 4;
    textoverrect.x = width / 2 - (textoverrect.w / 2);
    textoverrect.y = height / 2 - (textoverrect.h / 2);
    // printf("Text 2, Width: %d, Height: %d\n", textrect.w, textrect.h);
	return true;
}

static void G_FreeAssets(void) {
    SDL_DestroyTexture(pipetexture);
    SDL_DestroyTexture(birdtexture);
    SDL_DestroyTexture(bgtexture);
    SDL_DestroyTexture(groundtexture);
    IMG_Quit();
    TTF_Quit();
    Mix_FreeChunk(flapsound);
    Mix_FreeChunk(scoresound);
    Mix_FreeChunk(losesound);
    Mix_CloseAudio();
    Mix_Quit();
}

static void P_Start(SDL_Renderer *renderer) {
    if (spacedown) {
        if (birdhascollided) {
            loseplayedonce = false;
            state = STATE_PLAY;
            birdhascollided = false;
            score = 0;
        } else
            state++;
        spacedown = false;
    }

    SDL_SetRenderDrawColor(renderer, 0, 255/2, 255, 255);
    SDL_RenderClear(renderer);
    R_DrawBackground(&bgrect, renderer);
    R_DrawBird(&bird, &birdanim, renderer);
    R_DrawGround(&groundrect, renderer, 0);
    SDL_RenderCopy(renderer, texttexture, NULL, &textrect);
    for (uint i = 0; i < NUM_PIPES; i++) {
        R_DrawPipe(&pipes[i], &bird, renderer);
    }
    if (bird.x > pipes[NUM_PIPES - 2].x) {
        P_GeneratePipes();
    }
}

static void P_Play(SDL_Window *window, SDL_Renderer *renderer, double dt) {
    if (spacedown) {
        if (!birdhascollided) {
            Mix_PlayChannel(-1, flapsound, 0);
            bird.y -= 50;
        }
        spacedown = false;
    }
    SDL_SetRenderDrawColor(renderer, 0, 255/2, 255, 255);
    SDL_RenderClear(renderer);
    R_DrawBackground(&bgrect, renderer);
    bird.y += 1 * (int)dt / 6;
    R_DrawBird(&bird, &birdanim, renderer);
    R_DrawGround(&groundrect, renderer, dt);
    for (size_t i = 0; i < NUM_PIPES; i++) {
        pipes[i].x -= 1 * (int)dt / 6;
        R_DrawPipe(&pipes[i], &bird, renderer);
    }
    if (bird.x > pipes[NUM_PIPES - 2].x) {
        P_GeneratePipes();
    }
	P_UpdateScore(window);
}

static void P_Over(SDL_Renderer *renderer) {
    if (spacedown) {
        P_ResetPlayerPos();
        P_GeneratePipes();
        state = STATE_START;
        spacedown = false;
    }
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, textovertexture, NULL, &textoverrect);
    if (!loseplayedonce) {
        Mix_PlayChannel(-1, losesound, 0);
        loseplayedonce = true;
    }

}

static void R_DrawBackground(SDL_Rect *rect, SDL_Renderer *renderer) {
    rect->x = 0;
    rect->y = 0;
    rect->w = width;
    rect->h = height;
    SDL_RenderCopy(renderer, bgtexture, NULL, rect);
}

static void R_DrawBird(SDL_Rect *rectbird, SDL_Rect *rectanim,
					   SDL_Renderer *renderer) {
    if (rectbird->y >= height - (birdsize / 2)) {
        rectbird->y = height - (birdsize / 2);
    }
    // animate(rectanim, 100, 3);
    /* if (fps >= 30) {
        curframe++;
        fps = 0;
    } */
    curframe = (SDL_GetTicks() / speed) & NUM_FRAMES + 1;
    switch(curframe)
    {
    case 0:
        rectanim->x = 0;
        rectanim->y = 0;
        rectanim->w = 65;
        rectanim->h = 68;
        break;
    case 1:
        rectanim->x = 65;
        rectanim->y = 0;
        rectanim->w = 65;
        rectanim->h = 68;
        break;
    case 2:
        rectanim->x = 65 + 65;
        rectanim->y = 0;
        rectanim->w = 65;
        rectanim->h = 68;
        break;
    case 3:
        //curframe = 0;
        break;
    }
    SDL_RenderCopy(renderer, birdtexture, rectanim, rectbird);
}

static void R_DrawGround(SDL_Rect *rect, SDL_Renderer *renderer, double dt) {
    const int minx = width - (width * 2);
    if (rect->x <= minx) {
        rect->x = 0;
    }
    else {
        rect->x -= 1 * (int)dt / 6;
    }
    rect->y = 0;
    rect->w = width * 2;
    rect->h = height;
    SDL_RenderCopy(renderer, groundtexture, NULL, rect);
}

static void R_DrawPipe(SDL_Rect *rect, SDL_Rect *rectbird,
					   SDL_Renderer *renderer) {
    SDL_Rect toprect;
    SDL_Rect middlerect;
    SDL_Rect bottomrect;

    toprect.x = rect->x;
    toprect.y = rect->y - pipeheight - lengthinbetween;
    toprect.w = pipewidth;
    toprect.h = pipeheight;

    bottomrect.x = rect->x;
    bottomrect.y = rect->y + lengthinbetween;
    bottomrect.w = pipewidth;
    bottomrect.h = pipeheight;

    middlerect.x = rect->x + (pipewidth/2);
    middlerect.y = bottomrect.y - (lengthinbetween * 2);
    middlerect.w = 3;
    middlerect.h = lengthinbetween * 2;

    if (U_IsColliding(rectbird, &toprect) || U_IsColliding(rectbird,
														   &bottomrect)) {
        state = STATE_OVER;
        birdhascollided = true;
    }
    if (U_IsColliding(rectbird, &middlerect) && !hasscored &&
		rectbird->x > middlerect.x) {
        // I don't know why this works but it does
        Mix_PlayChannel(-1, scoresound, 0);
        score += 1;
        hasscored = true;
    } else {
        hasscored = false;
    }
    const SDL_RendererFlip flip = SDL_FLIP_VERTICAL;
    SDL_RenderCopyEx(renderer, pipetexture, NULL, &toprect, 0, NULL, flip);
    SDL_RenderCopy(renderer, pipetexture, NULL, &bottomrect);
}

static void P_ResetPlayerPos(void) {
    bird.x = 100;
    bird.y = 250;
    bird.w = birdsize;
    bird.h = birdsize;
}

static void P_GeneratePipes(void) {
    for (uint i = 0; i < NUM_PIPES; i++) {
        if (i > 0) {
            pipes[i].x = pipes[i-1].x + 350;
        } else {
            pipes[0].x = 350;
        }
        pipes[i].y = U_RandomNum(height / 2 - lengthinbetween,
								 height / 2 + lengthinbetween) + 200;
        //printf("Pipe at %d, Y: %d\n", i + 1, pipes[i].y);
    }
}

static void P_UpdateScore(SDL_Window *window) {
    static char scorestr[32];
    snprintf(scorestr, sizeof(scorestr), "%d", score);
    strcpy(title, "Flappy Birby, Score: ");
    strcat(title, scorestr);
    SDL_SetWindowTitle(window, title);
}

