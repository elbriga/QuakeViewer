#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <libgen.h> // basename

#include <sys/time.h> // gettimeofday localtime
#include <time.h>

#include "gfx_ptBR.h"

#include "readMdl.h"
#include "3d.h"

#include "render.h"

char paleta[256][3];
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

int main(int argc, char **argv)
{
	int janX = 520, janY = 520, totAnims;

	msg("Quake MDL Viewer");

	if (argc < 4 || strlen(argv[1]) < 4) {
		msg("Uso: mdlViewer ARQUIVO.mdl");
		exit(1);
	}

	obj3d_t *obj = readMdl(argv[1]);
	if (!obj) {
		msg("Falha ao carregar ARQUIVO.mdl");
		exit(2);
	}

	obj3d_t *obj2 = readMdl(argv[2]);
	if (!obj2) {
		msg("Falha ao carregar ARQUIVO2.mdl");
		exit(2);
	}

	obj3d_t *obj3 = readMdl(argv[3]);
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
	sprintf(tituloJanela, "MDL VIEWER - %s", basename(argv[1]));

	int erro = grafico_init(janX, janY, tituloJanela);
	if (erro) {
		msg("Erro ao inicializar graficos");
		exit(33);
	}


	int numAnimSel = 0;
	int numAnimSelAuto = 2;
	int numFrameSel = 0;
	int numFrameSel2 = 0;
	int numFrameSel3 = 0;
	char out[256];

	while (1)
	{
		char *framename = &obj->framenames[numFrameSel * 16];

		grafico_desenha_objeto(obj, numFrameSel, paleta);

		grafico_desenha_objeto(obj2, numFrameSel2, paleta);
		numFrameSel2++;
		if(numFrameSel2 >= obj2->numframes -1)
			numFrameSel2 = 0;

		grafico_desenha_objeto(obj3, numFrameSel3, paleta);
		// numFrameSel3++;
		// if(numFrameSel3 >= obj3->numframes -1)
		// 	numFrameSel3 = 0;

		grafico_mostra();

		sprintf(out, "Mostrando frame[%d]: %s", numFrameSel, framename);
		msg(out);

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

		// Wait for the user to press a character.
		char c = grafico_tecla();

		// Quit if it is the letter q.
		if (c == 'q')
			break;

		if (c == '\\') {
			numAnimSel--;
			if (numAnimSel < -1)
				numAnimSel = -1;
			
			numFrameSel = obj->framesanims[numAnimSel].frameI;
		} else if (c == 'z') {
			numAnimSel++;
			if (numAnimSel >= obj->totAnims)
				numAnimSel = obj->totAnims - 1;

			numFrameSel = obj->framesanims[numAnimSel].frameI;
		}

		if (numFrameSel < 0)
			numFrameSel = 0;
		if (numFrameSel >= obj->numframes)
			numFrameSel = obj->numframes - 1;
		
		//usleep(20000);
	}

	msg("Free Myke Tyson FREE");

	freeObj3D(obj2);
	freeObj3D(obj);

	grafico_desliga();

	return 0;
}
