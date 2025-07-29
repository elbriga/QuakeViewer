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

float vector_length(vetor3d_t *v);
vetor3d_t cross_product(vetor3d_t a, vetor3d_t b);
float dot_product(vetor3d_t a, vetor3d_t b);
void normalize(vetor3d_t *normal);

double to_radians(double deg);
void rotacao2DEixoX(vetor3d_t *p, int angulo);
void rotacao2DEixoY(vetor3d_t *p, int angulo);
void rotacao2DEixoZ(vetor3d_t *p, int angulo);

void camera_pitch(camera_t *cam, int step);
void camera_step(camera_t *cam, int step, mapa_t *mapa);
void camera_strafe(camera_t *cam, int step, mapa_t *mapa);

void projetaPonto3D(ponto_t *pnt, camera_t *cam);

#endif
