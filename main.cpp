#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <cstring>
#include <memory>

#include "pages/page.h"
#include "pages/File.h"
#include "buffer.h"
// #include "pages/File_Iterator.h"
// #include "pages/Page_Iterator.h"

#define PRINT_ERROR(str)  std::cerr << "On Line No:" << __LINE__ << "\n"; std::cerr << str << "\n"; exit(1);                                        

using namespace siprec;    

const PageId num = 100;
PageId      pid[num], pageno1, pageno2, pageno3, i;
RecordId    rid[num], rid2, rid3;
Page        *page, *page2, *page3;
char        tmpbuf[100];
BufMgr      *bufMgr;
File        *file1ptr, *file2ptr, *file3ptr, *file4ptr, *file5ptr;

void test1();
void test2();
void test3();
void test4();
void test5();
void test6();
void test7();
void test_unPinPage();
void testBufMgr();

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

    // Esta funcion prueba el administrador de bufer, comente esta linea si no desea probar el administrador de bufer
    testBufMgr();
}

void testBufMgr()
{
    // crear administrador de bufer
    bufMgr = new BufMgr(num);

    // crear archivos ficticios
    const std::string &filename1 = "test.1";
    const std::string &filename2 = "test.2";
    const std::string &filename3 = "test.3";
    const std::string &filename4 = "test.4";
    const std::string &filename5 = "test.5";

    File::remove(filename1);
    File::remove(filename2);
    File::remove(filename3);
    File::remove(filename4);
    File::remove(filename5);

    File file1 = File::create(filename1);
    File file2 = File::create(filename2);
    File file3 = File::create(filename3);
    File file4 = File::create(filename4);
    File file5 = File::create(filename5);

    file1ptr = &file1;
    file2ptr = &file2;
    file3ptr = &file3;
    file4ptr = &file4;
    file5ptr = &file5;

    // Test buffer manager
    // Comenta las pruebas que no deseas ejecutar ahora. Las pruebas dependen de sus pruebas anteriores. Por lo tanto, deben ejecutarse en el siguiente orden.
    // Comentar una prueba en particular requiere comentar todas las pruebas que le siguen, de lo contrario esas pruebas fallarian.
    test1();
    test2();
    test3();
    test4();
    test5();
    test6();
    test7();
    test_unPinPage();

    // Cerrar archivos antes de borrarlos
    // printf("~archivo\n");
    file1.~File();
    // printf("~file1\n");
    file2.~File();
    file3.~File();
    file4.~File();
    file5.~File();
    // printf("starting remove\n");
    // Delete files
    File::remove(filename1);
    File::remove(filename2);
    File::remove(filename3);
    File::remove(filename4);
    File::remove(filename5);
    printf("comenzando eliminar bufmgr\n");
    delete bufMgr;

    std::cout << "\n"
              << "Paso todas las pruebas."
              << "\n";
}

void test1()
{
    // Asignando paginas en un archivo...
    for (i = 0; i < num; i++)
    {
        bufMgr->allocPage(file1ptr, pid[i], page);
        sprintf((char *)tmpbuf, "test.1 Page %d %7.1f", pid[i], (float)pid[i]);
        rid[i] = page->insertRecord(tmpbuf);
        bufMgr->unPinPage(file1ptr, pid[i], true);
        // printf("index %i \n", i);
    }

    // Leyendo pages back...
    for (i = 0; i < num; i++)
    {
        // printf("start reading page, index %i\n", i);
        bufMgr->readPage(file1ptr, pid[i], page);
        // printf("readPage return\n");
        sprintf((char *)&tmpbuf, "test.1 Page %d %7.1f", pid[i], (float)pid[i]);
        if (strncmp(page->getRecord(rid[i]).c_str(), tmpbuf, strlen(tmpbuf)) != 0)
        {
            PRINT_ERROR("ERROR :: CONTENTS DID NOT MATCH");
        }
        // printf("passed error msg\n");
        bufMgr->unPinPage(file1ptr, pid[i], false);
    }
    std::cout << "Test 1 passed"
              << "\n";
}

void test2()
{
   // Escribir y leer varios archivos
    // El numero de pagina y el valor deben coincidir

    for (i = 0; i < num / 3; i++)
    {
        // printf("en la prueba 2, esperando allocPage %i veces\n", i);
        bufMgr->allocPage(file2ptr, pageno2, page2);
        sprintf((char *)tmpbuf, "test.2 Page %d %7.1f", pageno2, (float)pageno2);
        rid2 = page2->insertRecord(tmpbuf);
        // printf("pasando atraves de allocPage %i veces\n", i);
        int index = rand() % num;
        pageno1 = pid[index];
        bufMgr->readPage(file1ptr, pageno1, page);
        sprintf((char *)tmpbuf, "test.1 Page %d %7.1f", pageno1, (float)pageno1);
        if (strncmp(page->getRecord(rid[index]).c_str(), tmpbuf, strlen(tmpbuf)) != 0)
        {
            PRINT_ERROR("ERROR :: LOS CONTENIDOS NO COINCIDIERON");
        }

        bufMgr->allocPage(file3ptr, pageno3, page3);
        sprintf((char *)tmpbuf, "test.3 Page %d %7.1f", pageno3, (float)pageno3);
        rid3 = page3->insertRecord(tmpbuf);

        bufMgr->readPage(file2ptr, pageno2, page2);
        sprintf((char *)&tmpbuf, "test.2 Page %d %7.1f", pageno2, (float)pageno2);
        if (strncmp(page2->getRecord(rid2).c_str(), tmpbuf, strlen(tmpbuf)) != 0)
        {
            PRINT_ERROR("ERROR :: LOS CONTENIDOS NO COINCIDIERON");
        }

        bufMgr->readPage(file3ptr, pageno3, page3);
        sprintf((char *)&tmpbuf, "test.3 Page %d %7.1f", pageno3, (float)pageno3);
        if (strncmp(page3->getRecord(rid3).c_str(), tmpbuf, strlen(tmpbuf)) != 0)
        {
            PRINT_ERROR("ERROR :: LOS CONTENIDOS NO COINCIDIERON");
        }

        bufMgr->unPinPage(file1ptr, pageno1, false);
    }

    for (i = 0; i < num / 3; i++)
    {
        bufMgr->unPinPage(file2ptr, i + 1, true);
        bufMgr->unPinPage(file2ptr, i + 1, true);
        bufMgr->unPinPage(file3ptr, i + 1, true);
        bufMgr->unPinPage(file3ptr, i + 1, true);
    }

    std::cout << "Test 2 passed"
              << "\n";
}

void test3()
{

    bufMgr->readPage(file4ptr, 1, page);
    PRINT_ERROR("ERROR :: File4 no deberia existir. Se deberia haber lanzado una excepcion antes de que la ejecucion llegue a este punto.");

    std::cout << "Test 3 passed"
              << "\n";
}

void test4()
{
    bufMgr->allocPage(file4ptr, i, page);
    bufMgr->unPinPage(file4ptr, i, true);

    // printf("unpin try pageId: %i\n", i);
    bufMgr->unPinPage(file4ptr, i, false);
    PRINT_ERROR("ERROR :: La pagina ya no esta fijada. Se deberia haber lanzado una excepcion antes de que la ejecucion llegue a este punto.");

    printf("catch the exception in test 4\n");

    std::cout << "Test 4 passed"
              << "\n";
}

void test5()
{
    for (i = 0; i < num; i++)
    {
        bufMgr->allocPage(file5ptr, pid[i], page);
        sprintf((char *)tmpbuf, "test.5 Page %d %7.1f", pid[i], (float)pid[i]);
        rid[i] = page->insertRecord(tmpbuf);
    }

    PageId tmp;

    bufMgr->allocPage(file5ptr, tmp, page);
    PRINT_ERROR("ERROR :: No quedan mas marcos para la asignacion. Se deberia haber lanzado una excepcion antes de que la ejecucion llegue a este punto.");

    std::cout << "Test 5 passed"
              << "\n";

    for (i = 1; i <= num; i++)
        bufMgr->unPinPage(file5ptr, i, true);
}

void test6()
{
    // vaciando el archivo con paginas aun ancladas. Deberia generar un error
    for (i = 1; i <= num; i++)
    {
        bufMgr->readPage(file1ptr, i, page);
    }
    bufMgr->flushFile(file1ptr);
    PRINT_ERROR("ERROR :: Paginas ancladas para el archivo que se esta vaciando. Se deberia haber lanzado una excepcion antes de que la ejecucion llegue a este punto.");

    std::cout << "Test 6 passed"
              << "\n";

    for (i = 1; i <= num; i++)
        bufMgr->unPinPage(file1ptr, i, true);

    bufMgr->flushFile(file1ptr);
}

void test7()
{
    bufMgr->allocPage(file1ptr, pageno1, page);
    // disposePage debe lanzar PagePinnedException si la pagina aun esta anclada.

    bufMgr->disposePage(file1ptr, pageno1);
    PRINT_ERROR("ERROR :: Paginas ancladas para que el archivo se elimine. Se deberia haber lanzado una excepcion antes de que la ejecucion llegue a este punto.");

    // disposePage no debe lanzar excepciones aunque ya no este en el bufer
    bufMgr->unPinPage(file1ptr, pageno1, false);
    for (i = 0; i < num; ++i)
    {
        bufMgr->allocPage(file2ptr, pid[i], page); // Desalojar file1's frame
    }

    bufMgr->disposePage(file1ptr, pageno1);

    PRINT_ERROR("ERROR :: La pagina no esta fijada. disposePage no debe generar ninguna excepcion.");

    // La segunda disposePage deberia lanzar InvalidPageException.

    bufMgr->disposePage(file1ptr, pageno1);
    PRINT_ERROR("ERROR :: disposePage debe lanzar InvalidPageException cuando la pagina ya se elimino.");

    printf("Excepcion de pagina no valida aqui. No te preocupes, atrapandolo aqui =P\n");

    for (i = 0; i < num; ++i)
    {
        bufMgr->unPinPage(file2ptr, pid[i], false);
    }

    std::cout << "Test 7 passed"
              << "\n";
}

void test_unPinPage()
{
    // Condiciones de prueba: el archivo 1 existe y tiene al menos una pagina.
    // Despues de las condiciones de la prueba: el archivo 1 se vaciara del bufer.
    PageId firstPageID = 1;
    const bool notDirty = 0;
    const bool isDirty = 1;
    Page *testPage;

    // Vaciar todo el archivo 1 para deshacerse de la pagina de prueba
    bufMgr->flushFile(file1ptr);

    // Probar que no pasa nada si la pagina no esta en el bufer

    bufMgr->unPinPage(file1ptr, firstPageID, notDirty);

    PRINT_ERROR("ERROR :: La pagina no esta en el bufer. unPinPage no debe generar ninguna excepcion.");

    // Probar unPinPage en resultados de pagina no anclados en la excepcion PageNotPinned
    // tambien prueba implicitamente unPinPage exitoso si readPage esta funcionando
    bufMgr->readPage(file1ptr, firstPageID, testPage);
    bufMgr->unPinPage(file1ptr, firstPageID, notDirty);

    bufMgr->unPinPage(file1ptr, firstPageID, notDirty);
    PRINT_ERROR("ERROR :: La pagina ya no esta fijada. unPinPage deberia PageNotPinnedException.");

    // Probar unPinPage establece el bit sucio en verdadero
    bufMgr->readPage(file1ptr, firstPageID, testPage);
    sprintf((char *)tmpbuf, "Testing unPinPage");
    const RecordId &rid = testPage->insertRecord(tmpbuf);
    bufMgr->unPinPage(file1ptr, firstPageID, isDirty);
    bufMgr->flushFile(file1ptr);

    bufMgr->readPage(file1ptr, firstPageID, testPage);
    if (strncmp(testPage->getRecord(rid).c_str(), tmpbuf, strlen(tmpbuf)) != 0)
    {
        PRINT_ERROR("ERROR :: UnPinPage no configuro el dirty bit correctamente");
    }
    testPage->deleteRecord(rid);
    bufMgr->unPinPage(file1ptr, firstPageID, notDirty);

    // Prueba unPinPage no establece el bit sucio en falso
    bufMgr->readPage(file1ptr, firstPageID, testPage);
    sprintf((char *)tmpbuf, "Testing unPinPage");
    const RecordId &rid2 = testPage->insertRecord(tmpbuf); // rid2 == rid ya que somos los unicos usuarios?
    bufMgr->unPinPage(file1ptr, firstPageID, isDirty);
    bufMgr->readPage(file1ptr, firstPageID, testPage);
    bufMgr->unPinPage(file1ptr, firstPageID, notDirty);
    bufMgr->flushFile(file1ptr);

    bufMgr->readPage(file1ptr, firstPageID, testPage);
    if (strncmp(testPage->getRecord(rid2).c_str(), tmpbuf, strlen(tmpbuf)) != 0)
    {
        PRINT_ERROR("ERROR :: UnPinPage no configuro el dirty bit correctamente");
    }
    testPage->deleteRecord(rid2);
    bufMgr->unPinPage(file1ptr, firstPageID, notDirty);

    // Limpiar el archivo para alcanzar el estado de prueba posterior garantizado.
    bufMgr->flushFile(file1ptr);

    std::cout << "Test for unPinPage passed\n";
}
