#ifndef MONSTROS_H
#define MONSTROS_H

#include "QuakeViewer.h"
#include "entidade.h"

void monstro_update(mapa_t *mapa, entidade_t *monstro, float deltaTime);
void monstro_ajusta_chao(mapa_t *mapa, entidade_t *ent);

#endif