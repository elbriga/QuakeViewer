#include <X11/Xlib.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#include "gfx.h"
#include "gfx_ptBR.h"

void grafico_init( int width, int height, const char *title )
{
	gfx_open(width, height, title);

	gfx_color(200,200,200);
}

void grafico_limpa()
{
	gfx_clear();
}

void grafico_mostra()
{
	gfx_flush();
}

void grafico_cor( int r, int g, int b )
{
	gfx_color(r, g, b);
}

void grafico_ponto( int x, int y )
{
	gfx_point(x, y);
}

char grafico_tecla()
{
	return gfx_wait();
}

void grafico_triangulo(int x1, int y1, int x2, int y2, int x3, int y3,
	int r, int g, int b)
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

	grafico_cor( r,g,b );

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
			grafico_ponto(cX, cY);
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
}

void grafico_triangulo_textura(char *textura, int textW, int textH, char paleta[256][3],
    int x1, int y1, int x2, int y2, int x3, int y3,
    int ts1, int tt1, int ts2, int tt2, int ts3, int tt3)
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


	// Textura
	float texX = ts1;
	float texY = tt1;


	unsigned char idx_cor;
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
		texX = ts1;
		for (int cX = xL1; cX <= xL2; cX++)
		{
			idx_cor = textura[(int)texY * textW + (int)texX++];
			grafico_cor( paleta[idx_cor][0], paleta[idx_cor][1], paleta[idx_cor][2] );

			grafico_ponto(cX, cY);
		}
		texY++;

		// If came to middle point, change the deltaXF
		if (cY == ponto[pMedY][1])
		{
			xF = ponto[pMedY][0];
			deltaXF = (float)(ponto[pMedY][0] - ponto[pMaxY][0]) / (float)(ponto[pMedY][1] - ponto[pMaxY][1]);
		}

		xI += deltaXI;
		xF += deltaXF;
	}
}
