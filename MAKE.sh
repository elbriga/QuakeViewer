#!/bin/bash
gcc example.c gfx.c gfx_ptBR.c doublebuffer_pixmap.c readMdl.c 3d.c render.c -o example -lX11 -lm
