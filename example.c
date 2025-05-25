#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "gfx_ptBR.h"
#include "doublebuffer_pixmap.h"

#include "readMdl.h"
#include "3d.h"

// gcc example.c gfx.c gfx_ptBR.c -o example -lX11 -lm

void gfx_triangulo(int x1, int y1, int x2, int y2, int x3, int y3, int corR, int corG, int corB)
{
	// Transforma os params em array
	int ponto[3][2];
	ponto[0][0] = x1;
	ponto[0][1] = y1;
	ponto[1][0] = x2;
	ponto[1][1] = y2;
	ponto[2][0] = x3;
	ponto[2][1] = y3;

	// Ordena os pontos pelo Y
	int maxY = 0;
	int pMaxY = 0, pMedY = 0, pMinY = 0;
	for (int cP = 0; cP < 3; cP++)
	{
		if (ponto[cP][1] >= maxY)
		{
			maxY = ponto[cP][1];
			pMaxY = cP;
		}
	}
	maxY = 0;
	for (int cP = 0; cP < 3; cP++)
	{
		if (cP == pMaxY)
			continue;

		if (ponto[cP][1] >= maxY)
		{
			maxY = ponto[cP][1];
			pMedY = cP;
		}
	}
	pMinY = 3 - pMaxY - pMedY;

	// DESENHA
	float xI = ponto[pMinY][0];
	float xF = xI;

	float deltaXI = (float)(ponto[pMaxY][0] - ponto[pMinY][0]) / (float)(ponto[pMaxY][1] - ponto[pMinY][1]);
	float deltaXF = (float)(ponto[pMedY][0] - ponto[pMinY][0]) / (float)(ponto[pMedY][1] - ponto[pMinY][1]);

	for (int cY = ponto[pMinY][1]; cY <= ponto[pMaxY][1]; cY++)
	{
		int xL1, xL2;
		if (xF >= xI)
		{
			xL1 = (int)xI;
			xL2 = (int)xF;
		}
		else
		{
			xL1 = (int)xF;
			xL2 = (int)xI;
		}

		// Desenha LINHA HORIZONTAL no Y=cY, de xI ate xF
		for (int cX = xL1; cX <= xL2; cX++)
		{
			gfx_ponto(cX, cY, corR, corG, corB);
		}

		// If came to middle point, change the deltaXF
		if (cY == ponto[pMedY][1])
		{
			xF = ponto[pMedY][0];
			deltaXF = (float)(ponto[pMedY][0] - ponto[pMaxY][0]) / (float)(ponto[pMedY][1] - ponto[pMaxY][1]);
		}

		xI += deltaXI;
		xF += deltaXF;
	}

	// VERTICES
	// for (int c = 0; c < 3; c++)
	// {
	// gfx_ponto(p[c][0], p[c][1], 250, 250, 250);
	// }
}

int main(int argc, char **argv)
{
	int janX = 800, janY = 600;
	obj3d_t *obj;

	obj = readMdl(argv[1]);

	// Use current time as
	// seed for random generator
	srand(time(0));

	gfx_init(janX, janY, "MDL Viewer");

	// Trinagulo de teste
	int x[3], y[3];

	x[0] = 30;
	y[0] = 80;

	x[1] = 50;
	y[1] = 90;

	x[2] = 100;
	y[2] = 120;

	int pSel = 1;
	int numFrameSel = 0;
	while (1)
	{
		gfx_limpa();
/*
		for (int t = 0; t < 10; t++)
		{
			int px1 = rand() % janX;
			int py1 = rand() % janY;
			int px2 = rand() % janX;
			int py2 = rand() % janY;
			int px3 = rand() % janX;
			int py3 = rand() % janY;

			int corR = rand() % 255;
			int corG = rand() % 255;
			int corB = rand() % 255;

			gfx_triangulo(px1, py1, px2, py2, px3, py3, corR, corG, corB);
		}

		gfx_triangulo(x[0], y[0], x[1], y[1], x[2], y[2], 250, 200, 200);
		gfx_ponto(x[pSel], y[pSel], 250, 250, 250);
*/


		frame_t *frame = &obj->frames[numFrameSel];

		for (int cnt_tri=0; cnt_tri<obj->numtris; cnt_tri++) {
			triangulo_t *tri = &obj->tris[cnt_tri];

			vetor3d_t *vertice1 = &frame->verts[tri->v[0]];
			vetor3d_t *vertice2 = &frame->verts[tri->v[1]];
			vetor3d_t *vertice3 = &frame->verts[tri->v[2]];

			gfx_triangulo(vertice1->x, vertice1->y, vertice2->x, vertice2->y, vertice3->x, vertice3->y, 250, 200, 200);
			gfx_triangulo(vertice1->y+260, 255-vertice1->z, vertice2->y+260, 255-vertice2->z, vertice3->y+260, 255-vertice3->z, 250, 200, 200);
			gfx_triangulo(vertice1->x, (255-vertice1->z)+260, vertice2->x, (255-vertice2->z)+260, vertice3->x, (255-vertice3->z)+260, 250, 200, 200);
		}

		for (int cnt_vert=0; cnt_vert<obj->numverts; cnt_vert++) {
			vetor3d_t *vertice = &frame->verts[cnt_vert];

			gfx_ponto(vertice->x, vertice->y, 250, 250, 250);

			gfx_ponto(vertice->y + 260, 255 - vertice->z, 250, 250, 250);

			gfx_ponto(vertice->x, (255 - vertice->z) + 260, 250, 250, 250);

			//printf("PONTO X:%f Y:%f Z:%f\n", vertice->x, vertice->y, vertice->z);
		}

		
		// Wait for the user to press a character.
		char c = gfx_tecla();

		// Quit if it is the letter q.
		if (c == 'q')
			break;

		//
		if (c == 'w')
			y[pSel]--;
		if (c == 's')
			y[pSel]++;
		if (c == 'a')
			x[pSel]--;
		if (c == 'd')
			x[pSel]++;

		if (c == 'r')
			pSel--;
		if (c == 'f')
			pSel++;
		if (pSel < 0)
			pSel = 2;
		if (pSel > 2)
			pSel = 0;
		
		if (c == '\\')
			numFrameSel--;
		if (c == 'z')
			numFrameSel++;
		if (numFrameSel < 0)
			numFrameSel = 0;
		if (numFrameSel >= obj->numframes)
			numFrameSel = obj->numframes - 1;
	}

	freeObj3D(obj);

	return 0;
}
