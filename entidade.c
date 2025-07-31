#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "3d.h"
#include "readMdl.h"
#include "render.h"
#include "entidade.h"
#include "fisica.h"
#include "grafico.h"
#include "monstros.h"

obj3d_t *objBase[MAX_OBJS];
static int totObjs = 0;

entidade_t entidades[MAX_ENTIDADES];
static int totInstances = 0;

extern entidade_t *player;

entidade_t *entidade_get(int id)
{
    return (id < 0 || id >= totInstances) ? NULL : &entidades[id];
}

obj3d_t *obj_get_base(char *modelName)
{
    // Verificar se ja esta carregado
    for(int i=0; i<totObjs; i++)
        if (!strcmp(modelName, objBase[i]->nome))
            return objBase[i];
        
    // Carregar novo obj base
    int idNovo = totObjs++;
    objBase[idNovo] = readMdl(modelName);

    return objBase[idNovo];
}

void entidade_create(char *modelName, vetor3d_t pos, vetor3d_t ang)
{
    if (totInstances >= MAX_ENTIDADES) return; // TODO - erro

    int novoID = totInstances++;
    entidade_t *ent = &entidades[novoID];

    memset(ent, 0, sizeof(entidade_t));

    ent->id = novoID;
    ent->obj = obj_get_base(modelName);
    ent->posicao = pos;
    ent->rotacao = ang;
    ent->vivo = 1;
    ent->estado = MONSTRO_IDLE;

    entidade_set_anim(ent, ent->obj->numAnimIdle);
}

void entidade_set_state(entidade_t *m, entidade_estado_t estado)
{
    m->estado      = estado;
    m->tempoEstado = 0;
}

void entidades_update(mapa_t *mapa, camera_t *cam, float deltaTime)
{
    int i;
    entidade_t *monstro;

    for (i=0; i < totInstances; i++) {
        if (entidades[i].vivo) {
            entidade_inc_frame(i);
        }

        fisica_update_entidade(mapa, &entidades[i], deltaTime);

        if (i > 0) {
            monstro = &entidades[i];

            monstro_update(mapa, monstro, player, deltaTime);
        }
    }
}

void entidades_render(mapa_t *mapa, camera_t *cam)
{
    int i;

    for (i=0; i < totInstances; i++) {
        render_desenha_entidade(cam, &entidades[i]);
    }
}

void entidades_destroy()
{
    for(int i=0; i<totObjs; i++)
        freeObj3D(objBase[i]);
}

void entidades_pula()
{
    int i;

    for (i=0; i < totInstances; i++) {
        entidades[i].posicao.z += 100;
    }
}

vetor3d_t entidade_pos_olho(entidade_t *ent)
{
    // altura dos olhos pode ser no topo ou meio da cabeça
    vetor3d_t ret = ent->posicao;

    // vetor_add(&ret, &ent->obj->posOlho);
    ret.z += 50;

    return ret;
}

bool entidade_consegue_ver(mapa_t *mapa, entidade_t *monstro, entidade_t *jogador, float *dot, float *cross)
{
    vetor3d_t olhoM = entidade_pos_olho(monstro);
    vetor3d_t olhoJ = entidade_pos_olho(jogador);

    vetor3d_t dirPlayer = olhoJ;
    vetor_sub(&dirPlayer, &olhoM);

    float distancia = vetor_length(&dirPlayer);
    if (distancia > 800.0f) return false;  // muito longe

    vetor3d_t frente = angulo_para_direcao(monstro->rotacao.z, 0);  // vetor olhando

    vetor_normalize(&dirPlayer);
    *dot = vetor_dot_product(frente, dirPlayer);
    if (*dot < 0.7f) return false;  // fora do campo de visão (ex: > 45°)

    *cross = frente.x * dirPlayer.y - frente.y * dirPlayer.x;

    return mapa_trace_bsp_visibilidade(mapa, olhoM, olhoJ);
}

void entidade_projecao3D(camera_t *cam, entidade_t *ent)
{
    int         i;
    vetor3d_t   *base;
    ponto_t     *pnt;
    vetor3d_t   *triBase;
    triangulo_t *tri;

    obj3d_t *obj = ent->obj;

    base = &obj->frames[ent->numFrameSel * obj->numverts];
    pnt  = obj->verts;
    for (i=0; i<obj->numverts; i++, base++, pnt++) {
        // Reset - Coordenadas de Objeto
        pnt->rot.x = base->x;
        pnt->rot.y = base->y;
        pnt->rot.z = base->z;

        // Rotacao do objeto - coordenadas de objeto
        vetor_rotacao2DEixoX(&pnt->rot, ent->rotacao.x);
        vetor_rotacao2DEixoY(&pnt->rot, ent->rotacao.y);
        vetor_rotacao2DEixoZ(&pnt->rot, ent->rotacao.z);

        // Coordenadas de Mundo - posicao do objeto e posicao da camera
        pnt->rot.x += ent->posicao.x - cam->pos.x;
        pnt->rot.y += ent->posicao.y - cam->pos.y;
        pnt->rot.z += ent->posicao.z - cam->pos.z + obj->offsetChao;

        // Rotacao de Camera - coordenadas de camera
        vetor_rotacao2DEixoX(&pnt->rot, cam->ang.x);
        vetor_rotacao2DEixoY(&pnt->rot, cam->ang.y);
        vetor_rotacao2DEixoZ(&pnt->rot, cam->ang.z);

        // Projecao para 2D - so depois do clipping
        //grafico_projecao3D(pnt);
    }

    // Projetar as normais das faces
    triBase = &obj->trisnormals[ent->numFrameSel * obj->numtris];
    tri     = obj->tris;
    for (i=0; i<obj->numtris; i++, triBase++, tri++) {
        // Reset - Coordenadas de Objeto
        tri->normal.x = triBase->x;
        tri->normal.y = triBase->y;
        tri->normal.z = triBase->z;

        // Rotacao do objeto - coordenadas de objeto
        vetor_rotacao2DEixoX(&tri->normal, ent->rotacao.x);
        vetor_rotacao2DEixoY(&tri->normal, ent->rotacao.y);
        vetor_rotacao2DEixoZ(&tri->normal, ent->rotacao.z);

        // Rotacao de Camera - coordenadas de camera
        vetor_rotacao2DEixoX(&tri->normal, cam->ang.x);
        vetor_rotacao2DEixoY(&tri->normal, cam->ang.y);
        vetor_rotacao2DEixoZ(&tri->normal, cam->ang.z);

        // Normalização (opcional, mas recomendado)
        vetor_normalize(&tri->normal);
    }
}

void entidade_inc_frame(int id)
{
    entidade_t *ent = &entidades[id];

    ent->numFrameSel++;

    int naSel = (ent->numAnimSel == -1) ? ent->numAnimSelAuto : ent->numAnimSel;
    if (ent->numFrameSel >= ent->obj->framesanims[naSel].frameF) {
        if (ent->numAnimSel == -1) {
            ent->numAnimSelAuto = rand() % ent->obj->totAnims;
            ent->numFrameSel = ent->obj->framesanims[ent->numAnimSelAuto].frameI;
        } else {
            ent->numFrameSel = ent->obj->framesanims[naSel].frameI;
        }
    }
}

void entidade_set_anim(entidade_t *ent, int num)
{
    ent->numAnimSel = num;

    if (ent->numAnimSel < -1)
        ent->numAnimSel = -1;
    else if (ent->numAnimSel >= ent->obj->totAnims)
        ent->numAnimSel = ent->obj->totAnims - 1;
    
    int naSel = (ent->numAnimSel == -1) ? ent->numAnimSelAuto : ent->numAnimSel;
    ent->numFrameSel = ent->obj->framesanims[naSel].frameI;
}

void entidade_dec_anim(int id)
{
    entidade_t *ent = &entidades[id];
    entidade_set_anim(ent, ent->numAnimSel - 1);
}

void entidade_inc_anim(int id)
{
    entidade_t *ent = &entidades[id];
    entidade_set_anim(ent, ent->numAnimSel + 1);
}
