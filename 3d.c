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

void obj_projecao3D(camera_t *cam, obj3d_t *obj, int numFrame)
{
    int offset = numFrame * obj->numverts;

    for (int v=0; v<obj->numverts; v++) {
        vetor3d_t *base = &obj->frames[offset + v];
        ponto     *pnt  = &obj->verts[v];

        // Reset - Coordenadas de Objeto
        pnt->rot.x = base->x;
        pnt->rot.y = base->y;
        pnt->rot.z = base->z;

        // Rotacao do objeto - coordenadas de objeto
        rotacao2DEixoX(pnt, obj->rotacao.x);
        rotacao2DEixoY(pnt, obj->rotacao.y);
        rotacao2DEixoZ(pnt, obj->rotacao.z);

        // Coordenadas de Mundo - posicao do objeto e posicao da camera
        pnt->rot.x += obj->posicao.x + cam->pos.x;
        pnt->rot.y += obj->posicao.y + cam->pos.y;
        pnt->rot.z += obj->posicao.z + cam->pos.z;

        // Rotacao de Camera - coordenadas de camera
        rotacao2DEixoX(pnt, cam->ang.x);
        rotacao2DEixoY(pnt, cam->ang.y);
        rotacao2DEixoZ(pnt, cam->ang.z);

        // Projecao para 2D
        grafico_projecao3D(pnt);
    }
}

void obj_projecao3D_allFrames(camera_t *cam, obj3d_t *obj)
{
    for (int numFrame=0; numFrame < obj->numframes; numFrame++) {
        obj_projecao3D(cam, obj, numFrame);
    }
}

obj3d_t *obj_plano(int sizeX, int sizeY)
{
    obj3d_t *ret;

    int numtris  = (sizeX - 1) * 2 * (sizeY - 1);
    int numverts = (sizeX * sizeY);

    int totMemObj = sizeof(obj3d_t) +                 // ret
                    (numtris * sizeof(triangulo_t)) + // ret->tris
                    (16) +                            // ret->framenames
                    (numverts * sizeof(vetor3d_t)) +  // ret->frames
                    (numverts * sizeof(ponto));       // ret->verts

    ret = calloc(1, totMemObj);
    if (!ret) {
        printf("Erro malloc!\n\n");
        return NULL;
    }

    strcpy(ret->nome, "Plano");
    ret->tipo      = OBJ_TIPO_FLAT;
    ret->numframes = 1;
    ret->numverts  = numverts;
    ret->numtris   = numtris;

    ret->tris       = (triangulo_t *) &ret[1];
    ret->framenames = (char *)        &ret->tris[numtris];
    ret->frames     = (vetor3d_t *)   &ret->framenames[16];
    ret->verts      = (ponto *)       &ret->frames[numverts];
    ret->framesanims= NULL;

    strcpy(ret->framenames, "Plano01");

    // VXTs
    int PX, PZ = 0 - ((sizeY / 2) * 10);
    for (int Y=0; Y < sizeY; Y++) {
        PX = 0 - ((sizeX / 2) * 10);
        for (int X=0; X < sizeX; X++) {
            vetor3d_t *p = &ret->frames[X + Y * sizeX];

            p->x = PX;
            p->y = 0;
            p->z = PZ;

            PX += 10;
        }
        PZ += 10;
    }

    // TRIs
    cor_t cor_branco = { 255,255,255 };
    cor_t cor_cinza  = { 155,155,155 };
    for (int Y=0; Y < sizeY - 1; Y++) {
        for (int X=0; X < sizeX - 1; X++) {
            triangulo_t *tri1 = &ret->tris[(X*2) + Y * ((sizeX - 1) * 2)];
            triangulo_t *tri2 = &ret->tris[(X*2) + Y * ((sizeX - 1) * 2) + 1];

            tri1->cor  = cor_branco;
            tri1->v[0] = X + Y * sizeX;
            tri1->v[1] = X + Y * sizeX + 1;
            tri1->v[2] = X + (Y + 1) * sizeX;

            tri2->cor  = cor_cinza;
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
