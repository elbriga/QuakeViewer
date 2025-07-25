#include <stdio.h>
#include <stdint.h>
#include <unistd.h>

#include <SDL2/SDL.h>

#include "QuakeViewer.h"

int _gfxWidth, _gfxHeight;

// Framebuffer dinâmico
uint32_t *framebuffer = NULL;

SDL_Window		*janela;
SDL_Renderer	*renderer;
SDL_Texture		*texture;

int gfx_open( int width, int height, const char *title )
{
	_gfxWidth  = width;
	_gfxHeight = height;

	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("Erro ao iniciar SDL: %s\n", SDL_GetError());
        return 1;
    }

    janela = SDL_CreateWindow(title,
                              SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                              _gfxWidth, _gfxHeight, SDL_WINDOW_SHOWN);
    if (!janela) {
        printf("Erro ao criar janela: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    renderer = SDL_CreateRenderer(janela, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        printf("Erro ao criar renderer: %s\n", SDL_GetError());
        SDL_DestroyWindow(janela);
        SDL_Quit();
        return 1;
    }

    texture = SDL_CreateTexture(renderer,
                                SDL_PIXELFORMAT_ARGB8888,
                                SDL_TEXTUREACCESS_STREAMING,
                                _gfxWidth, _gfxHeight);
    if (!texture) {
        printf("Erro ao criar textura: %s\n", SDL_GetError());
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(janela);
        SDL_Quit();
        return 1;
    }

    // Aloca framebuffer na RAM
    framebuffer = malloc(sizeof(uint32_t) * _gfxWidth * _gfxHeight);
    if (!framebuffer) {
        printf("Erro ao alocar framebuffer\n");
        SDL_DestroyTexture(texture);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(janela);
        SDL_Quit();
        return 1;
    }

	return 0;
}

void gfx_close()
{
    free(framebuffer);

    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(janela);
	
    SDL_Quit();
}

void gfx_point( int x, int y, byte r, byte g, byte b )
{
	if (x < 0 || x >= _gfxWidth || y < 0 || y >= _gfxHeight) return;
    framebuffer[y * _gfxWidth + x] = (0xFF000000 | (r << 16) | (g << 8) | b);
}

void gfx_line( int x1, int y1, int x2, int y2, byte r, byte g, byte b )
{
	int dx = abs(x1 - x1);
    int dy = abs(y1 - y1);
    int sx = (x1 < x2) ? 1 : -1;
    int sy = (y1 < y2) ? 1 : -1;
    int err = dx - dy;

    while(1) {
        gfx_point(x1, y1, r,g,b);
        if((x1 == x2) && (y1 == y2)) break;
        int e2 = 2 * err;
        if(e2 > -dy) { err -= dy; x1 += sx; }
        if(e2 <  dx) { err += dx; y1 += sy; }
    }
}

void gfx_clear()
{
	for (int i = 0; i < _gfxWidth * _gfxHeight; i++) {
        framebuffer[i] = 0xFF000000;
    }
}

void gfx_flush()
{
	// Atualiza textura com o conteúdo do framebuffer
    SDL_UpdateTexture(texture, NULL, framebuffer, _gfxWidth * sizeof(uint32_t));

	SDL_RenderClear(renderer);
	SDL_RenderCopy(renderer, texture, NULL, NULL);
	SDL_RenderPresent(renderer);
}

char engine_get_key()
{
	SDL_Event evento;

	while (SDL_PollEvent(&evento)) {
		if (evento.type == SDL_QUIT) {
			return 'q';
		}
		// Captura texto real digitado (ASCII ou UTF-8)
		else if (evento.type == SDL_TEXTINPUT) {
			char caractere = evento.text.text[0];
			printf("ASCII: %d, char: '%c'\n", caractere, caractere);
			return caractere;
		}
		else if (evento.type == SDL_KEYDOWN) {
			SDL_Keycode tecla = evento.key.keysym.sym;

			if (tecla == SDLK_ESCAPE) {
				return 'q';  // ESC encerra o programa
			} else if (tecla == SDLK_LEFT) {
				return 81;
			} else if (tecla == SDLK_RIGHT) {
				return 83;
			} else if (tecla == SDLK_UP) {
				return 82;
			} else if (tecla == SDLK_DOWN) {
				return 84;
			} else if (tecla == SDLK_a) {
				printf("Tecla 'A' pressionada\n");
			}
		} else if (evento.type == SDL_KEYUP) {
			printf("Tecla liberada: %s\n", SDL_GetKeyName(evento.key.keysym.sym));
		}
	}
}

char engine_get_key_block()
{
	char key = 0;
	while(1) {
		key = engine_get_key();
		if (key) {
			break;
		}
		usleep(25000);
	}
	return key;
}
