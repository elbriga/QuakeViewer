#include <X11/Xlib.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <float.h>
#include <limits.h>
#include <math.h>

#include "3d.h"
#include "gfx.h"
#include "gfx_ptBR.h"

float *zBuffer = NULL;
int grafico_altura, grafico_largura;

float map(float x, float in_min, float in_max, float out_min, float out_max)
{
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

int map_scaleX(int v, mapa_t *mapa)
{
	return map(v, mapa->bbMin.x, mapa->bbMax.x, grafico_largura/2+10, grafico_largura - 10);
}
int map_scaleY(int v, mapa_t *mapa)
{
	return map((mapa->bbMax.y + mapa->bbMin.y) - v, mapa->bbMin.y, mapa->bbMax.y, grafico_altura/2+10, grafico_altura - 10);
}

void mostraMapa2D(mapa_t *mapa, camera_t *cam)
{
	vetor3d_t *b = mapa->base,  player_start = { 544, -808, 72 };
	ponto_t   *v = mapa->verts;
	edge_t    *e = mapa->edges;
	// leaf_t    *l = mapa->leafs;

	for (int i=0; i < mapa->numverts; i++, b++, v++) {
		v->rot.x = b->x;
		v->rot.y = b->y;
		v->rot.z = b->z;

		v->screen.x = map_scaleX(b->x, mapa);
		v->screen.y = map_scaleY(b->y, mapa);
	}

	grafico_cor(200,200,200);
	for (int i=0; i < mapa->numedges; i++, e++) {
		ponto_t *p1 = &mapa->verts[e->v[0]];
		ponto_t *p2 = &mapa->verts[e->v[1]];

		// if (i != 100) continue;

		grafico_linha( p1->screen.x,p1->screen.y, p2->screen.x,p2->screen.y );

		// vetor3d_t *vxt = &mapa->base[e->v[0]];
		// printf("LL%d,%d,%d > ", (int)vxt->x, (int)vxt->y, (int)vxt->z);
	}

	int camMX = map_scaleX(cam->pos.x, mapa);
	int camMY = map_scaleY(cam->pos.y, mapa);
	float camLenArrow = 10.0;
	float camAng = to_radians(cam->ang.y);
	int camAX = camMX + sin(camAng) * camLenArrow;
	int camAY = camMY - cos(camAng) * camLenArrow;

	grafico_cor(255,100,100);
	grafico_xis( camMX, camMY );

	grafico_cor(100,200,100);
	grafico_xis( camAX, camAY );
	grafico_linha(camMX, camMY, camAX, camAY);

	// for (int i=0; i < mapa->numleafs; i++, l++) {
	// 	int xi = map_scaleX(l->mins[0], mapa);
	// 	int xf = map_scaleX(l->maxs[0], mapa);
	// 	int yi = map_scaleY(l->mins[1], mapa);
	// 	int yf = map_scaleY(l->maxs[1], mapa);

	// 	grafico_linha( xi,yi, xf,yi );
	// 	grafico_linha( xf,yi, xf,yf );
	// 	grafico_linha( xf,yf, xi,yf );
	// 	grafico_linha( xi,yf, xi,yi );
	// }

	grafico_cor(255,200,200);
	grafico_xis( map_scaleX(player_start.x, mapa), map_scaleY(player_start.y, mapa) );

    // grafico_mostra();
    // grafico_tecla_espera();
}

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
	float *zbuf = &zBuffer[0];

	for(int zbz=0; zbz < grafico_altura * grafico_largura; zbz++, zbuf++) {
		*zbuf = FLT_MAX;
	}
}

void grafico_mostra()
{
	gfx_flush();
	grafico_limpa_zbuffer();
}

void grafico_cor( int r, int g, int b )
{
	gfx_color(r, g, b);
}

void grafico_ponto( int x, int y )
{
	gfx_point(x, y);
}

void grafico_xis( int x, int y )
{
	gfx_point(x,   y-1);
	gfx_point(x-1, y);
	gfx_point(x,   y);
	gfx_point(x+1, y);
	gfx_point(x,   y+1);
}

void grafico_linha( int x0, int y0, int x1, int y1 )
{
    // int dx = abs(x1 - x0);
    // int dy = abs(y1 - y0);
    // int sx = (x0 < x1) ? 1 : -1;
    // int sy = (y0 < y1) ? 1 : -1;
    // int err = dx - dy;

    // while(1) {
    //     grafico_ponto(x0, y0);
    //     if((x0 == x1) && (y0 == y1)) break;
    //     int e2 = 2 * err;
    //     if(e2 > -dy) { err -= dy; x0 += sx; }
    //     if(e2 <  dx) { err += dx; y0 += sy; }
    // }

	gfx_line( x0,y0, x1,y1 );
}

char grafico_tecla()
{
	return gfx_wait();
}

char grafico_tecla_espera()
{
	return gfx_wait_block();
}

void grafico_projecao3D(ponto_t *p)
{
    p->screen.x = ((FOV * p->rot.x) / p->rot.z) + (grafico_largura / 2);
    p->screen.y = ((FOV * p->rot.y) / p->rot.z) + (grafico_altura  / 2);
}

extern int _debug;
void grafico_triangulo(
	int x1, int y1, float z1,
	int x2, int y2, float z2,
	int x3, int y3, float z3,
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
	float pontoZ[3];
	pontoZ[0] = z1;
	pontoZ[1] = z2;
	pontoZ[2] = z3;

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

	// ZBuffer
	float zI = pontoZ[pMinY];
	float zF = zI;

	float deltaZI = (float)(pontoZ[pMaxY] - pontoZ[pMinY]) / (float)(ponto[pMaxY][1] - ponto[pMinY][1]);
	float deltaZF = (float)(pontoZ[pMedY] - pontoZ[pMinY]) / (float)(ponto[pMedY][1] - ponto[pMinY][1]);

	grafico_cor( r,g,b );

	int xL1, xL2;
	float zL1, zL2;
	float deltaZ, z;
	int zBufferBase, cX;
	for (int cY = ponto[pMinY][1]; cY <= ponto[pMaxY][1]; cY++)
	{
		if (cY >= grafico_altura) {
			// Linha saiu para fora da tela > CLIP
			break;
		}

		// If came to middle point, change the deltaXF
		if (cY == ponto[pMedY][1])
		{
			xF = ponto[pMedY][0];
			deltaXF = (float)(ponto[pMedY][0] - ponto[pMaxY][0]) / (float)(ponto[pMedY][1] - ponto[pMaxY][1]);

			// ZBuffer
			zF = pontoZ[pMedY];
			deltaZF = (float)(pontoZ[pMedY] - pontoZ[pMaxY]) / (float)(ponto[pMedY][1] - ponto[pMaxY][1]);
		}

		if (xF >= xI) {
			xL1 = (int)xI;
			xL2 = (int)xF;
			zL1 = zI;
			zL2 = zF;
		} else {
			xL1 = (int)xF;
			xL2 = (int)xI;
			zL1 = zF;
			zL2 = zI;
		}

		// zBuffer
		deltaZ = (zL2 - zL1) / (float)(xL2 - xL1);
		z = zI;

		if (_debug-- > 0) {
			printf("cY: %d >>> <%d a %d>\n", cY, xL1, xL2);

			if (!_debug) {
				printf("====================================\n");
				grafico_tecla_espera();
			}
		}

		// Desenha LINHA HORIZONTAL no Y=cY, de xI ate xF
		zBufferBase = cY * grafico_largura;
		for (cX = xL1; cX <= xL2; cX++)
		{
			if (cX >= 0) {
				if (cX >= grafico_largura) {
					// Linha saiu para fora da tela > CLIP
					break;
				}

				if (z > 10 && z < zBuffer[zBufferBase + cX]) {
					grafico_ponto(cX, cY);
					zBuffer[zBufferBase + cX] = z;
				}
			}

			z += deltaZ;
		}

		xI += deltaXI;
		xF += deltaXF;

		// ZBuffer
		zI += deltaZI;
		zF += deltaZF;
	}
}

void grafico_triangulo_wireZ(
	int x1,int y1,int z1,
	int x2,int y2,int z2,
	int x3,int y3,int z3)
{
	grafico_cor(255,255,255);

	// if (z1 > 50) grafico_ponto(x1, y1);
	// if (z2 > 50) grafico_ponto(x2, y2);
	// if (z3 > 50) grafico_ponto(x3, y3);

	grafico_linha(x1, y1, x2, y2);
	grafico_linha(x2, y2, x3, y3);
	grafico_linha(x3, y3, x1, y1);
}

void grafico_desenha_poligono(ponto_t **verticesPoligono, int numVerts, texture_t *tex, char paleta[256][3]) {
    int minY = INT_MAX, maxY = INT_MIN;

    for (int i = 0; i < numVerts; i++) {
        int y = verticesPoligono[i]->screen.y;
        if (y < minY) minY = y;
        if (y > maxY) maxY = y;
    }

    // Clamp para dentro da tela
    if (minY < 0) minY = 0;
    if (maxY >= grafico_altura) maxY = grafico_altura - 1;

    for (int y = minY; y <= maxY; y++) {
        struct {
            int x;
            float u_over_z, v_over_z, one_over_z;
            float z_view; // profundidade real do ponto
        } intersecoes[64];

        int count = 0;

        for (int i = 0; i < numVerts; i++) {
            ponto_t p1 = *verticesPoligono[i];
            ponto_t p2 = *verticesPoligono[(i + 1) % numVerts];

            int y1 = p1.screen.y;
            int y2 = p2.screen.y;

            if (y1 == y2) continue;

            if ((y >= y1 && y < y2) || (y >= y2 && y < y1)) {
                float t = (float)(y - y1) / (float)(y2 - y1);
                int x = p1.screen.x + (int)((p2.screen.x - p1.screen.x) * t);

                float z1 = p1.rot.z, z2 = p2.rot.z;
                float one_over_z1 = 1.0f / z1, one_over_z2 = 1.0f / z2;
                float u_over_z1 = p1.tex.x / z1, u_over_z2 = p2.tex.x / z2;
                float v_over_z1 = p1.tex.y / z1, v_over_z2 = p2.tex.y / z2;

                float one_over_z = one_over_z1 + (one_over_z2 - one_over_z1) * t;
                float u_over_z   = u_over_z1   + (u_over_z2   - u_over_z1)   * t;
                float v_over_z   = v_over_z1   + (v_over_z2   - v_over_z1)   * t;
                float z_view     = z1 + (z2 - z1) * t;

                intersecoes[count].x = x;
                intersecoes[count].u_over_z = u_over_z;
                intersecoes[count].v_over_z = v_over_z;
                intersecoes[count].one_over_z = one_over_z;
                intersecoes[count].z_view = z_view;
                count++;
            }
        }

        // Ordenar interseções por x
        for (int i = 0; i < count - 1; i++) {
            for (int j = 0; j < count - i - 1; j++) {
                if (intersecoes[j].x > intersecoes[j + 1].x) {
                    typeof(intersecoes[0]) temp = intersecoes[j];
                    intersecoes[j] = intersecoes[j + 1];
                    intersecoes[j + 1] = temp;
                }
            }
        }

		int zBufferBase = y * grafico_largura;
        for (int i = 0; i < count; i += 2) {
            if (i + 1 >= count) break;

            int x0 = intersecoes[i].x;
            int x1 = intersecoes[i + 1].x;

            if (x0 == x1) continue;
            if (x0 < 0) x0 = 0;
            if (x1 >= grafico_largura) x1 = grafico_largura - 1;

            float u0 = intersecoes[i].u_over_z;
            float v0 = intersecoes[i].v_over_z;
            float w0 = intersecoes[i].one_over_z;
            float z0 = intersecoes[i].z_view;

            float u1 = intersecoes[i + 1].u_over_z;
            float v1 = intersecoes[i + 1].v_over_z;
            float w1 = intersecoes[i + 1].one_over_z;
            float z1 = intersecoes[i + 1].z_view;

            int dx = x1 - x0;
            if (dx == 0) dx = 1;

            for (int x = x0; x <= x1; x++) {
                float t = (float)(x - x0) / dx;

                float u_over_z = u0 + (u1 - u0) * t;
                float v_over_z = v0 + (v1 - v0) * t;
                float one_over_z = w0 + (w1 - w0) * t;
                float z = z0 + (z1 - z0) * t;

                int u = (int)((float)(u_over_z / one_over_z) * tex->width ) % tex->width;
                int v = (int)((float)(v_over_z / one_over_z) * tex->height) % tex->height;

                // Teste de profundidade (Z-buffer)
                if (x >= 0 && x < grafico_largura && y >= 0 && y < grafico_altura) {
                    if (z < zBuffer[zBufferBase + x]) {
						unsigned char cor = tex->data[u + v * tex->width];
                        // int r, g, b;
                        // texture_sample(tex, u, v, &r, &g, &b);
                        grafico_cor(paleta[cor][0], paleta[cor][1], paleta[cor][2]);
                        grafico_ponto(x, y);
                        zBuffer[zBufferBase + x] = z;
                    }
                }
            }
        }
    }
}


void grafico_triangulo_textura(char *textura, int textW, int textH, char paleta[256][3],
    int x1,int y1,int z1, int ts1,int tt1,
	int x2,int y2,int z2, int ts2,int tt2,
	int x3,int y3,int z3, int ts3,int tt3)
{
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
	int xL1, xL2;
	int zL1, zL2;
	for (int cY = ponto[pMinY][1]; cY <= ponto[pMaxY][1]; cY++)
	{
		//printf("Y[%d] => texXI: %d - texYI: %d - texXF: %d - texYF: %d\n", cY, (int)texXI, (int)texXF, (int)texYI, (int)texYF);

		if (cY >= grafico_altura) {
			// Linha saiu para fora da tela > CLIP
			break;
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

		if (cY >= 0) {
			if (xF >= xI) {
				xL1 = (int)xI;
				xL2 = (int)xF;
				zL1 = (int)zI;
				zL2 = (int)zF;
			} else {
				xL1 = (int)xF;
				xL2 = (int)xI;
				zL1 = (int)zF;
				zL2 = (int)zI;
			}

			// Textura
			float deltaTexX = (texXF - texXI) / (float)(xL2 - xL1);
			float deltaTexY = (texYF - texYI) / (float)(xL2 - xL1);
			float texX = texXI;
			float texY = texYI;

			// zBuffer
			float deltaZ = (zL2 - zL1) / (float)(xL2 - xL1);
			float z = zI;

			// Desenha LINHA HORIZONTAL no Y=cY, de xI ate xF,
			//  partindo das coordenadas texXI,texYI ate texXF, texYF
			//  partido do Z zI ate o zF
			int zBufferBase = cY * grafico_largura;
			for (int cX = xL1; cX <= xL2; cX++)
			{
				// printf("z[%f] < zBuf[%f]\n", z, zBuffer[zBufferBase + cX]);
				// grafico_tecla_espera();

				if (cX >= 0) {
					if (cX >= grafico_largura) {
						// Linha saiu para fora da tela > CLIP
						break;
					}
					if (z > 5 && z < zBuffer[zBufferBase + cX]) {
						idx_cor = textura[((int)texY % textH) * textW + ((int)texX % textW)];
						grafico_cor( paleta[idx_cor][0] * ganhoCor, paleta[idx_cor][1] * ganhoCor, paleta[idx_cor][2] * ganhoCor );
						grafico_ponto(cX, cY);
						
						zBuffer[zBufferBase + cX] = z;
					}
				}

				texX += deltaTexX;
				texY += deltaTexY;

				z += deltaZ;
			}
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
