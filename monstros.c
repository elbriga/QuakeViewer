#include <stdio.h>
#include <stdlib.h>

#include "3d.h"
#include "entidade.h"
#include "monstros.h"
#include "fisica.h"

extern entidade_t entidades[MAX_ENTIDADES];
extern int totInstances;

void monstro_update(mapa_t *mapa, entidade_t *monstro, float deltaTime)
{
    bool ve;
    float dot, cross;
    entidade_t *outroMonstro;

    monstro->tempoEstado += deltaTime;

    switch (monstro->estado) {
    case MONSTRO_IDLE:
        monstro->velocidade.x = monstro->velocidade.y = 0;

        // Buscar um Alvo!
        for (int i=1; i<totInstances; i++) {
            if (i == monstro->id) continue;

            outroMonstro = &entidades[i];
            
            if (!outroMonstro->vida) continue;

            ve = entidade_consegue_ver(mapa, monstro, outroMonstro, &dot, &cross);
            if (ve) {
                // Travar no alvo
                monstro->alvo = outroMonstro;
                entidade_set_state(monstro, MONSTRO_VIRANDO);
            }
        }
        break;

    case MONSTRO_VIRANDO:
        ve = entidade_consegue_ver(mapa, monstro, monstro->alvo, &dot, &cross);
        if (!ve) {
            entidade_set_state(monstro, MONSTRO_IDLE);
            break;
        }
        if (dot < 0.97f) {
            float velRot = 120.0f * deltaTime;
            if (cross > 0) monstro->rotacao.z += velRot;
            else           monstro->rotacao.z -= velRot;
        } else {
            entidade_set_state(monstro, MONSTRO_ANDANDO);
        }
        break;

    case MONSTRO_ANDANDO:
        ve = entidade_consegue_ver(mapa, monstro, monstro->alvo, &dot, &cross);
        if (!ve) {
            entidade_set_state(monstro, MONSTRO_IDLE);
            break;
        }

        // checar se há piso antes de andar
        if (!entidade_tem_chao_a_frente(mapa, monstro)) {
            monstro->velocidade.x = monstro->velocidade.y = 0;
            entidade_set_state(monstro, MONSTRO_IDLE); // para na borda
            break;
        }

        // calcula distância ao player
        vetor3d_t olhoM = entidade_pos_olho(monstro);
        vetor3d_t olhoJ = entidade_pos_olho(monstro->alvo);
        float dx = olhoJ.x - olhoM.x;
        float dy = olhoJ.y - olhoM.y;
        float dist2 = dx*dx + dy*dy;

        if (dist2 < (64*64)) {
            entidade_set_state(monstro, MONSTRO_ATACANDO);
            monstro->velocidade.x = monstro->velocidade.y = 0;
            break;
        }

        // anda na direção
        {
            vetor3d_t frente = angulo_para_direcao(monstro->rotacao.z, 0);
            float vel = 100.0f;
            monstro->velocidade.x = frente.x * vel;
            monstro->velocidade.y = frente.y * vel;
        }

        monstro_ajusta_chao(mapa, monstro);
        break;

    case MONSTRO_ATACANDO:
        monstro->velocidade.x = monstro->velocidade.y = 0;

        if (monstro->tempoEstado > 0.25f && !monstro->jaDeuDano) {
            // no meio da animação do ataque → aplica dano uma vez
            int forca = 5 + (rand() % 10);
            entidade_aplica_dano(monstro, monstro->alvo, forca);
            monstro->jaDeuDano = true;

            if (!monstro->alvo->vida) {
                // Matou!
                entidade_set_state(monstro, MONSTRO_IDLE);
                break;
            }
        }
        
        if (monstro->tempoEstado > 0.5f) { // meio segundo de ataque
            monstro->jaDeuDano = false;
            entidade_set_state(monstro, MONSTRO_ANDANDO);
        }
        break;
    }
}   

void monstro_ajusta_chao(mapa_t *mapa, entidade_t *ent)
{
    // Posição base atual
    vetor3d_t base = ent->posicao;
    base.z += STEP_SIZE;

    // Posição desejada (XY atual, mas descendo bastante em Z)
    vetor3d_t fim = { base.x, base.y, base.z - 128.0f };

    float alturaChao;
    if (mapa_trace_bsp_chao(mapa, base, fim, &alturaChao)) {
        float delta = alturaChao - base.z;

        if (delta > -STEP_SIZE && delta < STEP_SIZE) {
            // ajusta suavemente pro novo piso
            ent->posicao.z = alturaChao;
            ent->noChao = 1;
            ent->velocidade.z = 0;
        }
    }
}

void monstro_novo(mapa_t *mapa)
{
    char models[5][16] = {
        "enforcer",
        "hknight",
        "ogre",
        "shambler",
        "zombie"
    };

    // Achar uma Posicao
    vetor3d_t pos = mapa_acha_posicao_spawn(mapa);
    if (!pos.x && !pos.y && !pos.z) {
        printf("Nao achei posicao!\n\n");
        return;
    }

    int modelNum = rand() % 5;
    entidade_create(models[modelNum], pos, rand() % 360);
}
