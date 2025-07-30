#ifndef ENTIDADE_H
#define ENTIDADE_H

#include "3d.h"
#include "mapa.h"

#define MAX_OBJS        32
#define MAX_ENTIDADES   256

typedef struct
{
    obj3d_t *obj;

    vetor3d_t posicao;
    vetor3d_t velocidade;
    vetor3d_t rotacao;

    int numFrameSel;
    int numAnimSel;
    int numAnimSelAuto;

    int vivo;
    int noChao;
} entidade_t;

void entidade_create(char *modelName, vetor3d_t pos, vetor3d_t ang);
void entidades_render(camera_t *cam);
void entidades_update(mapa_t *mapa, float deltaTime);
void entidades_destroy();

void entidades_pula(); // kkk

void entidade_projecao3D(camera_t *cam, entidade_t *ent);

void entidade_set_anim(entidade_t *ent, int num);
void entidade_inc_frame(int id);
void entidade_dec_anim(int id);
void entidade_inc_anim(int id);

#endif