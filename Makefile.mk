# Makefile para Windows

PROG = Trabalho1.exe
FONTES = DesenhaBezier.cpp Bezier.cpp Poligono.cpp Ponto.cpp ListaDeCoresRGB.cpp
OBJETOS = $(FONTES:.cpp=.cpp)
CFLAGS = -Iinclude\GL -Llib -o # -Wall -g  # Todas as warnings, infos de debug
#LDFLAGS = -Llib\GL -lfreeglut -lopengl32 -lglu32 -lm
LDFLAGS =  -lFreeGLUT -lglu32 -lopengl32 -lwinmm -lgdi32
CBFLAGS = -fexceptions -Wno-write-strings -Wno-sign-compare -Wno-narrowing -Wno-deprecated-declarations -Wall -g -Wno-unused-but-set-variable -DFREEGLUT_STATIC
CC = g++.exe

$(PROG): $(OBJETOS)
	$(CC) $(CBFLAGS) $(CFLAGS) $(PROG) $(FONTES) $(LDFLAGS)

