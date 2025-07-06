#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <float.h> // Required for FLT_MAX

#include "3d.h"
#include "bspfile.h"
// #include "gfx_ptBR.h"

void *readFile(char *fileName)
{
    void *buffer;
    FILE *fp;
    long size;

    fp = fopen(fileName, "rb");
    if (!fp) {
        return NULL;
    }

    fseek(fp, 0, SEEK_END);
    size = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    buffer = malloc(size);
    if (!buffer) {
        return NULL;
    }

    fread(buffer,1,size,fp);
    fclose(fp);

    return buffer;
}

float map(float x, float in_min, float in_max, float out_min, float out_max)
{
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

int loadVertexes (mapa_t *mapa, lump_t *l, byte *buffer)
{
	dvertex_t   *in;
	vetor3d_t   *base;
	int         i;

	mapa->numverts = l->filelen / sizeof(dvertex_t);

    mapa->base = (vetor3d_t *) malloc(mapa->numverts * sizeof(vetor3d_t));
    if (!mapa->base) return 1;

    mapa->verts = (ponto_t *) malloc(mapa->numverts * sizeof(ponto_t));
    if (!mapa->verts) return 2;

    base = mapa->base;
	in = (dvertex_t *)(buffer + l->fileofs);
	for (i=0 ; i<mapa->numverts ; i++, in++, base++) {
        base->x = in->point[0];
        base->y = in->point[1];
        base->z = in->point[2];

        rotacao2DEixoX(base, 90);
	}

    return 0;
}

int loadEdges (mapa_t *mapa, lump_t *l, byte *buffer)
{
    dsedge_t *in = (dsedge_t *)(buffer + l->fileofs);
    edge_t   *edge;
    int      i;
    
    mapa->numedges = l->filelen / sizeof(dsedge_t);

    mapa->edges = (edge_t *) malloc(mapa->numedges * sizeof(edge_t));
    if (!mapa->edges) return 1;

    edge = mapa->edges;
    for (i=0 ; i < mapa->numedges ; i++, in++, edge++) {
        edge->v[0] = in->v[0];
        edge->v[1] = in->v[1];

        // float xi = mapa->verts[in->v[0]].screen.x;
        // float xf = mapa->verts[in->v[1]].screen.x;
        // float yi = mapa->verts[in->v[0]].screen.y;
        // float yf = mapa->verts[in->v[1]].screen.y;

        // printf("Edge [%d] = [%f,%f] > [%f,%f]\n", i, xi,yi, xf,yf);
    }

    return 0;
}

int loadSurfEdges (mapa_t *mapa, lump_t *l, byte *buffer)
{
    int *in = (int *)(buffer + l->fileofs), *out;
    
    mapa->numledges = l->filelen / sizeof(int);

    mapa->ledges = (int *) malloc(mapa->numledges * sizeof(int));
    if (!mapa->ledges) return 1;

    memcpy(mapa->ledges, in, l->filelen);

    out = mapa->ledges;
    for (int i=0; i < mapa->numledges; i++, out++) {
        printf(">>> %d\n", *out);
    }

    return 0;
}

int loadPlanes (mapa_t *mapa, lump_t *l, byte *buffer)
{
    dplane_t *in = (dplane_t *)(buffer + l->fileofs);
    plano_t  *plano;
    int      i;
    
    mapa->numplanes = l->filelen / sizeof(dplane_t);

    mapa->planes = (plano_t *) malloc(mapa->numplanes * sizeof(plano_t));
    if (!mapa->planes) return 1;

    plano = mapa->planes;
    for (i=0 ; i < mapa->numplanes ; i++, in++, plano++) {
        plano->normal.x = in->normal[0];
        plano->normal.y = in->normal[1];
        plano->normal.z = in->normal[2];

        rotacao2DEixoX(&plano->normal, 90);

        plano->dist = in->dist;
        plano->type = in->type;
    }

    return 0;
}

int loadEntities (mapa_t *mapa, lump_t *l, byte *buffer)
{
    char *ents = (char *)(buffer + l->fileofs);
    
    mapa->entitieslen = strlen(ents);
    mapa->entities = (char *) malloc(mapa->entitieslen);
    if (!mapa->entities) return 1;

    memcpy(mapa->entities, ents, mapa->entitieslen);

    return 0;
}

int loadFaces (mapa_t *mapa, lump_t *l, byte *buffer)
{
	dsface_t	*ins;
    triangulo_t *tri;
    edge_t      *edge;
    int         i, *ledge;

    if (!mapa->ledges) {
        return 2;
    }

    mapa->numtris = l->filelen / sizeof(dsface_t);
    mapa->tris    = (triangulo_t *) malloc(mapa->numtris * sizeof(triangulo_t));
    if (!mapa->tris) return 1;

	ins = (dsface_t *)(buffer + l->fileofs);
    tri = (triangulo_t *)mapa->tris;
    for (i=0; i < mapa->numtris; i++, ins++, tri++) {
        // printf("face[%d] > numEdges: %d\n", i, ins->numedges);

        ledge = (int *)&mapa->ledges[ins->firstedge];
        for (int v=0; v < 3; v++, ledge++) {
            if (*ledge < 0) {
                edge = (edge_t *)&mapa->edges[-*ledge];
                tri->v[v] = edge->v[1];
            } else {
                edge = (edge_t *)&mapa->edges[*ledge];
                tri->v[v] = edge->v[0];
            }
        }

        tri->cor.r = 255;
        tri->cor.g = 255;
        tri->cor.b = 255;

        tri->planenum = ins->planenum;
        tri->texinfo  = ins->texinfo;
    }
    // grafico_tecla_espera();

    return 0;
}

int loadTexInfo (mapa_t *mapa, lump_t *l, byte *buffer)
{
    texinfo_t     *in = (texinfo_t *)(buffer + l->fileofs);
    textureinfo_t *out;

    mapa->numtexinfo = l->filelen / sizeof(texinfo_t);
    mapa->texinfo = (textureinfo_t *) malloc(mapa->numtexinfo * sizeof(textureinfo_t));
    if (!mapa->texinfo) return 1;

    out = mapa->texinfo;
    for (int i=0 ; i < mapa->numtexinfo ; i++, in++, out++) {
        out->vetorS.x = in->vecs[0][0];
        out->vetorS.y = in->vecs[0][1];
        out->vetorS.z = in->vecs[0][2];
        out->distS    = in->vecs[0][3];

        out->vetorT.x = in->vecs[1][0];
        out->vetorT.y = in->vecs[1][1];
        out->vetorT.z = in->vecs[1][2];
        out->distT    = in->vecs[1][3];

        rotacao2DEixoX(&out->vetorS, 90);
        rotacao2DEixoX(&out->vetorT, 90);

        out->miptex = in->miptex;
        out->flags  = in->flags;
    }
}

int loadTextures (mapa_t *mapa, lump_t *l, byte *buffer)
{
    dmiptexlump_t   *m = (dmiptexlump_t *)(buffer + l->fileofs);
    miptex_t        *mt;
    texture_t       *tex;
    char            *pixels;

    if (!l->filelen) {
        printf("Sem Texturas!\n");
        return 1;
    }

    // printf("Texture:\nNumMipTex: %d\n", m->nummiptex);

    mapa->numtextures = m->nummiptex;
    mapa->textures = (texture_t *) calloc(mapa->numtextures, sizeof(texture_t));
    if (!mapa->textures) return 1;

    tex = mapa->textures;
    for (int i=0; i < m->nummiptex; i++, tex++) {
        mt = (miptex_t *)((byte *)m + m->dataofs[i]);

        strncpy(tex->name, mt->name, 16);
        tex->width  = mt->width;
        tex->height = mt->height;

        tex->data = malloc(tex->width * tex->height);
        // TODO check

        pixels = (char *)(mt + 1);
        memcpy(tex->data, pixels, tex->width * tex->height);
    }

    return 0;
}

mapa_t *readBsp(char *fileName)
{
    mapa_t      *mapa;
    void        *buffer;
    dheader_t   *header;

    buffer = readFile(fileName);

    header = (dheader_t *)buffer;
    printf("Header version [%d]\n\n", header->version);

    mapa = calloc(1, sizeof(mapa_t));
    if (!mapa) {
        printf("readBsp: Erro malloc!\n\n");
        return NULL;
    }

    strncpy(mapa->nome, fileName, 64);
    mapa->tipo = OBJ_TIPO_MAPA;

    loadVertexes(mapa, &header->lumps[LUMP_VERTEXES], buffer);
    if (!mapa->numverts || !mapa->base || !mapa->verts) {
        printf("readBsp: erro loadVertexes!\n\n");
        freeMapa3D(mapa);
        return NULL;
    }

    loadPlanes(mapa, &header->lumps[LUMP_PLANES], buffer);
    if (!mapa->numplanes || !mapa->planes) {
        printf("readBsp: erro loadPlanes!\n\n");
        freeMapa3D(mapa);
        return NULL;
    }

    loadEdges(mapa, &header->lumps[LUMP_EDGES], buffer);
    if (!mapa->numedges || !mapa->edges) {
        printf("readBsp: erro loadEdges!\n\n");
        freeMapa3D(mapa);
        return NULL;
    }

    loadSurfEdges(mapa, &header->lumps[LUMP_SURFEDGES], buffer);
    if (!mapa->numledges || !mapa->ledges) {
        printf("readBsp: erro loadSurfEdges!\n\n");
        freeMapa3D(mapa);
        return NULL;
    }

    loadFaces(mapa, &header->lumps[LUMP_FACES], buffer);
    if (!mapa->numtris || !mapa->tris) {
        printf("readBsp: erro loadFaces!\n\n");
        freeMapa3D(mapa);
        return NULL;
    }

    loadTextures(mapa, &header->lumps[LUMP_TEXTURES], buffer);
    if (!mapa->numtextures || !mapa->textures) {
        printf("readBsp: erro loadTextures!\n\n");
        freeMapa3D(mapa);
        return NULL;
    }

    loadTexInfo(mapa, &header->lumps[LUMP_TEXINFO], buffer);
    if (!mapa->numtexinfo || !mapa->texinfo) {
        printf("readBsp: erro loadTexInfo!\n\n");
        freeMapa3D(mapa);
        return NULL;
    }

    loadEntities(mapa, &header->lumps[LUMP_ENTITIES], buffer);
    if (!mapa->entitieslen || !mapa->entities) {
        printf("readBsp: erro loadEntities!\n\n");
        freeMapa3D(mapa);
        return NULL;
    }
    printf("ents: %s\n\n", mapa->entities);

    free(buffer);

    return mapa;

    /*for (int l=0; l < HEADER_LUMPS; l++) {
        printf("Lump [%d] -> ofs:%d - size:%d\n", l, header->lumps[l].fileofs, header->lumps[l].filelen);

        switch (l) {
            case LUMP_ENTITIES:     break;
            case LUMP_PLANES:       break;
            case LUMP_TEXTURES:     break;
            case LUMP_VERTEXES:     break;
            case LUMP_VISIBILITY:   break;
            case LUMP_NODES:        break;
            case LUMP_TEXINFO:      break;
            case LUMP_FACES:        break;
            case LUMP_LIGHTING:     break;
            case LUMP_CLIPNODES:    break;
            case LUMP_LEAFS:        break;
            case LUMP_MARKSURFACES: break;
            case LUMP_EDGES:        break;
            case LUMP_SURFEDGES:    break;
            case LUMP_MODELS:       break;
        }
    }*/
}
