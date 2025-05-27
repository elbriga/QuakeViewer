#include <X11/Xlib.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

/*
COMPILAR COM
gcc teste.c -o teste -lX11 -lm
*/

typedef struct {
    int width, height;
    Display *display;
    Window window;
    GC gc;
    Pixmap back_buffer;
} DB;

void db_init(DB *db, Display *display, Window window, int width, int height)
{
    db->width  = width;
    db->height = height;

    db->display = display;
    db->window  = window;

    db->gc          = XCreateGC(display, window, 0, NULL);
    db->back_buffer = XCreatePixmap(display, window, db->width, db->height, 24);
}

void db_color(DB *db, int r, int g, int b)
{
    XSetForeground(db->display, db->gc, ((b&0xff) | ((g&0xff)<<8) | ((r&0xff)<<16)));
}

void db_point(DB *db, int x, int y)
{
    XDrawPoint(db->display, db->back_buffer, db->gc, x ,y);
}

void db_swap_buffers(DB *db)
{
    XCopyArea(db->display,
              db->back_buffer,
              db->window,
              db->gc,
              0, 0,
              db->width, db->height,
              0, 0);
}

int main()
{
	Display *gfx_display=0;
	Window  gfx_window;
	GC      gfx_gc;
	Colormap gfx_colormap;
	int      gfx_fast_color_mode = 0;
	int width = 800, height = 600;

	gfx_display = XOpenDisplay(0);
	if(!gfx_display) {
		fprintf(stderr,"gfx_open: unable to open the graphics window.\n");
		exit(1);
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

	XStoreName(gfx_display,gfx_window,"TESTE");

	XSelectInput(gfx_display, gfx_window, StructureNotifyMask|KeyPressMask|ButtonPressMask);

	XMapWindow(gfx_display,gfx_window);

	gfx_gc = XCreateGC(gfx_display, gfx_window, 0, 0);

	gfx_colormap = DefaultColormap(gfx_display,0);

	XSetForeground(gfx_display, gfx_gc, whiteColor);

	// Wait for the MapNotify event

	for(;;) {
		XEvent e;
		XNextEvent(gfx_display, &e);
		if (e.type == MapNotify)
			break;
	}

	XDrawPoint(gfx_display,gfx_window,gfx_gc, 10, 10);

	for(;;) {
		XEvent e;
		XNextEvent(gfx_display, &e);
		if (e.type == KeyPress)
			break;
	}





	DB db;
	db_init(&db, gfx_display, gfx_window, width, height);

	int r=250, g=2, b=2;
	db_color(&db, r, g, b);

	db_point(&db, 200, 200);
	db_swap_buffers(&db);






	for(;;) {
		XEvent e;
		XNextEvent(gfx_display, &e);
		if (e.type == KeyPress)
			break;
	}
}
