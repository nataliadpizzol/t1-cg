//
//  CurvaBezier.hpp
//  OpenGL
//
//  Created by Márcio Sarroglia Pinho on 10/10/21.
//  Copyright © 2021 Márcio Sarroglia Pinho. All rights reserved.
//

#ifndef Bezier_h
#define Bezier_h

#include <iostream>
#include <vector>
#include <algorithm>
using namespace std;


#ifdef WIN32
#include <windows.h>
#include <glut.h>
#endif

#ifdef __APPLE__
#include <GLUT/glut.h>
#endif

#ifdef __linux__
#include <glut.h>
#endif

#include "Ponto.h"
#include "ListaDeCoresRGB.h"

class Connection
{
public:
    int idCurve1;
    int pointIndex1;
    int idCurve2;
    int pointIndex2;
    int connectionType;
    
    Connection(int idCurve1, int pointIndex1, int idCurve2, int pointIndex2, int connectionType);
};

class Bezier
{
    Ponto Coords[3];
    static int id_helper;

public:
    float ComprimentoTotalDaCurva;
    int cor;
    int id;
    std::vector<Connection> connections;
    
    Bezier();
    Bezier(Ponto P0, Ponto P1, Ponto P2);
    Bezier(Ponto V[]);
    Ponto Calcula(double t);
    Ponto getPC(int i);
    void setPC(int i, Ponto P);
    void Traca();
    void TracaVertices(int cor, int corSelecionada, int index);
    void TracaPoligonoDeControle();
    double CalculaT(double distanciaPercorrida);
    void calculaComprimentoDaCurva();
    bool pontoEstaNaAresta(Ponto P);
};

#endif 
