// sdltest.c
#ifdef _WIN32a
#include "SDL.h"   /* All SDL App's need this */
#else
#include<linux/time.h>
#define __timespec_defined 1
#define __timeval_defined 1
#define __itimerspec_defined 1
#include "SDL2/SDL.h"
#endif
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include "hr_time.h"


#define QUITKEY SDLK_ESCAPE
#define WIDTH 1024
#define HEIGHT 768

SDL_Window* screen = NULL;
SDL_Renderer* renderer;
SDL_Event event;
SDL_Rect source, destination, dst;

int errorCount = 0;
int keypressed;
int rectCount = 0;
stopWatch s;

/* returns a number between 1 and max */
int Random(int max) {
	return (rand() % max) + 1;
}

void LogError(char* msg) {
	printf("%s\n", msg);
	errorCount++;
}

/* Sets Window caption according to state - eg in debug mode or showing fps */
void SetCaption(char* msg) {
	SDL_SetWindowTitle(screen, msg);
}

/* Initialize all setup, set screen mode, load images etc */
void InitSetup() {
	srand((int)time(NULL));
	SDL_Init(SDL_INIT_EVERYTHING);
	SDL_CreateWindowAndRenderer(WIDTH, HEIGHT, SDL_WINDOW_SHOWN, &screen, &renderer);
	if (!screen) {
		LogError("InitSetup failed to create window");
	}
	SetCaption("Example One");
}

/* Cleans up after game over */
void FinishOff() {
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(screen);
	//Quit SDL
	SDL_Quit();
	exit(0);
}

/* read a character */
char getaChar() {
	int result = -1;

	while (SDL_PollEvent(&event)) {
		if (event.type == SDL_KEYDOWN)
		{
			result = event.key.keysym.sym;
			break;
		}
	}
	return result;
}

void DrawRandomRectangle() {
	char buff[20];
	SDL_Rect rect;
	SDL_SetRenderDrawColor(renderer, Random(256) - 1, Random(256) - 1, Random(256) - 1, 255);
	rect.h = 120;// Random(100) + 20;
	rect.w = 120;// Random(100) + 20;
	rect.y = Random(HEIGHT - rect.h - 1);
	rect.x = Random(WIDTH - rect.w - 1);
	SDL_RenderFillRect(renderer, &rect);

	rectCount++;
	if (rectCount % 100000 == 0) {
		SDL_RenderPresent(renderer);
		stopTimer(&s);
#ifdef _WIN32
		sprintf_s(buff, sizeof(buff), "%10.6f", getElapsedTime(&s));
#else
		snprintf(buff, sizeof(buff),"%10.6f",diff(&s));
#endif
		SetCaption(buff);
		startTimer(&s);
	}
}

/* main game loop. Handles demo mode, high score and game play */
void GameLoop() {
	int gameRunning = 1;
	startTimer(&s);
	while (gameRunning)
	{
		DrawRandomRectangle();

		while (SDL_PollEvent(&event)) {
			switch (event.type) {
			case SDL_KEYDOWN:
				keypressed = event.key.keysym.sym;
				if (keypressed == QUITKEY)
				{
					gameRunning = 0;
					break;
				}

				break;
			case SDL_QUIT: /* if mouse click to close window */
			{
				gameRunning = 0;
				break;
			}
			case SDL_KEYUP: {
				break;
			}
			} /* switch */

		} /* while SDL_PollEvent */
	}
}

int main(int argc, char* args[])
{
	InitSetup();
	GameLoop();
	FinishOff();
	return 0;
}

