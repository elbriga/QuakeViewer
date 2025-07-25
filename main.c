#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <libgen.h> // basename

#include <sys/time.h> // gettimeofday localtime
#include <time.h>

#include "grafico.h"
#include "gfx.h"

#include "readBsp.h"
#include "readMdl.h"
#include "3d.h"

#include "render.h"
#include "mapa.h"

char paleta[256][3];
float oldTS = 0;

// int _debug = 5352;
int _debug = 0;
int _lightON = 1;

float tempo_de_jogo = 0.0f;

void msg(char *out) {
    struct timeval tv;
    struct tm *timeinfo;
    char buffDataHora[64];
	float newTS;
	float tempoDesdeUltimaMsg = 0.0;

    gettimeofday(&tv, NULL);
    timeinfo = localtime(&tv.tv_sec);
    
    strftime(buffDataHora, 64, "%H:%M:%S", timeinfo);

	newTS = (timeinfo->tm_hour * 3600) + (timeinfo->tm_min * 60) + timeinfo->tm_sec + 
			((float)tv.tv_usec / 1000000.0);

	if (oldTS > 0) {
		tempoDesdeUltimaMsg = newTS - oldTS;
	}
	oldTS = newTS;

	printf("[%s.%03ld] [%f / %f] > [%s]\n", buffDataHora, tv.tv_usec / 1000, tempoDesdeUltimaMsg, 1.0/tempoDesdeUltimaMsg, out);
}

void mostraTexturas(mapa_t *mapa)
{
	texture_t *tex = mapa->textures;
	char      *pixels;
	int        ofs = 10;

    for (int i=0; i < mapa->numtextures; i++, tex++) {
        printf("Texture[%d]: %s\n", i, tex->name);

        pixels = (char *)(tex->data);
        for (int y=0; y < tex->height; y++) {
            int yOfs = y * tex->width;
            for (int x=0; x < tex->width; x++) {
                byte cor = pixels[x + yOfs];

                gfx_point(x+ofs, y+ofs, paleta[cor][0], paleta[cor][1], paleta[cor][2]);
            }
        }

        ofs += 5;
    }

    grafico_mostra();
    engine_get_key_block();
}

int main(int argc, char **argv)
{
	int janX = 800, janY = 600, totAnims;
	camera_t cam;

	msg("Quake Viewer");

	obj3d_t *chao = obj_plano(10, 10);

	obj3d_t *obj = readMdl("data/models/hknight.mdl");
	if (!obj) {
		msg("Falha ao carregar ARQUIVO.mdl");
		exit(2);
	}

	obj3d_t *obj2 = readMdl("data/models/mon_minotaur.mdl");
	if (!obj2) {
		msg("Falha ao carregar ARQUIVO2.mdl");
		exit(2);
	}

	obj3d_t *obj3 = readMdl("data/models/enforcer.mdl");
	if (!obj3) {
		msg("Falha ao carregar ARQUIVO3.mdl");
		exit(2);
	}

	// Use current time as
	// seed for random generator
	srand(time(0));

	FILE *fpPal = fopen("data/paleta", "rb");
	if(!fpPal) {
		msg("Paleta nao encontrada!");
		exit(99);
	}
	int pRead = fread(&paleta, 1, 768, fpPal);
	fclose(fpPal);

	char tituloJanela[128];
	sprintf(tituloJanela, "Quake Viewer - %s", basename(argv[1]));

	int erro = grafico_init(janX, janY, tituloJanela);
	if (erro) {
		msg("Erro ao inicializar graficos");
		exit(33);
	}

	mapa_t *mapa = readBsp("data/maps/e2m1.bsp");
	if (!mapa) {
		msg("Erro ao carregar mapa");
		exit(34);
	}
	// printf("ENTs:\n%s\n", mapa->entities);
	mostraTexturas(mapa);

	int numAnimSel = 0;
	int numAnimSelAuto = 2;
	int numFrameSel = 0;
	int numFrameSel2 = 0;
	int numFrameSel3 = 0;
	char out[256];

	chao->posicao.y = 27;

	obj->posicao.x = 544;
	obj->posicao.y = -650;
	obj->posicao.z = 42;
	obj->rotacao.x = 270;

	obj2->posicao.x  = 40;
	obj2->posicao.y  = 0;
	obj2->posicao.z  = 0;
	obj2->rotacao.x = 0;

	obj3->posicao.x  = -40;
	obj3->posicao.y  = 0;
	obj3->posicao.z  = 0;
	obj3->rotacao.x = 0;

	cam.pos.x = 0;
	cam.pos.y = 0;
	cam.pos.z = 0;
	cam.ang.x = 0;
	cam.ang.y = 0;
	cam.ang.z = 0;

	mapa_loadEntities(mapa);

	FILE *camPosIn = fopen("cam.dat", "rb");
	if (camPosIn) {
		fread(&cam, 1, sizeof(camera_t), camPosIn);
		fclose(camPosIn);
	}

	// obj->tipo = OBJ_TIPO_WIRE;

	printf("Mapa BBox {\n%d,%d,%d\n%d,%d,%d\n}\n",
			(int)mapa->bbMin.x,
			(int)mapa->bbMin.y,
			(int)mapa->bbMin.z,
			(int)mapa->bbMax.x,
			(int)mapa->bbMax.y,
			(int)mapa->bbMax.z
		);

	printf("Init!\n");
	// grafico_tecla_espera();
	// printf("FOI!\n");

	// grafico_desenha_objeto(&cam, obj, numFrameSel, paleta);

	// grafico_desenha_objeto(&cam, chao, 0, NULL);

	ponto_t verticesPoligono[4] = {
				{
					.rot = {10.0f, 10.0f, 10.0f},
					.screen = {10.0f, 10.0f},
					.tex = {0.0f, 0.0f},
					.tex_luz = {0.0f, 0.0f}
				},
				{
					.rot = {550.0f, 25.0f, 10.0f},
					.screen = {550.0f, 25.0f},
					.tex = {1.0f, 0.0f},
					.tex_luz = {1.0f, 0.0f}
				},
				{
					.rot = {500.0f, 450.0f, 10.0f},
					.screen = {500.0f, 450.0f},
					.tex = {1.0f, 1.0f},
					.tex_luz = {1.0f, 1.0f}
				},
				{
					.rot = {5.0f, 300.0f, 10.0f},
					.screen = {5.0f, 300.0f},
					.tex = {0.0f, 1.0f},
					.tex_luz = {0.0f, 1.0f}
				}
			};
	
	ponto_t *pts[16] = {
		&verticesPoligono[0],
		&verticesPoligono[1],
		&verticesPoligono[2],
		&verticesPoligono[3]
	};

	face_t *fd = mapa->faces + _debug;

	int addX, addY, lightON = 1;
	while(1) {			
		grafico_desenha_poligono(pts, 4, &mapa->textures[3],
			lightON ? fd->light : NULL,fd->light_width,fd->light_height, paleta);
		grafico_mostra();

		char c = engine_get_key_block();
		if (c == 'q') break;

		addX = 0; addY = 0;
		if (c == 'a') addX = -10;
		if (c == 'd') addX =  10;
		if (c == 'w') addY = -10;
		if (c == 's') addY =  10;

		if (c == 'e') lightON = 1 - lightON;

		if (addX || addY) {
			for (int j=0; j<4; j++) {
				pts[j]->rot.x    += addX;
				pts[j]->screen.x += addX;
				pts[j]->rot.y    += addY;
				pts[j]->screen.y += addY;
			}
		}
	}

	int cntRender = 0;
	while (1)
	{
		if (cntRender++ > 10) {
			cntRender = 0;

			// tempo desde o último frame, em segundos ou milissegundos
			float delta_time = 0.5;//calcula_delta_tempo();
			tempo_de_jogo += delta_time;

			render_desenha_mapa(&cam, mapa, paleta);

			// grafico_desenha_objeto(&cam, chao, 0, NULL);

			// render_desenha_objeto(&cam, obj, numFrameSel, paleta);
/*
		grafico_desenha_objeto(&cam, obj2, numFrameSel2, paleta);
		numFrameSel2++;
		if(numFrameSel2 >= obj2->numframes -1)
			numFrameSel2 = 0;

		grafico_desenha_objeto(&cam, obj3, numFrameSel3, paleta);
		numFrameSel3++;
		if(numFrameSel3 >= 8)
			numFrameSel3 = 0;

		obj->rotacao.y++;
		if(obj->rotacao.y >= 360)
			obj->rotacao.y = 0;
		obj2->rotacao.y--;
		if(obj2->rotacao.y <= 0)
			obj2->rotacao.y = 360;
		// obj3->rotacao.z--;
		// if(obj3->rotacao.z <= 0)
		// 	obj3->rotacao.z = 360;
		chao->rotacao.y--;
		if(chao->rotacao.y <= 0)
			chao->rotacao.y = 360;
*/
			grafico_mostra();

		// char *framename = &obj->framenames[numFrameSel * 16];
		// sprintf(out, "cam{%d,%d,%d a:%d,%d,%d} Mostrando frame[%d]: %s > [%d]",
		// 	(int)cam.pos.x,(int)cam.pos.y,(int)cam.pos.z, (int)cam.ang.x,(int)cam.ang.y,(int)cam.ang.z,
		// 	numFrameSel, framename, (int)obj3->posicao.y);
		//  msg(out);

			numFrameSel++;

			int naSel = (numAnimSel == -1) ? numAnimSelAuto : numAnimSel;
			if (numFrameSel >= obj->framesanims[naSel].frameF) {
				if (numAnimSel == -1) {
					numAnimSelAuto = rand() % obj->totAnims;
					numFrameSel = obj->framesanims[numAnimSelAuto].frameI;
				} else {
					numFrameSel = obj->framesanims[naSel].frameI;
				}
			}

			printf("cam{%d,%d,%d a:%d,%d,%d} ",
				(int)cam.pos.x,(int)cam.pos.y,(int)cam.pos.z,
				(int)cam.ang.x,(int)cam.ang.y,(int)cam.ang.z);

			printf("[dbg:%d] >>>\n", _debug);
		}

		char c = engine_get_key();
		printf("c = %d\n", c);

		// Quit if it is the letter q.
		if (c == 'q')
			break;
		
		else if (c == 81) camera_pitch(&cam, -5); // ESQUERDA
		else if (c == 82) camera_step( &cam, -5, mapa); // CIMA
		else if (c == 83) camera_pitch(&cam,  5); // DIREITA
		else if (c == 84) camera_step( &cam,  5, mapa); // BAIXO

		else if (c == 'w') camera_step(&cam, -20, mapa);
		else if (c == 's') camera_step(&cam,  20, mapa);

		else if (c == 'a') camera_strafe(&cam,  10, mapa);
		else if (c == 'd') camera_strafe(&cam, -10, mapa);

		else if (c == 'o') _debug++;
		else if (c == 'l') _debug--;

		else if (c == 'e') _lightON = 1 - _lightON;

		else if (c == '\\') {
			numAnimSel--;
			if (numAnimSel < -1)
				numAnimSel = -1;
			
			numFrameSel = obj->framesanims[numAnimSel].frameI;
		} else if (c == 'z') {
			numAnimSel++;
			if (numAnimSel >= obj->totAnims)
				numAnimSel = obj->totAnims - 1;

			numFrameSel = obj->framesanims[numAnimSel].frameI;
		} else if (c == 'y') {
			cam.pos.z += 10;
		} else if (c == 'h') {
			cam.pos.z -= 10;
		} else if (c == 't') {
			obj->posicao.z+=20;
		} else if (c == 'g') {
			obj->posicao.z-=20;
		} else if (c == 'u') {
			cam.ang.x += 5;
		} else if (c == 'j') {
			cam.ang.x -= 5;
		} else if (c == '1') {
			cam.pos.x = mapa->player_start.x;
			cam.pos.y = mapa->player_start.y;
			cam.pos.z = mapa->player_start.z;

			cam.ang.y = mapa->player_start_angle;
		}

		if (numFrameSel < 0)
			numFrameSel = 0;
		if (numFrameSel >= obj->numframes)
			numFrameSel = obj->numframes - 1;
		
		usleep(500);
	}

	msg("Free Myke Tyson FREE");

	freeMapa3D(mapa);

	freeObj3D(obj3);
	freeObj3D(obj2);
	freeObj3D(obj);

	freeObj3D(chao);

	grafico_desliga();

	FILE *camPosOut = fopen("cam.dat", "wb");
	if (camPosOut) {
		fwrite(&cam, 1, sizeof(camera_t), camPosOut);
		fclose(camPosOut);
	}

	return 0;
}
