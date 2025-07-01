#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <float.h> // Required for FLT_MAX

#include "3d.h"
#include "bspfile.h"
#include "gfx_ptBR.h"

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

int loadPlanes (mapa_t *mapa, lump_t *l, byte *buffer)
{
    dplane_t *in = (dplane_t *)(buffer + l->fileofs);
    plane_t  *plane;
    int      i;
    
    mapa->numplanes = l->filelen / sizeof(dplane_t);

    mapa->planes = (plane_t *) malloc(mapa->numplanes * sizeof(plane_t));
    if (!mapa->planes) return 1;

    plane = mapa->planes;
    for (i=0 ; i < mapa->numplanes ; i++, in++, plane++) {
        plane->normal[0] = in->normal[0];
        plane->normal[1] = in->normal[1];
        plane->normal[2] = in->normal[2];

        plane->dist = in->dist;
        plane->type = in->type;
    }

    return 0;
}

int loadEntities (mapa_t *mapa, lump_t *l, byte *buffer)
{
    char *ents = (char *)(buffer + l->fileofs);

    printf("ents: %s\n\n", ents);
    grafico_tecla_espera();

    return 0;
}

int loadFaces (mapa_t *mapa, lump_t *l, byte *buffer)
{
	dsface_t	*ins;
    triangulo_t *tri;
    edge_t      *edge;
    int         i;

    mapa->numtris = l->filelen / sizeof(dsface_t);
    mapa->tris    = (triangulo_t *) malloc(mapa->numtris * sizeof(triangulo_t));
    if (!mapa->tris) return 1;

	ins = (dsface_t *)(buffer + l->fileofs);
    tri = (triangulo_t *)mapa->tris;
    for (i=0; i < mapa->numtris; i++, ins++, tri++) {
        printf("face[%d] > numEdges: %d\n", i, ins->numedges);

        edge = (edge_t *)&mapa->edges[ins->firstedge];
        for (int v=0; v < 3; v++, edge++) {
            tri->v[v] = edge->v[0];
        }

        tri->planenum = ins->planenum;
        tri->cor.r = 255;
        tri->cor.g = 255;
        tri->cor.b = 255;
    }
    grafico_tecla_espera();

    return 0;
}

int loadTextures (mapa_t *mapa, lump_t *l, byte *buffer, char paleta[256][3])
{
    dmiptexlump_t   *m = (dmiptexlump_t *)(buffer + l->fileofs);
    miptex_t        *mt;
    int ofs = 0;

    if (!l->filelen) {
        printf("Sem Texturas!\n");
        return 1;
    }

    printf("Texture:\nNumMipTex: %d\n", m->nummiptex);
    for (int i=0; i < m->nummiptex; i++) {
        mt = (miptex_t *)((byte *)m + m->dataofs[i]);
        printf("Texture[%d]: %s\n", i, mt->name);

        //grafico_limpa();

        char *pixels = (char *)(mt + 1);
        for (int y=0; y < mt->height; y++) {
            int yOfs = y * mt->width;
            for (int x=0; x < mt->width; x++) {
                byte cor = pixels[x + yOfs];

                grafico_cor(paleta[cor][0], paleta[cor][1], paleta[cor][2]);
                grafico_ponto(x+ofs, y+ofs);
            }
        }

        ofs += 5;
    }

    grafico_mostra();
    grafico_tecla_espera();

    return 0;
}

mapa_t *readBsp(char *fileName, char paleta[256][3])
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

    loadFaces(mapa, &header->lumps[LUMP_FACES], buffer);
    if (!mapa->numtris || !mapa->tris) {
        printf("readBsp: erro loadFaces!\n\n");
        freeMapa3D(mapa);
        return NULL;
    }

    loadEntities(mapa, &header->lumps[LUMP_ENTITIES], buffer);

    loadTextures(mapa, &header->lumps[LUMP_TEXTURES], buffer, paleta);

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
