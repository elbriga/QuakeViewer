#include <math.h>

#include "3d.h"

// https://microstudio.dev/community/articles/how-to-make-your-own-3d-3d-projection/37/

int FOV = 120;

void projecao3D(ponto *p)
{
    p->sX = (FOV * p->x) / p->z;
    p->sY = (FOV * p->y) / p->z;
}

void rotacao2D(ponto *p, int angulo)
{
    p->rotSX = cos(angulo) * p->sX - sin(angulo) * p->sY;
    p->rotSY = sin(angulo) * p->sX + cos(angulo) * p->sY;
}

void rotacao3D(ponto *p, int angulo)
{
    ponto pAux;

    pAux.sX = p->z;
    pAux.sY = p->x;

    p->rotX = cos(angulo) * p->sX - sin(angulo) * p->sY;
    p->rotY = sin(angulo) * p->sX + cos(angulo) * p->sY;
    p->rotZ = sin(angulo) * p->sX + cos(angulo) * p->sY;
}


