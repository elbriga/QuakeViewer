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
	int janX = 800, janY = 600, totAnims;

	msg("Quake MDL Viewer");

	if (argc < 2 || strlen(argv[1]) < 4) {
		msg("Uso: mdlViewer ARQUIVO.mdl");
		exit(1);
	}

	obj3d_t *obj = readMdl(argv[1]);
	if (!obj) {
		msg("Falha ao carregar ARQUIVO.mdl");
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
	grafico_init(janX, janY, tituloJanela);



	grafico_triangulo_textura(obj->skin, obj->skinwidth, obj->skinheight, paleta,
    	10,10, 300,30, 150,200,
    	20,20, 100,30, 50,100);
	grafico_mostra();

	char tecla;
	while(1) {
		tecla = grafico_tecla();
		if (tecla) {
			break;
		}
		usleep(1000);
	}




	int numAnimSel = 0;
	int numFrameSel = 0;
	char out[256];

	while (1)
	{
		frame_t *frame = &obj->frames[numFrameSel];

		grafico_desenha_objeto(obj, numFrameSel, paleta);

		grafico_mostra();

		sprintf(out, "Mostrando frame[%d]: %s", numFrameSel, frame->nome);
		msg(out);

		numFrameSel++;

		if (numFrameSel >= obj->framesanims[numAnimSel].frameF) {
			numFrameSel = obj->framesanims[numAnimSel].frameI;
		}

		// Wait for the user to press a character.
		char c = grafico_tecla();

		// Quit if it is the letter q.
		if (c == 'q')
			break;

		if (c == '\\') {
			numAnimSel--;
			if (numAnimSel < 0)
				numAnimSel = 0;
			
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
	freeObj3D(obj);

	return 0;
}
