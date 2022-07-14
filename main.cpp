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

    const std::string &filename = "test.txt";
    // Limpiar cualquier ejecucion anterior que falle.

    File::remove(filename);

    {
        // Crear un nuevo archivo de base de datos.
        File new_file = File::create(filename);

        // Asignar algunas paginas y poner datos en ellas.
        Page nueva_pagina = new_file.allocatePage();
        const RecordId &rid = nueva_pagina.insertRecord("Nueva registro\n");
        new_file.writePage(nueva_pagina);

        FileIterator File_iter = new_file.begin();
        PageIterator Page_iter = (*File_iter).begin();
        std::cout<<*Page_iter<<'\n';

        (*File_iter).updateRecord(rid,"registro actualizado");
        std::cout<<*Page_iter<<'\n';

        (*File_iter).deleteRecord(rid);
        std::cout<<*Page_iter<<'\n';

        std::cout<<(*File_iter).page_number()<<'\n';
        std::cout<<(*File_iter).getFreeSpace()<<'\n';
    }
    File::remove(filename);

}