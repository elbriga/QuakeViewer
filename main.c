#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <math.h>
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
#include "entidade.h"
#include "monstros.h"

int janX = 1600, janY = 800;

char paleta[256][3];

camera_t cam;
mapa_t *mapa;

entidade_t *player;

float tempo_de_jogo = 0.0f;

// int _debug = 5352;
int _debug = 0;
int _lightON = 1;
int _showRendering = 0;
int _showBBox = 0;
int _showMap2D = 0;
float _lightFactor = 2.0;
extern int FOV;

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
	mapa = NULL;

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

	cam.pos.x = 0;
	cam.pos.y = 0;
	cam.pos.z = 0;
	cam.ang.x = 90;
	cam.ang.y = 0;
	cam.ang.z = 0;

	FILE *camPosIn = fopen("cam.dat", "rb");
	if (camPosIn) {
		fread(&cam, 1, sizeof(camera_t), camPosIn);
		fclose(camPosIn);
	}

	// entidades[0] = player
	vetor3d_t playerPos = mapa->player_start;
	entidade_create("player", playerPos, mapa->player_start_angle);

// Testes
vetor3d_t pos = { 544, -650, 42 };
entidade_create("hknight", pos, 90);

pos.y += 200;
entidade_create("enforcer", pos, 270);

	player = entidade_get(0);

	return 0;
}

void saida(int err)
{
	grafico_desliga();

	msg("Free Myke Tyson FREE");

	freeMapa3D(mapa);

	entidades_destroy();

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

	entidade_t *monstro = entidade_get(1);

	while (1)
	{
		if (cntRender++ > 10) {
			cntRender = 0;

			// tempo desde o último frame, em segundos ou milissegundos
			// TODO
			float delta_time = 0.1;//calcula_delta_tempo();
			tempo_de_jogo += delta_time;

			render_desenha_mapa(mapa, &cam);

			entidades_update(mapa, &cam, delta_time);

			entidades_render(mapa, &cam);

// vetor3d_t p0 = cam.pos;
// float angRads = to_radians(cam.ang.y);
// p0.x += sin(angRads) * 50;
// p0.y += cos(angRads) * 50;
// vetor3d_t p1 = {1786,1111,342};
// grafico_linha_3D(p0, p1, &cam, 255,10,10);

			grafico_mostra();

			printf("cam{%d,%d,%d a:%d,%d,%d} ",
				(int)cam.pos.x,(int)cam.pos.y,(int)cam.pos.z,
				(int)cam.ang.x,(int)cam.ang.y,(int)cam.ang.z);

			printf("[dbg:%d][FOV:%d] >>>\n", _debug, FOV);
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

		else if (c == '0') FOV += 5;
		else if (c == '9') FOV -= 5;

		else if (c == '-') _lightFactor -= 0.1;
		else if (c == '=') _lightFactor += 0.1;

		else if (c == 'e') _lightON       = 1 - _lightON;
		else if (c == '2') _showRendering = 1 - _showRendering;
		else if (c == '3') _showMap2D     = 1 - _showMap2D;
		else if (c == '4') _showBBox      = 1 - _showBBox;

		else if (c == '5') cam.ang.x += 10;
		else if (c == '6') cam.ang.x -= 10;

		else if (c == '7') monstro_novo(mapa);
		else if (c == '8') {
			for(int z=0; z<50; z++)
				monstro_novo(mapa);
		}

		else if (c == '\\') {
			entidade_dec_anim(0);
		} else if (c == 'z') {
			entidade_inc_anim(0);
		} else if (c == 'y') {
			cam.pos.z += 10;
		} else if (c == 'h') {
			cam.pos.z -= 10;
		} else if (c == 'u') {
			monstro->rotacao.z += 5;
		} else if (c == 'j') {
			monstro->rotacao.z -= 5;
		} else if (c == '1') {
			cam.pos.x = mapa->player_start.x;
			cam.pos.y = mapa->player_start.y;
			cam.pos.z = mapa->player_start.z;

			cam.ang.y = mapa->player_start_angle;
		} else if (c == 'n') {
			vetor3d_t pos = cam.pos;
			float angRads = to_radians(cam.ang.y);
			pos.x += sin(angRads) * 50;
			pos.y += cos(angRads) * 50;
			pos.z += 100;
			entidade_create("ogre", pos, 90);
		} else if (c == 'm') {
			vetor3d_t pos = cam.pos;
			float angRads = to_radians(cam.ang.y);
			pos.x += sin(angRads) * 50;
			pos.y += cos(angRads) * 50;
			pos.z += 100;
			entidade_create("shambler", pos, 90);
		} else if (c == 'p') {
			// Pulo!!
			entidades_pula();
		}

		if (FOV < 100) FOV = 100;
		if (FOV > 800) FOV = 800;

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
	
	// mostraTexturas(mapa);
	// grafico_desenha_objeto(&cam, obj, numFrameSel, paleta);
	// grafico_desenha_objeto(&cam, chao, 0, NULL);
	// testePoligono(mapa);

	msg("Loop!");
	err = loopPrincipal();

	msg("Saida!");
	saida(err);
}
