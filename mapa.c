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
