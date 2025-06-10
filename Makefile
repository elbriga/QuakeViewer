CC=gcc
CFLAGS=-lX11 -lm -g
DEPS = 3d.h  doublebuffer_pixmap.h  gfx.h  gfx_ptBR.h  readMdl.h  render.h
OBJ =  3d.o  doublebuffer_pixmap.o  gfx.o  gfx_ptBR.o  readMdl.o  render.o  main.o

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

mdlViewer: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

.PHONY: clean

clean:
	rm -f *.o mdlViewer

