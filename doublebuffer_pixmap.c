#include <X11/Xlib.h>

#include "doublebuffer_pixmap.h"

void db_init(DB *db, Display *display, Window window, int width, int height)
{
    db->width  = width;
    db->height = height;

    db->display = display;
    db->window  = window;
    
    db->gc          = XCreateGC(display, window, 0, NULL);
    db->back_buffer = XCreatePixmap(display, window, db->width, db->height, 24);
}

void db_clear(DB *db)
{
    XSetForeground(db->display, db->gc, 0);
    XFillRectangle(db->display, db->back_buffer, db->gc, 0, 0, db->width, db->height);
}

void db_fill_rect(DB *db, int x, int y, unsigned int w, unsigned int h)
{
    XSetForeground(db->display, db->gc, 0xFF0000);
    XFillRectangle(db->display, db->back_buffer, db->gc, x, y, w, h);
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
