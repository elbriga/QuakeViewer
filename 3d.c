#include <stdlib.h>
#include <math.h>

#include "3d.h"
#include "gfx_ptBR.h"
#include "render.h"

#ifndef M_PI
#define M_PI		3.14159265358979323846	/* pi */
#endif

// https://microstudio.dev/community/articles/how-to-make-your-own-3d-3d-projection/37/

double to_radians(double deg)
{
  return deg * M_PI / 180.0;
}

void rotacao2DEixoX(ponto *p, int angulo)
{
    double anguloRad = to_radians(angulo);
    float valY = p->rot.y;

    p->rot.y = cos(anguloRad) * valY - sin(anguloRad) * p->rot.z;
    p->rot.z = sin(anguloRad) * valY + cos(anguloRad) * p->rot.z;
}

void rotacao2DEixoY(ponto *p, int angulo)
{
    double anguloRad = to_radians(angulo);
    float valX = p->rot.x;

    p->rot.x = cos(anguloRad) * valX - sin(anguloRad) * p->rot.z;
    p->rot.z = sin(anguloRad) * valX + cos(anguloRad) * p->rot.z;
}

void rotacao2DEixoZ(ponto *p, int angulo)
{
    double anguloRad = to_radians(angulo);
    float valX = p->rot.x;

    p->rot.x = cos(anguloRad) * valX - sin(anguloRad) * p->rot.y;
    p->rot.y = sin(anguloRad) * valX + cos(anguloRad) * p->rot.y;
}

void obj_translate(obj3d_t *obj, vetor3d_t pos)
{

}

void obj_reset(obj3d_t *obj, int numFrame)
{
    int base = numFrame * obj->numverts;

    for (int v=0; v<obj->numverts; v++) {
        obj->frames[base + v].rot.x = obj->frames[base + v].pos.x;
        obj->frames[base + v].rot.y = obj->frames[base + v].pos.y;
        obj->frames[base + v].rot.z = obj->frames[base + v].pos.z;
    }
}

void obj_projecao3D(camera_t *cam, obj3d_t *obj, int numFrame)
{
    int base = numFrame * obj->numverts;

    // obj_reset(obj, numFrame);
    for (int v=0; v<obj->numverts; v++) {
        ponto *pnt = &obj->frames[base + v];

        pnt->rot.x = pnt->pos.x;
        pnt->rot.y = pnt->pos.y;
        pnt->rot.z = pnt->pos.z;

        rotacao2DEixoX(pnt, obj->rotacao.x);
        rotacao2DEixoY(pnt, obj->rotacao.y);
        rotacao2DEixoZ(pnt, obj->rotacao.z);

        pnt->rot.x += obj->posicao.x;
        pnt->rot.y += obj->posicao.y;
        pnt->rot.z += obj->posicao.z;

        grafico_projecao3D(pnt);
    }
}

void obj_projecao3D_allFrames(camera_t *cam, obj3d_t *obj)
{
    for (int numFrame=0; numFrame < obj->numframes; numFrame++) {
        obj_projecao3D(cam, obj, numFrame);
    }
}

void obj_save(obj3d_t *obj)
{
    for (int v=0; v<obj->numverts * obj->numframes; v++) {
        obj->frames[v].pos.x = obj->frames[v].rot.x;
        obj->frames[v].pos.y = obj->frames[v].rot.y;
        obj->frames[v].pos.z = obj->frames[v].rot.z;
    }
}

void freeObj3D(obj3d_t *obj)
{
    if (!obj) return;
    
    free(obj->framesanims);
    free(obj);
}
