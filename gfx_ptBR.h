#ifndef GFXPTBR_H
#define GFXPTBR_H

void gfx_init( int width, int height, const char *title );
void gfx_limpa();
void gfx_ponto( int x, int y, int r, int g, int b );
char gfx_tecla();

#endif
