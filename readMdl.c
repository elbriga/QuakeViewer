#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "readMdl.h"
#include "3d.h"

int readLittleEndian(FILE *fp)
{
    int ret = 0;
    unsigned char in[4];

    fread(&in, 1, 4, fp);

    ret = (in[3] << 24) + (in[2] << 16) + (in[1] << 8) + in[0];

    return ret;
}

obj3d_t *readMdl(char *mdlfilename)
{
    obj3d_t *ret;

    FILE *fp;
    mdl_t header;

    fp = fopen(mdlfilename, "rb");

    if(!fp) {
        printf("Arquivo %s nao encontrado!\n\n", mdlfilename);
        return NULL;
    }

    fread(&header, 1, sizeof(mdl_t), fp);
    if(strncmp(header.ident, "IDPO", 4)) {
        fclose(fp);
        printf("Arquivo nao é MDL!\n\n");
        return NULL;
    }

    printf("HeaderSize: 0x%X\n\n", (int)sizeof(mdl_t));

    printf("Version...: %d\n", header.version);
    printf("Scale.....: %f %f %f\n", header.scale[0], header.scale[1], header.scale[2]);
    printf("ScaleOrg..: %f %f %f\n", header.scale_origin[0], header.scale_origin[1], header.scale_origin[2]);
    printf("EyePosit..: %f %f %f\n", header.eyeposition[0], header.eyeposition[1], header.eyeposition[2]);
    printf("NumSkins: %d - skinW: %d - skinH: %d\n", header.numskins, header.skinheight, header.skinwidth);
    printf("NumVerts: %d - numTris: %d - numFrames: %d\n", header.numverts, header.numtris, header.numframes);
    printf("SyncType: %d - flags: %d - Size: %f\n", header.synctype, header.flags, header.size);

    ret = calloc(1, sizeof(obj3d_t));
    if (!ret) {
        fclose(fp);
        printf("Erro malloc!\n\n");
        return NULL;
    }

    strncpy(ret->nome, mdlfilename, 64);
    ret->numframes = header.numframes;
    ret->numverts  = header.numverts;
    ret->numtris   = header.numtris;

    ret->skinwidth  = header.skinwidth;
    ret->skinheight = header.skinheight;

    // CARREGAR SKINS ==========================================

    ret->skin = malloc(header.skinwidth * header.skinheight);

    printf("Carregando %d Skins [%d x %d\n", header.numskins, header.skinwidth, header.skinheight);
    aliasskintype_t tipoSkin;
    unsigned char pixelSkin;

    for (int cnt_skin=0; cnt_skin<header.numskins; cnt_skin++) {
        fread(&tipoSkin, 1, 4, fp);

        printf("TipoSkin[%d]: %d\n", cnt_skin, tipoSkin);

        if (ALIAS_SKIN_SINGLE == tipoSkin) {
            /*for (int y=0; y<header.skinheight; y++) {
                for (int x=0; x<header.skinwidth; x++) {
                    fread(&pixelSkin, 1, 1, fp);
                    // PONTO gfx
                }
            }*/

            fread(ret->skin, 1, header.skinwidth * header.skinheight, fp);
        } else {
            printf("SKIN MULTIPLA!\n\n");
            fclose(fp);
            freeObj3D(ret);
            return NULL;
        }
    }

    // CARREGAR VERTS ==========================================
    printf("Carregando %d Verts\n", header.numverts);

    ret->skinmap = malloc(header.numverts * sizeof(skinvert_t));

    stvert_t vert;
    for (int cnt_vert=0; cnt_vert<header.numverts; cnt_vert++) {
        // TODO littleLong??
        fread(&vert, 1, sizeof(stvert_t), fp);

        ret->skinmap[cnt_vert].onseam = vert.onseam;
        ret->skinmap[cnt_vert].s      = vert.s;
        ret->skinmap[cnt_vert].t      = vert.t;

//        printf("Vert[%d]: on:%d T:%d S:%d\n", cnt_vert, vert.onseam, vert.t, vert.s);
    }

    // CARREGAR TRIS ==========================================
    printf("Carregando %d Tris\n", header.numtris);

    ret->tris = malloc(header.numtris * sizeof(triangulo_t));

    dtriangle_t tri;
    for (int cnt_tris=0; cnt_tris<header.numtris; cnt_tris++) {
        // TODO littleLong??
        fread(&tri, 1, sizeof(dtriangle_t), fp);

        ret->tris[cnt_tris].v[0] = tri.vertindex[0];
        ret->tris[cnt_tris].v[1] = tri.vertindex[1];
        ret->tris[cnt_tris].v[2] = tri.vertindex[2];

        ret->tris[cnt_tris].cor.r = 250;
        ret->tris[cnt_tris].cor.g = 250;
        ret->tris[cnt_tris].cor.b = 250;

//        printf("Tri[%d]: v1:%d v2:%d v3:%d\n", cnt_tris, tri.vertindex[0], tri.vertindex[1], tri.vertindex[2]);
    }

    // CARREGAR FRAMES ==========================================
    printf("Carregando %d Frames\n", header.numframes);

    ret->frames = malloc(header.numframes * sizeof(frame_t));
    if (!ret->frames) {
        printf("Erro malloc frames\n\n");
        fclose(fp);
        freeObj3D(ret);
        return NULL;
    }

    aliasframetype_t tipoFrame;
    trivertx_t vertFrame;
    for (int cnt_frames=0; cnt_frames<header.numframes; cnt_frames++) {
        fread(&tipoFrame, 1, 4, fp);

        //printf("TipoFrame[%d]: %d\n", cnt_frames, tipoFrame);

        if (ALIAS_SINGLE == tipoFrame) {
            daliasframe_t frame;
            fread(&frame, 1, sizeof(daliasframe_t), fp);

            //printf("NOME Frame[%d]: %s\n", cnt_frames, frame.name);

            strcpy(ret->frames[cnt_frames].nome, frame.name);

            ret->frames[cnt_frames].verts = malloc(header.numverts * sizeof(vetor3d_t));

            for (int cnt_vert=0; cnt_vert<header.numverts; cnt_vert++) {
                fread(&vertFrame, 1, sizeof(trivertx_t), fp);

                ret->frames[cnt_frames].verts[cnt_vert].x = vertFrame.v[0];
                ret->frames[cnt_frames].verts[cnt_vert].y = vertFrame.v[1];
                ret->frames[cnt_frames].verts[cnt_vert].z = vertFrame.v[2];

                //printf("Frame[%d]Vert[%d]: v1:%d v2:%d v3:%d\n", cnt_frames, cnt_vert, vertFrame.v[0], vertFrame.v[1], vertFrame.v[2]);

                /*if (cnt_frames == mostraFrame) {
                    gfx_ponto(vertFrame.v[0], vertFrame.v[1], 250, 250, 250);

                    gfx_ponto(vertFrame.v[1]+260, 255 - vertFrame.v[2], 250, 250, 250);

                    gfx_ponto(vertFrame.v[0], (255 - vertFrame.v[2])+260, 250, 250, 250);
                }*/
            }
        } else {
            printf("TipoFrame GROUP!\n\n");
            freeObj3D(ret);
            fclose(fp);
            return NULL;
        }
    }

    char basenome[16] = {0}, strFramesAnims[256] = {0}, strNumFrames[16] = {0};
	int frameInicial, frameFinal;
	int totAnims = 0;
    for (int nf=0; nf<ret->numframes; nf++) {
        char *nomeFrame = ret->frames[nf].nome;

        if (!strlen(basenome)) {
            // Achar a base do nome do frame, sem o numero
            strncpy(basenome, nomeFrame, 16);

            for (char n=0; n<16; n++) {
                if (nomeFrame[n] >= '0' && nomeFrame[n] <= '9') {
                    basenome[n] = 0;
                }
            }

            frameInicial = nf;
            totAnims++;
        } else {
            if (!strncmp(nomeFrame, basenome, strlen(basenome))) {
                // Ainda estamos no mesmo basenome
                frameFinal = nf;
            } else {
                sprintf(strNumFrames, "%d-%d ", frameInicial, frameFinal);
                strcat(strFramesAnims, strNumFrames);

                printf("Base %s :: %d-%d\n", basenome, frameInicial, frameFinal);

                basenome[0] = 0; // proximo
                nf--;
            }
        }
    }

    printf("Base %s :: %d-%d\n", basenome, frameInicial, frameFinal);
    sprintf(strNumFrames, "%d-%d", frameInicial, frameFinal);
    strcat(strFramesAnims, strNumFrames);

    ret->totAnims    = totAnims;
    ret->framesanims = malloc(totAnims * sizeof(animationframes_t));

    //printf("frames: %s\n", strFramesAnims);

    char *tok = strtok(strFramesAnims, " ");
    int numAnim = 0;
    while (tok) {
        sscanf(tok, "%d-%d", &ret->framesanims[numAnim].frameI, &ret->framesanims[numAnim].frameF);

        tok = strtok(NULL, " ");
        numAnim++;
    }

    if (numAnim != ret->totAnims) {
        printf("ERRO!!!!!!!!!!!!! numAnim: %d - totAnims: %d\n", numAnim, ret->totAnims);
    }

    fclose(fp);

    printf("Modelo carregado!\n\n\n");

    return ret;
}
