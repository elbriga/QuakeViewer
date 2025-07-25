#ifndef GRAFICO_H
#define GRAFICO_H

#define FOV     200
#define M_PI    3.14159265358979323846	/* pi */

#include "3d.h"

int  grafico_init( int altura, int largura, const char *titulo );
void grafico_desliga();

void grafico_limpa();
void grafico_limpa_zbuffer();
void grafico_mostra();

void grafico_xis( int x, int y, byte r, byte g, byte b );

void grafico_projecao3D(ponto_t *p);

void grafico_desenha_poligono(ponto_t **verticesPoligono, int numVerts, texture_t *tex, byte *light, int lightW, int lightH, char paleta[256][3]);
void grafico_desenha_poligono_sky(ponto_t **verticesPoligono, int numVerts, texture_t *sky, float tempo, char paleta[256][3]);

#endif
