#include <stdio.h>

#include "3d.h"

leaf_t *discover_leaf(vetor3d_t *pos, mapa_t *mapa)
{
    node_t  *node = mapa->nodes;
    plano_t *plano;
    float    d;

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
