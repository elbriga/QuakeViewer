#include <X11/Xlib.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "gfx.h"
#include "gfx_ptBR.h"

float *zBuffer = NULL;
int grafico_altura, grafico_largura;

int grafico_init( int width, int height, const char *title )
{
	grafico_largura = width;
	grafico_altura  = height;
	gfx_open(width, height, title);

	gfx_color(200,200,200);

	zBuffer = (float *)malloc(width * height * sizeof(float));
	if (!zBuffer)
		return 1;
	grafico_limpa_zbuffer();
	
	return 0;
}

void grafico_desliga()
{
	if (zBuffer)
		free(zBuffer);
}

void grafico_limpa()
{
	gfx_clear();
}

void grafico_limpa_zbuffer()
{
	for(int zbz=0; zbz < grafico_altura * grafico_largura; zbz++) {
		zBuffer[zbz] = 999999.999;
	}
}

char drawnZBufferTri = 0;
void grafico_mostra()
{
	gfx_flush();

	if (drawnZBufferTri) {
		grafico_limpa_zbuffer();
		drawnZBufferTri = 0;
	}
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

		// Desenha LINHA HORIZONTAL no Y=cY, de xI ate xF,
		//  partindo das coordenadas texXI,texYI ate texXF, texYF
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

void grafico_triangulo_textura_zbuffer(char *textura, int textW, int textH, char paleta[256][3],
    int x1,int y1,int z1, int ts1,int tt1,
	int x2,int y2,int z2, int ts2,int tt2,
	int x3,int y3,int z3, int ts3,int tt3)
{
	drawnZBufferTri = 1;

	float ganhoCor = 2;

	// Transforma os params em array
	int ponto[3][3];
	ponto[0][0] = x1;
	ponto[0][1] = y1;
	ponto[0][2] = z1;
	ponto[1][0] = x2;
	ponto[1][1] = y2;
	ponto[1][2] = z2;
	ponto[2][0] = x3;
	ponto[2][1] = y3;
	ponto[2][2] = z3;

	int pontoTex[3][2];
	pontoTex[0][0] = ts1;
	pontoTex[0][1] = tt1;
	pontoTex[1][0] = ts2;
	pontoTex[1][1] = tt2;
	pontoTex[2][0] = ts3;
	pontoTex[2][1] = tt3;

	// Ordena os pontos pelo Y
	int maxY = -1000000;
	int pMaxY = 0, pMedY = 0, pMinY = 0;
	for (int cP = 0; cP < 3; cP++)
	{
		if (ponto[cP][1] >= maxY)
		{
			maxY = ponto[cP][1];
			pMaxY = cP;
		}
	}
	maxY = -1000000;
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

	if (!(ponto[pMaxY][1] - ponto[pMinY][1])) {
		return;
	}

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

	// ZBuffer
	float zI = ponto[pMinY][2];
	float zF = zI;

	float deltaZI = (float)(ponto[pMaxY][2] - ponto[pMinY][2]) / (float)(ponto[pMaxY][1] - ponto[pMinY][1]);
	float deltaZF = (float)(ponto[pMedY][2] - ponto[pMinY][2]) / (float)(ponto[pMedY][1] - ponto[pMinY][1]);

	unsigned char idx_cor;
	for (int cY = ponto[pMinY][1]; cY <= ponto[pMaxY][1]; cY++)
	{
		//printf("Y[%d] => texXI: %d - texYI: %d - texXF: %d - texYF: %d\n", cY, (int)texXI, (int)texXF, (int)texYI, (int)texYF);

		if (cY >= grafico_altura) {
			// Linha saiu para fora da tela > CLIP
			break;
		}

		if (cY >= 0) {
			int xL1, xL2;
			if (xF >= xI) {
				xL1 = (int)xI;
				xL2 = (int)xF;
			} else {
				xL1 = (int)xF;
				xL2 = (int)xI;
			}

			// Textura
			float deltaTexX = (texXF - texXI) / (float)(xL2 - xL1);
			float deltaTexY = (texYF - texYI) / (float)(xL2 - xL1);
			float texX = texXI;
			float texY = texYI;

			// zBuffer
			float deltaZ = (zF - zI) / (float)(xL2 - xL1);
			float z = zI;

			// Desenha LINHA HORIZONTAL no Y=cY, de xI ate xF,
			//  partindo das coordenadas texXI,texYI ate texXF, texYF
			//  partido do Z zI ate o zF
			for (int cX = xL1; cX <= xL2; cX++)
			{
				// printf("z[%f] < zBuf[%f]\n", z, zBuffer[(cY * grafico_largura) + cX]);
				// grafico_tecla_espera();

				if (cX > 0) {
					if (cX >= grafico_largura) {
						// Linha saiu para fora da tela > CLIP
						break;
					}
					if (z < zBuffer[(cY * grafico_largura) + cX]) {
						idx_cor = textura[(int)texY * textW + (int)texX];

						grafico_cor( paleta[idx_cor][0] * ganhoCor, paleta[idx_cor][1] * ganhoCor, paleta[idx_cor][2] * ganhoCor );

						grafico_ponto(cX, cY);
						zBuffer[(cY * grafico_largura) + cX] = z;
					}
				}

				texX += deltaTexX;
				texY += deltaTexY;

				z += deltaZ;
			}
		}

		// Se chegamos no ponto do meio mudar os deltas de XF,texXF,texYF
		if (cY == ponto[pMedY][1])
		{
			// Triangulo
			xF = ponto[pMedY][0];
			deltaXF = (float)(ponto[pMedY][0] - ponto[pMaxY][0]) / (float)(ponto[pMedY][1] - ponto[pMaxY][1]);

			// Textura
			texXF = pontoTex[pMedY][0];
			deltaTexXF = (float)(pontoTex[pMedY][0] - pontoTex[pMaxY][0]) / (float)(ponto[pMedY][1] - ponto[pMaxY][1]);

			texYF = pontoTex[pMedY][1];
			deltaTexYF = (float)(pontoTex[pMedY][1] - pontoTex[pMaxY][1]) / (float)(ponto[pMedY][1] - ponto[pMaxY][1]);

			// ZBuffer
			zF = ponto[pMedY][2];
			deltaZF = (float)(ponto[pMedY][2] - ponto[pMaxY][2]) / (float)(ponto[pMedY][1] - ponto[pMaxY][1]);
		}

		// Triangulo
		xI += deltaXI;
		xF += deltaXF;

		// Textura
		texXI += deltaTexXI;
		texYI += deltaTexYI;
		texXF += deltaTexXF;
		texYF += deltaTexYF;

		// ZBuffer
		zI += deltaZI;
		zF += deltaZF;
	}
}
