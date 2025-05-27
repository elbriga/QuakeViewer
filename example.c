#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <unistd.h>

#include "gfx_ptBR.h"

#include "readMdl.h"
#include "3d.h"

#include "render.h"

char paleta[256][3];

int main(int argc, char **argv)
{
	int janX = 800, janY = 600, totAnims;

	obj3d_t *obj = readMdl(argv[1]);

	// Use current time as
	// seed for random generator
	srand(time(0));

	FILE *fpPal = fopen("paleta", "rb");
	if(!fpPal) {
		printf("Paleta nao encontrada!\n");
		return 99;
	}
	int pRead = fread(&paleta, 1, 768, fpPal);
	fclose(fpPal);

	printf("Paleta OK [%d]\n", pRead);

	grafico_init(janX, janY, "MDL Viewer");

	int numAnimSel = 0;
	int numFrameSel = 0;
	while (1)
	{
		frame_t *frame = &obj->frames[numFrameSel];
		printf("Mostrando frame[%d]: %s\n", numFrameSel, frame->nome);

		clock_t clock_start = clock();

		grafico_desenha_objeto(obj, numFrameSel, paleta);

		grafico_mostra();

		clock_t clock_end = clock();
		float tempo_um_frame = (float)(clock_end - clock_start) / CLOCKS_PER_SEC;

		printf("Tempo um frame: %f\n", tempo_um_frame);

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
		
		usleep(20000);
	}

	freeObj3D(obj);

	return 0;
}
