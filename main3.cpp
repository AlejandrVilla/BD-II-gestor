#include <bits/stdc++.h>
#include "file.h"
#include "page.h"

using namespace std;
using namespace DB;

int main()
{
    // Probando File y page
    // Fase de lectura
    string file_name = "test2.txt";
    File new_file(file_name);       // debe dar error, no existe file "test2.txt" para leer
    new_file.open('$');

    file_name = "test.txt";
    new_file.set_filename(file_name);   // test.txt si existe, lee todas las paginas guardadas
    new_file.open('$');

    cout<<'\n';
    cout<<"TOTAL DE PAGINAS EN EL FILE: "<<new_file.get_num_pages()<<'\n';
    for(auto it = new_file.begin() ; it!=new_file.end() ; ++it)
        cout<<"REGISTROS GUARDADOS EN LA PAGINA "<<it->page_number()<<": "<<it->get_num_slots()<<'\n';
    cout<<'\n';

    // lee pagina del file con iteradores   
    for(auto it = new_file.begin() ; it!=new_file.end() ; ++it)
    {
        cout<<"PAGINA: "<<it->page_number()<<'\n';
        for(auto it2 = it->begin() ; it2 != it->end() ; ++it2)
            cout<<"SLOT "<<it2->slot_number<<" RECORD_ID "<<it2->record_ID<<" RECORD: "<<it2->record <<'\n';
        cout<<'\n';
    }
    cout<<'\n';

    // Fase modificando file y guardando
    cout<<"MODIFICANDO FILE Y GUARDANDO\n";
    int page_id = new_file.allocatePage();
    vector<string> data = {"how", "many", "lessons", "have", "to learn"};
    vector<int> slots;
    vector<int> pages;
    Page new_page(page_id);
    for(int i=0 ; i<data.size() ; ++i)
    {
        int rid = new_page.insert_record(data[i]);
        slots.push_back(rid);
    }
    new_file.write_page(page_id, &new_page);

    cout<<'\n';
    cout<<"TOTAL DE PAGINAS EN EL FILE: "<<new_file.get_num_pages()<<'\n';
    for(auto it = new_file.begin() ; it!=new_file.end() ; ++it)
    {
        cout<<"REGISTROS GUARDADOS EN LA PAGINA "<<it->page_number()<<": "<<it->get_num_slots()<<'\n';
    }
    cout<<'\n';

    // lee pagina del file con iteradores   
    for(auto it = new_file.begin() ; it!=new_file.end() ; ++it)
    {
        cout<<"PAGINA: "<<it->page_number()<<'\n';
        for(auto it2 = it->begin() ; it2 != it->end() ; ++it2)
            cout<<"SLOT "<<it2->slot_number<<" RECORD_ID "<<it2->record_ID<<" RECORD: "<<it2->record <<'\n';
        cout<<'\n';
    }
    cout<<'\n';
    
    // guardando en nuevo file
    cout<<"GUARDANDO PAGINAS EN UN NUEVO FILE\n";
    // new_file.deletePage(0);
    // new_file.deletePage(0);
    string new_file_name = "data.txt";
    new_file.set_filename(new_file_name);

    cout<<'\n';
    cout<<"TOTAL DE PAGINAS EN EL FILE: "<<new_file.get_num_pages()<<'\n';
    for(auto it = new_file.begin() ; it!=new_file.end() ; ++it)
    {
        cout<<"REGISTROS GUARDADOS EN LA PAGINA "<<it->page_number()<<": "<<it->get_num_slots()<<'\n';
    }
    cout<<'\n';

    // lee pagina del file con iteradores   
    for(auto it = new_file.begin() ; it!=new_file.end() ; ++it)
    {
        cout<<"PAGINA: "<<it->page_number()<<'\n';
        for(auto it2 = it->begin() ; it2 != it->end() ; ++it2)
            cout<<"SLOT "<<it2->slot_number<<" RECORD_ID "<<it2->record_ID<<" RECORD: "<<it2->record <<'\n';
        cout<<'\n';
    }
    cout<<'\n';
}