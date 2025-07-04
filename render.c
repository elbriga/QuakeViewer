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

void grafico_desenha_mapa(camera_t *cam, mapa_t *mapa, char paleta[256][3])
{
	textureinfo_t *texinfo;
	texture_t     *tex;
	int s1,t1, s2,t2, s3,t3;
	float texScale = (1.0f/32.0f);

	mapa_projecao3D(cam, mapa);

	grafico_cor(255,255,255);

	triangulo_t *tri = mapa->tris;
	for (int i=0; i < mapa->numtris; i++, tri++) {

		// Backface culling
		// if (tri->normal.z < 0) {
		// 	continue;
		// }

        texinfo = &mapa->texinfo[tri->texinfo];
        tex = &mapa->textures[texinfo->miptex];

		ponto_t *vertice1 = &mapa->verts[tri->v[0]];
		ponto_t *vertice2 = &mapa->verts[tri->v[1]];
		ponto_t *vertice3 = &mapa->verts[tri->v[2]];

		if (vertice1->rot.z > 10) grafico_xis( vertice1->screen.x, vertice1->screen.y );
		if (vertice2->rot.z > 10) grafico_xis( vertice2->screen.x, vertice2->screen.y );
		if (vertice3->rot.z > 10) grafico_xis( vertice3->screen.x, vertice3->screen.y );

//dbg
if (i < 90 || i > 110) continue;

		vetor3d_t *vBase1 = &mapa->base[tri->v[0]];
		vetor3d_t *vBase2 = &mapa->base[tri->v[1]];
		vetor3d_t *vBase3 = &mapa->base[tri->v[2]];
		s1 = (int)((dot_product(*vBase1, texinfo->vetorS) + texinfo->distS) * texScale);
		t1 = (int)((dot_product(*vBase1, texinfo->vetorT) + texinfo->distT) * texScale);
		s2 = (int)((dot_product(*vBase2, texinfo->vetorS) + texinfo->distS) * texScale);
		t2 = (int)((dot_product(*vBase2, texinfo->vetorT) + texinfo->distT) * texScale);
		s3 = (int)((dot_product(*vBase3, texinfo->vetorS) + texinfo->distS) * texScale);
		t3 = (int)((dot_product(*vBase3, texinfo->vetorT) + texinfo->distT) * texScale);

// printf("n[%.4f,%.4f,%.4f] v1{%d,%d,%d}s[%d,%d], v2{%d,%d,%d}s[%d,%d], v3{%d,%d,%d}s[%d,%d] ",
// 	tri->normal.x, tri->normal.y, tri->normal.z,
// 	(int)vertice1->rot.x, (int)vertice1->rot.y, (int)vertice1->rot.z, (int)vertice1->screen.x, (int)vertice1->screen.y,
// 	(int)vertice2->rot.x, (int)vertice2->rot.y, (int)vertice2->rot.z, (int)vertice2->screen.x, (int)vertice2->screen.y,
// 	(int)vertice3->rot.x, (int)vertice3->rot.y, (int)vertice3->rot.z, (int)vertice3->screen.x, (int)vertice3->screen.y
// );
printf("vS{%.3f,%.3f,%.3f}+%.3f ", texinfo->vetorS.x, texinfo->vetorS.y, texinfo->vetorS.z, texinfo->distS);
printf("vT{%.3f,%.3f,%.3f}+%.3f ", texinfo->vetorT.x, texinfo->vetorT.y, texinfo->vetorT.z, texinfo->distT);
		grafico_triangulo_textura(tex->data, tex->width, tex->height, paleta,
				vertice1->screen.x, vertice1->screen.y, vertice1->rot.z, s1, t1,
				vertice2->screen.x, vertice2->screen.y, vertice2->rot.z, s2, t2,
				vertice3->screen.x, vertice3->screen.y, vertice3->rot.z, s3, t3);
	}
}
