#include <stdio.h>

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

			grafico_triangulo_textura_zbuffer(obj->skin, obj->skinwidth, obj->skinheight, paleta,
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

void grafico_desenha_mapa(camera_t *cam, mapa_t *mapa, char paleta[256][3])
{
	mapa_projecao3D(cam, mapa);

	grafico_cor(255,255,255);

	triangulo_t *tri = mapa->tris;
	for (int i=0; i < mapa->numtris; i++, tri++) {
		// Backface culling
		if (tri->normal.z < 0) {
			continue;
		}

		ponto_t *vertice1 = &mapa->verts[tri->v[0]];
		ponto_t *vertice2 = &mapa->verts[tri->v[1]];
		ponto_t *vertice3 = &mapa->verts[tri->v[2]];

		grafico_xis( vertice1->screen.x, vertice1->screen.y );
		grafico_xis( vertice2->screen.x, vertice2->screen.y );
		grafico_xis( vertice3->screen.x, vertice3->screen.y );
	}
}
