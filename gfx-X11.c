#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <X11/Xlib.h>

#include "QuakeViewer.h"
#include "gfx.h"

int _gfxWidth, _gfxHeight;

static Display *gfx_display=0;
static Window  gfx_window;
static GC      gfx_gc;
static Colormap gfx_colormap;
static int      gfx_fast_color_mode = 0;

// double buffering
GC     back_buffer_gc;
Pixmap back_buffer;

/* Open a new graphics window. */
int gfx_open( int width, int height, const char *title )
{
	_gfxWidth  = width;
	_gfxHeight = height;

	gfx_display = XOpenDisplay(0);
	if(!gfx_display) {
		fprintf(stderr,"gfx_open: unable to open the graphics window.\n");
		return 1;
	}

	Visual *visual = DefaultVisual(gfx_display,0);
	if(visual && visual->class==TrueColor) {
		gfx_fast_color_mode = 1;
	} else {
		gfx_fast_color_mode = 0;
	}

	int blackColor = BlackPixel(gfx_display, DefaultScreen(gfx_display));
	int whiteColor = WhitePixel(gfx_display, DefaultScreen(gfx_display));

	gfx_window = XCreateSimpleWindow(gfx_display, DefaultRootWindow(gfx_display), 0, 0, width, height, 0, blackColor, blackColor);

	XSetWindowAttributes attr;
	attr.backing_store = Always;

	XChangeWindowAttributes(gfx_display,gfx_window,CWBackingStore,&attr);

	XStoreName(gfx_display,gfx_window,title);

	XSelectInput(gfx_display, gfx_window, StructureNotifyMask|KeyPressMask|ButtonPressMask);

	XMapWindow(gfx_display,gfx_window);

	gfx_gc = XCreateGC(gfx_display, gfx_window, 0, 0);

	gfx_colormap = DefaultColormap(gfx_display,0);

	XSetForeground(gfx_display, gfx_gc, whiteColor);

	// Handle window close
	Atom wm_delete = XInternAtom( gfx_display, "WM_DELETE_WINDOW", 1 );
	XSetWMProtocols( gfx_display, gfx_window, &wm_delete, 1 );

	// Init Double Buffering
	back_buffer_gc = XCreateGC(gfx_display, gfx_window, 0, NULL);
    back_buffer    = XCreatePixmap(gfx_display, gfx_window, width, height, 24);

	// Wait for the MapNotify event
	for(;;) {
		XEvent e;
		XNextEvent(gfx_display, &e);
		if (e.type == MapNotify)
			break;
	}

	return 0;
}

/* Draw a single point at (x,y) */

void gfx_point( int x, int y )
{
	XDrawPoint(gfx_display, back_buffer, back_buffer_gc, x ,y);
}

/* Draw a line from (x1,y1) to (x2,y2) */

void gfx_line( int x1, int y1, int x2, int y2 )
{
	XDrawLine(gfx_display, back_buffer, back_buffer_gc, x1, y1, x2, y2);
}

/* Change the current drawing color. */
void gfx_color( byte r, byte g, byte b )
{
	XSetForeground(gfx_display, back_buffer_gc, (b | (g << 8) | (r << 16)));
}

/* Clear the graphics window to the background color. */
void gfx_clear()
{
	XSetForeground(gfx_display, back_buffer_gc, 0);
    XFillRectangle(gfx_display, back_buffer, back_buffer_gc, 0, 0, _gfxWidth, _gfxHeight);
}

/* Flush all previous output to the window. */
void gfx_flush()
{
	XCopyArea(gfx_display,
              back_buffer,
              gfx_window,
              back_buffer_gc,
              0, 0,
              _gfxWidth, _gfxHeight,
              0, 0);
    
    gfx_clear();
}
/*
int gfx_event_waiting()
{
       XEvent event;

       gfx_flush();

       while (1) {
               if(XCheckMaskEvent(gfx_display,-1,&event)) {
                       if(event.type==KeyPress) {
                               XPutBackEvent(gfx_display,&event);
                               return 1;
                       } else if (event.type==ButtonPress) {
                               XPutBackEvent(gfx_display,&event);
                               return 1;
                       } else {
                               return 0;
                       }
               } else {
                       return 0;
               }
       }
}
*/
/* Wait for the user to press a key or mouse button. */

char engine_get_key()
{
	XEvent event;

	if (XCheckWindowEvent(gfx_display, gfx_window, 0xFFFFFFFF, &event)) {
		if(event.type==KeyPress) {
			return XLookupKeysym(&event.xkey,0);
		} else if (event.type==ClientMessage) {
			// Handle window close
			return 'q';
		} else if (event.type==NoExpose) {
			// Remover os eventos 14 da queue
			return engine_get_key();
		}
	}

	return 0;
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
