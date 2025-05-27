#ifndef GFXPTBR_H
#define GFXPTBR_H

void grafico_init( int altura, int largura, const char *titulo );

void grafico_limpa();
void grafico_mostra();

void grafico_cor( int r, int g, int b );
void grafico_ponto( int x, int y );
void grafico_triangulo(int x1, int y1, int x2, int y2, int x3, int y3, int corR, int corG, int corB);

char grafico_tecla();

#endif
