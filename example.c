#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "gfx_ptBR.h"

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

		// Draw HORIZONTAL LINE at Y=cY, from XL1 to xL2
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

	// for (int c = 0; c < 3; c++)
	// {
	// gfx_ponto(p[c][0], p[c][1], 250, 250, 250);
	// }
}

int main()
{
	int janX = 800, janY = 600;

	// Use current time as
	// seed for random generator
	srand(time(0));

	gfx_init(janX, janY, "Example Graphics Program");

	// Trinagulo de teste
	int x[3], y[3];

	x[0] = 30;
	y[0] = 80;

	x[1] = 50;
	y[1] = 90;

	x[2] = 100;
	y[2] = 120;

	int pSel = 1;
	while (1)
	{
		gfx_limpa();

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
	}

	return 0;
}
