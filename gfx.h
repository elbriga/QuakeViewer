#ifndef GFX_H
#define GFX_H

#include "QuakeViewer.h"

int  gfx_open( int width, int height, const char *title );
void gfx_close();

void gfx_point( int x, int y, byte r, byte g, byte b );
void gfx_line( int x1, int y1, int x2, int y2, byte r, byte g, byte b );

void gfx_clear();
void gfx_flush();

char engine_get_key();
char engine_get_key_block();

#endif