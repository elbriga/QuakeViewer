#ifndef RENDER_H
#define RENDER_H

#include "3d.h"
#include "entidade.h"

#define NEAR_Z 1.0f

void render_desenha_mapa(mapa_t *mapa, camera_t *cam);
void render_desenha_entidade(camera_t *cam, entidade_t *ent);

#endif