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

/* typedef struct Bird {
    SDL_Rect pos;
    SDL_Rect anim;
    SDL_Texture *texture;
    float angle;
    const int size = 70;
} bird; */

static void G_AppendAssetPath(char *path);
static boolean G_LoadAssets(SDL_Renderer *renderer);
static boolean G_SetupFont(SDL_Renderer *renderer);
static void G_FreeAssets(void);
static void P_Start(SDL_Renderer *renderer);
static void P_Play(SDL_Window *window, SDL_Renderer *renderer, double dt);
static void P_Over(SDL_Renderer *renderer);
static void P_ResetBirdPos(void);
static void R_DrawBackground(SDL_Rect *rect, SDL_Renderer *renderer);
static void R_DrawBird(SDL_Rect *rectBird, SDL_Rect *rectAnim,
					   SDL_Renderer *renderer);
static void R_DrawGround(SDL_Rect *rect, SDL_Renderer *renderer, double dt);
static void R_DrawPipe(SDL_Rect *rect, SDL_Rect *rectBird,
					   SDL_Renderer *renderer);
static void P_GeneratePipes(void);
static void P_UpdateScore(SDL_Window *window);

static char windowTitle[75] = "Flappy Birby, Score: ";
static const uint windowWidth = 800;
static const uint windowHeight = 600;
static const uint pipeWidth = 150;
static const int lengthInBetweenPipes = 100;
static const uint pipeHeight = 400;
static SDL_Rect pipes[NUM_PIPES];
static SDL_Texture *pipeTexture = NULL;
static const int birdSize = 70;
static boolean birdHasCollided = false;
static int currFrame = 0;
static int speed = 250;
static boolean hasScored = false;
static SDL_Rect bird;
static SDL_Rect birdAnim;
static float birdAngle;
static SDL_Texture *birdTexture = NULL;
static SDL_Texture *bgTexture = NULL;
static SDL_Rect bgRect;
static SDL_Texture *groundTexture = NULL;
static SDL_Rect groundRect;
static int score = 0;
static TTF_Font *font = NULL;
static SDL_Surface *textSurface = NULL;
static SDL_Texture *textTexture = NULL;
static SDL_Texture *textOverTexture = NULL;
static SDL_Rect textRect;
static SDL_Rect textOverRect;
static Mix_Chunk *flapSound = NULL;
static Mix_Chunk *scoreSound = NULL;
static Mix_Chunk *loseSound = NULL;
static boolean losePlayedOnce = false;
static boolean spaceDown = false;
static GameState state;

int main(int argc, char *argv[]) {
	SDL_Window *window;
	SDL_Renderer *renderer;
	SDL_Surface *windowIcon;
    Uint64 start, end;
    double deltatime;
    boolean running;
	/* BEGIN INITIALIZATION */
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0) {
        LOG_ERROR("Could not initialize SDL: %s\n", SDL_GetError());
        return EXIT_FAILURE;
    }
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048);
    Mix_Volume(-1, 5);

    window = SDL_CreateWindow(windowTitle, SDL_WINDOWPOS_UNDEFINED,
							  SDL_WINDOWPOS_UNDEFINED, windowWidth,
							  windowHeight, SDL_WINDOW_SHOWN);
    if (window == NULL) {
        LOG_ERROR("SDL Window Creation failed: %s\n", SDL_GetError());
        return EXIT_FAILURE;
    }
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED |
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

    windowIcon = IMG_Load(ASSETS_DIR"birb0.webp");
    if (windowIcon == NULL) {
        LOG_ERROR("SDL Icon failed to load: %s\n", SDL_GetError());
        return EXIT_FAILURE;
    }
    SDL_SetWindowIcon(window, windowIcon); SDL_FreeSurface(windowIcon);

	P_ResetBirdPos(); /* set the bird to it's initial position */
    state = STATE_START; /* important piece of initialization here */
    P_GeneratePipes(); /* initiallly generate pipe positions */
	/* END OF INITIALIZATION */

    start = SDL_GetPerformanceCounter(); end = 0;
    deltatime = 0;
    running = true;
    while (running) {
		SDL_Event event;

        end = start;
        start = SDL_GetPerformanceCounter();
        deltatime = (double)((start - end) * 1000 /
					 (double)SDL_GetPerformanceFrequency());
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
            case SDL_QUIT:
                running = false;
                break;
            case SDL_KEYDOWN: {
                if (event.key.repeat == 0) {
                    if (event.key.keysym.sym == SDLK_SPACE) {
                        spaceDown = true;
                    }
					else if (event.key.keysym.sym == SDLK_ESCAPE) {
                        running = false;
                    }
                }
				else {
                    spaceDown = false;
                }
                break;
			}
            }
        }
        /* fps++; */
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
    SDL_DestroyTexture(textTexture);
    SDL_DestroyTexture(textOverTexture);
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
    pipeTexture = IMG_LoadTexture(renderer, ASSETS_DIR"pipe.webp");
    if (pipeTexture == NULL) {
        LOG_ERROR("Image could not be found: %s\n", SDL_GetError());
        return false;
    }
    birdTexture = IMG_LoadTexture(renderer, ASSETS_DIR"birbTile.webp");
    if (birdTexture == NULL) {
        LOG_ERROR("Image could not be found: %s\n", SDL_GetError());
        return false;
    }
    SDL_QueryTexture(birdTexture, NULL, NULL, &birdAnim.w, &birdAnim.h);

    bgTexture = IMG_LoadTexture(renderer, ASSETS_DIR"bgTex.webp");
    if (bgTexture == NULL) {
        LOG_ERROR("Image could not be found: %s\n", SDL_GetError());
        return false;
    }
    groundTexture = IMG_LoadTexture(renderer, ASSETS_DIR"ground.webp");
    if (groundTexture == NULL) {
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
    flapSound = Mix_LoadWAV(ASSETS_DIR"flap.wav");
    if (flapSound == NULL) return false;
    scoreSound = Mix_LoadWAV(ASSETS_DIR"score.wav");
    if (scoreSound == NULL) return false;
    loseSound = Mix_LoadWAV(ASSETS_DIR"lose.wav");
    if (loseSound == NULL) return false;
    return true;
}

static boolean G_SetupFont(SDL_Renderer *renderer) {
    const SDL_Color textColor = { 0, 0, 0, 255 };
    const SDL_Color textColor2 = { 255, 255, 255, 255 };
    textSurface = TTF_RenderText_Blended_Wrapped(font, "Press Space to Start",
												 textColor, 60);
    textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
    SDL_FreeSurface(textSurface);
    SDL_QueryTexture(textTexture, NULL, NULL, &textRect.w, &textRect.h);
    textRect.w *= 4;
    textRect.h *= 4;
    textRect.x = 200 - (textRect.w / 2);
    textRect.y = 125 - (textRect.h /2);
    /* printf("Text 1, Width: %d, Height: %d\n", textRect.w, textRect.h); */
    textSurface = TTF_RenderText_Blended_Wrapped(font, "Press Space to Restart",
												 textColor2, 60);
    textOverTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
    SDL_FreeSurface(textSurface);
    SDL_QueryTexture(textOverTexture, NULL, NULL, &textOverRect.w,
					 &textOverRect.h);
    textOverRect.w *= 4;
    textOverRect.h *= 4;
    textOverRect.x = windowWidth / 2 - (textOverRect.w / 2);
    textOverRect.y = windowHeight / 2 - (textOverRect.h / 2);
    /* printf("Text 2, Width: %d, Height: %d\n", textRect.w, textRect.h); */
	return true;
}

static void G_FreeAssets(void) {
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

static void P_Start(SDL_Renderer *renderer) {
	size_t index;

    if (spaceDown) {
        if (birdHasCollided) {
            losePlayedOnce = false;
            state = STATE_PLAY;
            birdHasCollided = false;
            score = 0;
        }
		else
            state++;
        spaceDown = false;
    }

    SDL_SetRenderDrawColor(renderer, 0, 255/2, 255, 255);
    SDL_RenderClear(renderer);
    R_DrawBackground(&bgRect, renderer);
    R_DrawBird(&bird, &birdAnim, renderer);
    R_DrawGround(&groundRect, renderer, 0);
    SDL_RenderCopy(renderer, textTexture, NULL, &textRect);
    for (index = 0; index < NUM_PIPES; index++)
        R_DrawPipe(&pipes[index], &bird, renderer);

    if (bird.x > pipes[NUM_PIPES - 2].x)
        P_GeneratePipes();
}

static void P_Play(SDL_Window *window, SDL_Renderer *renderer, double dt) {
	size_t index;
    static const int maxBirdMoveUp = 50;
    static int birdStartingY = 0;
    static int birdMove = 0;
    static int lastBirdMove = 0;
    static boolean birdInMotion = false;
    static const float maxAngle = -45.0f;
    static float lastAngle = 0.0f;
    if (spaceDown) {
        if (!birdHasCollided) {
            Mix_PlayChannel(-1, flapSound, 0);
            /* if (bird.y < maxBirdMoveUp) {
                birdMove += lastBirdMove * 2 + 1;
                bird.y -= birdMove;
                lastBirdMove = birdMove;
            } else {
                birdMove = 0;
                lastBirdMove = 0;
            } */
            birdInMotion = true;
            birdStartingY = bird.y;
            printf("birdStartingY=%d|%d\n", birdStartingY, birdStartingY - maxBirdMoveUp);
            // bird.y -= 50;
        }
        spaceDown = false;
    }

    if (birdInMotion) {
        if (bird.y > birdStartingY - maxBirdMoveUp) {
            birdMove += lastBirdMove * 2 + 1;
            bird.y -= birdMove;
            lastBirdMove = birdMove;
        }
        else {
            birdMove = 0;
            lastBirdMove = 0;
            birdStartingY = bird.y;
            birdInMotion = false;
        }

        if (birdAngle > maxAngle) {
            birdAngle -= lastAngle * 3.0f + 10.0f;
            lastAngle = birdAngle;
            printf("birdAngle=%.2f|lastAngle=%.2f\n", birdAngle, lastAngle);
        }
        else {
            lastAngle = 0.0f;
            birdAngle = 0.0f;
        }
    } else {
        birdAngle = 0.0f;
        lastAngle = 0.0f;
    }

    SDL_SetRenderDrawColor(renderer, 0, 255/2, 255, 255);
    SDL_RenderClear(renderer);
    R_DrawBackground(&bgRect, renderer);

    bird.y += 1 * (int)dt / 6;
    R_DrawBird(&bird, &birdAnim, renderer);
    R_DrawGround(&groundRect, renderer, dt);
    for (index = 0; index < NUM_PIPES; index++) {
        pipes[index].x -= 1 * (int)dt / 6;
        R_DrawPipe(&pipes[index], &bird, renderer);
    }

    if (bird.x > pipes[NUM_PIPES - 2].x) {
		P_GeneratePipes();
	}

	P_UpdateScore(window);
}

static void P_Over(SDL_Renderer *renderer) {
    if (spaceDown) {
        P_ResetBirdPos();
        P_GeneratePipes();
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

static void R_DrawBackground(SDL_Rect *rect, SDL_Renderer *renderer) {
    rect->x = 0;
    rect->y = 0;
    rect->w = windowWidth;
    rect->h = windowHeight;
    SDL_RenderCopy(renderer, bgTexture, NULL, rect);
}

static void R_DrawBird(SDL_Rect *rectBird, SDL_Rect *rectAnim,
					   SDL_Renderer *renderer) {
    if (rectBird->y >= ((int)windowHeight) - (birdSize / 2)) {
        rectBird->y = ((int)windowHeight) - (birdSize / 2);
    }
    /* animate(rectAnim, 100, 3); */
    /* if (fps >= 30) {
        currFrame++;
        fps = 0;
    } */
    currFrame = (SDL_GetTicks() / speed) & (NUM_FRAMES + 1);
    switch(currFrame)
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
        /* currFrame = 0; */
        break;
    }
    // SDL_RenderCopy(renderer, birdTexture, rectAnim, rectBird);
    const SDL_Point center = {
        .x = rectAnim->w / 2,
        .y = rectAnim->h / 2
    };
    SDL_RenderCopyEx(renderer, birdTexture, rectAnim, rectBird, birdAngle, &center, SDL_FLIP_NONE);
}

static void R_DrawGround(SDL_Rect *rect, SDL_Renderer *renderer, double dt) {
    const int minX = windowWidth - (windowWidth * 2);
    if (rect->x <= minX) {
        rect->x = 0;
    }
    else {
        rect->x -= 1 * (int)dt / 6;
    }
    rect->y = 0;
    rect->w = windowWidth * 2;
    rect->h = windowHeight;
    SDL_RenderCopy(renderer, groundTexture, NULL, rect);
}

static void R_DrawPipe(SDL_Rect *rect, SDL_Rect *rectBird,
					   SDL_Renderer *renderer) {
    SDL_Rect topRect;
    SDL_Rect middleRect;
    SDL_Rect bottomRect;
	static const SDL_RendererFlip renderFlip = SDL_FLIP_VERTICAL;

	topRect.x = rect->x;
    topRect.y = rect->y - pipeHeight - lengthInBetweenPipes;
    topRect.w = pipeWidth;
    topRect.h = pipeHeight;

    bottomRect.x = rect->x;
    bottomRect.y = rect->y + lengthInBetweenPipes;
    bottomRect.w = pipeWidth;
    bottomRect.h = pipeHeight;

    middleRect.x = rect->x + (pipeWidth/2);
    middleRect.y = bottomRect.y - (lengthInBetweenPipes * 2);
    middleRect.w = 3;
    middleRect.h = lengthInBetweenPipes * 2;

    if (U_IsColliding(rectBird, &topRect) || U_IsColliding(rectBird,
														   &bottomRect)) {
        state = STATE_OVER;
        birdHasCollided = true;
    }
    if (U_IsColliding(rectBird, &middleRect) && !hasScored &&
		rectBird->x > middleRect.x) {
        /* I don't know why this works but it does */
        Mix_PlayChannel(-1, scoreSound, 0);
        score += 1;
        hasScored = true;
    }
	else {
        hasScored = false;
    }

    SDL_RenderCopyEx(renderer, pipeTexture, NULL, &topRect, 0, NULL, renderFlip);
    SDL_RenderCopy(renderer, pipeTexture, NULL, &bottomRect);
}

__inline static void P_ResetBirdPos(void) {
    bird.x = 100;
    bird.y = 250;
    bird.w = birdSize;
    bird.h = birdSize;
}

static void P_GeneratePipes(void) {
	size_t index;
    for (index = 0; index < NUM_PIPES; index++) {
        if (index > 0)
            pipes[index].x = pipes[index - 1].x + 350;
        else
            pipes[0].x = 350;

        pipes[index].y = U_RandomNum(windowHeight / 2 - lengthInBetweenPipes,
								 windowHeight / 2 + lengthInBetweenPipes) + 200;
        /* printf("Pipe at %d, Y: %d\n", i + 1, pipes[i].y); */
    }
}

static void P_UpdateScore(SDL_Window *window) {
    static char scoreBuf[32];
    snprintf(scoreBuf, sizeof(scoreBuf), "%d", score);
    strcpy(windowTitle, "Flappy Birby, Score: ");
    strcat(windowTitle, scoreBuf);
    SDL_SetWindowTitle(window, windowTitle);
}

