#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "3d.h"
#include "entidade.h"
#include "monstros.h"
#include "mapa.h"
#include "fisica.h"

extern entidade_t entidades[MAX_ENTIDADES];
extern int totInstances;

void monstro_busca_alvo(entidade_t *monstro, mapa_t *mapa)
{
    bool ve;
    float dot, cross;
    entidade_t *outroMonstro;

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
}

void monstro_update(mapa_t *mapa, entidade_t *monstro, float deltaTime)
{
    bool ve;
    float dot, cross;
    entidade_t *outroMonstro;

    monstro->tempoEstado += deltaTime;

    switch (monstro->estado) {
    case MONSTRO_IDLE:
        monstro->velocidade.x = monstro->velocidade.y = 0;

        // chance de começar a andar após ficar parado
        if (monstro->tempoEstado > 2.0f) { // parado por 2s
            if ((rand() % 100) < 10) {   // 10% de chance a cada frame
                monstro->direcaoIdle = (float)(rand() % 360); // ângulo aleatório
                monstro->tempoAndando = 111.0f + (rand() % 900) / 100.0f; // 1.0 a 10.0s
                entidade_set_state(monstro, MONSTRO_PASSEANDO_VIRANDO);
            }
        }

        monstro_busca_alvo(monstro, mapa);
        break;
    
    case MONSTRO_PASSEANDO_VIRANDO: {
        monstro->velocidade.x = monstro->velocidade.y = 0;

        float atual = monstro->rotacao.z;
        float alvo  = monstro->direcaoIdle;
        float step  = 60.0f * deltaTime; // velocidade de rotação

        // Normaliza ângulos para [0, 360)
        while (atual < 0)     atual += 360;
        while (atual >= 360)  atual -= 360;
        while (alvo < 0)      alvo += 360;
        while (alvo >= 360)   alvo -= 360;

        // Calcula diferença entre ângulos
        float diff = alvo - atual;

        // Ajusta para menor ângulo (-180 a 180)
        if (diff > 180)  diff -= 360;
        if (diff < -180) diff += 360;

        if (fabsf(diff) < 15.0f) {
            monstro->rotacao.z = alvo;
            entidade_set_state(monstro, MONSTRO_PASSEANDO);
        } else {
            monstro->rotacao.z += (diff > 0 ? step : -step);
        }

        // procura inimigo durante o passeio também
        monstro_busca_alvo(monstro, mapa);
    }
    break;

    
    case MONSTRO_PASSEANDO: {
        vetor3d_t frente = angulo_para_direcao(monstro->direcaoIdle, 0);
        float vel = 60.0f; // mais lento que perseguindo

        monstro->velocidade.x = frente.x * vel;
        monstro->velocidade.y = frente.y * vel;

        monstro->tempoAndando -= deltaTime;

        // se acabou o tempo ou encontrou obstáculo → volta pro idle
        if (monstro->tempoAndando <= 0 || !entidade_tem_chao_a_frente(mapa, monstro)) {
            entidade_set_state(monstro, MONSTRO_IDLE);
            break;
        }

        // procura inimigo durante o passeio também
        monstro_busca_alvo(monstro, mapa);
    } break;

    case MONSTRO_VIRANDO:
        ve = entidade_consegue_ver(mapa, monstro, monstro->alvo, &dot, &cross);
        if (monstro->tempoEstado > 4.0f && !ve) {
            entidade_set_state(monstro, MONSTRO_IDLE);
            break;
        }
        if (dot < 0.94f) {
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

        // calcula distância ao alvo
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
    char models[10][16] = {
        "demon",
        "dog",
        "enforcer",
        "hknight",
        "knight",
        "ogre",
        "shambler",
        "soldier",
        "tarbaby",
        "zombie"
    };

    // Achar uma Posicao
    vetor3d_t pos = mapa_acha_posicao_spawn(mapa);
    if (!pos.x && !pos.y && !pos.z) {
        printf("Nao achei posicao!\n\n");
        return;
    }

    int modelNum = rand() % 10;
    entidade_create(models[modelNum], pos, rand() % 360);
}
