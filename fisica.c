#include <math.h>

#include "QuakeViewer.h"
#include "3d.h"
#include "fisica.h"
#include "entidade.h"

bool trace_bsp(mapa_t *mapa, vetor3d_t de, vetor3d_t para, float *alturaChao);

void fisica_update_entidade(mapa_t *mapa, entidade_t *ent, float deltaTime)
{
    // aplica gravidade
    if (!ent->noChao)
        ent->velocidade.z -= GRAVIDADE * deltaTime;

    // calcula nova posição pretendida
    vetor3d_t novaPos = ent->posicao;
    novaPos.z += ent->velocidade.z * deltaTime;

    // checa colisão com o mundo abaixo
    float altura = 0;
    if (trace_bsp(mapa, ent->posicao, novaPos, &altura)) {
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

bool ponto_dentro_face_2d(mapa_t *mapa, face_t *face, float px, float py)
{
    int i, *ledge;
    int cruzamentos = 0;

    edge_t      *e;
    vetor3d_t   *v1, *v2;

    ledge = (int *)face->firstledge;
    for (i=0; i<face->numedges; i++, ledge++) {
        if (*ledge >= 0) {
			v1 = &mapa->base[mapa->edges[*ledge].v[0]];
			v2 = &mapa->base[mapa->edges[*ledge].v[1]];
        } else {
			v1 = &mapa->base[mapa->edges[-*ledge].v[1]];
			v2 = &mapa->base[mapa->edges[-*ledge].v[0]];
        }
        
        // projetar os vértices no plano XY
        float x1 = v1->x, y1 = v1->y;
        float x2 = v2->x, y2 = v2->y;

        // checa se o segmento cruza uma linha horizontal à altura py
        if (((y1 <= py) && (y2 > py)) || ((y2 <= py) && (y1 > py))) {
            float t = (py - y1) / (y2 - y1);
            float x_intersec = x1 + t * (x2 - x1);

            if (x_intersec > px)
                cruzamentos++;
        }
    }

    return (cruzamentos % 2) == 1;  // ímpar: dentro
}

// A função checa apenas o eixo Z, e retorna true se colidiu com chão
bool trace_bsp(mapa_t *mapa, vetor3d_t de, vetor3d_t para, float *alturaChao)
{
    int edgeId;

    // percorre faces do mapa (ou use o BSP traversal)
    face_t *face = mapa->faces;
    for (int i = 0; i < mapa->numfaces; i++, face++) {
        
        // if (!face_solida(face)) continue;

        // verifica se a face é plana no eixo Z (horizontal)
        if (fabs(face->plano->normal.x) > 0.1f || fabs(face->plano->normal.y) > 0.1f)
            continue;

        // verifica se houve interseção com o plano da face
        edgeId = (*face->firstledge > 0) ? *face->firstledge : -*face->firstledge;
        float zPlano = mapa->base[mapa->edges[edgeId].v[0]].z; // z constante
        if ((de.z >= zPlano && para.z < zPlano) || (de.z <= zPlano && para.z > zPlano)) {
            // se dentro da projeção da face (XY)
            float px = de.x;  // para simplificar, use a mesma X e Y
            float py = de.y;

            if (ponto_dentro_face_2d(mapa, face, px, py)) {
                *alturaChao = zPlano;
                return true;
            }
        }
    }

    return false;
}
