#ifndef gfx3D_H
#define gfx3D_H

#define OBJ_TIPO_WIRE    1
#define OBJ_TIPO_FLAT    2
#define OBJ_TIPO_GOURAD  4
#define OBJ_TIPO_TEXTURE 8
#define OBJ_TIPO_MAPA    256

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
    vetor3d_t	normal;
	float	    dist;
	int		    type;
} plano_t;

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
    vetor3d_t rot;
    vetor2d_t screen;
    vetor2d_t tex;
} ponto_t;

typedef struct
{
    int v[2];
} edge_t;

typedef struct
{
    int v[3];
    vetor3d_t normal;
    char isFront;
    cor_t cor;
    int planenum;
    int texinfo;
} triangulo_t;

typedef struct
{
    short		planenum;
	short		side;

	int			firstedge;
	short		numedges;
	short		texinfo;

    vetor3d_t   normal;
} face_t;

typedef struct
{
	char		name[16];
	unsigned	width, height;
    char        *data;
} texture_t;

typedef struct
{
    vetor3d_t   vetorS;
    float       distS;
    
    vetor3d_t   vetorT;
    float       distT;

	int			miptex;
	int			flags;
} textureinfo_t;

typedef struct
{
	int frameI;
    int frameF;
} animationframes_t;

typedef struct
{
    char nome[64];

    int tipo;
    int numverts;
    int numtris;
    int numframes;
    int totAnims;

    int   skinwidth;
    int   skinheight;

    char       *skin;
    skinvert_t *skinmap;

    char        *framenames;
    vetor3d_t   *frames;
    triangulo_t *tris;

    ponto_t       *verts;

    animationframes_t *framesanims;

    vetor3d_t *trisnormals;

    // run
    vetor3d_t posicao;
    vetor3d_t rotacao;
} obj3d_t;

typedef struct
{
    char nome[64];

    int tipo;
    int numverts;
    int numedges;
    int numledges;
    int numplanes;
    int numfaces;
    int numtextures;
    int numtexinfo;

    vetor3d_t bbMin, bbMax;

    vetor3d_t   *base;

    edge_t      *edges;
    int         *ledges;

    plano_t       *planes;
    face_t        *faces;
    texture_t     *textures;
    textureinfo_t *texinfo;

    char *entities;
    int   entitieslen;

    ponto_t *verts;
} mapa_t;

typedef struct
{
	vetor3d_t pos;
    vetor3d_t ang;
} camera_t;

void freeObj3D(obj3d_t *obj);
void freeMapa3D(mapa_t *mapa);

float dot_product(vetor3d_t a, vetor3d_t b);
void normalize(vetor3d_t *normal);

void obj_calculate_face_normals(obj3d_t *obj);

void obj_projecao3D(camera_t *cam, obj3d_t *obj, int numFrame);

void mapa_projecao3D(camera_t *cam, mapa_t *mapa);

obj3d_t *obj_plano(int sizeX, int sizeY);

void rotacao2DEixoX(vetor3d_t *p, int angulo);
void rotacao2DEixoY(vetor3d_t *p, int angulo);
void rotacao2DEixoZ(vetor3d_t *p, int angulo);

void camera_pitch(camera_t *cam, int step);
void camera_step(camera_t *cam, int step);
void camera_strafe(camera_t *cam, int step);

#endif
