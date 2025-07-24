#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdint.h>

SDL_Window *janela;
SDL_Renderer *renderer;
SDL_Texture *framebuffer;
int gfxLARGURA=0, gfxALTURA=0;

// Ponteiro global para os pixels da textura
uint32_t *framebuffer_pixels = NULL;
int framebuffer_pitch = 0;

// Função para desenhar um pixel no framebuffer
void put_pixel(int x, int y, uint32_t cor) {
    if (x < 0 || x >= gfxLARGURA || y < 0 || y >= gfxALTURA) return;  // Clipping

    framebuffer_pixels[y * (framebuffer_pitch / 4) + x] = cor;
}

int gfx_open( int width, int height, const char *title )
{
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("Erro ao iniciar SDL: %s\n", SDL_GetError());
        return 1;
    }

    janela = SDL_CreateWindow(title,
                              SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                              width, height, SDL_WINDOW_SHOWN);
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

    framebuffer = SDL_CreateTexture(renderer,
                                    SDL_PIXELFORMAT_ARGB8888,
                                    SDL_TEXTUREACCESS_STREAMING,
                                    width, height);
    if (!framebuffer) {
        printf("Erro ao criar textura: %s\n", SDL_GetError());
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(janela);
        SDL_Quit();
        return 1;
    }

    gfxLARGURA = width;
    gfxALTURA  = height;

    return 0;
}

int main(int argc, char*argv[])
{
    if( gfx_open( 800, 600, "Teste SDL" ) ) {
        SDL_Quit();
        return 1;
    }

    // Lock da textura
    void *pixels;
    if (SDL_LockTexture(framebuffer, NULL, &pixels, &framebuffer_pitch) < 0) {
        printf("Erro ao fazer lock na textura: %s\n", SDL_GetError());
    } else {
        framebuffer_pixels = (uint32_t *)pixels;

        // Exemplo: desenha um quadrado de pixels vermelhos
        for (int y = 100; y < 110; y++) {
            for (int x = 100; x < 110; x++) {
                put_pixel(x, y, 0xFFFF0000);  // Vermelho
            }
        }

        SDL_UnlockTexture(framebuffer);
    }

    // Loop principal
    int rodando = 1;
    SDL_Event evento;

    while (rodando) {
        while (SDL_PollEvent(&evento)) {
            if (evento.type == SDL_QUIT) {
                rodando = 0;
            }
        }

        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, framebuffer, NULL, NULL);
        SDL_RenderPresent(renderer);

        SDL_Delay(16);
    }

    SDL_DestroyTexture(framebuffer);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(janela);
    SDL_Quit();
    return 0;
}
