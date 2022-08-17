#include <bits/stdc++.h>
#include "buffer.h"

using namespace std;
using namespace DB;

int main()
{
    // fase de lectura de disco
    string name = "data.txt";
    File n_file(name);
    n_file.open('$');
    for(auto it = n_file.begin() ; it!=n_file.end() ; ++it)
    {
        cout<<"PAGINA: "<<it->page_number()<<'\n';
        for(auto it2 = it->begin() ; it2 != it->end() ; ++it2)
            cout<<"SLOT: "<<it2->slot_number << " RECORD_ID: "<<it2->record_ID<<" RECORD: "<<it2->record <<'\n';
        cout<<'\n';
    }
    cout<<'\n';

    // fase de llenado de buffer
    cout << "LLENANDO BUFFER\n";
    Buffer n_buffer(3,&n_file);
    Page *pag;
    for(auto it = n_file.begin(); it != n_file.end(); it++){
        pag = n_buffer.FetchPageImpl(it->page_number());
        n_buffer.UnpinPageImpl(it->page_number(),0);
        cout << "PAGINA DESDE BUFFER: "<< it->page_number() <<"\n";
        for(auto it2 = pag->begin() ; it2 != pag->end() ; ++it2){
            cout<<"SLOT: "<<it2->slot_number << " RECORD_ID: "<<it2->record_ID<<" RECORD: "<<it2->record <<'\n';
        }
        cout<<'\n';
    }

    // Fase de eliminacion de paginas del buffer
    cout<<"ELIMINANDO PAGINA DEL BUFFER\n";
    for(auto it = n_file.begin() ; it!=n_file.end() ; ++it)
    {
        if(n_buffer.DeletePageImpl(it->page_number()))
        {
            cout<<"ELIMINANDO PAGINA "<<it->page_number()<<"\n";
            break;
        }
        else
            cout<<"PAGINA NO SE ENCUENTRA EN EL BUFFER, PAGINA: "<<it->page_number()<<'\n';
    }

    cout<<"COMPROBANDO PAGINAS ACTUALES EN EL BUFFER\n";
    for(auto it = n_buffer.buffer_pool.begin() ; it!=n_buffer.buffer_pool.end() ; ++it )
    {
        if(it->page)
            for(auto it2 = it->page->begin() ; it2!=it->page->end() ; ++it2 )
            {
                cout<<"PAGINA: "<<it2->page_number<<" SLOT: "<<it2->slot_number<<" RECORD_ID "<<it2->record_ID<<" RECORD: "<<it2->record<<'\n';
            }
        else
            cout<<"FRAME LIBRE\n";
        cout<<'\n';
    }

    // fase de creacion de una pagina dentro del buffer
    cout<< "CREANDO PAGINA DENTRO DEL BUFFER\n";
    int new_page_id;
    Page* page_buff = NULL;
    page_buff = n_buffer.NewPageImpl(&new_page_id);
    if(page_buff)
    {
        cout<<"ID DE NUEVA PAGINA "<< page_buff->page_number() << "\n";
        cout << "MODIFICANDO PAGINA Y SOBRESCRIBIENDO EN DISCO\n";
        vector<string> new_data = {"making","a","new","page","from","buffer","to","File"};
        vector<int> record_ids;
        for(int i=0 ; i<new_data.size() ; ++i)
        {
            int rid = page_buff->insert_record(new_data[i]);
            if(rid!=-1)
                record_ids.push_back(rid); 
        }
        n_buffer.UnpinPageImpl(new_page_id,1);
        n_buffer.FlushPageImpl(new_page_id);

        cout << "REGISTROS GUARDADOS: "<< record_ids.size() <<'\n';
        Page* page_file = NULL;    
        page_file = n_file.readPage(new_page_id);
        for(int i=0 ; i<record_ids.size() ; ++i)
            cout << page_file->get_record(record_ids[i])<<'\n';
    }
    else
        cout<<"NO SE CREO PAGINA\n";
    cout<<"GUARDANDO EN OTRO FILE\n";
    string new_file_name = "data2.txt";
    n_file.set_filename(new_file_name);
}