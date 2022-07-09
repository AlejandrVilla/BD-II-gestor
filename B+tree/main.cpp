#include <iostream>
#include <cstdlib>
#include <stdlib.h>
#include <fstream>
#include "arbol.h"
using namespace std;

int main(){
    string mensaje = "mensaje";
    int orden = 3;


    cout << "\nLos tests de cargar valores se encuentran en 'casos' \n \
        Los tests para eliminar valores se encuentran en 'casosDeBorrado'\n" << endl;

    Arbol A(orden);

    cout << "\n\nIngrese el nombre del archivo con valores para\nagregar al arbol, en la carpeta 'casos':  ";
    cin >> mensaje;

    A.lecturaLlaves(mensaje);

    cout << "\nArbol generado por archivo\n";
    A.pintar();

    cout << "\n\nDeseas borrar valores del arbol desde un archivo?\n [1]Si\n [Otra tecla]NO\n \nOpcion:  ";

    cin >> mensaje;
    if(mensaje == "1"){
        cout << "\n\nIngrese el nombre del archivo con valores para\nborrar del arbol, en la carpeta casosDeBorrados:  ";
        cin >> mensaje;

        A.borradoLlaves(mensaje);
        cout << "\nArbol con valores borrados\n";
        A.pintar();
    }


    system("pause");
    return 0;
}

