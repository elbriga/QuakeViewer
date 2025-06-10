#ifndef gfx3D_H
#define gfx3D_H

typedef struct
{
    int x, y;
} vetor2d_t;

typedef struct
{
    float x, y, z;
} vetor3d_t;

typedef struct
{
    int r, g, b;
} cor_t;

typedef struct {
	int		onseam;
	int		s;
	int		t;
} skinvert_t;

typedef struct
{
    vetor3d_t pos;
    vetor3d_t rot;

    vetor2d_t screen;
} ponto;

typedef struct
{
    int v[3];
    char isFront;
} triangulo_t;

typedef struct
{
	int frameI;
    int frameF;
} animationframes_t;

typedef struct
{
    char nome[64];

    int numverts;
    int numtris;
    int numframes;
    int totAnims;

    int   skinwidth;
    int   skinheight;

    char       *skin;
    skinvert_t *skinmap;

    char        *framenames;
    ponto       *frames;
    triangulo_t *tris;

    animationframes_t *framesanims;
} obj3d_t;

void freeObj3D(obj3d_t *obj);

#endif
