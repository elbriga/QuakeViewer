#include <stdlib.h>
#include <math.h>

#include "3d.h"

// https://microstudio.dev/community/articles/how-to-make-your-own-3d-3d-projection/37/

int FOV = 120;

void projecao3D(ponto *p)
{
    p->screen.x = (FOV * p->pos.x) / p->pos.z;
    p->screen.y = (FOV * p->pos.y) / p->pos.z;
}

void rotacao2D(ponto *p, int angulo)
{
    p->rot.x = cos(angulo) * p->screen.x - sin(angulo) * p->screen.y;
    p->rot.y = sin(angulo) * p->screen.x + cos(angulo) * p->screen.y;
}

void rotacao3D(ponto *p, int angulo)
{
    ponto pAux;
}

void freeObj3D(obj3d_t *obj)
{
    if (!obj) return;

    if (obj->frames) {
        for (int c=0; c<obj->numframes; c++) {
            if (obj->frames[c].verts) {
                free(obj->frames[c].verts);
            }
        }
    }

    free(obj->framesanims);
    free(obj->skin);
    free(obj->skinmap);
    free(obj->frames);
    free(obj->tris);
    free(obj);
}
