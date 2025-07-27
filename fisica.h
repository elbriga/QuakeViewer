#ifndef FISICA_H
#define FISICA_H

#include "obj3d.h"
#include "entidade.h"

#define GRAVIDADE 800.0f
#define PASSO_MAX 8.0f  // distância máxima vertical que "sobe degrau"

void fisica_update_entidade(mapa_t *mapa, entidade_t *ent, float deltaTime);

#endif