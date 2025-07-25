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

int janX = 800, janY = 600;

char paleta[256][3];

camera_t cam;
obj3d_t *chao, *obj, *obj2, *obj3;
mapa_t *mapa;

float tempo_de_jogo = 0.0f;

// int _debug = 5352;
int _debug = 0;
int _lightON = 1;

float oldTS = 0;
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

void testePoligono(mapa_t *mapa)
{
	msg("testePoligono");

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
			lightON ? fd->light : NULL,fd->light_width,fd->light_height);
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
}

int loadData()
{
	chao = obj = obj2 = obj3 = NULL;
	mapa = NULL;

	chao = obj_plano(10, 10);

	obj = readMdl("data/models/hknight.mdl");
	if (!obj) {
		msg("Falha ao carregar ARQUIVO.mdl");
		return 1;
	}

	obj2 = readMdl("data/models/mon_minotaur.mdl");
	if (!obj2) {
		msg("Falha ao carregar ARQUIVO2.mdl");
		return 2;
	}

	obj3 = readMdl("data/models/enforcer.mdl");
	if (!obj3) {
		msg("Falha ao carregar ARQUIVO3.mdl");
		return 3;
	}

	// Use current time as
	// seed for random generator
	srand(time(0));

	FILE *fpPal = fopen("data/paleta", "rb");
	if(!fpPal) {
		msg("Paleta nao encontrada!");
		return 99;
	}
	int pRead = fread(&paleta, 1, 768, fpPal);
	fclose(fpPal);

	mapa = readBsp("data/maps/e2m1.bsp");
	if (!mapa) {
		msg("Erro ao carregar mapa");
		return 34;
	}

	mapa_loadEntities(mapa);

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

	FILE *camPosIn = fopen("cam.dat", "rb");
	if (camPosIn) {
		fread(&cam, 1, sizeof(camera_t), camPosIn);
		fclose(camPosIn);
	}

	return 0;
}

void saida(int err)
{
	grafico_desliga();

	msg("Free Myke Tyson FREE");

	freeMapa3D(mapa);

	freeObj3D(obj3);
	freeObj3D(obj2);
	freeObj3D(obj);

	freeObj3D(chao);

	FILE *camPosOut = fopen("cam.dat", "wb");
	if (camPosOut) {
		fwrite(&cam, 1, sizeof(camera_t), camPosOut);
		fclose(camPosOut);
	}

	exit(err);
}

int loopPrincipal()
{
	int numAnimSel     = 0;
	int numAnimSelAuto = 2;

	int cntRender = 20;

	while (1)
	{
		if (cntRender++ > 10) {
			cntRender = 0;

			// tempo desde o último frame, em segundos ou milissegundos
			float delta_time = 0.5;//calcula_delta_tempo();
			tempo_de_jogo += delta_time;

			render_desenha_mapa(&cam, mapa);

			//render_desenha_objeto(&cam, chao);

			render_desenha_objeto(&cam, obj);
/*
		render_desenha_objeto(&cam, obj2);
		obj2->numFrameSel++;
		if(numFrameSel2 >= obj2->numframes -1)
			numFrameSel2 = 0;

		render_desenha_objeto(&cam, obj3);
		obj3->numFrameSel++;
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

			obj_inc_frame(obj);

			printf("cam{%d,%d,%d a:%d,%d,%d} ",
				(int)cam.pos.x,(int)cam.pos.y,(int)cam.pos.z,
				(int)cam.ang.x,(int)cam.ang.y,(int)cam.ang.z);

			printf("[dbg:%d] >>>\n", _debug);
		}

		char c = engine_get_key();
		// printf("c = %d\n", c);

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
			obj_dec_anim(obj);
		} else if (c == 'z') {
			obj_inc_anim(obj);
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

		usleep(500);
	}

	return 0;
}

int main(int argc, char **argv)
{
	int err;

	msg("Quake Viewer");

	err = loadData();
	if (err) {
		msg("Erro ao carregar dados");
		saida(err);
	}
	// printf("ENTs:\n%s\n", mapa->entities);

	err = grafico_init(janX, janY, "Quake Viewer");
	if (err) {
		msg("Erro ao inicializar graficos");
		saida(err);
	}

	msg("Init!");
	
	mostraTexturas(mapa);
	// grafico_desenha_objeto(&cam, obj, numFrameSel, paleta);
	// grafico_desenha_objeto(&cam, chao, 0, NULL);
	testePoligono(mapa);

	msg("Loop!");
	err = loopPrincipal();

	msg("Saida!");
	saida(err);
}
