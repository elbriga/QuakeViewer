#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "3d.h"
#include "gfx_ptBR.h"
#include "render.h"

void grafico_desenha_objeto(camera_t *cam, obj3d_t *obj, int numFrameSel, char paleta[256][3])
{
	int meiaSkin = obj->skinwidth / 2;

	obj_projecao3D(cam, obj, numFrameSel);

	triangulo_t *tri = obj->tris;
	for (int cnt_tri=0; cnt_tri<obj->numtris; cnt_tri++, tri++) {
		// Backface culling
		if (tri->normal.z < 0) {
			continue;
		}

		ponto_t *vertice1 = &obj->verts[tri->v[0]];
		ponto_t *vertice2 = &obj->verts[tri->v[1]];
		ponto_t *vertice3 = &obj->verts[tri->v[2]];
		switch (obj->tipo)
		{
		case OBJ_TIPO_WIRE:
			grafico_triangulo_wireZ(
				vertice1->screen.x, vertice1->screen.y, vertice1->rot.z,
				vertice2->screen.x, vertice2->screen.y, vertice2->rot.z,
				vertice3->screen.x, vertice3->screen.y, vertice3->rot.z);
			break;

		case OBJ_TIPO_FLAT:
			grafico_triangulo(
				vertice1->screen.x, vertice1->screen.y, vertice1->rot.z,
				vertice2->screen.x, vertice2->screen.y, vertice2->rot.z,
				vertice3->screen.x, vertice3->screen.y, vertice3->rot.z,
				tri->cor.r, tri->cor.g, tri->cor.b);
			break;

		case OBJ_TIPO_TEXTURE:
			skinvert_t *svxt1 = &obj->skinmap[tri->v[0]];
			skinvert_t *svxt2 = &obj->skinmap[tri->v[1]];
			skinvert_t *svxt3 = &obj->skinmap[tri->v[2]];

			int skinX1 = svxt1->s;
			int skinY1 = svxt1->t;
			int skinX2 = svxt2->s;
			int skinY2 = svxt2->t;
			int skinX3 = svxt3->s;
			int skinY3 = svxt3->t;

			if (!tri->isFront) {
				if (svxt1->onseam) skinX1 += meiaSkin;
				if (svxt2->onseam) skinX2 += meiaSkin;
				if (svxt3->onseam) skinX3 += meiaSkin;
			}

			grafico_triangulo_textura(obj->skin, obj->skinwidth, obj->skinheight, paleta,
				vertice1->screen.x, vertice1->screen.y, vertice1->rot.z, skinX1,skinY1,
				vertice2->screen.x, vertice2->screen.y, vertice2->rot.z, skinX2,skinY2,
				vertice3->screen.x, vertice3->screen.y, vertice3->rot.z, skinX3,skinY3);
			break;
		
		default:
			printf("OBJ tipo desconhecido");
			break;
		}
		
	}
}

#define MAX_VERTS_POR_POLIGONO 16
#define NEAR_Z 0.1f
#define FAR_CLIP 500.0

int mapa_clip_near_face(
    ponto_t *in[MAX_VERTS_POR_POLIGONO],
    int in_count,
    ponto_t out[MAX_VERTS_POR_POLIGONO * 2]
) {
    int out_count = 0;

    for (int i = 0; i < in_count; i++) {
        ponto_t *cur = in[i];
        ponto_t *prev = in[(i - 1 + in_count) % in_count];

        float d_cur = cur->rot.z - NEAR_Z;
        float d_prev = prev->rot.z - NEAR_Z;

        int inside_cur = d_cur >= 0;
        int inside_prev = d_prev >= 0;

        if (inside_cur) {
            if (!inside_prev) {
                float t = d_prev / (d_prev - d_cur);
                ponto_t p;
                p.rot.x = prev->rot.x + t * (cur->rot.x - prev->rot.x);
                p.rot.y = prev->rot.y + t * (cur->rot.y - prev->rot.y);
                p.rot.z = NEAR_Z;

                p.tex.x = prev->tex.x + t * (cur->tex.x - prev->tex.x);
                p.tex.y = prev->tex.y + t * (cur->tex.y - prev->tex.y);

                out[out_count++] = p;
            }
            out[out_count++] = *cur;
        } else if (inside_prev) {
            float t = d_prev / (d_prev - d_cur);
            ponto_t p;
            p.rot.x = prev->rot.x + t * (cur->rot.x - prev->rot.x);
            p.rot.y = prev->rot.y + t * (cur->rot.y - prev->rot.y);
            p.rot.z = NEAR_Z;

            p.tex.x = prev->tex.x + t * (cur->tex.x - prev->tex.x);
            p.tex.y = prev->tex.y + t * (cur->tex.y - prev->tex.y);

            out[out_count++] = p;
        }
    }

    return out_count;
}

void grafico_desenha_mapa(camera_t *cam, mapa_t *mapa, char paleta[256][3])
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

	mapa_projecao3D(cam, mapa);


	face_t *face = mapa->faces;
	for (int i=0; i < mapa->numfaces; i++, face++) {
		if (face->numedges > MAX_VERTS_POR_POLIGONO) {
			//printf("face[%d] > numEgdes %d muito grande! ", i, face->numedges);
			continue;
		}

		// Backface culling
		// if (tri->normal.z < 0) {
		// 	continue;
		// }

        texinfo = &mapa->texinfo[face->texinfo];
		if (texinfo->miptex == mapa->numTextureTrigger) continue;

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

			if (v == 0) {
				dist = vector_length(&verts[0]->rot);
				if (dist > FAR_CLIP) {
					break;
				}
			}

			// if (verts[v]->rot.z > 10) {
			// 	grafico_xis( verts[v]->screen.x, verts[v]->screen.y );
			// }

			vBase = &mapa->base[vxtNum];
			verts[v]->tex.x = (dot_product(*vBase, texinfo->vetorS) + texinfo->distS) / tex->width;
			verts[v]->tex.y = (dot_product(*vBase, texinfo->vetorT) + texinfo->distT) / tex->height;
		}
//dbg
// if (i != 100) continue;

		if (dist > FAR_CLIP) continue;

		// Faz o clipping contra o plano NEAR
        int clipped_count = mapa_clip_near_face(verts, face->numedges, clipped);
        if (clipped_count < 3) continue;

		// Projeta os vértices válidos
        for (int v = 0; v < clipped_count; v++) {
            grafico_projecao3D(&clipped[v]);
            clipped_ptrs[v] = &clipped[v];
        }

		grafico_desenha_poligono(clipped_ptrs, clipped_count, tex, paleta);

// printf("n[%.4f,%.4f,%.4f] v1{%d,%d,%d}s[%d,%d], v2{%d,%d,%d}s[%d,%d], v3{%d,%d,%d}s[%d,%d] ",
// 	tri->normal.x, tri->normal.y, tri->normal.z,
// 	(int)vBase1->x, (int)vBase1->y, (int)vBase1->z, s1,t1,
// 	(int)vBase2->x, (int)vBase2->y, (int)vBase2->z, s2,t2,
// 	(int)vBase3->x, (int)vBase3->y, (int)vBase3->z, s3,t3
// );
// printf("vS{%.3f,%.3f,%.3f} ", texinfo->vetorS.x, texinfo->vetorS.y, texinfo->vetorS.z);
// printf("vT{%.3f,%.3f,%.3f} ", texinfo->vetorT.x, texinfo->vetorT.y, texinfo->vetorT.z);
// 		grafico_triangulo_textura(tex->data, tex->width, tex->height, paleta,
// 				vertice1->screen.x, vertice1->screen.y, vertice1->rot.z, s1, t1,
// 				vertice2->screen.x, vertice2->screen.y, vertice2->rot.z, s2, t2,
// 				vertice3->screen.x, vertice3->screen.y, vertice3->rot.z, s3, t3
// 			);
	}
}
