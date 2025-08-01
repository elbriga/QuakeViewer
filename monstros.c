#include "3d.h"
#include "entidade.h"
#include "monstros.h"
#include "fisica.h"

void monstro_update(mapa_t *mapa, entidade_t *monstro, entidade_t *jogador, float deltaTime)
{
    float dot, cross;
    bool ve = entidade_consegue_ver(mapa, monstro, jogador, &dot, &cross);

    monstro->tempoEstado += deltaTime;

    switch (monstro->estado) {
    case MONSTRO_IDLE:
        monstro->velocidade.x = monstro->velocidade.y = 0;
        if (ve) {
            entidade_set_state(monstro, MONSTRO_VIRANDO);
        }
        break;

    case MONSTRO_VIRANDO:
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
        vetor3d_t olhoJ = entidade_pos_olho(jogador);
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
        if (monstro->tempoEstado > 0.5f) { // meio segundo de ataque
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
