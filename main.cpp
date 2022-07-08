#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <cstring>
#include <memory>

#include "pages/page.h"
#include "pages/File.h"

#define PRINT_ERROR(str)  std::cerr << "On Line No:" << __LINE__ << "\n"; std::cerr << str << "\n"; exit(1);                                        

using namespace DB;    

const PageId num = 100;
PageId      pid[num], pageno1, pageno2, pageno3, i;
RecordId    rid[num], rid2, rid3;
Page        *page, *page2, *page3;
char        tmpbuf[100];
// BufMgr      *bufMgr;
File        *file1ptr, *file2ptr, *file3ptr, *file4ptr, *file5ptr;


int main()
{
    // El siguiente codigo muestra como usar las clases de archivo y pagina

    const std::string &filename = "test.db";
    // Limpiar cualquier ejecucion anterior que falle.

    File::remove(filename);

    {
        // Crear un nuevo archivo de base de datos.
        File new_file = File::create(filename);

        // Asignar algunas paginas y poner datos en ellas.
        PageId third_page_number;
        for (int i = 0; i < 5; ++i)
        {
            Page new_page = new_file.allocatePage();
            if (i == 3)
            {
                // Mantenga un registro del identificador de la tercera pagina para que podamos leerlo
                // luego.
                third_page_number = new_page.page_number();
            }
            new_page.insertRecord("hello!");
            // Vuelve a escribir la pagina en el archivo (con los nuevos datos).
            new_file.writePage(new_page);
        }

        // Iterar a traves de todas las paginas del archivo.
        for (FileIterator iter = new_file.begin() ; iter != new_file.end() ; ++iter)
        {
           //Page it = *iter;
           // Iterar a traves de todos los registros en la pagina.
            for (PageIterator page_iter = (*iter).begin() ; page_iter != (*iter).end() ; ++page_iter)
            {
                std::cout << "Registro encontrado: " << *page_iter << " en la pagina " << (*iter).page_number() << "\n";
            }
        }

        // Recuperar la tercera pagina y agregarle otro registro.
        Page third_page = new_file.readPage(third_page_number);
        const RecordId &rid = third_page.insertRecord("world!");
        new_file.writePage(third_page);

        // Recuperar el registro que acabamos de agregar a la tercera pagina.
        std::cout << "La tercera pagina tiene un nuevo registro: "
                  << third_page.getRecord(rid) << "\n\n";
    }
    // new_file queda fuera del alcance aqui, por lo que el archivo se cierra automaticamente.

     // Borra el archivo ya que hemos terminado con el.
    File::remove(filename);

}