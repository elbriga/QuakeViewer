#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include "3d.h"
#include "gfx.h"
#include "grafico.h"
#include "render.h"
#include "mapa.h"

#define MAX_VERTS_POR_POLIGONO 16
#define NEAR_Z 0.1f
#define FAR_CLIP 500.0

int facesRendered;

extern int _debug;
extern int _lightON;
extern int _showRendering;

extern float tempo_de_jogo;

int render_clip_near_face(
    ponto_t *in[MAX_VERTS_POR_POLIGONO],
    int in_count,
    ponto_t out[MAX_VERTS_POR_POLIGONO * 2]
) {
    int out_count = 0;

    for (int i = 0; i < in_count; i++) {
        ponto_t *cur = in[i];
        ponto_t *prev = in[(i - 1 + in_count) % in_count];

        float d_cur  = cur->rot.z - NEAR_Z;
        float d_prev = prev->rot.z - NEAR_Z;

        int inside_cur  = d_cur  >= 0.0f;
        int inside_prev = d_prev >= 0.0f;

        if (inside_cur) {
            if (!inside_prev) {
                // Interseção entrada — calcula ponto novo
                float t = d_prev / (d_prev - d_cur);

                ponto_t p;

                // Interpolação linear para posição
                p.rot.x = prev->rot.x + t * (cur->rot.x - prev->rot.x);
                p.rot.y = prev->rot.y + t * (cur->rot.y - prev->rot.y);
                p.rot.z = NEAR_Z;

                // Correção de perspectiva (1/z)
                float inv_z1 = 1.0f / prev->rot.z;
                float inv_z2 = 1.0f / cur->rot.z;

                float u1_corr = prev->tex.x * inv_z1;
                float v1_corr = prev->tex.y * inv_z1;
                float u2_corr = cur->tex.x * inv_z2;
                float v2_corr = cur->tex.y * inv_z2;

                float inv_z = inv_z1 + t * (inv_z2 - inv_z1);
                float u_corr = u1_corr + t * (u2_corr - u1_corr);
                float v_corr = v1_corr + t * (v2_corr - v1_corr);

                p.tex.x = u_corr / inv_z;
                p.tex.y = v_corr / inv_z;

                out[out_count++] = p;
            }

            // Ponto atual é visível — copia direto
            out[out_count++] = *cur;
        }
        else if (inside_prev) {
            // Interseção saída — calcula ponto novo
            float t = d_prev / (d_prev - d_cur);

            ponto_t p;

            p.rot.x = prev->rot.x + t * (cur->rot.x - prev->rot.x);
            p.rot.y = prev->rot.y + t * (cur->rot.y - prev->rot.y);
            p.rot.z = NEAR_Z;

            float inv_z1 = 1.0f / prev->rot.z;
            float inv_z2 = 1.0f / cur->rot.z;

            float u1_corr = prev->tex.x * inv_z1;
            float v1_corr = prev->tex.y * inv_z1;
            float u2_corr = cur->tex.x * inv_z2;
            float v2_corr = cur->tex.y * inv_z2;

            float inv_z = inv_z1 + t * (inv_z2 - inv_z1);
            float u_corr = u1_corr + t * (u2_corr - u1_corr);
            float v_corr = v1_corr + t * (v2_corr - v1_corr);

            p.tex.x = u_corr / inv_z;
            p.tex.y = v_corr / inv_z;

            out[out_count++] = p;
        }
    }

    return out_count;
}

void render_desenha_instance(camera_t *cam, instance_t *inst)
{
	obj3d_t *obj = inst->obj;
	
	ponto_t	*verts[MAX_VERTS_POR_POLIGONO];
	ponto_t  clipped[MAX_VERTS_POR_POLIGONO * 2];
    ponto_t *clipped_ptrs[MAX_VERTS_POR_POLIGONO * 2];

	vetor3d_t	viewDir;
	triangulo_t	*tri;
	skinvert_t	*svxt1, *svxt2, *svxt3;

	int i, j, clipped_count;
	// int objFacesRendered = 0;

	instance_projecao3D(cam, inst);

	for (i=0, tri = obj->tris; i<obj->numtris; i++, tri++) {
		// Pega os vértices do triângulo
		verts[0] = &obj->verts[tri->v[0]];
		verts[1] = &obj->verts[tri->v[1]];
		verts[2] = &obj->verts[tri->v[2]];

		// Backface culling
		// if (tri->normal.z < 0) {
		// 	// continue;
		// }
		// Calcula o centro da face (em espaço da câmera)
		viewDir.x = (verts[0]->rot.x + verts[1]->rot.x + verts[2]->rot.x) / 3.0f;
		viewDir.y = (verts[0]->rot.y + verts[1]->rot.y + verts[2]->rot.y) / 3.0f;
		viewDir.z = (verts[0]->rot.z + verts[1]->rot.z + verts[2]->rot.z) / 3.0f;
		normalize(&viewDir); // opcional, mas bom para estabilidade
		// Dot product entre normal da face e viewDir
		if (dot_product(tri->normal, viewDir) >= 0) {
			continue; // Culling: está de costas
		}

		// coordenadas de textura
		svxt1 = &obj->skinmap[tri->v[0]];
		svxt2 = &obj->skinmap[tri->v[1]];
		svxt3 = &obj->skinmap[tri->v[2]];
		verts[0]->tex.x = svxt1->s;
		verts[0]->tex.y = svxt1->t;
		verts[1]->tex.x = svxt2->s;
		verts[1]->tex.y = svxt2->t;
		verts[2]->tex.x = svxt3->s;
		verts[2]->tex.y = svxt3->t;
		if (!tri->isFront) {
			if (svxt1->onseam) verts[0]->tex.x += 0.5;
			if (svxt2->onseam) verts[1]->tex.x += 0.5;
			if (svxt3->onseam) verts[2]->tex.x += 0.5;
		}

		// Faz o clipping contra o plano NEAR
		clipped_count = render_clip_near_face(verts, 3, clipped);
		if (clipped_count < 3) continue;

		// Projeta os vértices válidos
		for (j=0; j < clipped_count; j++) {
			grafico_projecao3D(&clipped[j]);
			clipped_ptrs[j] = &clipped[j];
		}

		grafico_desenha_poligono(clipped_ptrs, clipped_count, &obj->texture, NULL,0,0);
		// objFacesRendered++;
	}

	// printf(" obj[%s]fr[%d de %d]", obj->nome, objFacesRendered, obj->numtris);
}

int render_desenhaFace(face_t *face, mapa_t *mapa)
{
	ponto_t	*verts[MAX_VERTS_POR_POLIGONO];
	ponto_t  clipped[MAX_VERTS_POR_POLIGONO * 2];
    ponto_t *clipped_ptrs[MAX_VERTS_POR_POLIGONO * 2];

	edge_t			*edge;
    int				*ledge, vxtNum, s, t;
	vetor3d_t		*vBase;

	face->drawn = 1;

	if (face->numedges > MAX_VERTS_POR_POLIGONO) {
		//printf("face[%d] > numEgdes %d muito grande! ", i, face->numedges);
		return 1;
	}

	if (face->texinfo->miptex == mapa->numTextureTrigger)
		return 2;

	ledge = (int *)face->firstledge;
	for (int v=0; v < face->numedges; v++, ledge++) {
		if (*ledge < 0) {
			edge = (edge_t *)&mapa->edges[-*ledge];
			vxtNum = edge->v[1];
		} else {
			edge = (edge_t *)&mapa->edges[*ledge];
			vxtNum = edge->v[0];
		}
		verts[v] = &mapa->verts[vxtNum];

		vBase = &mapa->base[vxtNum];
		float s = dot_product(*vBase, face->texinfo->vetorS) + face->texinfo->distS;
		float t = dot_product(*vBase, face->texinfo->vetorT) + face->texinfo->distT;

		// Coordenadas da textura (normalizadas 0–1)
		verts[v]->tex.x = s / face->texture->width;
		verts[v]->tex.y = t / face->texture->height;

		// Coordenadas do lightmap (em escala de texel)
		verts[v]->tex_luz.x = (s - face->light_mins_s) / face->light_width;//(s / 16.0f) - face->light_mins_s;
		verts[v]->tex_luz.y = (t - face->light_mins_t) / face->light_height;//(t / 16.0f) - face->light_mins_t;
	}
//dbg
// if (i != 100) continue;

	// Faz o clipping contra o plano NEAR
	int clipped_count = render_clip_near_face(verts, face->numedges, clipped);
	if (clipped_count < 3) return 3;

	// Projeta os vértices válidos
	for (int v = 0; v < clipped_count; v++) {
		grafico_projecao3D(&clipped[v]);
		clipped_ptrs[v] = &clipped[v];
	}

if (_debug)
printf("\nFace{lW:%d-lH:%d}{minsS:%d-minsT:%d}[S:%.1f,%.1f,%.1f+%.1f--T:%.1f,%.1f,%.1f+%.1f]\n",
		face->light_width, face->light_height, face->light_mins_s, face->light_mins_t,
		face->texinfo->vetorS.x, face->texinfo->vetorS.y, face->texinfo->vetorS.z, face->texinfo->distS,
		face->texinfo->vetorT.x, face->texinfo->vetorT.y, face->texinfo->vetorT.z, face->texinfo->distT
);

	if (strncmp(face->texture->name, "sky", 3) == 0) {
		// Use função especial para céu
		grafico_desenha_poligono_sky(clipped_ptrs, clipped_count,
			face->texture, tempo_de_jogo);
	} else {
		grafico_desenha_poligono(clipped_ptrs, clipped_count, face->texture, 
			_lightON ? face->light : NULL, face->light_width, face->light_height);
	}

	return 0;
}

void render_desenha_node(node_t *node, mapa_t *mapa, camera_t *cam, char *vis)
{
    if (node->contents < 0) {
		// É umm leaf
        leaf_t *leaf = (leaf_t *)node;

        // Verifica se o leaf está visível
        if (!(vis[leaf->key >> 3] & (1 << (leaf->key & 7))))
			return;

        // Render LEAF
		int		j;
		face_t	*face, **mark;
		for (j=0, mark = leaf->firstmarksurface; j < leaf->nummarksurfaces; j++, mark++) {
			face = *mark;

			if (!face->drawn && // ignore dups
				((face->plano->ON && !face->side) || (!face->plano->ON && face->side)) && // backface culling
				(!_debug || _debug == face->id)) { // debug

					if (!render_desenhaFace(face, mapa)) {
						facesRendered++;

						if (_showRendering) gfx_flush();
					}
			}
		}
        return;
    }

    // Nó interno
    plano_t *plano = node->plane;

    // Distância da câmera ao plano do nó
    float dist = plano->normal.x * cam->pos.x +
                 plano->normal.y * cam->pos.y +
                 plano->normal.z * cam->pos.z - plano->dist;

    int frente = (dist >= 0) ? 0 : 1;
    int tras = 1 - frente;

	// travessia da arvore BSP INVERTIDA! Para usar o zBuffer e evitar pixels
    // Depois desenha o lado mais próximo
    render_desenha_node(node->children[frente], mapa, cam, vis);

    // Primeiro desenha o lado mais distante (painters algo)
    render_desenha_node(node->children[tras], mapa, cam, vis);
}

void render_desenha_mapa(camera_t *cam, mapa_t *mapa)
{
	int		 i, bspON = 1;
	leaf_t	*leafCAM;
	face_t	*face;
	byte	*vis;

	mapa_projecao3D(cam, mapa);

	leafCAM = mapa_discoverLeaf(&cam->pos, mapa);
	printf("L:%d ", leafCAM->visofs);

	vis = mapa_leafVIS(leafCAM, mapa);

	for (i=0, face = mapa->faces; i < mapa->numfaces; i++, face++)
		face->drawn = 0;
	
	facesRendered = 0;

	if (bspON) {
		// Iniciar a recursao pela arvore BSP
		render_desenha_node(mapa->nodes, mapa, cam, vis);
	} else {
		int j;
		face_t **mark;
		leaf_t *leaf = &mapa->leafs[1];

		for (i=0; i < mapa->numleafs - 1; i++, leaf++) {
			if (!(vis[i >> 3] & (1 << (i & 7)))) {
				// LEAF nao visivel
				continue;
			}

			// Render LEAF
			for (j=0, mark = leaf->firstmarksurface; j < leaf->nummarksurfaces; j++, mark++) {
				face = *mark;

				if (!face->drawn && // ignore dups
					((face->plano->ON && !face->side) || (!face->plano->ON && face->side)) && // backface culling
					(!_debug || _debug == face->id)) { // debug

						if (!render_desenhaFace(face, mapa)) {
							facesRendered++;
						}
				}
			}
		}
	}

	printf(" facesRender[%d de %d]", facesRendered, mapa->numfaces);

	if (!_showRendering)
		mostraMapa2D(mapa, cam, vis);

	// face = mapa->faces;
	// for (int i=0; i < mapa->numfaces; i++, face++) {
	// 	render_desenhaFace(face, mapa, paleta);
	// }
}
