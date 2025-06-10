#include <stdlib.h>
#include <math.h>

#include "3d.h"

// https://microstudio.dev/community/articles/how-to-make-your-own-3d-3d-projection/37/

int FOV = 120;

void projecao3D(ponto *p)
{
    p->screen.x = (FOV * p->rot.x) / p->rot.z;
    p->screen.y = (FOV * p->rot.y) / p->rot.z;
}

void rotacao2DEixoX(ponto *p, int angulo)
{
    p->rot.y = cos(angulo) * p->pos.y - sin(angulo) * p->pos.z;
    p->rot.z = sin(angulo) * p->pos.y + cos(angulo) * p->pos.z;
}

void rotacao2DEixoY(ponto *p, int angulo)
{
    p->rot.x = cos(angulo) * p->pos.x - sin(angulo) * p->pos.z;
    p->rot.z = sin(angulo) * p->pos.x + cos(angulo) * p->pos.z;
}

void rotacao2DEixoZ(ponto *p, int angulo)
{
    p->rot.x = cos(angulo) * p->pos.x - sin(angulo) * p->pos.y;
    p->rot.y = sin(angulo) * p->pos.x + cos(angulo) * p->pos.y;
}

void rotacao3D(ponto *p, int angulo)
{
    ponto pAux;
}

void freeObj3D(obj3d_t *obj)
{
    if (!obj) return;
    
    free(obj->framesanims);
    free(obj);
}
