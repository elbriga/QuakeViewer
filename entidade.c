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

obj3d_t *objBaseList = NULL; // Linked list

int proxID = 0;
entidade_t *entList = NULL; // Linked list

extern entidade_t *player;

entidade_t *entidade_get(int id)
{
    entidade_t *ent = entList;

    while (ent) {
        if (ent->id == id)
            return ent;
        ent = ent->next;
    }

    return NULL;
}

obj3d_t *obj_get_base(char *modelName)
{
    obj3d_t **pp = &objBaseList;

    // procurar na lista
    while (*pp) {
        if (!strcmp(modelName, (*pp)->nome))
            return *pp;
        pp = &(*pp)->next;
    }

    // não achou -> cria novo e adiciona no fim
    *pp = readMdl(modelName);
    (*pp)->next = NULL;

    return *pp;
}

void entidade_create(char *modelName, vetor3d_t pos, int ang)
{
    // aloca nova entidade
    entidade_t *nova = malloc(sizeof(entidade_t));
    if (!nova) return; // TODO: tratar erro

    memset(nova, 0, sizeof(entidade_t));

    // inicializa os campos
    nova->id  = proxID++;
    nova->obj = obj_get_base(modelName);

    nova->posicao = pos;
    nova->rotacao = (vetor3d_t){ 270, 0, ang };

    nova->vida = 100;
    if (!strcmp(modelName, "shambler")) {
        nova->vida += 100;
    }

    nova->vivo   = true;
    nova->estado = MONSTRO_IDLE;

    nova->alvo       = 0;
    nova->jaDeuDano  = false;

    entidade_set_anim(nova, nova->obj->numAnimIdle);

    // insere no fim da lista
    nova->next = NULL;
    entidade_t **pp = &entList;

    // acha o último ponteiro da lista (ou a raiz, se estiver vazia)
    while (*pp)
        pp = &(*pp)->next;

    // liga o novo nó
    *pp = nova;
}

void entidade_destroy(entidade_t *ent)
{
    entidade_t **pp = &entList;

    while (*pp) {
        if (*pp == ent) {
            // achou -> remove da lista
            *pp = ent->next;
            free(ent);
            return;
        }
        pp = &(*pp)->next;
    }
}

void entidade_set_state(entidade_t *m, entidade_estado_t estado)
{
    m->estado      = estado;
    m->tempoEstado = 0;

    switch (estado) {
        case MONSTRO_IDLE:
            m->alvo = 0;
            entidade_set_anim(m, m->obj->numAnimIdle);
            break;
        
        case MONSTRO_ANDANDO:
        case MONSTRO_PASSEANDO:
            entidade_set_anim(m, m->obj->numAnimWalk);
            break;
        
        case MONSTRO_ATACANDO: {
            int numAnim = rand() % m->obj->totAnimAttack;
            entidade_set_anim(m, m->obj->numAnimAttack[numAnim]);
        } break;
        
        case MONSTRO_MORTO: {
            int numAnim = rand() % m->obj->totAnimDeath;
            entidade_set_anim(m, m->obj->numAnimDeath[numAnim]);
        } break;
    }
}

void entidades_update(mapa_t *mapa, camera_t *cam, float deltaTime)
{
    entidade_t *ent = entList;
    while (ent) {
        if (ent->vivo) {
            entidade_inc_frame(ent);
        }

        entidade_t *prox = ent->next; // salva antes pois a funcao abaixo pode remover
        
        fisica_update_entidade(mapa, ent, deltaTime);

        if (ent != entList) { // pular o player
            monstro_update(mapa, ent, deltaTime);
        }

        if (ent->posicao.z < -9999.0f) {
            // caiu no void!
            printf("caiu no void!\n");
            entidade_destroy(ent);
        }

        ent = prox;
    }
}

void entidades_render(mapa_t *mapa, camera_t *cam)
{
    entidade_t *ent = entList;
    while (ent) {
        render_desenha_entidade(cam, ent);
        ent = ent->next;
    }
}

void entidades_destroy()
{
    obj3d_t *obj = objBaseList, *next;
    while(obj) {
        next = obj->next;
        freeObj3D(obj);
        obj = next;
    }

    entidade_t *ent = entList, *prox;
    while (ent) {
        prox = ent->next;
        free(ent);
        ent = prox;
    }
}

void entidades_pula()
{
    entidade_t *ent = entList;
    while (ent) {
        ent->posicao.z += 100;
        ent = ent->next;
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

bool entidade_tem_chao_a_frente(mapa_t *mapa, entidade_t *ent)
{
    // direção olhando no plano XY
    vetor3d_t frente = angulo_para_direcao(ent->rotacao.z, 0);

    // ponto à frente, mas na mesma altura
    vetor3d_t pontoTeste = {
        ent->posicao.x + frente.x * 24.0f, // distância à frente
        ent->posicao.y + frente.y * 24.0f,
        ent->posicao.z + 24.0f             // um pouco acima dos pés
    };

    // desce bastante para procurar piso
    vetor3d_t pontoFim = {
        pontoTeste.x,
        pontoTeste.y,
        pontoTeste.z - 128.0f
    };

    float alturaChao;
    if (mapa_trace_bsp_chao(mapa, pontoTeste, pontoFim, &alturaChao)) {
        // existe piso visível à frente
        float delta = alturaChao - ent->posicao.z;
        return (delta > -STEP_SIZE && delta < STEP_SIZE);
    }

    // nada encontrado = sem chão
    return false;
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

void entidade_inc_frame(entidade_t *ent)
{
    ent->numFrameSel++;

    int naSel = (ent->numAnimSel == -1) ? ent->numAnimSelAuto : ent->numAnimSel;
    if (ent->numFrameSel >= ent->obj->framesanims[naSel].frameF) {
        if (ent->numAnimSel == -1) {
            ent->numAnimSelAuto = rand() % ent->obj->totAnims;
            ent->numFrameSel = ent->obj->framesanims[ent->numAnimSelAuto].frameI;
        } else {
            if (!ent->vida) {
                // fim da animacao de morte
                ent->vivo = 0;
            } else {
                ent->numFrameSel = ent->obj->framesanims[naSel].frameI;
            }
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

void entidade_dec_anim(entidade_t *ent)
{
    entidade_set_anim(ent, ent->numAnimSel - 1);
}

void entidade_inc_anim(entidade_t *ent)
{
    entidade_set_anim(ent, ent->numAnimSel + 1);
}

void entidade_aplica_dano(entidade_t *origem, entidade_t *alvo, int dano)
{
    if (alvo->vida <= 0) return;

    if (!strcmp(origem->obj->nome, "shambler")) {
        dano = dano * 2;
    }

    alvo->vida -= dano;
    if (alvo->vida <= 0) {
        alvo->vida = 0;
        entidade_set_state(alvo, MONSTRO_MORTO);
    }

    // Revidar!
    if (alvo->alvo == 0) {
        alvo->alvo = origem;
        entidade_set_state(alvo, MONSTRO_VIRANDO);
    }
}
