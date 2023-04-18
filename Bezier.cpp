//
//  Bezier.cpp
//  OpenGL
// 

#include "Bezier.h"

int Bezier::id_helper = 0;

Connection::Connection(int idCurve1, int pointIndex1, int idCurve2, int pointIndex2, int connectionType) {
    this->idCurve1 = idCurve1;
    this->pointIndex1 = pointIndex1;
    this->idCurve2 = idCurve2;
    this->pointIndex2 = pointIndex2;
    this->connectionType = connectionType;
}

// **********************************************************************
Bezier::Bezier()
{
    for (int i=0;i<3;i++)
        Coords[i] = Ponto(0,0,0);
    ComprimentoTotalDaCurva = 0;
    cor = rand() % 100;
    id = id_helper++;
}
// **********************************************************************
//
// **********************************************************************
void Bezier::calculaComprimentoDaCurva()
{
    double DeltaT = 1.0/50;
    double t=DeltaT;
    Ponto P1, P2;
    
    ComprimentoTotalDaCurva = 0;
    
    P1 = Calcula(0.0);
    while(t<1.0)
    {
        P2 = Calcula(t);
        ComprimentoTotalDaCurva += calculaDistancia(P1,P2);
        P1 = P2;
        t += DeltaT;
    }
    P2 = Calcula(1.0); // faz o fechamento da curva
    ComprimentoTotalDaCurva += calculaDistancia(P1,P2);
    
}
// **********************************************************************
Bezier::Bezier(Ponto P0, Ponto P1, Ponto P2)
{
    Coords[0] = P0;
    Coords[1] = P1;
    Coords[2] = P2;
    calculaComprimentoDaCurva();
    cor = rand() % 100;
    id = id_helper++;
}
// **********************************************************************
Bezier::Bezier(Ponto V[])
{
    for (int i=0;i<3;i++)
        Coords[i] = V[i];
    calculaComprimentoDaCurva();
    cor = rand() % 100;
    id = id_helper++;
}
// **********************************************************************
//
// **********************************************************************
Ponto Bezier::Calcula(double t)
{
    Ponto P;
    double UmMenosT = 1-t;
    
    P =  Coords[0] * UmMenosT * UmMenosT + Coords[1] * 2 * UmMenosT * t + Coords[2] * t*t;
    return P;
}
// **********************************************************************
//
// **********************************************************************
double Bezier::CalculaT(double distanciaPercorrida)
{
    return (distanciaPercorrida/ComprimentoTotalDaCurva);
}
// **********************************************************************
//
// **********************************************************************
Ponto Bezier::getPC(int i)
{
    return Coords[i];
}

void Bezier::setPC(int i, Ponto P)
{
    Coords[i] = P;
}
// **********************************************************************
void Bezier::Traca()
{
    double t=0.0;
    double DeltaT = 1.0/50;
    Ponto P;
    //cout << "DeltaT: " << DeltaT << endl;
    glBegin(GL_LINE_STRIP);
    
    while(t<1.0)
    {
        P = Calcula(t);
        //P.imprime("P: ");
        glVertex2f(P.x, P.y);
        t += DeltaT;
    }
    P = Calcula(1.0); // faz o fechamento da curva
    glVertex2f(P.x, P.y);
    glEnd();
}
// **********************************************************************
//
// **********************************************************************
void Bezier::TracaVertices(int cor, int corSelecionada, int index)
{
    glPointSize(4);
    glBegin(GL_POINTS);
    for(size_t i=0; i < 3; i++)
    {
        index == i ? defineCor(corSelecionada) : defineCor(cor);
        glVertex2f(Coords[i].x,Coords[i].y);   
    }
    glEnd();
    glPointSize(1);
    
}

void Bezier::TracaPoligonoDeControle()
{
    glBegin(GL_LINE_LOOP);
    for(int i=0;i<3;i++)
        glVertex3f(Coords[i].x,Coords[i].y,Coords[i].z);
    glEnd();
    
}

bool Bezier::pontoEstaNaAresta(Ponto P) {
    for (int i = 0; i < 3; i++) {
        Ponto P1 = Coords[i];
        Ponto P2 = Coords[(i+1)%3];

        if (P1.x == P2.x && P.x == P1.x && P.y >= min(P1.y, P2.y) && P.y <= max(P1.y, P2.y)) {
            return true;
        }

        float slope = (P2.y - P1.y) / (P2.x - P1.x);
    
        // Calculate the y-intercept of the line
        float y_intercept = P1.y - slope * P1.x;
        
        // Calculate the y-coordinate of the point on the line for the given x-coordinate
        float y_line = slope * P.x + y_intercept;
        
        // Test if the point lies on the line (within a small tolerance) and if it lies within the x-range of the line
        if (fabs(P.y - y_line) < 0.1 && P.x >= min(P1.x, P2.x) && P.x <= max(P1.x, P2.x)) return true;
    }
    return false;
}