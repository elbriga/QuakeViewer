CC=gcc
CFLAGS=-lSDL2 -lm -g
DEPS = 3d.h  entidade.h  gfx.h      grafico.h  mapa.h  obj3d.h  readMdl.h  readBsp.h  render.h
OBJ =  3d.o  entidade.o  gfx-SDL.o  grafico.o  mapa.o  obj3d.o  readMdl.o  readBsp.o  render.o  main.o

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

QuakeViewerSDL: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

.PHONY: clean

clean:
	rm -f *.o QuakeViewerSDL
