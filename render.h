#ifndef RENDER_H
#define RENDER_H

#include "3d.h"
#include "gfx_ptBR.h"

void render_desenha_mapa(camera_t *cam, mapa_t *mapa, char paleta[256][3]);
void render_desenha_objeto(camera_t *cam, obj3d_t *obj, int numFrameSel, char paleta[256][3]);

#endif