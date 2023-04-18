#ifndef Configuracao
#define Configuracao

#include <iostream>

using namespace std;

enum Configuracao {
    Sem_Continuidade,
    Continuidade_Posicao,
    Continuidade_Derivada
};

enum Screen {
    Screen_Menu,
    Screen_Status,
    Screen_Drawing
};

enum MenuItems {
    Menu_Sem_Continuidade = 1,
    Menu_Continuidade_Posicao,
    Menu_Continuidade_Derivada,
    Menu_Toggle_Curvas,
    Menu_Toggle_Poligonos_Controle,
};

#endif
