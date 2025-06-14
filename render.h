#ifndef RENDER_H
#define RENDER_H

#include "3d.h"
#include "gfx_ptBR.h"

void grafico_desenha_objeto(camera_t *cam, obj3d_t *obj, int numFrameSel, char paleta[256][3]);

#endif