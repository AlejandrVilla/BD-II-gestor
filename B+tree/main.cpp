#include <iostream>
#include <cstdlib>
#include <stdlib.h>
#include <fstream>
#include "Btree.h"
using namespace std;

int main(){
    string cadena;
    int orden = 3;

    cout << "\nTests insertions en 'casos' \n";
    cout << "Tests eliminations en 'casosDeBorrado'\n" << endl;
    Btree A(orden);

    cout << "\n\nIngrese el nombre del archivo de la carpeta 'casos':  ";
    cin >> cadena;

    A.readKeys(cadena);

    cout << "\nBtree generado por archivo\n";
    A.draw();

    cout << "\n\n Para borrar valores del Btree, digite ...\n [1]Si\n [2]NO\n\nOpcion:  ";
    cin >> cadena;

    if(cadena == "1"){
        cout << "\n\nIngrese el nombre del archivo en la carpeta casosDeBorrados:  ";
        cin >> cadena;
        A.eraseKeys(cadena);
        cout << "\nBtree con valores borrados\n";
        A.draw();
    }


    system("pause");
    return 0;
}

