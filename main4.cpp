#include <bits/stdc++.h>
#include "buffer.h"

using namespace std;
using namespace DB;

int main()
{
    //Lee del txt
    string name = "data.txt";
    File n_file(name);
    n_file.open('$');
    cout<<"MOSTRANDO DESDE FILE\n";
    for(auto it = n_file.begin() ; it!=n_file.end() ; ++it)
    {
        cout<<"PAGINA: "<<it->page_number()<<'\n';
        for(auto it2 = it->begin() ; it2 != it->end() ; ++it2)
            cout<<"SLOT: "<<it2->slot_number << " RECORD_ID: "<<it2->record_ID<<" RECORD: "<<it2->record <<'\n';
        cout<<'\n';
    }

    //Testeando bufer
    cout<<'\n';
    cout << "USANDO BUFFER\n";
    Buffer n_buffer(3, &n_file);
    Page *pag;
    for(auto it = n_file.begin(); it != n_file.end(); it++)
    {
        pag = n_buffer.FetchPageImpl(it->page_number());
        n_buffer.UnpinPageImpl(it->page_number(),0);
        cout << "PAGINA DESDE BUFFER: "<< it->page_number() <<"\n";
        for(auto it2 = pag->begin() ; it2 != pag->end() ; ++it2)
            cout<<"SLOT: "<<it2->slot_number << " RECORD_ID: "<<it2->record_ID<<" RECORD: "<<it2->record <<'\n';
        cout<<'\n';
    }

    cout<<'\n';
    cout<<"COMPROBANDO PAGINAS ACTUALES EN EL BUFFER\n";
    for(auto it = n_buffer.buffer_pool.begin() ; it!=n_buffer.buffer_pool.end() ; ++it )
    {
        for(auto it2 = it->page->begin() ; it2!=it->page->end() ; ++it2 )
            cout<<"PAGINA: "<<it2->page_number<<" SLOT: "<<it2->slot_number<<" RECORD_ID "<<it2->record_ID<<" RECORD: "<<it2->record<<'\n';
        cout<<'\n';
    }   
}