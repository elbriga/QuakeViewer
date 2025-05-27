#include <X11/Xlib.h>

typedef struct {
    int width, height;
    Display *display;
    Window window;
    GC gc;
    Pixmap back_buffer;
} DB;

void db_init(DB *db, Display *display, Window window, int width, int height);
void db_clear(DB *db);
void db_swap_buffers(DB *db);

void db_color(DB *db, int r, int g, int b);
void db_point(DB *db, int x, int y);
void db_fill_rect(DB *db, int x, int y, unsigned int w, unsigned int h);
