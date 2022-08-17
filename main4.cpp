#include <bits/stdc++.h>
#include "Buffer.h"

using namespace std;

int main()
{
    // lee del txt
    string name = "data.txt";
    File n_file(name);
    n_file.open('|');
    cout<<"MOSTRANDO DESDE FILE\n";
    for(auto it = n_file.begin() ; it!=n_file.end() ; ++it)
    {
        cout<<"Pagina: "<<it->page_number()<<'\n';
        for(auto it2 = it->begin() ; it2 != it->end() ; ++it2)
            cout<<"Slot: "<<it2->slot_number << " Record_id: "<<it2->record_ID<<" "<<it2->record <<'\n';
        cout<<'\n';
    }

    // testeando bufer
    cout<<'\n';
    cout << "USANDO BUFFER\n";
    Buffer n_buffer(3, &n_file);
    Page *pag;
    for(auto it = n_file.begin(); it != n_file.end(); it++)
    {
        pag = n_buffer.FetchPageImpl(it->page_number());
        cout << "Pagina desde Buffer "<< it->page_number() <<"\n";
        for(auto it2 = pag->begin() ; it2 != pag->end() ; ++it2)
            cout<<"Slot: "<<it2->slot_number << " Record_id: "<<it2->record_ID<<" "<<it2->record <<'\n';
        cout<<'\n';
    }

    cout<<'\n';
    cout<<"COMPROBANDO PAGINAS ACTUALES EN EL BUFFER\n";
    for(auto it = n_buffer.buffer_pool.begin() ; it!=n_buffer.buffer_pool.end() ; ++it )
    {
        for(auto it2 = it->page->begin() ; it2!=it->page->end() ; ++it2 )
            cout<<"Pagina: "<<it2->page_number<<" Slot: "<<it2->slot_number<<" record_ID "<<it2->record_ID<<" Record: "<<it2->record<<'\n';
        cout<<'\n';
    }
    
}