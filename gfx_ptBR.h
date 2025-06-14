#ifndef GFXPTBR_H
#define GFXPTBR_H

#define FOV 300
#include "3d.h"

int grafico_init( int altura, int largura, const char *titulo );
void grafico_desliga();

void grafico_limpa();
void grafico_limpa_zbuffer();
void grafico_mostra();

void grafico_cor( int r, int g, int b );
void grafico_ponto( int x, int y );

void grafico_projecao3D(ponto *p);

void grafico_triangulo(int x1, int y1, int x2, int y2, int x3, int y3,
	int r, int g, int b);
void grafico_triangulo_textura(char *textura, int textW, int textH, char paleta[256][3],
    int x1, int y1, int x2, int y2, int x3, int y3,
    int ts1, int tt1, int ts2, int tt2, int ts3, int tt3);
void grafico_triangulo_textura_zbuffer(char *textura, int textW, int textH, char paleta[256][3],
    int x1,int y1,int z1, int x2,int y2,int z2, int x3,int y3,int z3,
    int ts1, int tt1,     int ts2, int tt2,     int ts3, int tt3);

char grafico_tecla();
char grafico_tecla_espera();

#endif
