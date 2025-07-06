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
void grafico_xis( int x, int y );
void grafico_linha( int x0, int y0, int x1, int y1 );

void grafico_projecao3D(ponto_t *p);

void grafico_triangulo(
	int x1, int y1, float z1,
	int x2, int y2, float z2,
	int x3, int y3, float z3,
	int r, int g, int b);
void grafico_triangulo_textura(char *textura, int textW, int textH, char paleta[256][3],
    int x1,int y1,int z1, int ts1,int tt1,
	int x2,int y2,int z2, int ts2,int tt2,
	int x3,int y3,int z3, int ts3,int tt3);
void grafico_triangulo_wireZ(int x1,int y1,int z1, int x2,int y2,int z2, int x3,int y3,int z3);

void grafico_desenha_poligono(ponto_t *verticesPoligono, int numVerts, texture_t *tex);

char grafico_tecla();
char grafico_tecla_espera();

#endif
