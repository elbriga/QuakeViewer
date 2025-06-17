#include <stdio.h>

#include "3d.h"
#include "gfx_ptBR.h"
#include "render.h"

void grafico_desenha_objeto(camera_t *cam, obj3d_t *obj, int numFrameSel, char paleta[256][3])
{
	int meiaSkin = obj->skinwidth / 2;
	ponto *verts = &obj->frames[numFrameSel * obj->numverts];

	obj_projecao3D(cam, obj, numFrameSel);

		for (int cnt_tri=0; cnt_tri<obj->numtris; cnt_tri++) {
			triangulo_t *tri = &obj->tris[cnt_tri];

			ponto *vertice1 = &verts[tri->v[0]];
			ponto *vertice2 = &verts[tri->v[1]];
			ponto *vertice3 = &verts[tri->v[2]];

			switch (obj->tipo)
			{
			case OBJ_TIPO_WIRE:
				grafico_triangulo_wireZ(
					vertice1->screen.x, vertice1->screen.y, vertice1->rot.z,
					vertice2->screen.x, vertice2->screen.y, vertice2->rot.z,
					vertice3->screen.x, vertice3->screen.y, vertice3->rot.z);
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
