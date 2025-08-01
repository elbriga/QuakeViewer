#include <math.h>

#include "QuakeViewer.h"
#include "3d.h"
#include "fisica.h"
#include "entidade.h"
#include "mapa.h"

void fisica_update_entidade(mapa_t *mapa, entidade_t *ent, float deltaTime)
{
    // aplica gravidade
    if (!ent->noChao)
        ent->velocidade.z -= GRAVIDADE * deltaTime;

    // calcula nova posição pretendida
    vetor3d_t novaPos = ent->posicao;

    novaPos.x += ent->velocidade.x * deltaTime;
    novaPos.y += ent->velocidade.y * deltaTime;
    novaPos.z += ent->velocidade.z * deltaTime;

    // checa colisão com o mundo abaixo
    float altura = 0;
    if (mapa_trace_bsp_chao(mapa, ent->posicao, novaPos, &altura)) {
        // houve colisão com o chão (ou teto, se vel.z > 0)
        if (ent->velocidade.z < 0) {
            // caiu no chão
            ent->posicao.z = altura;
            ent->velocidade.z = 0;
            ent->noChao = 1;
        } else {
            // bateu no teto
            ent->velocidade.z = 0;
        }
    } else {
        // sem colisão, atualiza posição normalmente
        ent->posicao = novaPos;
        ent->noChao = 0;
    }
}
