#include <stdio.h>

#include "gfx_ptBR.h"
#include "3d.h"

void grafico_desenha_objeto(obj3d_t *obj, int numFrameSel, char paleta[256][3])
{
	frame_t *frame = &obj->frames[numFrameSel];

	for (int cnt_tri=0; cnt_tri<obj->numtris; cnt_tri++) {
		triangulo_t *tri = &obj->tris[cnt_tri];

		vetor3d_t *vertice1 = &frame->verts[tri->v[0]];
		vetor3d_t *vertice2 = &frame->verts[tri->v[1]];
		vetor3d_t *vertice3 = &frame->verts[tri->v[2]];

		skinvert_t *svxt1 = &obj->skinmap[tri->v[0]];
		skinvert_t *svxt2 = &obj->skinmap[tri->v[1]];
		skinvert_t *svxt3 = &obj->skinmap[tri->v[2]];

		grafico_triangulo_textura(obj->skin, obj->skinwidth, obj->skinheight, paleta, vertice1->x, vertice1->y, vertice2->x, vertice2->y, vertice3->x, vertice3->y, svxt1->s,svxt1->t, svxt2->s,svxt2->t, svxt3->s,svxt3->t);
		grafico_triangulo_textura(obj->skin, obj->skinwidth, obj->skinheight, paleta, vertice1->y+260, 255-vertice1->z, vertice2->y+260, 255-vertice2->z, vertice3->y+260, 255-vertice3->z, svxt1->s,svxt1->t, svxt2->s,svxt2->t, svxt3->s,svxt3->t);
		grafico_triangulo_textura(obj->skin, obj->skinwidth, obj->skinheight, paleta, vertice1->x, (255-vertice1->z)+260, vertice2->x, (255-vertice2->z)+260, vertice3->x, (255-vertice3->z)+260, svxt1->s,svxt1->t, svxt2->s,svxt2->t, svxt3->s,svxt3->t);
	}

	grafico_cor( 0, 250, 0 );
	for (int cnt_vert=0; cnt_vert<obj->numverts; cnt_vert++) {
		vetor3d_t *vertice = &frame->verts[cnt_vert];

		grafico_ponto(vertice->x, vertice->y);

		grafico_ponto(vertice->y + 260, 255 - vertice->z);

		grafico_ponto(vertice->x, (255 - vertice->z) + 260);

		//printf("PONTO X:%f Y:%f Z:%f\n", vertice->x, vertice->y, vertice->z);
	}

	// SKIN
	for (int skinY=0; skinY<obj->skinheight; skinY++) {
		for (int skinX=0; skinX<obj->skinwidth; skinX++) {
			unsigned char idx_cor = obj->skin[skinY * obj->skinwidth + skinX];

			grafico_cor( paleta[idx_cor][0], paleta[idx_cor][1], paleta[idx_cor][2] );
			grafico_ponto(skinX + 260, skinY + 260);
		}
	}
	// SKINMAP
	for (int cnt_vert=0; cnt_vert<obj->numverts; cnt_vert++) {
		skinvert_t *skinvert = &obj->skinmap[cnt_vert];

		if (skinvert->onseam) grafico_cor( 0,255,0 );
		else                  grafico_cor( 255,255,255 );

		grafico_ponto(skinvert->s + 260, skinvert->t + 260);
	}
}
