#include <stdio.h>
#include <stdlib.h>
#include <memory.h>

#include "3d.h"

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
		d = dot_product (*pos, plano->normal) - plano->dist;
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
=============
mapa_getEntities

Busca na string de entities uma chave e retorna seu valor
=============
*/
int mapa_loadEntities (mapa_t *mapa, camera_t *cam)
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
                                cam->ang.x = atoi(value[j]);
                                ok++;
                            }

                            if (!strcmp(key[j], "origin")) {
                                sscanf(value[j], "%d %d %d", &camX, &camY, &camZ);
                                printf("Player Start > POS [%d %d %d]\n", camX, camY, camZ);
                                cam->pos.x = camX;
                                cam->pos.y = camY;
                                cam->pos.z = camZ;
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
