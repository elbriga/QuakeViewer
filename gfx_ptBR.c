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

char grafico_tecla_espera()
{
	return gfx_wait_block();
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

	int pontoTex[3][2];
	pontoTex[0][0] = ts1;
	pontoTex[0][1] = tt1;
	pontoTex[1][0] = ts2;
	pontoTex[1][1] = tt2;
	pontoTex[2][0] = ts3;
	pontoTex[2][1] = tt3;

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
	float texXI = pontoTex[pMinY][0];
	float texYI = pontoTex[pMinY][1];
	float texXF = texXI;
	float texYF = texYI;

	float deltaTexXI = (float)(pontoTex[pMaxY][0] - pontoTex[pMinY][0]) / (float)(ponto[pMaxY][1] - ponto[pMinY][1]);
	float deltaTexYI = (float)(pontoTex[pMaxY][1] - pontoTex[pMinY][1]) / (float)(ponto[pMaxY][1] - ponto[pMinY][1]);
	float deltaTexXF = (float)(pontoTex[pMedY][0] - pontoTex[pMinY][0]) / (float)(ponto[pMedY][1] - ponto[pMinY][1]);
	float deltaTexYF = (float)(pontoTex[pMedY][1] - pontoTex[pMinY][1]) / (float)(ponto[pMedY][1] - ponto[pMinY][1]);

	float texX = ts1;
	float texY = tt1;

	unsigned char idx_cor;
	for (int cY = ponto[pMinY][1]; cY <= ponto[pMaxY][1]; cY++)
	{
		//printf("Y[%d] => texXI: %d - texYI: %d - texXF: %d - texYF: %d\n", cY, (int)texXI, (int)texXF, (int)texYI, (int)texYF);

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
		float deltaTexX = (texXF - texXI) / (float)(xL2 - xL1);
		float deltaTexY = (texYF - texYI) / (float)(xL2 - xL1);

		texX = texXI;
		texY = texYI;
		for (int cX = xL1; cX <= xL2; cX++)
		{
			idx_cor = textura[(int)texY * textW + (int)texX];
			grafico_cor( paleta[idx_cor][0], paleta[idx_cor][1], paleta[idx_cor][2] );

			grafico_ponto(cX, cY);

			texX += deltaTexX;
			texY += deltaTexY;
		}

		// Se chegamos no ponto do meio mudar os deltas de XF,texXF,texYF
		if (cY == ponto[pMedY][1])
		{
			xF = ponto[pMedY][0];
			deltaXF = (float)(ponto[pMedY][0] - ponto[pMaxY][0]) / (float)(ponto[pMedY][1] - ponto[pMaxY][1]);

			texXF = pontoTex[pMedY][0];
			deltaTexXF = (float)(pontoTex[pMedY][0] - pontoTex[pMaxY][0]) / (float)(ponto[pMedY][1] - ponto[pMaxY][1]);

			texYF = pontoTex[pMedY][1];
			deltaTexYF = (float)(pontoTex[pMedY][1] - pontoTex[pMaxY][1]) / (float)(ponto[pMedY][1] - ponto[pMaxY][1]);
		}

		xI += deltaXI;
		xF += deltaXF;

		texXI += deltaTexXI;
		texYI += deltaTexYI;
		texXF += deltaTexXF;
		texYF += deltaTexYF;
	}
}
