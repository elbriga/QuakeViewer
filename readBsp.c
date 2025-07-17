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

    mapa->bbMin.x = FLT_MAX;
    mapa->bbMin.y = FLT_MAX;
    mapa->bbMin.z = FLT_MAX;

    mapa->bbMax.x = -FLT_MAX;
    mapa->bbMax.y = -FLT_MAX;
    mapa->bbMax.z = -FLT_MAX;

    base = mapa->base;
	in = (dvertex_t *)(buffer + l->fileofs);
	for (i=0 ; i<mapa->numverts ; i++, in++, base++) {
        base->x = in->point[0];
        base->y = in->point[1];
        base->z = in->point[2];

        if (base->x < mapa->bbMin.x) mapa->bbMin.x = base->x;
        if (base->y < mapa->bbMin.y) mapa->bbMin.y = base->y;
        if (base->z < mapa->bbMin.z) mapa->bbMin.z = base->z;

        if (base->x > mapa->bbMax.x) mapa->bbMax.x = base->x;
        if (base->y > mapa->bbMax.y) mapa->bbMax.y = base->y;
        if (base->z > mapa->bbMax.z) mapa->bbMax.z = base->z;

        //rotacao2DEixoX(base, 90);
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

    // out = mapa->ledges;
    // for (int i=0; i < mapa->numledges; i++, out++) {
    //     printf(">>> %d\n", *out);
    // }

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

        //rotacao2DEixoX(&plano->normal, 90);

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

int loadLighting (mapa_t *mapa, lump_t *l, byte *buffer)
{
    char *light = (char *)(buffer + l->fileofs);
    
    mapa->lightinglen = l->filelen;
    mapa->lighting = (char *) malloc(mapa->lightinglen);
    if (!mapa->lighting) return 1;

    memcpy(mapa->lighting, light, mapa->lightinglen);

    return 0;
}

int loadFaces (mapa_t *mapa, lump_t *l, byte *buffer)
{
	dsface_t	*ins;
    face_t      *face;
    plano_t     *plano;

    if (!mapa->ledges) {
        return 2;
    }

    if (!mapa->lighting) {
        return 3;
    }

    if (!mapa->planes) {
        return 4;
    }

    if (!mapa->verts) {
        return 5;
    }

    mapa->numfaces = l->filelen / sizeof(dsface_t);
    mapa->faces    = (face_t *) malloc(mapa->numfaces * sizeof(face_t));
    if (!mapa->faces) return 1;

	ins  = (dsface_t *)(buffer + l->fileofs);
    face = (face_t *)mapa->faces;
    for (int i=0; i < mapa->numfaces; i++, ins++, face++) {
        // printf("face[%d] > numEdges: %d\n", i, ins->numedges);

        face->id       = i;
        face->planenum = ins->planenum;
        face->side     = ins->side;

        face->firstedge = ins->firstedge;
        face->numedges  = ins->numedges;
        face->texinfo   = ins->texinfo;

        face->light = (byte *)(mapa->lighting + ins->lightofs);

        plano = &mapa->planes[face->planenum];
        
        face->light_width  = -1;
        face->light_height = -1;
    }
    // grafico_tecla_espera();

    return 0;
}

int loadMarkSurfaces (mapa_t *mapa, lump_t *l, byte *buffer)
{
	short   *in;
    face_t **out;
    int      i, j;

    if (!mapa->faces) {
        return 2;
    }

    mapa->nummarksurfaces = l->filelen / sizeof(short);
    mapa->marksurfaces    = (face_t **) malloc(mapa->nummarksurfaces * sizeof(face_t *));
    if (!mapa->marksurfaces) return 1;

	in  = (short *)(buffer + l->fileofs);
    out = (face_t **)mapa->marksurfaces;
    for (i=0; i < mapa->nummarksurfaces; i++, in++) {
        j = (unsigned short)*in;
        if (j > mapa->numfaces) return 10;

        out[i] = mapa->faces + j;
    }

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

        // rotacao2DEixoX(&out->vetorS, 90);
        // rotacao2DEixoX(&out->vetorT, 90);

        out->miptex = in->miptex;
        out->flags  = in->flags;
    }

    return 0;
}

int loadVisibility (mapa_t *mapa, lump_t *l, byte *buffer)
{
    char *in = (char *)(buffer + l->fileofs);

    mapa->visibilitylen = l->filelen;
    mapa->visibility = (char *) malloc(mapa->visibilitylen);
    if (!mapa->visibility) return 1;

    memcpy(mapa->visibility, in, mapa->visibilitylen);

    return 0;
}

int loadNodes (mapa_t *mapa, lump_t *l, byte *buffer)
{
    int       i, j, p;
    dsnode_t *in = (dsnode_t *)(buffer + l->fileofs);
    node_t   *out;

    if (!mapa->planes) return 20;
    if (!mapa->leafs)  return 30;
    
    mapa->numnodes = l->filelen / sizeof(dsnode_t);

    mapa->nodes = (node_t *) malloc(mapa->numnodes * sizeof(node_t));
    if (!mapa->nodes) return 1;

    out = mapa->nodes;
    for (i=0; i < mapa->numnodes; i++, in++, out++) {
        out->contents = 0;
        out->visofs   = 0;

        out->min.x = in->mins[0];
        out->min.y = in->mins[1];
        out->min.z = in->mins[2];

        out->max.x = in->mins[3];
        out->max.y = in->mins[4];
        out->max.z = in->mins[5];

        out->plane = mapa->planes + in->planenum;

        out->firstsurface = in->firstface;
        out->numsurfaces  = in->numfaces;

        // printf(">>> [%d] Node[%d][%d][%d]:\n", mapa->numnodes, i, (unsigned short)in->children[0], (unsigned short)in->children[1]);

        for (j=0 ; j<2 ; j++)
		{
			//johnfitz -- hack to handle nodes > 32k, adapted from darkplaces
			p = (unsigned short)in->children[j];
			if (p < mapa->numnodes) {
				out->children[j] = mapa->nodes + p;
                // printf("child%dNODE:%d\n", j, p);
            } else {
				p = 65535 - p; //note this uses 65535 intentionally, -1 is leaf 0
				if (p >= 0 && p < mapa->numleafs) {
					out->children[j] = (node_t *)(mapa->leafs + p);
                    // printf("child%dLEAF:%d ", j, p);
                } else {
					printf("Mod_LoadNodes: invalid leaf index %i (file has only %i leafs)\n", p, mapa->numleafs);
					out->children[j] = (node_t *)(mapa->leafs); //map it to the solid leaf
				}
			}
			//johnfitz
		}
    }

    return 0;
}

int loadLeafs (mapa_t *mapa, lump_t *l, byte *buffer)
{
    // loadVisibility antes
    if (!mapa->visibility) return 10;
    if (!mapa->marksurfaces) return 20;

    dsleaf_t *in = (dsleaf_t *)(buffer + l->fileofs);
    leaf_t *out;

    mapa->numleafs = l->filelen / sizeof(dsleaf_t);
    mapa->leafs = (leaf_t *) malloc(mapa->numleafs * sizeof(leaf_t));
    if (!mapa->leafs) return 1;

    out = mapa->leafs;
    for (int i=0; i < mapa->numleafs; i++, in++, out++) {
        out->contents = in->contents;
        out->visofs   = in->visofs;

        out->min.x = in->mins[0];
        out->min.y = in->mins[1];
        out->min.z = in->mins[2];

        out->max.x = in->mins[3];
        out->max.y = in->mins[4];
        out->max.z = in->mins[5];

        if (out->visofs == -1)
			out->compressed_vis = NULL;
		else
			out->compressed_vis = mapa->visibility + out->visofs;
        
        out->firstmarksurface = (face_t **)(mapa->marksurfaces + (unsigned short)in->firstmarksurface);
        out->nummarksurfaces  = (unsigned short)in->nummarksurfaces;
    }

    return 0;
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
        if (!tex->data) {
            return 1000 + i;
        }

        pixels = (char *)(mt + 1);
        memcpy(tex->data, pixels, tex->width * tex->height);

        if (!strcmp(tex->name, "trigger")) {
            mapa->numTextureTrigger = i;
        }
    }

    return 0;
}

mapa_t *readBsp(char *fileName)
{
    mapa_t      *mapa;
    void        *buffer;
    dheader_t   *header;
    int          err;

    buffer = readFile(fileName);
    if (!buffer) {
        printf("readBsp: Erro read file %s!\n\n", fileName);
        return NULL;
    }

    header = (dheader_t *)buffer;
    printf("Header version [%d]\n\n", header->version);

    mapa = calloc(1, sizeof(mapa_t));
    if (!mapa) {
        printf("readBsp: Erro malloc!\n\n");
        return NULL;
    }

    strncpy(mapa->nome, fileName, 64);
    mapa->tipo = OBJ_TIPO_MAPA;
    mapa->numTextureTrigger = -1;

    err = loadVertexes(mapa, &header->lumps[LUMP_VERTEXES], buffer);
    if (err) {
        printf("readBsp: erro %d loadVertexes!\n\n", err);
        freeMapa3D(mapa);
        return NULL;
    }

    err = loadPlanes(mapa, &header->lumps[LUMP_PLANES], buffer);
    if (err) {
        printf("readBsp: erro %d loadPlanes!\n\n", err);
        freeMapa3D(mapa);
        return NULL;
    }

    err = loadEdges(mapa, &header->lumps[LUMP_EDGES], buffer);
    if (err) {
        printf("readBsp: erro %d loadEdges!\n\n", err);
        freeMapa3D(mapa);
        return NULL;
    }

    err = loadSurfEdges(mapa, &header->lumps[LUMP_SURFEDGES], buffer);
    if (err) {
        printf("readBsp: erro %d loadSurfEdges!\n\n", err);
        freeMapa3D(mapa);
        return NULL;
    }

    err = loadLighting(mapa, &header->lumps[LUMP_LIGHTING], buffer);
    if (err) {
        printf("readBsp: erro %d loadLighting!\n\n", err);
        freeMapa3D(mapa);
        return NULL;
    }

    err = loadFaces(mapa, &header->lumps[LUMP_FACES], buffer);
    if (err) {
        printf("readBsp: erro %d loadFaces!\n\n", err);
        freeMapa3D(mapa);
        return NULL;
    }

    err = loadMarkSurfaces(mapa, &header->lumps[LUMP_MARKSURFACES], buffer);
    if (err) {
        printf("readBsp: erro %d loadMarkSurfaces!\n\n", err);
        freeMapa3D(mapa);
        return NULL;
    }

    err = loadVisibility(mapa, &header->lumps[LUMP_VISIBILITY], buffer);
    if (err) {
        printf("readBsp: erro %d loadVisibility!\n\n", err);
        freeMapa3D(mapa);
        return NULL;
    }
    // printf("VIS: %s\n\n", mapa->visibility);

    err = loadLeafs(mapa, &header->lumps[LUMP_LEAFS], buffer);
    if (err) {
        printf("readBsp: erro %d loadLeafs!\n\n", err);
        freeMapa3D(mapa);
        return NULL;
    }

    err = loadNodes(mapa, &header->lumps[LUMP_NODES], buffer);
    if (err) {
        printf("readBsp: erro %d loadNodes!\n\n", err);
        freeMapa3D(mapa);
        return NULL;
    }

    err = loadTextures(mapa, &header->lumps[LUMP_TEXTURES], buffer);
    if (err) {
        printf("readBsp: erro %d loadTextures!\n\n", err);
        freeMapa3D(mapa);
        return NULL;
    }

    err = loadTexInfo(mapa, &header->lumps[LUMP_TEXINFO], buffer);
    if (err) {
        printf("readBsp: erro %d loadTexInfo!\n\n", err);
        freeMapa3D(mapa);
        return NULL;
    }

    err = loadEntities(mapa, &header->lumps[LUMP_ENTITIES], buffer);
    if (err) {
        printf("readBsp: erro %d loadEntities!\n\n", err);
        freeMapa3D(mapa);
        return NULL;
    }
    //printf("ents: %s\n\n", mapa->entities);

    free(buffer);

    return mapa;

    /*for (int l=0; l < HEADER_LUMPS; l++) {
        printf("Lump [%d] -> ofs:%d - size:%d\n", l, header->lumps[l].fileofs, header->lumps[l].filelen);

        switch (l) {
            case LUMP_LIGHTING:     break;
            case LUMP_CLIPNODES:    break;
            case LUMP_MODELS:       break;
        }
    }*/
}
