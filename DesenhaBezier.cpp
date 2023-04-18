// Para uso no Xcode:
// Abra o menu Product -> Scheme -> Edit Scheme -> Use custom working directory
// Selecione a pasta onde voce descompactou o ZIP que continha este arquivo.

#include <iostream>
#include <cmath>
#include <ctime>
#include <fstream>
#include <algorithm>

using namespace std;

#ifdef WIN32
#include <windows.h>
#include <glut.h>
#else
#include <sys/time.h>
#endif

#ifdef __APPLE__
#include <GLUT/glut.h>
#endif

#ifdef __linux__
#include <glut.h>
#endif

#include "Ponto.h"
#include "Poligono.h"
#include "Bezier.h"

#include "ListaDeCoresRGB.h"
#include "Configuracao.h"

#define LIM 15
#define MENU_WIDTH 105
#define MENU_ITEM_HEIGHT 2.0f
#define STATUS_AREA_HEIGHT 75

const double padding_height = 0.2f;
const double padding_width = 1.0f;

std::vector<Bezier> Curvas;
std::vector<Connection> connections;
int curvaSelecionada = -1;
int pontoSelecionado = -1;
std::vector<std::pair<int, int>> pontosSelecionados; // (indice da curva, indice do ponto)
bool permiteMovimentacaoPonto = false;
int botaoPressionado = -1;

// Limites logicos da area de desenho
Ponto Min = {-LIM, -LIM, 0};
Ponto Max = {LIM, LIM, 0};

int width = 900, height = 550;

std::vector<Ponto> pontos;
int config = Sem_Continuidade;
bool desenhaCurvas = true;
bool desenhaPoligonosControle = true;
int pressedScreen = Screen_Drawing;
int rightButtonPressedScreen = Screen_Drawing;

GLint viewports[3][4] = {};

int menuItems[] = {
    Menu_Sem_Continuidade,
    Menu_Continuidade_Posicao,
    Menu_Continuidade_Derivada,
    Menu_Toggle_Curvas,
    Menu_Toggle_Poligonos_Controle
};

string MenuString[] = {
    " Sem continuidade",
    "   Cont. posicao ",
    "   Cont. derivada",
    "   Desen. curvas ",
    " Desen. poligonos"
};

// **********************************************************************
// Imprime o texto S na posicao (x,y), com a cor 'cor'
// **********************************************************************
void printString(string S, int x, int y, int cor)
{
    defineCor(cor);
    glRasterPos3f(x, y, 0); //define posicao na tela
    for (int i = 0; i < S.length(); i++)
    {
        glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, S[i]);
    }
}
#include <sstream>
using std::stringstream;

string toString(float f)
{
    stringstream S;
    S << f;
    return S.str();
}
// **********************************************************************
// Imprime as coordenadas do ponto P na posicao (x,y), com a cor 'cor'
// **********************************************************************
void ImprimePonto(Ponto P, int x, int y, int cor)
{
    string S;
    S = "( " + toString(P.x) + ", " + toString(P.y) + ")" ;
    printString(S, x, y, cor);
}

string StatusString[] = {
    "Modo sem continuidade",
    "Modo com continuidade de posicao",
    "Modo com continuidade de derivada"
};

void ImprimeStatus()
{
    std::vector<Connection> conn;
    std::copy_if(connections.begin(), connections.end(), std::back_inserter(conn), [](Connection c) {
        return (Curvas[curvaSelecionada].id == c.idCurve1 && c.pointIndex1 == pontoSelecionado) 
            || (Curvas[curvaSelecionada].id == c.idCurve2 && c.pointIndex2 == pontoSelecionado);
    });
    if (pontoSelecionado != -1 && conn.size() > 0) {
        printString("Ponto Selecionado: " + StatusString[conn.front().connectionType], -14, -1, Red);
    } else {
        printString(StatusString[config], -14, -1, Black);
    }
}

// **********************************************************************
//  void reshape( int w, int h )
//  trata o redimensionamento da janela OpenGL
// **********************************************************************
void reshape( int w, int h )
{
    width = w;
    height = h;
}

void DesenhaEixos()
{
    Ponto Meio;
    Meio.x = (Max.x+Min.x)/2;
    Meio.y = (Max.y+Min.y)/2;
    Meio.z = (Max.z+Min.z)/2;

    glBegin(GL_LINES);
    //  eixo horizontal
        glVertex2f(Min.x,Meio.y);
        glVertex2f(Max.x,Meio.y);
    //  eixo vertical
        glVertex2f(Meio.x,Min.y);
        glVertex2f(Meio.x,Max.y);
    glEnd();
}

void DesenhaCurvas()
{
    for(size_t i = 0; i < Curvas.size(); i++) {
        int pos = -1;
        defineCor(Yellow);
        if (desenhaCurvas) Curvas[i].Traca();
        curvaSelecionada == i && pontoSelecionado == -1 ? defineCor(LightBlue) : defineCor(Brown);
        if(desenhaPoligonosControle) Curvas[i].TracaPoligonoDeControle();
        defineCor(LimeGreen);
        for (auto selected : pontosSelecionados) {
            if (selected.first == i) {
                pos = selected.second;
            }
        }
        Curvas[i].TracaVertices(LimeGreen, White, pos);
    }
}

void DesenhaSegmento()
{
    defineCor(Red);
    if ((config == Sem_Continuidade || Curvas.size() == 0) && pontos.size() == 2) {
        glBegin(GL_LINES);
        glVertex2f(pontos[0].x,pontos[0].y);
        glVertex2f(pontos[1].x,pontos[1].y);
        glEnd();
    } else if (config == Continuidade_Posicao && Curvas.size() >= 1 && pontos.size() == 1) {
        Bezier& curva = pontoSelecionado != 0 && pontoSelecionado != 2 ? Curvas.back() : Curvas[curvaSelecionada];
        int pos = pontoSelecionado != 0 && pontoSelecionado != 2 ? 2 : pontoSelecionado;
        glBegin(GL_LINES);
        glVertex2f(curva.getPC(pos).x,curva.getPC(pos).y);
        glVertex2f(pontos[0].x,pontos[0].y);
        glEnd();
    }
}

void DesenhaPontos()
{
    defineCor(Red);
    glPointSize(4);
    glBegin(GL_POINTS);
    for(size_t i=0; i < pontos.size(); i++)
    {
        glVertex2f(pontos[i].x, pontos[i].y);   
    }
    glEnd();
    glPointSize(1);

}

void drawMenu() {
    for (auto menuItem : menuItems) {
        glBegin(GL_POLYGON);
        defineCor(Light_Purple);
        glVertex2f(Min.x + padding_width, Max.y - (menuItem * MENU_ITEM_HEIGHT) + padding_height);
        glVertex2f(Min.x + padding_width, Max.y - ((menuItem - 1) * MENU_ITEM_HEIGHT) - padding_height);
        glVertex2f(Max.x - padding_width, Max.y - ((menuItem - 1) * MENU_ITEM_HEIGHT) - padding_height);
        glVertex2f(Max.x - padding_width, Max.y - (menuItem * MENU_ITEM_HEIGHT) + padding_height);
        glEnd();

        glBegin(GL_LINE_LOOP);
        defineCor(Black);
        glVertex2f(Min.x + padding_width, Max.y - (menuItem * MENU_ITEM_HEIGHT) + padding_height);
        glVertex2f(Min.x + padding_width, Max.y - ((menuItem - 1) * MENU_ITEM_HEIGHT) - padding_height);
        glVertex2f(Max.x - padding_width, Max.y - ((menuItem - 1) * MENU_ITEM_HEIGHT) - padding_height);
        glVertex2f(Max.x - padding_width, Max.y - (menuItem * MENU_ITEM_HEIGHT) + padding_height);
        glEnd();

        defineCor(Black);
        glRasterPos3f(Min.x + padding_width + 0.8f, Max.y - (menuItem * MENU_ITEM_HEIGHT) + 1.0f, 0);
        for (int i = 0; i < MenuString[menuItem-1].length(); i++)
        {
            glutBitmapCharacter(GLUT_BITMAP_HELVETICA_10, MenuString[menuItem-1][i]);
        }
    }
}

void drawBackgroundSquare(int color) {
    glBegin(GL_POLYGON);
    defineCor(color);
    glVertex2f(Min.x, Min.y);
    glVertex2f(Min.x, Max.y);
    glVertex2f(Max.x, Max.y);
    glVertex2f(Max.x, Min.y);
    glEnd();
}


void displaySidebarMenu() {
    glViewport(0, 0, MENU_WIDTH, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(Min.x,Max.x, Min.y,Max.y, -10,+10);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glGetIntegerv(GL_VIEWPORT,viewports[Screen_Menu]);

	drawBackgroundSquare(White);
    drawMenu();
}

void displayStatusArea() {
    glViewport(MENU_WIDTH, 0, width-MENU_WIDTH, STATUS_AREA_HEIGHT);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(Min.x,Max.x, Min.y,Max.y, -10,+10);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glGetIntegerv(GL_VIEWPORT,viewports[Screen_Status]);

	drawBackgroundSquare(Light_Purple);
    ImprimeStatus();
}

void displayDrawingSpace() {
    glViewport(MENU_WIDTH, STATUS_AREA_HEIGHT, width-MENU_WIDTH, height-STATUS_AREA_HEIGHT);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(Min.x,Max.x, Min.y,Max.y, -10,+10);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glGetIntegerv(GL_VIEWPORT,viewports[Screen_Drawing]);

	glLineWidth(1);
	glColor3f(1,1,1);

    DesenhaEixos();

    glLineWidth(3);
    glColor3f(1,0,0);
    DesenhaPontos();
    DesenhaCurvas();
    DesenhaSegmento();
}

void display( void )
{
	// Limpa a tela
	glClear(GL_COLOR_BUFFER_BIT);

    displaySidebarMenu();
    displayStatusArea();
    displayDrawingSpace();

	glutSwapBuffers();
}

void deleteCurve() {
    if (curvaSelecionada != -1) {
        Curvas.erase(Curvas.begin() + curvaSelecionada);
        curvaSelecionada = -1;
        glutPostRedisplay();
    }
}

void keyboard(unsigned char key, int x, int y)
{
	switch ( key )
	{
		case 27: // ESC Termina o programa qdo
			exit ( 0 ); 
			break;
        case 127: // DEL
            deleteCurve();
            break;
		default:
			break;
	}
}

void arrow_keys ( int a_keys, int x, int y )
{
	switch ( a_keys )
	{
        case GLUT_KEY_LEFT:
  
            break;
        case GLUT_KEY_RIGHT:

            break;
		case GLUT_KEY_UP:       // Se pressionar UP
			glutFullScreen ( ); // Vai para Full Screen
			break;
	    case GLUT_KEY_DOWN:     // Se pressionar UP
								// Reposiciona a janela
            glutPositionWindow (50,50);
			glutReshapeWindow ( 700, 500 );
			break;
		default:
			break;
	}
}

// **********************************************************************
// Converte as coordenadas do ponto P de coordenadas de tela para
// coordenadas de universo (sistema de refer�ncia definido na glOrtho
// Codigo adaptado de http://hamala.se/forums/viewtopic.php?t=20
// **********************************************************************
Ponto ConvertePonto(Ponto P, int screen)
{
    GLdouble modelview[16],projection[16];
    GLfloat wx=P.x,wy,wz;
    GLdouble ox=0.0,oy=0.0,oz=0.0;
    viewports[screen][1] = 0;
    P.y=viewports[screen][3]-P.y;
    wy=P.y;
    glGetDoublev(GL_MODELVIEW_MATRIX,modelview);
    glGetDoublev(GL_PROJECTION_MATRIX,projection);
    glReadPixels(P.x,P.y,1,1,GL_DEPTH_COMPONENT,GL_FLOAT,&wz);
    gluUnProject(wx,wy,wz,modelview,projection,viewports[screen],&ox,&oy,&oz);
    return Ponto(ox,oy,oz);
}

int CheckScreenClick(int x, int y) {
    if (x < MENU_WIDTH) {
        return Screen_Menu;
    } else if (y > height - STATUS_AREA_HEIGHT) {
        return Screen_Status;
    } else {
        return Screen_Drawing;
    }
}

Bezier& getCurvaById(int id) {
    for (int i = 0; i < Curvas.size(); i++) {
        if (Curvas[i].id == id) {
            return Curvas.at(i);
        }
    }
}

bool jaAtualizouCurva(int id, std::vector<int>& pontosAtualizados) {
    return std::find(pontosAtualizados.begin(), pontosAtualizados.end(), id) != pontosAtualizados.end();
}

void atualizaRecursive(Bezier& curva, std::vector<int>& pontosAtualizados) {
    if (jaAtualizouCurva(curva.id, pontosAtualizados)) {
        return;
    }
    pontosAtualizados.push_back(curva.id);
    for (auto connection : connections) {
        if (connection.idCurve1 != curva.id && connection.idCurve2 != curva.id) {
            continue;
        }
        Bezier& c = getCurvaById(connection.idCurve1 == curva.id ? connection.idCurve2 : connection.idCurve1);
        c.setPC(connection.idCurve1 == curva.id ? connection.pointIndex2 : connection.pointIndex1, curva.getPC(connection.idCurve1 == curva.id ? connection.pointIndex1 : connection.pointIndex2));
        if (connection.connectionType == Continuidade_Derivada) {
            Ponto v = curva.getPC(connection.idCurve1 == curva.id ? connection.pointIndex1 : connection.pointIndex2) - curva.getPC(1);
            c.setPC(1, curva.getPC(connection.idCurve1 == curva.id ? connection.pointIndex1 : connection.pointIndex2) + v);
        }
        atualizaRecursive(getCurvaById(connection.idCurve1 == curva.id ? connection.idCurve2 : connection.idCurve1), pontosAtualizados);
    }
}

void atualizaContinuidadeCurvas() {
    std::vector<int> pontosAtualizados;
    for (auto p_index : pontosSelecionados) {
        for (auto& conn : connections) {
            if (conn.idCurve1 == p_index.first && conn.pointIndex1 == p_index.second) {
                conn.connectionType = config;
            }
        }
        Bezier curva = getCurvaById(p_index.first);
        atualizaRecursive(curva, pontosAtualizados);
    }
}

void HandleMenuClick(int x, int y) {
    Ponto p = ConvertePonto(Ponto(x,y), Screen_Menu);
    int menuItem = (int) LIM - ((p.y - Min.y) / MENU_ITEM_HEIGHT) + 1;
    switch (menuItem)
    {
    case Menu_Sem_Continuidade:
        config = Sem_Continuidade;
        break;
    case Menu_Continuidade_Posicao:
        config = Continuidade_Posicao;
        atualizaContinuidadeCurvas();
        break;
    case Menu_Continuidade_Derivada:
        config = Continuidade_Derivada;
        atualizaContinuidadeCurvas();
        break;
    case Menu_Toggle_Curvas:
        desenhaCurvas = !desenhaCurvas;
        break;
    case Menu_Toggle_Poligonos_Controle:
        desenhaPoligonosControle = !desenhaPoligonosControle;
        if (!desenhaPoligonosControle) {
            curvaSelecionada = -1;
        }
        break;
    default:
        break;
    }
}

void clearSelectedPoints() {
    pontoSelecionado = -1;
    curvaSelecionada = -1;
    pontosSelecionados.clear();
}

void clicaPonto(Ponto p)
{
    pontos.push_back(p);
    if ((config == Sem_Continuidade || !Curvas.size()) && pontos.size() == 3) {
        Curvas.push_back(Bezier(pontos[0], pontos[1], pontos[2]));
        pontos.clear();
        clearSelectedPoints();
    } else if (config == Continuidade_Posicao && Curvas.size() >= 1 && pontos.size() == 2) {
        Bezier& curva = pontoSelecionado != 0 && pontoSelecionado != 2 ? Curvas.back() : Curvas[curvaSelecionada];
        int pos = pontoSelecionado != 0 && pontoSelecionado != 2 ? 2 : pontoSelecionado;
        Bezier novaCurva = Bezier(curva.getPC(pos), pontos[0], pontos[1]);
        connections.push_back(Connection(curva.id, pos, novaCurva.id, 0, Continuidade_Posicao));
        Curvas.push_back(novaCurva);
        pontos.clear();
        clearSelectedPoints();
    }  else if (config == Continuidade_Derivada && Curvas.size() >= 1 && pontos.size() == 1) {
        Bezier& curva = pontoSelecionado != 0 && pontoSelecionado != 2 ? Curvas.back() : Curvas[curvaSelecionada];
        int pos = pontoSelecionado != 0 && pontoSelecionado != 2 ? 2 : pontoSelecionado;
        Ponto v = curva.getPC(pos) - curva.getPC(1);
        Bezier novaCurva = Bezier(curva.getPC(pos), curva.getPC(pos) + v, pontos[0]);
        connections.push_back(Connection(curva.id, pos, novaCurva.id, 0, Continuidade_Derivada));
        Curvas.push_back(novaCurva);
        pontos.clear();
        clearSelectedPoints();
    }
}

void HandleDrawingClick(int x, int y) {
    Ponto ponto = ConvertePonto(Ponto(x,y), Screen_Drawing);
    clicaPonto(ponto);
}

void selecionaAresta(int x, int y) {
    Ponto ponto = ConvertePonto(Ponto(x,y), Screen_Drawing);
    for (int i = 0; i < Curvas.size(); i++) {
        if (Curvas[i].pontoEstaNaAresta(ponto)) {
            curvaSelecionada = i;
            return;
        }
    }
    curvaSelecionada = -1;
}

void getAllPoints() {
    pontosSelecionados.clear();
    if (pontoSelecionado == -1 || curvaSelecionada == -1) {
        return;
    }
    Ponto p = Curvas[curvaSelecionada].getPC(pontoSelecionado);
    for (int i = 0; i < Curvas.size(); i++) {
        for (int j = 0; j < 3; j++) {
            if (Curvas[i].getPC(j) == p) {
                pontosSelecionados.push_back(std::make_pair(i, j));
            }
        }
    }
}

bool verificaClickPonto(int x, int y) {
    Ponto ponto = ConvertePonto(Ponto(x,y), Screen_Drawing);
    float menorDistancia = 1.0f;
    pontoSelecionado = -1;
    curvaSelecionada = -1;
    permiteMovimentacaoPonto = false;
    for (int i = 0; i < Curvas.size(); i++) {
        for (int j = 0; j < 3; j++) {
            auto distancia = calculaDistancia(ponto, Curvas[i].getPC(j));
            if (distancia < 0.1f && distancia < menorDistancia) {
                menorDistancia = distancia;
                pontoSelecionado = j;
                curvaSelecionada = i;
                permiteMovimentacaoPonto = true;
            }
        }
    }
    getAllPoints();
    return pontoSelecionado != -1;
}

void Mouse(int button,int state,int x,int y)
{
    int screen = CheckScreenClick(x, y);
    if (state==GLUT_DOWN && button==GLUT_RIGHT_BUTTON) {
        rightButtonPressedScreen = screen;
    }

    if(state==GLUT_DOWN && button==GLUT_LEFT_BUTTON)
    {
        pressedScreen = screen;
        botaoPressionado = GLUT_LEFT_BUTTON;
    }
    if(state==GLUT_UP && button==GLUT_LEFT_BUTTON)
    {
        botaoPressionado = -1;
        return;
    }
    if(state==GLUT_DOWN && button==GLUT_RIGHT_BUTTON && screen == Screen_Drawing) {
        botaoPressionado = GLUT_RIGHT_BUTTON;
        if (verificaClickPonto(x, y)) {
            return;
        } else if (desenhaPoligonosControle) {
            selecionaAresta(x, y);
        }
    };
    if(state==GLUT_UP && button==GLUT_RIGHT_BUTTON) {
        botaoPressionado = -1;
        permiteMovimentacaoPonto = false;
    };
    if(state==GLUT_UP || button==GLUT_RIGHT_BUTTON) {
        botaoPressionado = -1;
        return;
    };

    switch (screen) {
        case Screen_Menu:
            HandleMenuClick(x, y);
            break;
        case Screen_Status:
            break;
        case Screen_Drawing:
            HandleDrawingClick(x, y);
            break;
    }
    
}

// **********************************************************************
// Captura as coordenadas do mouse do mouse sobre a area de
// desenho, enquanto um dos botoes esta sendo pressionado
// **********************************************************************
void Motion(int x, int y)
{
    x = max(MENU_WIDTH, min(x, width));
    y = min(height - STATUS_AREA_HEIGHT, max(y, 0));
    Ponto P(x,y);
    Ponto p_motion = ConvertePonto(P, Screen_Drawing);
    if (pressedScreen == Screen_Drawing && botaoPressionado == GLUT_LEFT_BUTTON) {
        if (pontos.empty()) {
            Curvas.back().setPC(2, p_motion);
        } else if (pontos.size() == 1 && (config == Sem_Continuidade || Curvas.size() == 0)) {
            pontos.push_back(p_motion);
        } else {
            pontos.back() = p_motion;
        }
    } else if (rightButtonPressedScreen == Screen_Drawing) {
        if (permiteMovimentacaoPonto && curvaSelecionada >= 0) {
            std::vector<int> pontosAtualizados;
            Curvas[curvaSelecionada].setPC(pontoSelecionado, p_motion);
            atualizaRecursive(Curvas.at(curvaSelecionada), pontosAtualizados);
        }
    }
    glutPostRedisplay();
}


int  main ( int argc, char** argv )
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH | GLUT_RGB );
    glutInitWindowPosition (0, 0);

    // Define o tamanho inicial da janela grafica do programa
    glutInitWindowSize (width, height);

    // Cria a janela na tela, definindo o nome da
    // que aparecera na barra de t�tulo da janela.
    glutCreateWindow("Trabalho 1 - Computacao Grafica");

    // Define que o tratador de evento para
    // o redesenho da tela. A funcao "display"
    // ser� chamada automaticamente quando
    // for necess�rio redesenhar a janela
    glutDisplayFunc(display);

    // Define que o tratador de evento para
    // o redimensionamento da janela. A funcao "reshape"
    // ser� chamada automaticamente quando
    // o usu�rio alterar o tamanho da janela
    glutReshapeFunc(reshape);

    // Define que o tratador de evento para
    // as teclas. A funcao "keyboard"
    // ser� chamada automaticamente sempre
    // o usu�rio pressionar uma tecla comum
    glutKeyboardFunc(keyboard);

    // Define que o tratador de evento para
    // as teclas especiais(F1, F2,... ALT-A,
    // ALT-B, Teclas de Seta, ...).
    // A funcao "arrow_keys" ser� chamada
    // automaticamente sempre o usu�rio
    // pressionar uma tecla especial
    glutSpecialFunc(arrow_keys);
    glutMouseFunc(Mouse);
    glutMotionFunc(Motion);

    // inicia o tratamento dos eventos
    glutMainLoop();

    return 0;
}
