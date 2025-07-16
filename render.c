#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "3d.h"
#include "gfx_ptBR.h"
#include "render.h"
#include "mapa.h"

#define MAX_VERTS_POR_POLIGONO 16
#define NEAR_Z 0.1f
#define FAR_CLIP 500.0

extern int _debug;

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
                float z1 = prev->rot.z;
                float z2 = cur->rot.z;
                float inv_z1 = 1.0f / z1;
                float inv_z2 = 1.0f / z2;

                float u1 = prev->tex.x;
                float v1 = prev->tex.y;
                float u2 = cur->tex.x;
                float v2 = cur->tex.y;

                float u1_corr = u1 * inv_z1;
                float v1_corr = v1 * inv_z1;
                float u2_corr = u2 * inv_z2;
                float v2_corr = v2 * inv_z2;

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

            float z1 = prev->rot.z;
            float z2 = cur->rot.z;
            float inv_z1 = 1.0f / z1;
            float inv_z2 = 1.0f / z2;

            float u1 = prev->tex.x;
            float v1 = prev->tex.y;
            float u2 = cur->tex.x;
            float v2 = cur->tex.y;

            float u1_corr = u1 * inv_z1;
            float v1_corr = v1 * inv_z1;
            float u2_corr = u2 * inv_z2;
            float v2_corr = v2 * inv_z2;

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

void render_desenha_objeto(camera_t *cam, obj3d_t *obj, int numFrameSel, char paleta[256][3])
{
	ponto_t	*verts[MAX_VERTS_POR_POLIGONO];
	ponto_t  clipped[MAX_VERTS_POR_POLIGONO * 2];
    ponto_t *clipped_ptrs[MAX_VERTS_POR_POLIGONO * 2];

	texture_t texture;

	obj_projecao3D(cam, obj, numFrameSel);

	texture.data   = obj->skin;
	texture.width  = obj->skinwidth;
	texture.height = obj->skinheight;

	triangulo_t *tri = obj->tris;
	for (int cnt_tri=0; cnt_tri<obj->numtris; cnt_tri++, tri++) {
		// Backface culling
		if (tri->normal.z < 0) {
			continue;
		}

		verts[0] = &obj->verts[tri->v[0]];
		verts[1] = &obj->verts[tri->v[1]];
		verts[2] = &obj->verts[tri->v[2]];

		skinvert_t *svxt1 = &obj->skinmap[tri->v[0]];
		skinvert_t *svxt2 = &obj->skinmap[tri->v[1]];
		skinvert_t *svxt3 = &obj->skinmap[tri->v[2]];

		float skinX1 = (float)svxt1->s / obj->skinwidth;
		float skinY1 = (float)svxt1->t / obj->skinheight;
		float skinX2 = (float)svxt2->s / obj->skinwidth;
		float skinY2 = (float)svxt2->t / obj->skinheight;
		float skinX3 = (float)svxt3->s / obj->skinwidth;
		float skinY3 = (float)svxt3->t / obj->skinheight;

		if (!tri->isFront) {
			if (svxt1->onseam) skinX1 += 0.5;
			if (svxt2->onseam) skinX2 += 0.5;
			if (svxt3->onseam) skinX3 += 0.5;
		}

		verts[0]->tex.x = skinX1;
		verts[0]->tex.y = skinY1;
		verts[1]->tex.x = skinX2;
		verts[1]->tex.y = skinY2;
		verts[2]->tex.x = skinX3;
		verts[2]->tex.y = skinY3;

		// Faz o clipping contra o plano NEAR
		int clipped_count = render_clip_near_face(verts, 3, clipped);
		if (clipped_count < 3) return;

		// Projeta os vértices válidos
		for (int v = 0; v < clipped_count; v++) {
			grafico_projecao3D(&clipped[v]);
			clipped_ptrs[v] = &clipped[v];
		}

		grafico_desenha_poligono(clipped_ptrs, clipped_count, &texture, paleta);
	}
}

int render_desenhaFace(face_t *face, mapa_t *mapa, char paleta[256][3])
{
	ponto_t	*verts[MAX_VERTS_POR_POLIGONO];
	ponto_t  clipped[MAX_VERTS_POR_POLIGONO * 2];
    ponto_t *clipped_ptrs[MAX_VERTS_POR_POLIGONO * 2];

	textureinfo_t	*texinfo;
	texture_t		*tex;
	edge_t			*edge;
    int				*ledge, vxtNum, s, t;
	vetor3d_t		*vBase;
	float            dist;

	face->drawn = 1;

	if (face->numedges > MAX_VERTS_POR_POLIGONO) {
		//printf("face[%d] > numEgdes %d muito grande! ", i, face->numedges);
		return 1;
	}

	// Backface culling
	// if (tri->normal.z < 0) {
	// 	continue;
	// }

	texinfo = &mapa->texinfo[face->texinfo];
	if (texinfo->miptex == mapa->numTextureTrigger) return 2;

	tex = &mapa->textures[texinfo->miptex];

	ledge = (int *)&mapa->ledges[face->firstedge];
	grafico_cor(255,255,255);
	dist = 0.0;
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
		verts[v]->tex.x = (dot_product(*vBase, texinfo->vetorS) + texinfo->distS) / tex->width;
		verts[v]->tex.y = (dot_product(*vBase, texinfo->vetorT) + texinfo->distT) / tex->height;
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

	grafico_desenha_poligono(clipped_ptrs, clipped_count, tex, paleta);

	return 0;
}

void render_desenha_mapa(camera_t *cam, mapa_t *mapa, char paleta[256][3])
{
	int		 i, j, facesRendered = 0;
	leaf_t	*leafCAM, *leaf;
	face_t	*face, **mark;
	byte	*vis;

	mapa_projecao3D(cam, mapa);

	leafCAM = mapa_discoverLeaf(&cam->pos, mapa);
	printf("L:%d ", leafCAM->visofs);

	vis = mapa_leafVIS(leafCAM, mapa);

	for (i=0, face = mapa->faces; i < mapa->numfaces; i++, face++)
		face->drawn = 0;

	leaf = &mapa->leafs[1];
	for (i=0; i < mapa->numleafs - 1; i++, leaf++) {
		if (!(vis[i >> 3] & (1 << (i & 7)))) {
			// LEAF nao visivel
			continue;
		}

		// Render LEAF
		for (j=0, mark = leaf->firstmarksurface; j < leaf->nummarksurfaces; j++, mark++) {
			face = *mark;

			if (!face->drawn && (!_debug || _debug == face->id)) {
				if (!render_desenhaFace(face, mapa, paleta)) {
					facesRendered++;
				}
			}
		}
	}

	printf(" facesRender[%d de %d]", facesRendered, mapa->numfaces);

	mostraMapa2D(mapa, cam, vis);

	// face = mapa->faces;
	// for (int i=0; i < mapa->numfaces; i++, face++) {
	// 	render_desenhaFace(face, mapa, paleta);
	// }
}
