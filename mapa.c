#include "3d.h"

int discover_leaf(vetor3d_t *pos, mapa_t *mapa)
{
    leaf_t *l = mapa->leafs;

    for (int i=0; i < mapa->numleafs; i++, l++) {
        if (pos->x >= l->mins[0] && pos->x <= l->maxs[0] &&
            pos->y >= l->mins[1] && pos->y <= l->maxs[1] &&
            pos->z >= l->mins[2] && pos->z <= l->maxs[2]) {
            
            return i;
        }
    }

    return -1;
}
