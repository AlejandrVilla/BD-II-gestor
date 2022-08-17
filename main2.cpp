#include <bits/stdc++.h>
#include "file.h"
#include "page.h"

using namespace std;
using namespace DB;

int main()
{
    // Probando file y page
    string file_name = "test.txt";
    File new_file(file_name);

    // fase insercion
    cout<<"FASE DE INSERCION\n";
    vector<string> data = {"insertando", "una", "nueva", "pagina", "en", "file"};
    vector<string> data2 = {"pero", "que", "ven", "mis", "oidos", "mano"};
    vector<string> data3 = {"base", "de", "datos", "es", "increible"};
    vector<int> slots1;
    vector<int> slots2;
    vector<int> slots3;
    vector<int> pages;

    // primera pagina
    int page_id = new_file.allocatePage();      // crea pagina en file
    pages.push_back(page_id);                   // guarda page_id
    Page new_page(page_id);
    for(int i=0 ; i<data.size() ; ++i)
    {
        int rid = new_page.insert_record(data[i]);
        slots1.push_back(rid);                  // guarda slots de la pagina 1
    }
    new_file.write_page(page_id, &new_page);    // guarda pagina en el file

    // segunda pagina
    page_id = new_file.allocatePage();          // crea pagina en file
    pages.push_back(page_id);                   // guarda page_id
    new_page.clear();
    new_page.set_page_number(page_id);
    for(int i=0 ; i<data2.size() ; ++i)
    {
        int rid = new_page.insert_record(data2[i]);
        slots2.push_back(rid);                  // guarda slots de la pagina 2
    }
    new_file.write_page(page_id, &new_page);    // guarda pagina en el file

    // tercera pagina
    page_id = new_file.allocatePage();          // crea pagina en file
    pages.push_back(page_id);                   // guarda page_id
    new_page.clear();
    new_page.set_page_number(page_id);
    for(int i=0 ; i<data3.size() ; ++i)
    {
        int rid = new_page.insert_record(data3[i]);
        slots3.push_back(rid);                  // guarda slots de la pagina 2
    }
    new_file.write_page(page_id, &new_page);    // guarda pagina en el file

    // lee pagina del file con iteradores
    for(auto it = new_file.begin() ; it!=new_file.end() ; ++it)
    {
        cout<<"PAGINA: "<<it->page_number()<<'\n';
        for(auto it2 = it->begin() ; it2 != it->end() ; ++it2)
            cout<< it2->record <<' ';
        cout<<'\n';
    }
    cout<<'\n';

    // lee paginas del file con page_id
    for(int k=0 ; k<pages.size() ; ++k)
    {
        Page* current_page = new_file.readPage(pages[k]);
        cout<<"PAGINA "<<current_page->page_number()<<' ';
        cout<<"REGISTROS ALMACENADOS:\n";
        for(auto it = current_page->begin() ; it != current_page->end() ; ++it)
            cout<< it->record <<' ';
        cout<<'\n';
    }
    
    cout<<'\n';
    cout<<"TOTAL DE PAGINAS EN EL FILE: "<<new_file.get_num_pages()<<'\n';
    for(auto it = new_file.begin() ; it!=new_file.end() ; ++it)
    {
        cout<<"REGISTROS GUARDADOS EN LA PAGINA "<<it->page_number()<<": "<<it->get_num_slots()<<'\n';
    }

    // fase de eliminacion
    // cout<<"\nFASE DE ELIMINACION\n";
    // new_file.deletePage(pages[1]);
    // cout<<'\n';
    // cout<<"paginas en el file: "<<new_file.get_num_pages()<<'\n';
    // for(auto it = new_file.begin() ; it!=new_file.end() ; ++it)
    // {
    //     cout<<"registros en la pagina "<<it->page_number()<<": "<<it->get_num_slots()<<'\n';
    // }
    // cout<<'\n';

    // // lee pagina del file con iteradores
    // for(auto it = new_file.begin() ; it!=new_file.end() ; ++it)
    // {
    //     cout<<"pagina: "<<it->page_number()<<'\n';
    //     for(auto it2 = it->begin() ; it2 != it->end() ; ++it2)
    //         cout<< it2->record <<' ';
    //     cout<<'\n';
    // }
    // cout<<'\n';

    // // lee paginas del file con page_id
    // for(int k=0 ; k<pages.size() ; ++k)
    // {
    //     Page* current_page = new_file.readPage(pages[k]);
    //     if( current_page )
    //     {
    //         cout<<"pagina "<<current_page->page_number()<<'\n';
    //         cout<<"registros guardados:\n";
    //         for(auto it = current_page->begin() ; it != current_page->end() ; ++it)
    //             cout<< it->record <<' ';
    //         cout<<'\n';
    //     }
    // }
    // cout<<'\n';

    // new_file.deletePage(pages[1]);
    // cout<<"paginas en el file: "<<new_file.get_num_pages()<<'\n';
    // for(auto it = new_file.begin() ; it!=new_file.end() ; ++it)
    // {
    //     cout<<"registros en la pagina "<<it->page_number()<<": "<<it->get_num_slots()<<'\n';
    // }
    // cout<<'\n';
    // new_file.deletePage(pages[0]);
    // cout<<"paginas en el file: "<<new_file.get_num_pages()<<'\n';
    // for(auto it = new_file.begin() ; it!=new_file.end() ; ++it)
    // {
    //     cout<<"registros en la pagina "<<it->page_number()<<": "<<it->get_num_slots()<<'\n';
    // }
    // cout<<'\n';
    
}