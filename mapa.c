#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <math.h>

#include "3d.h"
#include "gfx.h"
#include "grafico.h"

extern int grafico_altura, grafico_largura; // de gfx.c

bool mapa_face_solida(face_t *face)
{
	return (face->texture->name[0] != '*');
}

/**
 * mapa_discover_leaf
 * Retorna o Leaf que o ponto esta no mapa
 */
leaf_t *mapa_discoverLeaf(vetor3d_t *pos, mapa_t *mapa)
{
    node_t  *node;
    plano_t *plano;
    float    d;

    node = mapa->nodes;
	while (1)
	{
		if (node->contents < 0)
			return (leaf_t *)node;
        
		plano = node->plane;
		d = vetor_dot_product (*pos, plano->normal) - plano->dist;
		if (d > 0)
			node = node->children[0];
		else
			node = node->children[1];
	}

	return NULL;	// never reached
}

int mapa_canMoveTo(float px, float py, float pz, mapa_t *mapa)
{
    leaf_t  *leaf;
    vetor3d_t pos = {px,py,pz};

	leaf = mapa_discoverLeaf(&pos, mapa);

    return ((node_t *)leaf != mapa->nodes && leaf->visofs);
}

bool mapa_trace_bsp_visibilidade(mapa_t *mapa, vetor3d_t de, vetor3d_t para)
{
    // Raycast BSP traversal:
    // percorra os nós da BSP e pare se colidir com um sólido
    // você pode usar uma versão simplificada do seu trace de física

    vetor3d_t direcao = para;
	vetor_sub(&direcao, &de);

    float distancia = vetor_length(&direcao);

    // normalize a direção
	vetor_normalize(&direcao);

    // trace em pequenos passos (step trace simplificado)
    vetor3d_t pos = de;
    for (float t = 0; t <= distancia; t += 4.0f) {
        vetor3d_t ponto = {
            de.x + direcao.x * t,
            de.y + direcao.y * t,
            de.z + direcao.z * t
        };

        // testando se esse ponto está dentro de uma parte sólida
        if (!mapa_canMoveTo(ponto.x, ponto.y, ponto.z, mapa))
            return false;
    }

    return true;  // não encontrou obstáculo
}

bool ponto_dentro_face_2d(mapa_t *mapa, face_t *face, float px, float py)
{
    int i, *ledge;
    int cruzamentos = 0;

    edge_t      *e;
    vetor3d_t   *v1, *v2;

    ledge = (int *)face->firstledge;
    for (i=0; i<face->numedges; i++, ledge++) {
        if (*ledge >= 0) {
			v1 = &mapa->base[mapa->edges[*ledge].v[0]];
			v2 = &mapa->base[mapa->edges[*ledge].v[1]];
        } else {
			v1 = &mapa->base[mapa->edges[-*ledge].v[1]];
			v2 = &mapa->base[mapa->edges[-*ledge].v[0]];
        }
        
        // projetar os vértices no plano XY
        float x1 = v1->x, y1 = v1->y;
        float x2 = v2->x, y2 = v2->y;

        // checa se o segmento cruza uma linha horizontal à altura py
        if (((y1 <= py) && (y2 > py)) || ((y2 <= py) && (y1 > py))) {
            float t = (py - y1) / (y2 - y1);
            float x_intersec = x1 + t * (x2 - x1);

            if (x_intersec > px)
                cruzamentos++;
        }
    }

    return (cruzamentos % 2) == 1;  // ímpar: dentro
}

// A função checa apenas o eixo Z, e retorna true se colidiu com chão
bool mapa_trace_bsp_chao(mapa_t *mapa, vetor3d_t de, vetor3d_t para, float *alturaChao)
{
    int edgeId;

    // percorre faces do mapa (ou use o BSP traversal)
    face_t *face = mapa->faces;
    for (int i = 0; i < mapa->numfaces; i++, face++) {
		// Chao sempre tem side = 0!
		if (face->side) continue;
        
        if (!mapa_face_solida(face)) continue;

        // verifica se a face é plana no eixo Z (horizontal)
        if (fabs(face->plano->normal.x) > 0.1f || fabs(face->plano->normal.y) > 0.1f)
            continue;

        // verifica se houve interseção com o plano da face
        edgeId = (*face->firstledge > 0) ? *face->firstledge : -*face->firstledge;
        float zPlano = mapa->base[mapa->edges[edgeId].v[0]].z; // z constante
        if ((de.z >= zPlano && para.z < zPlano) || (de.z <= zPlano && para.z > zPlano)) {
            // se dentro da projeção da face (XY)
            float px = de.x;  // para simplificar, use a mesma X e Y
            float py = de.y;

            if (ponto_dentro_face_2d(mapa, face, px, py)) {
                *alturaChao = zPlano;
                return true;
            }
        }
    }

    return false;
}

vetor3d_t mapa_acha_posicao_spawn(mapa_t *mapa)
{
    for (int tentativas = 0; tentativas < 100; tentativas++) {
        // 1. pega limites globais do mapa
        float minX = mapa->bbMin.x;
        float maxX = mapa->bbMax.x;
        float minY = mapa->bbMin.y;
        float maxY = mapa->bbMax.y;

        // 2. gera ponto aleatório
        float x = minX + (rand() / (float)RAND_MAX) * (maxX - minX);
        float y = minY + (rand() / (float)RAND_MAX) * (maxY - minY);
        float z = mapa->bbMax.z; // começa lá em cima

        vetor3d_t origem = { x, y, z };
        vetor3d_t fim    = { x, y, mapa->bbMin.z - 512.0f };

        // 3. trace vertical para achar piso
        float alturaChao;
        if (!mapa_trace_bsp_chao(mapa, origem, fim, &alturaChao))
            continue; // nada sólido embaixo

        // 4. testa se o ponto está dentro de sólido
        vetor3d_t pos = { x, y, alturaChao };
        // if (mapa_ponto_em_solido(mapa, pos))
        //     continue;

        return pos;
    }

    return (vetor3d_t){ 0,0,0 }; // não achou lugar válido
}

/*
===================
Mod_DecompressVis
===================
*/
static byte	*mod_novis;
static int	mod_novis_capacity;

static byte	*mod_decompressed;
static int	mod_decompressed_capacity;

static byte *mapa_decompressVis (byte *in, mapa_t *mapa)
{
	int		c;
	byte	*out;
	byte	*outend;
	int		row;

	row = (mapa->numleafs + 7) >> 3;
	if (mod_decompressed == NULL || row > mod_decompressed_capacity)
	{
		mod_decompressed_capacity = row;
		mod_decompressed = (byte *) realloc (mod_decompressed, mod_decompressed_capacity);
		if (!mod_decompressed) {
			printf("Mod_DecompressVis: realloc() failed on %d bytes", mod_decompressed_capacity);
            exit(1);
        }
	}
	out = mod_decompressed;
	outend = mod_decompressed + row;

	if (!in)
	{	// no vis info, so make all visible
		while (row)
		{
			*out++ = 0xff;
			row--;
		}
		return mod_decompressed;
	}

	do
	{
		if (*in)
		{
			*out++ = *in++;
			continue;
		}

		c = in[1];
		in += 2;
		while (c)
		{
			if (out == outend)
			{
				// if(!mapa->viswarn) {
				// 	mapa->viswarn = true;
				// 	Con_Warning("Mod_DecompressVis: output overrun on model \"%s\"\n", model->name);
				// }
				return mod_decompressed;
			}
			*out++ = 0;
			c--;
		}
	} while (out - mod_decompressed < row);

	return mod_decompressed;
}

byte *mapa_noVisPVS (mapa_t *mapa)
{
	int pvsbytes;
 
	pvsbytes = (mapa->numleafs + 7) >> 3;
	if (mod_novis == NULL || pvsbytes > mod_novis_capacity)
	{
		mod_novis_capacity = pvsbytes;
		mod_novis = (byte *) realloc (mod_novis, mod_novis_capacity);
		if (!mod_novis) {
			printf("Mod_NoVisPVS: realloc() failed on %d bytes", mod_novis_capacity);
            exit(2);
        }
		
		memset(mod_novis, 0xff, mod_novis_capacity);
	}
	return mod_novis;
}

byte *mapa_leafVIS (leaf_t *leaf, mapa_t *mapa)
{
	if (leaf == mapa->leafs)
		return mapa_noVisPVS (mapa);
    
	return mapa_decompressVis (leaf->compressed_vis, mapa);
}

typedef enum
{
	CPE_NOTRUNC,		// return parse error in case of overflow
	CPE_ALLOWTRUNC		// truncate com_token in case of overflow
} cpe_mode;

char    com_token[1024];

/* for array size: */
#define Q_COUNTOF(x) (sizeof(x) / sizeof((x)[0]))

/*
==============
COM_ParseEx

Parse a token out of a string

The mode argument controls how overflow is handled:
- CPE_NOTRUNC:		return NULL (abort parsing)
- CPE_ALLOWTRUNC:	truncate com_token (ignore the extra characters in this token)
==============
*/
const char *COM_ParseEx (const char *data, cpe_mode mode)
{
	int		c;
	int		len;

	len = 0;
	com_token[0] = 0;

	if (!data)
		return NULL;

// skip whitespace
skipwhite:
	while ((c = *data) <= ' ')
	{
		if (c == 0)
			return NULL;	// end of file
		data++;
	}

// skip // comments
	if (c == '/' && data[1] == '/')
	{
		while (*data && *data != '\n')
			data++;
		goto skipwhite;
	}

// skip /*..*/ comments
	if (c == '/' && data[1] == '*')
	{
		data += 2;
		while (*data && !(*data == '*' && data[1] == '/'))
			data++;
		if (*data)
			data += 2;
		goto skipwhite;
	}

// handle quoted strings specially
	if (c == '\"')
	{
		data++;
		while (1)
		{
			if ((c = *data) != 0)
				++data;
			if (c == '\"' || !c)
			{
				com_token[len] = 0;
				return data;
			}
			if (len < Q_COUNTOF(com_token) - 1)
				com_token[len++] = c;
			else if (mode == CPE_NOTRUNC)
				return NULL;
		}
	}

// parse single characters
	if (c == '{' || c == '}'|| c == '('|| c == ')' || c == '\'' || c == ':')
	{
		if (len < Q_COUNTOF(com_token) - 1)
			com_token[len++] = c;
		else if (mode == CPE_NOTRUNC)
			return NULL;
		com_token[len] = 0;
		return data+1;
	}

// parse a regular word
	do
	{
		if (len < Q_COUNTOF(com_token) - 1)
			com_token[len++] = c;
		else if (mode == CPE_NOTRUNC)
			return NULL;
		data++;
		c = *data;
		/* commented out the check for ':' so that ip:port works */
		if (c == '{' || c == '}'|| c == '('|| c == ')' || c == '\''/* || c == ':' */)
			break;
	} while (c > 32);

	com_token[len] = 0;
	return data;
}


/*
==============
COM_Parse

Parse a token out of a string

Return NULL in case of overflow
==============
*/
const char *COM_Parse (const char *data)
{
	return COM_ParseEx (data, CPE_NOTRUNC);
}

/*
 * Copy src to string dst of size siz.  At most siz-1 characters
 * will be copied.  Always NUL terminates (unless siz == 0).
 * Returns strlen(src); if retval >= siz, truncation occurred.
 */

size_t
q_strlcpy (char *dst, const char *src, size_t siz)
{
	char *d = dst;
	const char *s = src;
	size_t n = siz;

	/* Copy as many bytes as will fit */
	if (n != 0) {
		while (--n != 0) {
			if ((*d++ = *s++) == '\0')
				break;
		}
	}

	/* Not enough room in dst, add NUL and traverse rest of src */
	if (n == 0) {
		if (siz != 0)
			*d = '\0';		/* NUL-terminate dst */
		while (*s++)
			;
	}

	return(s - src - 1);	/* count does not include NUL */
}

#define MAX_ATRS_ENTITIES 8

/*
Busca na string de entities as informações de player_start
*/
int mapa_loadEntities (mapa_t *mapa)
{
	char key[MAX_ATRS_ENTITIES][128], value[MAX_ATRS_ENTITIES][4096];
	const char *data;
    int inicio = 2;
    int idBloco = 0;
    int numAtr = 0;
    int i, j, ok = 0;
    int camX, camY, camZ;

	while (ok < 2)
	{
        if (inicio) {
            if (inicio == 1) {
                // printf(">>>>>>>>>>>> Novo bloco[%d]\n", idBloco);
                for (i=0; i<numAtr; i++) {
                    if (!strcmp(key[i], "classname") && !strcmp(value[i], "info_player_start")) {
                        for (j=0; j<numAtr; j++) {
                            if (j == i) continue;

                            if (!strcmp(key[j], "angle")) {
                                printf("Player Start > angulo [%s]\n", value[j]);
                                mapa->player_start_angle = atoi(value[j]);
                                ok++;
                            }

                            if (!strcmp(key[j], "origin")) {
                                sscanf(value[j], "%d %d %d", &camX, &camY, &camZ);
                                printf("Player Start > POS [%d %d %d]\n", camX, camY, camZ);
                                mapa->player_start.x = camX;
                                mapa->player_start.y = camY;
                                mapa->player_start.z = camZ;
                                ok++;
                            }
                        }
                    }
                }

                idBloco++;
                numAtr = 0;
            }

            data = (inicio == 2) ?
                COM_Parse(mapa->entities) :
                COM_Parse(data);
            
            if (!data)
                return 0; // OEF
            if (com_token[0] != '{')
                return 2; // error
            
            inicio = 0;
        }

		data = COM_Parse(data);
		if (!data)
			return 3; // error
        
		if (com_token[0] == '}') {
            inicio = 1;
            continue;
        }

        if (numAtr < MAX_ATRS_ENTITIES) {
            if (com_token[0] == '_')
                q_strlcpy(key[numAtr], com_token + 1, sizeof(key[0]));
            else
                q_strlcpy(key[numAtr], com_token, sizeof(key[0]));
            
            while (key[numAtr][0] && key[numAtr][strlen(key[numAtr])-1] == ' ') // remove trailing spaces
                key[numAtr][strlen(key[numAtr])-1] = 0;
        }
        
		data = COM_ParseEx(data, CPE_ALLOWTRUNC);
		if (!data)
			return 4; // error
        
        if (numAtr < MAX_ATRS_ENTITIES) {
		    q_strlcpy(value[numAtr], com_token, sizeof(value[0]));
            numAtr++;
        }
	}

    return 0; // never reached
}

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
	return map((mapa->bbMax.y + mapa->bbMin.y) - v, mapa->bbMin.y, mapa->bbMax.y, grafico_altura/2 + 10, grafico_altura - 10);
}

void mostraMapa2D(mapa_t *mapa, camera_t *cam, byte *vis)
{
	int mostraComVIS = !!(vis);
	vetor3d_t *b = mapa->base;
	ponto_t   *v = mapa->verts;
	edge_t    *e;
	byte       red,green,blue;

	for (int i=0; i < mapa->numverts; i++, b++, v++) {
		v->rot.x = b->x;
		v->rot.y = b->y;
		v->rot.z = b->z;

		v->screen.x = map_scaleX(b->x, mapa);
		v->screen.y = map_scaleY(b->y, mapa);
	}

	if (mostraComVIS) {
		int		 i, j, k, *ledge;
		leaf_t	*leaf;
		face_t	*face, **mark;

		for (i=0, face = mapa->faces; i < mapa->numfaces; i++, face++)
			face->drawn = 0;

		leaf = &mapa->leafs[1];
		for (i=0; i < mapa->numleafs - 1; i++, leaf++) {
			if (!(vis[i >> 3] & (1 << (i & 7)))) {
				// LEAF nao visivel
				red   = 200;
				green = 20;
				blue  = 20;
			} else {
				red   = 20;
				green = 200;
				blue  = 120;
			}

			// Render LEAF
			for (j=0, mark = leaf->firstmarksurface; j < leaf->nummarksurfaces; j++, mark++) {
				face = *mark;

				if (face->drawn)
					continue;
				face->drawn = 1;

				ledge = (int *)face->firstledge;
				for (k=0; k<face->numedges; k++, ledge++) {
					e = (*ledge >= 0) ? &mapa->edges[*ledge] : &mapa->edges[-*ledge];
					ponto_t *p1 = &mapa->verts[e->v[0]];
					ponto_t *p2 = &mapa->verts[e->v[1]];

					gfx_line( p1->screen.x,p1->screen.y, p2->screen.x,p2->screen.y, red,green,blue );
				}
			}
		}
	} else {
		int i;

		for (i=0, e = mapa->edges; i < mapa->numedges; i++, e++) {
			ponto_t *p1 = &mapa->verts[e->v[0]];
			ponto_t *p2 = &mapa->verts[e->v[1]];

			// if (i != 100) continue;

			gfx_line( p1->screen.x,p1->screen.y, p2->screen.x,p2->screen.y, 200,200,200 );

			// vetor3d_t *vxt = &mapa->base[e->v[0]];
			// printf("LL%d,%d,%d > ", (int)vxt->x, (int)vxt->y, (int)vxt->z);
		}
	}

	int camMX = map_scaleX(cam->pos.x, mapa);
	int camMY = map_scaleY(cam->pos.y, mapa);
	float camLenArrow = 10.0;
	float camAng = to_radians(cam->ang.y);
	int camAX = camMX + sin(camAng) * camLenArrow;
	int camAY = camMY - cos(camAng) * camLenArrow;
	
	grafico_xis( camMX, camMY, 255,100,100 );
	grafico_xis( camAX, camAY, 100,200,100 );
	gfx_line(camMX, camMY, camAX, camAY, 100,200,100);
}
