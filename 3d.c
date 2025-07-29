#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "3d.h"
#include "obj3d.h"
#include "grafico.h"
#include "mapa.h"

// https://microstudio.dev/community/articles/how-to-make-your-own-3d-3d-projection/37/

double to_radians(double deg)
{
  return deg * M_PI / 180.0;
}

void vetor_add(vetor3d_t *um, vetor3d_t *dois)
{
    um->x += dois->x;
    um->y += dois->y;
    um->z += dois->z;
}

void vetor_sub(vetor3d_t *um, vetor3d_t *dois)
{
    um->x -= dois->x;
    um->y -= dois->y;
    um->z -= dois->z;
}

float vetor_length(vetor3d_t *v)
{
    return sqrt(v->x*v->x + v->y*v->y + v->z*v->z);
}

void vetor_normalize(vetor3d_t *normal)
{
    float len = 1.0 / vetor_length(normal);
    normal->x *= len;
    normal->y *= len;
    normal->z *= len;
}

vetor3d_t vetor_cross_product(vetor3d_t a, vetor3d_t b)
{
    vetor3d_t result;

    result.x = a.y * b.z - a.z * b.y;
    result.y = a.z * b.x - a.x * b.z;
    result.z = a.x * b.y - a.y * b.x;

    return result;
}

float vetor_dot_product(vetor3d_t a, vetor3d_t b)
{
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

void camera_pitch(camera_t *cam, int step)
{
    cam->ang.y = (int)(cam->ang.y + step) % 360;
}

void camera_step(camera_t *cam, int step, mapa_t *mapa)
{
    double anguloRad = to_radians(cam->ang.y);
    float newPX = cam->pos.x - sin(anguloRad) * step;
    float newPY = cam->pos.y - cos(anguloRad) * step;

    if (mapa_canMoveTo(newPX, newPY, cam->pos.z, mapa)) {
        cam->pos.x = newPX;
        cam->pos.y = newPY;
    }
}

void camera_strafe(camera_t *cam, int step, mapa_t *mapa)
{
    double anguloRad = to_radians(cam->ang.y);
    float newPX = cam->pos.x - cos(anguloRad) * step;
    float newPY = cam->pos.y + sin(anguloRad) * step;

    if (mapa_canMoveTo(newPX, newPY, cam->pos.z, mapa)) {
        cam->pos.x = newPX;
        cam->pos.y = newPY;
    }
}

void vetor_rotacao2DEixoX(vetor3d_t *p, int angulo)
{
    double anguloRad = to_radians(angulo);
    float valY = p->y;

    p->y = cos(anguloRad) * valY - sin(anguloRad) * p->z;
    p->z = sin(anguloRad) * valY + cos(anguloRad) * p->z;
}

void vetor_rotacao2DEixoY(vetor3d_t *p, int angulo)
{
    double anguloRad = to_radians(angulo);
    float valX = p->x;

    p->x = cos(anguloRad) * valX - sin(anguloRad) * p->z;
    p->z = sin(anguloRad) * valX + cos(anguloRad) * p->z;
}

void vetor_rotacao2DEixoZ(vetor3d_t *p, int angulo)
{
    double anguloRad = to_radians(angulo);
    float valX = p->x;

    p->x = cos(anguloRad) * valX - sin(anguloRad) * p->y;
    p->y = sin(anguloRad) * valX + cos(anguloRad) * p->y;
}

void vetor_projetaPonto3D(ponto_t *pnt, camera_t *cam)
{
    // Coordenadas de Mundo - posicao do objeto e posicao da camera
        pnt->rot.x -= cam->pos.x;
        pnt->rot.y -= cam->pos.y;
        pnt->rot.z -= cam->pos.z;// + obj->offsetChao;

        // Rotacao de Camera - coordenadas de camera
        vetor_rotacao2DEixoX(&pnt->rot, cam->ang.x);
        vetor_rotacao2DEixoY(&pnt->rot, cam->ang.y);
        vetor_rotacao2DEixoZ(&pnt->rot, cam->ang.z);

        // Projecao para 2D - so depois do clipping
        grafico_projecao3D(pnt);
}
