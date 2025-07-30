#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "3d.h"
#include "readMdl.h"

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

    printf("Version...: %d\n", header.version);
    printf("Scale.....: %f %f %f\n", header.scale[0], header.scale[1], header.scale[2]);
    printf("ScaleOrg..: %f %f %f\n", header.scale_origin[0], header.scale_origin[1], header.scale_origin[2]);
    printf("EyePosit..: %f %f %f\n", header.eyeposition[0], header.eyeposition[1], header.eyeposition[2]);
    printf("NumSkins: %d - skinW: %d - skinH: %d\n", header.numskins, header.skinheight, header.skinwidth);
    printf("NumVerts: %d - numTris: %d - numFrames: %d\n", header.numverts, header.numtris, header.numframes);
    printf("SyncType: %d - flags: %d - Size: %f\n", header.synctype, header.flags, header.size);

    int totMemObj = sizeof(obj3d_t) +                        // ret
                    (header.skinwidth * header.skinheight) + // ret->skin
                    (header.numverts * sizeof(skinvert_t)) + // ret->skinmap
                    (header.numtris * sizeof(triangulo_t)) + // ret->tris
                    (header.numframes * sizeof(frameinfo_t))+// ret->frameinfo
                    (header.numframes * header.numverts * sizeof(vetor3d_t)) + // ret->frames
                    (header.numverts * sizeof(ponto_t));     // ret->verts

    ret = calloc(1, totMemObj);
    if (!ret) {
        fclose(fp);
        printf("Erro malloc!\n\n");
        return NULL;
    }

    strncpy(ret->nome, mdlfilename, 64);
    ret->numframes = header.numframes;
    ret->numverts  = header.numverts;
    ret->numtris   = header.numtris;

    ret->posOlho.x = header.eyeposition[0];
    ret->posOlho.y = header.eyeposition[1];
    ret->posOlho.z = header.eyeposition[2];

    ret->texture.width  = header.skinwidth;
    ret->texture.height = header.skinheight;

    // CARREGAR SKINS ==========================================

    ret->texture.data = (char *)&ret[1];

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

            fread(ret->texture.data, 1, header.skinwidth * header.skinheight, fp);
        } else {
            printf("SKIN MULTIPLA!\n\n");
            fclose(fp);
            freeObj3D(ret);
            return NULL;
        }
    }

    // CARREGAR VERTS ==========================================
    printf("Carregando %d Verts\n", header.numverts);

    ret->skinmap = (skinvert_t *)&ret->texture.data[(ret->texture.width * ret->texture.height)];

    stvert_t vert;
    for (int cnt_vert=0; cnt_vert<header.numverts; cnt_vert++) {
        // TODO littleLong??
        fread(&vert, 1, sizeof(stvert_t), fp);

        ret->skinmap[cnt_vert].onseam = vert.onseam;
        ret->skinmap[cnt_vert].s      = (float)vert.s / ret->texture.width;
        ret->skinmap[cnt_vert].t      = (float)vert.t / ret->texture.height;

//        printf("Vert[%d]: on:%d T:%d S:%d\n", cnt_vert, vert.onseam, vert.t, vert.s);
    }

    // CARREGAR TRIS ==========================================
    printf("Carregando %d Tris\n", header.numtris);

    ret->tris = (triangulo_t *)&ret->skinmap[header.numverts];

    dtriangle_t tri;
    for (int cnt_tris=0; cnt_tris<header.numtris; cnt_tris++) {
        // TODO littleLong??
        fread(&tri, 1, sizeof(dtriangle_t), fp);

        ret->tris[cnt_tris].v[0] = tri.vertindex[0];
        ret->tris[cnt_tris].v[1] = tri.vertindex[1];
        ret->tris[cnt_tris].v[2] = tri.vertindex[2];

        ret->tris[cnt_tris].isFront = tri.facesfront;

//        printf("Tri[%d]: v1:%d v2:%d v3:%d\n", cnt_tris, tri.vertindex[0], tri.vertindex[1], tri.vertindex[2]);
    }

    // CARREGAR FRAMES ==========================================
    printf("Carregando %d Frames\n", header.numframes);

    ret->frameinfo = (frameinfo_t *)&ret->tris[header.numtris];
    ret->frames = (vetor3d_t *)&ret->frameinfo[header.numframes];
    ret->verts  = (ponto_t *)&ret->frames[header.numframes * header.numverts];

    aliasframetype_t tipoFrame;
    trivertx_t vertFrame;
    vetor3d_t ponto;
    frameinfo_t *frameinfo = ret->frameinfo;
    for (int cnt_frames=0; cnt_frames<header.numframes; cnt_frames++, frameinfo++) {
        fread(&tipoFrame, 1, 4, fp);

        //printf("TipoFrame[%d]: %d\n", cnt_frames, tipoFrame);

        if (ALIAS_SINGLE == tipoFrame) {
            daliasframe_t frame;
            fread(&frame, 1, sizeof(daliasframe_t), fp);

            //printf("NOME Frame[%d]: %s\n", cnt_frames, frame.name);

            strcpy(frameinfo->nome, frame.name);
            frameinfo->bboxmin.x = (float)frame.bboxmin.v[0] * header.scale[0] + header.scale_origin[0];
            frameinfo->bboxmin.y = (float)frame.bboxmin.v[1] * header.scale[1] + header.scale_origin[1];
            frameinfo->bboxmin.z = (float)frame.bboxmin.v[2] * header.scale[2] + header.scale_origin[2];
            frameinfo->bboxmax.x = (float)frame.bboxmax.v[0] * header.scale[0] + header.scale_origin[0];
            frameinfo->bboxmax.y = (float)frame.bboxmax.v[1] * header.scale[1] + header.scale_origin[1];
            frameinfo->bboxmax.z = (float)frame.bboxmax.v[2] * header.scale[2] + header.scale_origin[2];

            vetor3d_t *pnt = &ret->frames[cnt_frames * header.numverts];
            for (int cnt_vert=0; cnt_vert<header.numverts; cnt_vert++, pnt++) {
                fread(&vertFrame, 1, sizeof(trivertx_t), fp);    

                ponto.x = (float)vertFrame.v[0] * header.scale[0] + header.scale_origin[0];
                ponto.y = (float)vertFrame.v[1] * header.scale[1] + header.scale_origin[1];
                ponto.z = (float)vertFrame.v[2] * header.scale[2] + header.scale_origin[2];
                
                vetor_rotacao2DEixoX(&ponto, 90); // TODO - nao esta acabando com as normais aqui?

                *pnt = ponto;
            }

        } else {
            printf("TipoFrame GROUP!\n\n");
            freeObj3D(ret);
            fclose(fp);
            return NULL;
        }
    }

    char basenome[16] = {0}, strFramesAnims[256] = {0}, strNumFrames[16] = {0};
    char strNomesAnims[265] = {0};
	int frameInicial, frameFinal;
	int totAnims = 0;
    for (int nf=0; nf<ret->numframes; nf++) {
        char *nomeFrame = ret->frameinfo[nf].nome;

        if (!strlen(basenome)) {
            // Achar a base do nome do frame, sem o numero
            strncpy(basenome, nomeFrame, 16);

            for (char n=0; n<16; n++) {
                if (nomeFrame[n] >= '0' && nomeFrame[n] <= '9') {
                    basenome[n] = 0;
                    break;
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

                strcat(strNomesAnims, basenome);
                strcat(strNomesAnims, " ");

                basenome[0] = 0; // proximo
                nf--;
            }
        }
    }

    sprintf(strNumFrames, "%d-%d", frameInicial, frameFinal);
    strcat(strFramesAnims, strNumFrames);
    strcat(strNomesAnims, basenome);

    ret->totAnims    = totAnims;
    ret->framesanims = malloc(totAnims * sizeof(animationframes_t));

    //printf("frames: %s\n", strFramesAnims);

    char *tok = strtok(strFramesAnims, " ");
    animationframes_t *fa = ret->framesanims;
    int numAnim = 0;
    while (tok) {
        sscanf(tok, "%d-%d", &fa->frameI, &fa->frameF);

        tok = strtok(NULL, " ");
        numAnim++;
        fa++;
    }

    if (numAnim != ret->totAnims) {
        printf("ERRO!!!!!!!!!!!!! numAnim: %d - totAnims: %d\n", numAnim, ret->totAnims);
    }

    char *tokNome = strtok(strNomesAnims, " ");
    fa = ret->framesanims;
    while (tokNome) {
        strcpy(fa->nome, tokNome);
        printf("Base %s :: %d-%d\n", fa->nome, fa->frameI, fa->frameF);

        tokNome = strtok(NULL, " ");
        fa++;
    }

    fclose(fp);

    // Anims
    for (int i=0; i<ret->totAnims; i++) {
        if (!strcmp(ret->framesanims[i].nome, "stand")) {
            ret->numAnimIdle = i;
        } else if (!strcmp(ret->framesanims[i].nome, "axrun")) {
            ret->numAnimWalk = i;
        } else if (!strcmp(ret->framesanims[i].nome, "shotatt")) {
            ret->numAnimAttack = i;
        }
    }

    obj_calculate_face_normals(ret);
    obj_calculate_offsetChao(ret);

    printf("Modelo carregado!\n\n\n");

    return ret;
}
