#include <X11/Xlib.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#include "gfx.h"
#include "gfx_ptBR.h"

void gfx_init( int width, int height, const char *title )
{
	gfx_open(width, height, title);

	gfx_color(200,200,200);
}

void gfx_limpa()
{
	gfx_clear();
}

/* Draw a single point at (x,y) */

void gfx_ponto( int x, int y, int r, int g, int b )
{
	// gfx_color(r, g, b);
	gfx_point(x, y);
}

char gfx_tecla()
{
	return gfx_wait();
}