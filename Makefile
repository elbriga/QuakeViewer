CC=gcc
CFLAGS=-lSDL2 -lm -g
DEPS = 3d.h  gfx.h      grafico.h  mapa.h  readMdl.h  readBsp.h  render.h
OBJ =  3d.o  gfx-SDL.o  grafico.o  mapa.o  readMdl.o  readBsp.o  render.o  main.o

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

QuakeViewerSDL: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

.PHONY: clean

clean:
	rm -f *.o QuakeViewerSDL
