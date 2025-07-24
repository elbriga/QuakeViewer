#include "QuakeViewer.h"

int gfx_open( int width, int height, const char *title )
{
}

void gfx_point( int x, int y )
{
}

void gfx_line( int x1, int y1, int x2, int y2 )
{
}

void gfx_color( byte r, byte g, byte b )
{
}

void gfx_clear()
{
}

void gfx_flush()
{
}

char engine_get_key()
{
}

char engine_get_key_block()
{
	char key = 0;
	while(1) {
		key = engine_get_key();
		if (key) {
			break;
		}
		usleep(25000);
	}
	return key;
}
