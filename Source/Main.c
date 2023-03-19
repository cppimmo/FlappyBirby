/*=============================================================================*
 * Main.c - Entry point and game code.
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_ttf.h>

#include "Utility.h"

#define kASSETS_DIR "Assets/"

#define kNUM_FRAMES 2
// 800 / 2 + 100 = 525
// 800 / 2 - 100 = 300

typedef enum
{
    GAMESTATE_START = 0,
    GAMESTATE_PLAY,
    GAMESTATE_OVER,
} GameState;

// TODO: Ensure the members of each struct are initialized.

typedef struct
{
	SDL_Window *pWindow;
	SDL_Surface *pWindowIcon;
	SDL_Renderer *pRenderer;
	char windowTitle[75];
	GameState state;
	boolean isRunning;
	int currFrame;
	int currScore;
	int highScore; // Highest score from previous game
} Game;

#define kBIRD_SIZE 70

typedef struct
{
	SDL_Texture *pTexture;
	SDL_Rect rect; // Bird rect
	SDL_Rect animRect; // Animation rect
	int moveSpeed; // Lateral movement speed
	float angle; // Degrees
	boolean hasScored;
	boolean hasCollided;
} FlappyBirb;

//static boolean birdHasCollided = false;
//static int currFrame = 0;
//static int speed = 250;
//static boolean hasScored = false;
//static SDL_Rect bird;
//static SDL_Rect s_birb.animRect;
//static float birdAngle;
//static SDL_Texture *birdTexture = NULL;
#define kNUM_PIPES 16
#define kPIPE_WIDTH 150
#define kPIPE_HEIGHT 400
#define kLENGTH_BETWEEN_PIPES 100

typedef struct
{
	SDL_Rect rects[kNUM_PIPES];
	SDL_Texture *pTexture;
} PipeData;

typedef struct
{
	SDL_Rect rect;
	SDL_Texture *pTexture;
} WorldItem;

static void FB_AppendAssetPath(char *pPath);
static boolean FB_LoadAssets(SDL_Renderer *pRenderer);
static boolean FB_SetupFont(SDL_Renderer *pRenderer);
static boolean FB_Init(void);
static void FB_MainLoop(void);
static void FB_Shutdown(void);
static void FB_Start(SDL_Renderer *pRenderer); // Game start
static void FB_Play(SDL_Window *pWindow, SDL_Renderer *pRenderer, float deltaTime); // Gameplay
static void FB_Over(SDL_Renderer *pRenderer); // Game over
static void FB_ResetBirdPos(void);
static void FB_DrawBackground(SDL_Rect *pRect, SDL_Renderer *pRenderer);
static void FB_DrawBird(SDL_Rect *pRectBird, SDL_Rect *pRectAnim, SDL_Renderer *pRenderer);
static void FB_DrawGround(SDL_Rect *pRect, SDL_Renderer *pRenderer, float deltaTime);
static void FB_DrawPipe(SDL_Rect *pRect, SDL_Rect *pRectBird, SDL_Renderer *pRenderer);
static void FB_GeneratePipes(void);
static void FB_UpdateScore(SDL_Window *pWindow);
// Game:
#define kWINDOW_WIDTH 800
#define kWINDOW_HEIGHT 600

static Game s_game;
// Pipes:
static PipeData s_pipeData;
// Bird:
static FlappyBirb s_birb;
// World:
static WorldItem s_worldBackground;
static WorldItem s_worldGround;
// UI:
static TTF_Font *s_pFont = NULL;
static SDL_Surface *s_pTextSurface = NULL;
static SDL_Texture *s_pTextTexture = NULL;
static SDL_Texture *s_pTextOverTexture = NULL;
static SDL_Rect s_textRect;
static SDL_Rect s_textOverRect;
// Audio:
static Mix_Chunk *s_pFlapSound = NULL;
static Mix_Chunk *s_pScoreSound = NULL;
static Mix_Chunk *s_pLoseSound = NULL;
static boolean s_losePlayedOnce = false;
// Controls:
static boolean s_spaceDown = false;

int main(int numArgs, char *pArgv[])
{
    if (!FB_Init())
	{
		FB_ERR("Could not start the game!");
		return EXIT_FAILURE;
	}

	FB_MainLoop();
    FB_Shutdown();
    return EXIT_SUCCESS;
}

void FB_AppendAssetPath(char *path)
{
	char cpystr[256];
	strcpy(cpystr, path);
	strcat(path, kASSETS_DIR);
	strcat(path, cpystr);
}

boolean FB_LoadAssets(SDL_Renderer *renderer)
{
    s_pipeData.pTexture = IMG_LoadTexture(renderer, kASSETS_DIR"pipe.webp");
    if (s_pipeData.pTexture == NULL)
	{
        FB_ERR("Image could not be found: %s\n", SDL_GetError());
        return false;
    }
    s_birb.pTexture = IMG_LoadTexture(renderer, kASSETS_DIR"birbTile.webp");
    if (s_birb.pTexture == NULL)
	{
        FB_ERR("Image could not be found: %s\n", SDL_GetError());
        return false;
    }
    SDL_QueryTexture(s_birb.pTexture, NULL, NULL, &s_birb.animRect.w, &s_birb.animRect.h);

    s_worldBackground.pTexture = IMG_LoadTexture(renderer, kASSETS_DIR"bgTex.webp");
    if (s_worldBackground.pTexture == NULL)
	{
        FB_ERR("Image could not be found: %s\n", SDL_GetError());
        return false;
    }
    s_worldGround.pTexture = IMG_LoadTexture(renderer, kASSETS_DIR"ground.webp");
    if (s_worldGround.pTexture == NULL)
	{
        FB_ERR("Image could not be found: %s\n", SDL_GetError());
        return false;
    }
	s_pFont = TTF_OpenFont(kASSETS_DIR"FlappyBirdy.ttf", 22);
    if (s_pFont == NULL)
	{
        FB_ERR("Font failed to load: %s\n", SDL_GetError());
        return false;
    }
	if (!FB_SetupFont(renderer))
	{
		FB_ERR("FB_SetupFont(): failed: %s!\n", TTF_GetError());
	}
    s_pFlapSound = Mix_LoadWAV(kASSETS_DIR"flap.wav");
    if (s_pFlapSound == NULL) return false;
    s_pScoreSound = Mix_LoadWAV(kASSETS_DIR"score.wav");
    if (s_pScoreSound == NULL) return false;
    s_pLoseSound = Mix_LoadWAV(kASSETS_DIR"lose.wav");
    if (s_pLoseSound == NULL) return false;
    return true;
}

boolean FB_SetupFont(SDL_Renderer *renderer)
{
    const SDL_Color textColor = { 0, 0, 0, 255 };
    const SDL_Color textColor2 = { 255, 255, 255, 255 };
    s_pTextSurface = TTF_RenderText_Blended_Wrapped(s_pFont, "Press Space to Start",
												 textColor, 60);
    s_pTextTexture = SDL_CreateTextureFromSurface(renderer, s_pTextSurface);
    SDL_FreeSurface(s_pTextSurface);
    SDL_QueryTexture(s_pTextTexture, NULL, NULL, &s_textRect.w, &s_textRect.h);
    s_textRect.w *= 4;
    s_textRect.h *= 4;
    s_textRect.x = 200 - (s_textRect.w / 2);
    s_textRect.y = 125 - (s_textRect.h /2);
    /* printf("Text 1, Width: %d, Height: %d\n", s_textRect.w, s_textRect.h); */
    s_pTextSurface = TTF_RenderText_Blended_Wrapped(s_pFont, "Press Space to Restart",
												 textColor2, 60);
    s_pTextOverTexture = SDL_CreateTextureFromSurface(renderer, s_pTextSurface);
    SDL_FreeSurface(s_pTextSurface);
    SDL_QueryTexture(s_pTextOverTexture, NULL, NULL, &s_textOverRect.w,
					 &s_textOverRect.h);
    s_textOverRect.w *= 4;
    s_textOverRect.h *= 4;
    s_textOverRect.x = kWINDOW_WIDTH / 2 - (s_textOverRect.w / 2);
    s_textOverRect.y = kWINDOW_HEIGHT / 2 - (s_textOverRect.h / 2);
    /* printf("Text 2, Width: %d, Height: %d\n", s_textRect.w, s_textRect.h); */
	return true;
}

boolean FB_Init(void)
{
	// BEGIN INITIALIZATION
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0)
	{
        FB_ERR("Could not initialize SDL: %s\n", SDL_GetError());
        return EXIT_FAILURE;
    }
	
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048);
    Mix_Volume(-1, 5);

    s_game.pWindow = SDL_CreateWindow(s_game.windowTitle, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
									  kWINDOW_WIDTH, kWINDOW_HEIGHT, SDL_WINDOW_SHOWN);
    if (s_game.pWindow == NULL)
	{
        FB_ERR("SDL Window Creation failed: %s\n", SDL_GetError());
        return EXIT_FAILURE;
    }
    s_game.pRenderer = SDL_CreateRenderer(s_game.pWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (s_game.pRenderer == NULL)
	{
        FB_ERR("SDL Renderer creation failed: %s\n", SDL_GetError());
        return false;
    }

    if (IMG_Init(IMG_INIT_WEBP) != IMG_INIT_WEBP)
	{
        FB_ERR("Image initialization failed : %s\n", SDL_GetError());
        return false;
    }

    if (TTF_Init() == -1)
	{
        FB_ERR("TrueTypeFont init failed: %s\n", SDL_GetError());
        return false;
    }

    if (!FB_LoadAssets(s_game.pRenderer))
	{
        FB_ERR("Could not load crucial game assets.");
        return false;
    }

    s_game.pWindowIcon = IMG_Load(kASSETS_DIR"birb0.webp");
    if (s_game.pWindowIcon == NULL)
	{
        FB_ERR("SDL Icon failed to load: %s\n", SDL_GetError());
        return false;
    }
    SDL_SetWindowIcon(s_game.pWindow, s_game.pWindowIcon);
	SDL_FreeSurface(s_game.pWindowIcon);

	s_birb.moveSpeed = 250;
	FB_ResetBirdPos(); /* set the bird to it's initial position */
    s_game.state = GAMESTATE_START; /* important piece of initialization here */
    FB_GeneratePipes(); /* initiallly generate pipe positions */
	/* END OF INITIALIZATION */
	return true;
}

void FB_MainLoop(void)
{
    Uint64 start = SDL_GetPerformanceCounter(), end = 0;
    float deltaTime = 0.0f;
    s_game.isRunning = true;
	
    while (s_game.isRunning)
	{
		SDL_Event event;

        end = start;
        start = SDL_GetPerformanceCounter();
        deltaTime = (float)((start - end) * 1000 / (float)SDL_GetPerformanceFrequency());
        while (SDL_PollEvent(&event))
		{
            switch (event.type)
			{
            case SDL_QUIT:
                s_game.isRunning = false;
                break;
            case SDL_KEYDOWN:
			{
                if (event.key.repeat == 0)
				{
                    if (event.key.keysym.sym == SDLK_SPACE)
					{
                        s_spaceDown = true;
                    }
					else if (event.key.keysym.sym == SDLK_ESCAPE)
					{
                        s_game.isRunning = false;
                    }
                }
				else
				{
                    s_spaceDown = false;
                }
                break;
			}}
        }

        switch (s_game.state)
		{
        case GAMESTATE_START:
            FB_Start(s_game.pRenderer);
            break;
        case GAMESTATE_PLAY:
            FB_Play(s_game.pWindow, s_game.pRenderer, deltaTime);
            break;
        case GAMESTATE_OVER:
            FB_Over(s_game.pRenderer);
            break;
        }
        SDL_RenderPresent(s_game.pRenderer);
    }
}

void FB_Shutdown(void)
{
    SDL_DestroyTexture(s_pipeData.pTexture);
    SDL_DestroyTexture(s_birb.pTexture);
    SDL_DestroyTexture(s_worldBackground.pTexture);
    SDL_DestroyTexture(s_worldGround.pTexture);
    IMG_Quit();
	
	SDL_DestroyTexture(s_pTextTexture);
    SDL_DestroyTexture(s_pTextOverTexture);
    TTF_CloseFont(s_pFont);
    TTF_Quit();
	
    Mix_FreeChunk(s_pFlapSound);
    Mix_FreeChunk(s_pScoreSound);
    Mix_FreeChunk(s_pLoseSound);
    Mix_CloseAudio();
	Mix_Quit();

	SDL_DestroyRenderer(s_game.pRenderer);
    SDL_DestroyWindow(s_game.pWindow);
	SDL_Quit();
}

void FB_Start(SDL_Renderer *renderer)
{
	size_t index;

    if (s_spaceDown)
	{
        if (s_birb.hasCollided)
		{
            s_losePlayedOnce = false;
            s_game.state = GAMESTATE_PLAY;
            s_birb.hasCollided = false;
            s_game.currScore = 0;
        }
		else
            s_game.state++;
        s_spaceDown = false;
    }

    SDL_SetRenderDrawColor(renderer, 0, 255/2, 255, 255);
    SDL_RenderClear(renderer);
    FB_DrawBackground(&s_worldBackground.rect, renderer);
    FB_DrawBird(&s_birb.rect, &s_birb.animRect, renderer);
    FB_DrawGround(&s_worldGround.rect, renderer, 0);
	
    SDL_RenderCopy(renderer, s_pTextTexture, NULL, &s_textRect);

	for (index = 0; index < kNUM_PIPES; index++)
        FB_DrawPipe(&s_pipeData.rects[index], &s_birb.rect, renderer);

    if (s_birb.rect.x > s_pipeData.rects[kNUM_PIPES - 2].x)
        FB_GeneratePipes();
}

void FB_Play(SDL_Window *window, SDL_Renderer *renderer, float deltaTime)
{
	size_t index;
    static const int maxBirdMoveUp = 50;
    static int birdStartingY = 0;
    static int birdMove = 0;
    static int lastBirdMove = 0;
    static boolean birdInMotion = false;
    static const float maxAngle = -45.0f;
    static float lastAngle = 0.0f;
	
    if (s_spaceDown)
	{
        if (!s_birb.hasCollided)
		{
            Mix_PlayChannel(-1, s_pFlapSound, 0);
            /* if (s_birb.rect.y < maxBirdMoveUp) {
                birdMove += lastBirdMove * 2 + 1;
                s_birb.rect.y -= birdMove;
                lastBirdMove = birdMove;
            } else {
                birdMove = 0;
                lastBirdMove = 0;
            } */
            birdInMotion = true;
            birdStartingY = s_birb.rect.y;
            printf("birdStartingY=%d|%d\n", birdStartingY, birdStartingY - maxBirdMoveUp);
            // s_birb.rect.y -= 50;
        }
        s_spaceDown = false;
    }

    if (birdInMotion)
	{
        if (s_birb.rect.y > birdStartingY - maxBirdMoveUp)
		{
            birdMove += lastBirdMove * 2 + 1;
            s_birb.rect.y -= birdMove;
            lastBirdMove = birdMove;
        }
        else
		{
            birdMove = 0;
            lastBirdMove = 0;
            birdStartingY = s_birb.rect.y;
            birdInMotion = false;
        }

        if (s_birb.angle > maxAngle)
		{
            s_birb.angle -= lastAngle * 3.0f + 10.0f;
            lastAngle = s_birb.angle;
            printf("s_birb.angle=%.2f|lastAngle=%.2f\n", s_birb.angle, lastAngle);
        }
        else
		{
            lastAngle = 0.0f;
            s_birb.angle = 0.0f;
        }
    }
	else
	{
        s_birb.angle = 0.0f;
        lastAngle = 0.0f;
    }

    SDL_SetRenderDrawColor(renderer, 0, 255/2, 255, 255);
    SDL_RenderClear(renderer);
    FB_DrawBackground(&s_worldBackground.rect, renderer);

    s_birb.rect.y += 1 * (int)deltaTime / 6;
    FB_DrawBird(&s_birb.rect, &s_birb.animRect, renderer);
    FB_DrawGround(&s_worldGround.rect, renderer, deltaTime);
    for (index = 0; index < kNUM_PIPES; index++)
	{
        s_pipeData.rects[index].x -= 1 * (int)deltaTime / 6;
        FB_DrawPipe(&s_pipeData.rects[index], &s_birb.rect, renderer);
    }

    if (s_birb.rect.x > s_pipeData.rects[kNUM_PIPES - 2].x)
	{
		FB_GeneratePipes();
	}

	FB_UpdateScore(window);
}

void FB_Over(SDL_Renderer *renderer)
{
    if (s_spaceDown)
	{
        FB_ResetBirdPos();
        FB_GeneratePipes();
        s_game.state = GAMESTATE_START;
        s_spaceDown = false;
    }

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, s_pTextOverTexture, NULL, &s_textOverRect);

    if (!s_losePlayedOnce)
	{
        Mix_PlayChannel(-1, s_pLoseSound, 0);
        s_losePlayedOnce = true;
    }
}

void FB_DrawBackground(SDL_Rect *rect, SDL_Renderer *renderer)
{
    rect->x = 0;
    rect->y = 0;
    rect->w = kWINDOW_WIDTH;
    rect->h = kWINDOW_HEIGHT;
    SDL_RenderCopy(renderer, s_worldBackground.pTexture, NULL, rect);
}

void FB_DrawBird(SDL_Rect *rectBird, SDL_Rect *rectAnim, SDL_Renderer *renderer)
{
    if (rectBird->y >= ((int)kWINDOW_HEIGHT) - (kBIRD_SIZE / 2))
	{
        rectBird->y = ((int)kWINDOW_HEIGHT) - (kBIRD_SIZE / 2);
    }
    /* animate(rectAnim, 100, 3); */
    /* if (fps >= 30) {
        currFrame++;
        fps = 0;
    } */
    s_game.currFrame = (SDL_GetTicks64() / s_birb.moveSpeed) & (kNUM_FRAMES + 1);
	FB_LOG("currFrame %d\n");
    switch(s_game.currFrame)
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
    // SDL_RenderCopy(renderer, s_birb.pTexture, rectAnim, rectBird);
    const SDL_Point center =
	{
        .x = rectAnim->w / 2,
        .y = rectAnim->h / 2
    };
    SDL_RenderCopyEx(renderer, s_birb.pTexture, rectAnim, rectBird, s_birb.angle, &center, SDL_FLIP_NONE);
}

void FB_DrawGround(SDL_Rect *rect, SDL_Renderer *renderer, float deltaTime)
{
    const int minX = kWINDOW_WIDTH - (kWINDOW_WIDTH * 2);
    if (rect->x <= minX)
	{
        rect->x = 0;
    }
    else
	{
        rect->x -= 1 * (int)deltaTime / 6;
    }
    rect->y = 0;
    rect->w = kWINDOW_WIDTH * 2;
    rect->h = kWINDOW_HEIGHT;
    SDL_RenderCopy(renderer, s_worldGround.pTexture, NULL, rect);
}

void FB_DrawPipe(SDL_Rect *rect, SDL_Rect *rectBird, SDL_Renderer *renderer)
{
    SDL_Rect topRect, middleRect, bottomRect;
	static const SDL_RendererFlip c_kRenderFlip = SDL_FLIP_VERTICAL;

	topRect.x = rect->x;
    topRect.y = rect->y - kPIPE_HEIGHT - kLENGTH_BETWEEN_PIPES;
    topRect.w = kPIPE_WIDTH;
    topRect.h = kPIPE_HEIGHT;

    bottomRect.x = rect->x;
    bottomRect.y = rect->y + kLENGTH_BETWEEN_PIPES;
    bottomRect.w = kPIPE_WIDTH;
    bottomRect.h = kPIPE_HEIGHT;

    middleRect.x = rect->x + (kPIPE_WIDTH / 2);
    middleRect.y = bottomRect.y - (kLENGTH_BETWEEN_PIPES * 2);
    middleRect.w = 3;
    middleRect.h = kLENGTH_BETWEEN_PIPES * 2;

    if (U_IsColliding(rectBird, &topRect) || U_IsColliding(rectBird, &bottomRect))
	{
        s_game.state = GAMESTATE_OVER;
        s_birb.hasCollided = true;
    }
	
    if (U_IsColliding(rectBird, &middleRect) && !s_birb.hasScored && rectBird->x > middleRect.x)
	{
        /* I don't know why this works but it does */
        Mix_PlayChannel(-1, s_pScoreSound, 0);
        s_game.currScore += 1;
        s_birb.hasScored = true;
    }
	else
	{
        s_birb.hasScored = false;
    }

    SDL_RenderCopyEx(renderer, s_pipeData.pTexture, NULL, &topRect, 0, NULL, c_kRenderFlip);
    SDL_RenderCopy(renderer, s_pipeData.pTexture, NULL, &bottomRect);
}

void FB_ResetBirdPos(void)
{
    s_birb.rect.x = 100;
    s_birb.rect.y = 250;
    s_birb.rect.w = kBIRD_SIZE;
    s_birb.rect.h = kBIRD_SIZE;
}

void FB_GeneratePipes(void)
{
    for (size_t index = 0; index < kNUM_PIPES; index++)
	{
        if (index > 0)
            s_pipeData.rects[index].x = s_pipeData.rects[index - 1].x + 350;
        else
            s_pipeData.rects[0].x = 350;

        s_pipeData.rects[index].y = U_RandomNum(kWINDOW_HEIGHT / 2 - kLENGTH_BETWEEN_PIPES,
													kWINDOW_HEIGHT / 2 + kLENGTH_BETWEEN_PIPES) + 200;
        //printf("Pipe at %d, Y: %d\n", i + 1, s_pipeData.rects[i].y);
    }
}

void FB_UpdateScore(SDL_Window *window)
{
    static char c_scoreBuffer[32];

	snprintf(c_scoreBuffer, sizeof(c_scoreBuffer), "%d", s_game.currScore);
    strcpy(s_game.windowTitle, "Flappy Birby, Score: ");
    strcat(s_game.windowTitle, c_scoreBuffer);
	// Update the window title:
	SDL_SetWindowTitle(window, s_game.windowTitle);
}
