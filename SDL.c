#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>  // malloc, free

// Framebuffer dinâmico
uint32_t *framebuffer = NULL;
int largura = 640;
int altura  = 480;

// Função para desenhar um pixel
void put_pixel(int x, int y, uint32_t cor) {
    if (x < 0 || x >= largura || y < 0 || y >= altura) return;
    framebuffer[y * largura + x] = cor;
}

// Função para limpar a tela
void limpa_tela(uint32_t cor) {
    for (int i = 0; i < largura * altura; i++) {
        framebuffer[i] = cor;
    }
}

int main(int argc, char *argv[]) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("Erro ao iniciar SDL: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Window *janela = SDL_CreateWindow("Framebuffer Variável",
                                          SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                          largura, altura, SDL_WINDOW_SHOWN);
    if (!janela) {
        printf("Erro ao criar janela: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_Renderer *renderer = SDL_CreateRenderer(janela, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        printf("Erro ao criar renderer: %s\n", SDL_GetError());
        SDL_DestroyWindow(janela);
        SDL_Quit();
        return 1;
    }

    SDL_Texture *texture = SDL_CreateTexture(renderer,
                                             SDL_PIXELFORMAT_ARGB8888,
                                             SDL_TEXTUREACCESS_STREAMING,
                                             largura, altura);
    if (!texture) {
        printf("Erro ao criar textura: %s\n", SDL_GetError());
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(janela);
        SDL_Quit();
        return 1;
    }

    // Aloca framebuffer na RAM
    framebuffer = malloc(sizeof(uint32_t) * largura * altura);
    if (!framebuffer) {
        printf("Erro ao alocar framebuffer\n");
        SDL_DestroyTexture(texture);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(janela);
        SDL_Quit();
        return 1;
    }

    int rodando = 1;
    SDL_Event evento;

    while (rodando) {
        while (SDL_PollEvent(&evento)) {
            if (evento.type == SDL_QUIT) {
                rodando = 0;
            }
        }

        // Limpa e desenha
        limpa_tela(0xFF000000); // preto

        // Exemplo: quadrado laranja
        for (int y = 100; y < 120; y++) {
            for (int x = 100; x < 120; x++) {
                put_pixel(x, y, 0xFFFF9900);
            }
        }

        // Atualiza textura com o conteúdo do framebuffer
        SDL_UpdateTexture(texture, NULL, framebuffer, largura * sizeof(uint32_t));

        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, NULL, NULL);
        SDL_RenderPresent(renderer);

        SDL_Delay(16);
    }

    // Libera tudo
    free(framebuffer);
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(janela);
    SDL_Quit();

    return 0;
}
