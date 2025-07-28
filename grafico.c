#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <float.h>
#include <math.h>
#include <unistd.h>
#include <limits.h>

#include "QuakeViewer.h"
#include "3d.h"
#include "gfx.h"
#include "grafico.h"
#include "mapa.h"

extern char paleta[256][3];

extern int _debug;
extern float _lightFactor;
extern float tempo_de_jogo;

float *zBuffer = NULL;
int grafico_altura, grafico_largura, FOV = 500;

int grafico_init( int width, int height, const char *title )
{
	grafico_largura = width;
	grafico_altura  = height;
	gfx_open(width, height, title);

	zBuffer = (float *)malloc(width * height * sizeof(float));
	if (!zBuffer)
		return 1;
	grafico_limpa_zbuffer();
	
	return 0;
}

void grafico_desliga()
{
    gfx_close();
    
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
    gfx_clear();
	grafico_limpa_zbuffer();
}

void grafico_xis( int x, int y, byte r, byte g, byte b )
{
	gfx_point(x,   y-1, r,g,b);
	gfx_point(x-1, y,   r,g,b);
	gfx_point(x,   y,   r,g,b);
	gfx_point(x+1, y,   r,g,b);
	gfx_point(x,   y+1, r,g,b);
}

void grafico_projecao3D(ponto_t *p)
{
    p->screen.x = ((FOV * p->rot.x) / p->rot.z) + (grafico_largura / 2);
    p->screen.y = ((FOV * p->rot.y) / p->rot.z) + (grafico_altura  / 2);
}

byte get_light_level(char *light, int light_u, int light_v, int lightW, int lightH)
{
    if (!light) return 255;

    // Lightmap tem resolução 1/16 (>>4), então pegamos o valor inteiro e a fração
    float fu = (float)light_u / 16.0f;
    float fv = (float)light_v / 16.0f;

    int u0 = (int)fu;
    int v0 = (int)fv;
    float frac_u = fu - u0;
    float frac_v = fv - v0;

    // Clamp para evitar acesso fora do buffer
    int lw = lightW >> 4;
    int lh = lightH >> 4;
    
    if (u0 < 0) u0 = 0; else if (u0 >= lw - 1) u0 = lw - 2;
    if (v0 < 0) v0 = 0; else if (v0 >= lh - 1) v0 = lh - 2;

    int idx00 = u0     + v0     * lw;
    int idx10 = (u0+1) + v0     * lw;
    int idx01 = u0     + (v0+1) * lw;
    int idx11 = (u0+1) + (v0+1) * lw;

    float l00 = light[idx00];
    float l10 = light[idx10];
    float l01 = light[idx01];
    float l11 = light[idx11];

    // Interpolação bilinear
    float i0 = l00 * (1 - frac_u) + l10 * frac_u;
    float i1 = l01 * (1 - frac_u) + l11 * frac_u;
    float value = i0 * (1 - frac_v) + i1 * frac_v;

    return (byte)value * _lightFactor;
}

void grafico_desenha_poligono(ponto_t **verticesPoligono, int numVerts, texture_t *tex, byte *light, int lightW, int lightH)
{
    int minY = INT_MAX, maxY = INT_MIN, lightON = !!(light);
    char ehLiquido = (tex->name && tex->name[0] == '*');

    for (int i = 0; i < numVerts; i++) {
if (_debug) {
	ponto_t *vp = verticesPoligono[i];
	printf("\nvP{%.1f,%.1f,%.1f}{%.1f,%.1f}{%.1f,%.1f}",
		vp->rot.x, vp->rot.y, vp->rot.z, vp->tex.x, vp->tex.y, vp->tex_luz.x, vp->tex_luz.y);
}
        int y = verticesPoligono[i]->screen.y;
        if (y < minY) minY = y;
        if (y > maxY) maxY = y;
    }
if (_debug) {
	printf("\n");

	for (int ly=0; ly < lightH; ly++) {
		for (int lx=0; lx < lightW; lx++) {
			byte intensidade = get_light_level(light, lx, ly, lightW, lightH);
			gfx_point( lx, ly, intensidade,intensidade,intensidade );
		}
	}
}
    if (minY < 0) minY = 0;
    if (maxY >= grafico_altura) maxY = grafico_altura - 1;

    for (int y = minY; y <= maxY; y++) {
        struct {
            int x;
            float u_over_z, v_over_z, one_over_z;
            float lu_over_z, lv_over_z;
            float z_view;
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
                float lu_over_z1 = p1.tex_luz.x / z1, lu_over_z2 = p2.tex_luz.x / z2;
                float lv_over_z1 = p1.tex_luz.y / z1, lv_over_z2 = p2.tex_luz.y / z2;

                float one_over_z = one_over_z1 + (one_over_z2 - one_over_z1) * t;
                float u_over_z   = u_over_z1   + (u_over_z2   - u_over_z1) * t;
                float v_over_z   = v_over_z1   + (v_over_z2   - v_over_z1) * t;
                float lu_over_z  = lu_over_z1  + (lu_over_z2  - lu_over_z1) * t;
                float lv_over_z  = lv_over_z1  + (lv_over_z2  - lv_over_z1) * t;
                float z_view     = z1 + (z2 - z1) * t;

                intersecoes[count].x = x;
                intersecoes[count].u_over_z = u_over_z;
                intersecoes[count].v_over_z = v_over_z;
                intersecoes[count].lu_over_z = lu_over_z;
                intersecoes[count].lv_over_z = lv_over_z;
                intersecoes[count].one_over_z = one_over_z;
                intersecoes[count].z_view = z_view;
                count++;
            }
        }

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

            float u0 = intersecoes[i].u_over_z;
            float v0 = intersecoes[i].v_over_z;
            float w0 = intersecoes[i].one_over_z;
            float z0 = intersecoes[i].z_view;
            float lu0 = intersecoes[i].lu_over_z;
            float lv0 = intersecoes[i].lv_over_z;

            float u1 = intersecoes[i + 1].u_over_z;
            float v1 = intersecoes[i + 1].v_over_z;
            float w1 = intersecoes[i + 1].one_over_z;
            float z1 = intersecoes[i + 1].z_view;
            float lu1 = intersecoes[i + 1].lu_over_z;
            float lv1 = intersecoes[i + 1].lv_over_z;

            int original_x0 = x0;
            int original_x1 = x1;
            int dx = x1 - x0;
            if (dx == 0) dx = 1;

            // Clamp horizontal com correção
            if (x0 < 0) {
                float t = (float)(0 - original_x0) / dx;
                x0 = 0;
                u0 += (u1 - u0) * t;
                v0 += (v1 - v0) * t;
                w0 += (w1 - w0) * t;
                z0 += (z1 - z0) * t;
                lu0 += (lu1 - lu0) * t;
                lv0 += (lv1 - lv0) * t;
            }
            if (x1 >= grafico_largura) {
                float t = (float)(grafico_largura - 1 - original_x0) / dx;
                x1 = grafico_largura - 1;
                u1 = u0 + (u1 - u0) * t;
                v1 = v0 + (v1 - v0) * t;
                w1 = w0 + (w1 - w0) * t;
                z1 = z0 + (z1 - z0) * t;
                lu1 = lu0 + (lu1 - lu0) * t;
                lv1 = lv0 + (lv1 - lv0) * t;
            }

            for (int x = x0; x <= x1; x++) {
                float t = (float)(x - x0) / (float)(x1 - x0);

                float u_over_z = u0 + (u1 - u0) * t;
                float v_over_z = v0 + (v1 - v0) * t;
                float one_over_z = w0 + (w1 - w0) * t;
                float z = z0 + (z1 - z0) * t;
                float lu_over_z = lu0 + (lu1 - lu0) * t;
                float lv_over_z = lv0 + (lv1 - lv0) * t;

                float u = (u_over_z / one_over_z) * tex->width;
                float v = (v_over_z / one_over_z) * tex->height;

                // Efeito swirl para texturas de água (nome começa com '*')
                if (ehLiquido) {
                    float cx = tex->width  / 2.0f;
                    float cy = tex->height / 2.0f;

                    float dx = u - cx;
                    float dy = v - cy;
                    float dist = sqrtf(dx * dx + dy * dy);

                    float swirl_strength = 0.005f; // força do swirl (ajustável)
                    float swirl_speed = 1.0f;     // velocidade da animação
                    float angle = dist * swirl_strength + tempo_de_jogo * swirl_speed;

                    float sin_a = sinf(angle);
                    float cos_a = cosf(angle);

                    float new_dx = dx * cos_a - dy * sin_a;
                    float new_dy = dx * sin_a + dy * cos_a;

                    u = cx + new_dx;
                    v = cy + new_dy;
                }

                int tex_u = ((int)u % tex->width + tex->width) % tex->width;
                int tex_v = ((int)v % tex->height + tex->height) % tex->height;
//printf("(lu:%.2f-W:%d-H:%d)", lu, lightW, lightH);
// grafico_tecla_espera();
				int light_u=0, light_v=0;
				if (lightON) {
					float lu = (lu_over_z / one_over_z);
					float lv = (lv_over_z / one_over_z);

					if (lu < 0) lu = 0;
					if (lu > 1) lu = 1;
					if (lv < 0) lv = 0;
					if (lv > 1) lv = 1;
					light_u = lu * lightW; //((int)lu % lightW + lightW) % lightW;
					light_v = lv * lightH; //((int)lv % lightH + lightH) % lightH;
				}
                if (x >= 0 && x < grafico_largura && y >= 0 && y < grafico_altura) {
                    if (z < zBuffer[zBufferBase + x]) {
                        unsigned char cor = tex->data[tex_u + tex_v * tex->width];

						int r,g,b;
						if (lightON) {
							unsigned char intensidade = get_light_level(light, light_u, light_v, lightW, lightH);
							float fator = intensidade / 255.0f;

							r = (byte)paleta[cor][0] * fator;
							g = (byte)paleta[cor][1] * fator;
							b = (byte)paleta[cor][2] * fator;
						} else {
							r = (byte)paleta[cor][0];
							g = (byte)paleta[cor][1];
							b = (byte)paleta[cor][2];
						}

                        gfx_point(x, y, r, g, b);
                        zBuffer[zBufferBase + x] = z;
                    }
                }
            }
        }
    }
}

// Retorna a cor interpolada da textura, usando UV normalizados
byte bilinear_sample(texture_t *tex, float u, float v) {
    if (!tex || !tex->data) return 0;

    int texW = tex->width;
    int texH = tex->height;

    // Mapeia UVs para coordenadas reais
    float x = u * texW;
    float y = v * texH;

    int x0 = (int)x;
    int y0 = (int)y;
    int x1 = (x0 + 1) % texW;
    int y1 = (y0 + 1) % texH;

    float tx = x - x0;
    float ty = y - y0;

    unsigned char c00 = tex->data[x0 + y0 * texW];
    unsigned char c10 = tex->data[x1 + y0 * texW];
    unsigned char c01 = tex->data[x0 + y1 * texW];
    unsigned char c11 = tex->data[x1 + y1 * texW];

    float w00 = (1.0f - tx) * (1.0f - ty);
    float w10 = tx * (1.0f - ty);
    float w01 = (1.0f - tx) * ty;
    float w11 = tx * ty;

    float cor = w00 * c00 + w10 * c10 + w01 * c01 + w11 * c11;

    return (byte)cor;
}

void grafico_desenha_poligono_sky(ponto_t **verticesPoligono, int numVerts, texture_t *sky, float tempo) {
    int minY = INT_MAX, maxY = INT_MIN;

    for (int i = 0; i < numVerts; i++) {
        int y = verticesPoligono[i]->screen.y;
        if (y < minY) minY = y;
        if (y > maxY) maxY = y;
    }

    if (minY < 0) minY = 0;
    if (maxY >= grafico_altura) maxY = grafico_altura - 1;

    for (int y = minY; y <= maxY; y++) {
        struct {
            int x;
            float ndc_x, ndc_y;
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

                float ndc_x = ((float)x / grafico_largura) * 2.0f - 1.0f;
                float ndc_y = 1.0f - ((float)y / grafico_altura) * 2.0f;

                intersecoes[count].x = x;
                intersecoes[count].ndc_x = ndc_x;
                intersecoes[count].ndc_y = ndc_y;
                count++;
            }
        }

        // Ordena por X
        for (int i = 0; i < count - 1; i++) {
            for (int j = 0; j < count - i - 1; j++) {
                if (intersecoes[j].x > intersecoes[j + 1].x) {
                    typeof(intersecoes[0]) tmp = intersecoes[j];
                    intersecoes[j] = intersecoes[j + 1];
                    intersecoes[j + 1] = tmp;
                }
            }
        }

        for (int i = 0; i < count; i += 2) {
            if (i + 1 >= count) break;

            int x0 = intersecoes[i].x;
            int x1 = intersecoes[i + 1].x;
            int dx = x1 - x0;
            if (dx == 0) dx = 1;

            for (int x = x0; x <= x1; x++) {
                float t = (float)(x - x0) / dx;

                float ndc_x = intersecoes[i].ndc_x + (intersecoes[i + 1].ndc_x - intersecoes[i].ndc_x) * t;
                float ndc_y = intersecoes[i].ndc_y + (intersecoes[i + 1].ndc_y - intersecoes[i].ndc_y) * t;

                // Vetor da direção da câmera (para céu)
                vetor3d_t dir = { ndc_x, ndc_y, 1.0f };
                float len = sqrtf(dir.x * dir.x + dir.y * dir.y + dir.z * dir.z);
                dir.x /= len;
                dir.y /= len;
                dir.z /= len;

                // UV base
                float u = 0.5f + (atan2f(dir.x, dir.z) / (2.0f * M_PI));
                float v = 0.5f - (asinf(dir.y) / M_PI);

                // MOVIMENTO: deslocamento suave para SKYTOP
                float scrollU = tempo * 0.01f;
                float scrollV = tempo * 0.005f;

                // TEX SKYBASE (fundo)
                float v_base = fmodf(v, 0.5f);                 // [0.0 ~ 0.5]
				float v_top  = fmodf(v + scrollV, 0.5f);       // [0.0 ~ 0.5] + scroll
				// TEX SKYTOP (nuvens)
				float u_top  = fmodf(u + scrollU, 1.0f);
				float u_base = fmodf(u, 1.0f);

				// Interpolação bilinear (mapa base)
				unsigned char cor_base = bilinear_sample(sky, u_base, v_base);

				// Interpolação bilinear (camada do topo)
				float v_top_real = v_top + 0.5f;               // desloca para metade inferior
				unsigned char cor_top = bilinear_sample(sky, u_top, v_top_real);

                // Sem alpha: apenas usa a cor do topo se for != 0, senão a do fundo
                unsigned char final_cor = (cor_top != 0) ? cor_top : cor_base;

                gfx_point(x, y, paleta[final_cor][0], paleta[final_cor][1], paleta[final_cor][2]);
            }
        }
    }
}
