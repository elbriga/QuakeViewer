#ifndef ENTIDADE_H
#define ENTIDADE_H

#include "3d.h"

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
} entidade_t;

void entidade_create(char *modelName, vetor3d_t pos, vetor3d_t ang);
void entidades_render(camera_t *cam);
void entidades_destroy();

void entidade_projecao3D(camera_t *cam, entidade_t *ent);

void entidade_inc_frame(int id);
void entidade_dec_anim(int id);
void entidade_inc_anim(int id);

#endif