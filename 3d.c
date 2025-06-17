#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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

        pnt->rot.x += obj->posicao.x + cam->pos.x;
        pnt->rot.y += obj->posicao.y + cam->pos.y;
        pnt->rot.z += obj->posicao.z + cam->pos.z;

        rotacao2DEixoX(pnt, cam->ang.x);
        rotacao2DEixoY(pnt, cam->ang.y);
        rotacao2DEixoZ(pnt, cam->ang.z);

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

obj3d_t *obj_plano(int sizeX, int sizeY)
{
    obj3d_t *ret;

    int totMemObj = sizeof(obj3d_t) +                                       // ret
                    ((sizeX - 1) * 2 * (sizeY - 1) * sizeof(triangulo_t)) + // ret->tris
                    (16) +                                                  // ret->framenames
                    (sizeX * sizeY * sizeof(ponto));                        // ret->frames

    ret = calloc(1, totMemObj);
    if (!ret) {
        printf("Erro malloc!\n\n");
        return NULL;
    }

    strcpy(ret->nome, "Plano");
    ret->tipo      = OBJ_TIPO_WIRE;
    ret->numframes = 1;
    ret->numverts  = sizeX * sizeY;
    ret->numtris   = (sizeX - 1) * 2 * (sizeY - 1);

    ret->tris       = (triangulo_t *) &ret[1];
    ret->framenames = (char *)        &ret->tris[ret->numtris];
    ret->frames     = (ponto *)       &ret->framenames[17];

    strcpy(ret->framenames, "Plano01");

    // VXTs
    int PX, PZ = 0 - ((sizeY / 2) * 10);
    for (int Y=0; Y < sizeY; Y++) {
        PX = 0 - ((sizeX / 2) * 10);
        for (int X=0; X < sizeX; X++) {
            ponto *p = &ret->frames[X + Y * sizeX];

            p->pos.x = PX;
            p->pos.y = 0;
            p->pos.z = PZ;

            PX += 10;
        }
        PZ += 10;
    }

    // TRIs
    for (int Y=0; Y < sizeY - 1; Y++) {
        for (int X=0; X < sizeX - 1; X++) {
            triangulo_t *tri1 = &ret->tris[X + Y * ((sizeX - 1) * 2)];
            triangulo_t *tri2 = &ret->tris[X + Y * ((sizeX - 1) * 2) + 1];

            tri1->v[0] = X + Y * sizeX;
            tri1->v[1] = X + Y * sizeX + 1;
            tri1->v[2] = X + (Y + 1) * sizeX;

            tri2->v[0] = X + Y * sizeX + 1;
            tri2->v[1] = X + (Y + 1) * sizeX + 1;
            tri2->v[2] = X + (Y + 1) * sizeX;
        }
    }

    return ret;
}

void freeObj3D(obj3d_t *obj)
{
    if (!obj) return;
    
    if(obj->framesanims)
        free(obj->framesanims);
    
    free(obj);
}
