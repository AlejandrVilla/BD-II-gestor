#include <iostream>
#include <cstdlib>
#include <stdlib.h>
#include <fstream>
#include "arbol.h"
using namespace std;

int main(){
    string cadena;
    int orden = 3;

    cout << "\nTests para las insersiones en 'casos' \n \
               Tests para las eliminaciones en 'casosDeBorrado'\n" << endl;
    Arbol A(orden);
    
    cout << "\n\nIngrese el nombre del archivo de la carpeta 'casos':  ";
    cin >> cadena;

    A.lecturaLlaves(cadena);

    cout << "\nArbol generado por archivo\n";
    A.pintar();

    cout << "\n\n Para borrar valores del arbol de un archivo , digite ...
             \n [1]Si
             \n [2]NO
             \n\nOpcion:  ";
    cin >> cadena;

    if(cadena == "1"){
        cout << "\n\nIngrese el nombre del archivo en la carpeta casosDeBorrados:  ";
        cin >> cadena;
        A.borradoLlaves(cadena);
        cout << "\nArbol con valores borrados\n";
        A.pintar();
    }


    system("pause");
    return 0;
}

