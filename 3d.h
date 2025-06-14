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

    // run
    vetor3d_t posicao;
    vetor3d_t rotacao;
} obj3d_t;

typedef struct
{
	vetor3d_t pos;
    vetor3d_t ang;
} camera_t;

void freeObj3D(obj3d_t *obj);

void obj_reset(obj3d_t *obj, int numFrame);
void obj_projecao3D(camera_t *cam, obj3d_t *obj, int numFrame);
void obj_save(obj3d_t *obj);

void rotacao2DEixoX(ponto *p, int angulo);
void rotacao2DEixoY(ponto *p, int angulo);
void rotacao2DEixoZ(ponto *p, int angulo);

#endif
