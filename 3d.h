#ifndef TRESD_H
#define TRESD_H

#include "QuakeViewer.h"

typedef struct
{
    int x, y;
} vetor2d_t;

typedef struct
{
    float x, y;
} vetor2df_t;

typedef struct
{
    float x, y, z;
} vetor3d_t;

typedef struct
{
    vetor3d_t	normal;
    vetor3d_t	ponto;
	float	    dist;
	int		    type;
    int         ON;     // backface culling
} plano_t;

typedef struct
{
    vetor3d_t rot;
    vetor2d_t screen;
    vetor2df_t tex;
    vetor2df_t tex_luz;
} ponto_t;

typedef struct
{
	vetor3d_t pos;
    vetor3d_t ang;
} camera_t;

#include "obj3d.h"

void vetor_add(vetor3d_t *um, vetor3d_t *dois);
void vetor_sub(vetor3d_t *um, vetor3d_t *dois);

float vetor_length(vetor3d_t *v);
vetor3d_t vetor_cross_product(vetor3d_t a, vetor3d_t b);
float vetor_dot_product(vetor3d_t a, vetor3d_t b);
void vetor_normalize(vetor3d_t *normal);

double to_radians(double deg);
void vetor_rotacao2DEixoX(vetor3d_t *p, int angulo);
void vetor_rotacao2DEixoY(vetor3d_t *p, int angulo);
void vetor_rotacao2DEixoZ(vetor3d_t *p, int angulo);

void camera_pitch(camera_t *cam, int step);
void camera_step(camera_t *cam, int step, mapa_t *mapa);
void camera_strafe(camera_t *cam, int step, mapa_t *mapa);

void vetor_projetaPonto3D(ponto_t *pnt, camera_t *cam);

#endif
