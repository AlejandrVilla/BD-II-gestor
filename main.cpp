#include <bits/stdc++.h>
#include "page.h"

using namespace std;

int main()
{
    vector<string> data = {"este", "es", "un", "conjunto", "de", "registros"};
    vector<int> slots;
    int k=1;
    Page new_page(k);
    for(int i=0 ; i<data.size() ; ++i)
    {
        int rid;
        rid = new_page.insert_record(data[i]);
        slots.push_back(rid);
    }
    for(auto it = new_page.begin() ; it!=new_page.end() ; ++it )
        cout<<"pagina "<<it->page_number<<" slot "<<it->slot_number<<" record_id "<<it->record_ID <<' '<<it->record <<'\n';
    cout<<'\n';

    cout<<new_page.get_record(slots[1])<<'\n';

    new_page.delete_record(slots[1]);
    cout<<new_page.get_record(slots[1])<<'\n';

    for(auto it = new_page.begin() ; it!=new_page.end() ; ++it )
        cout<<"pagina "<<it->page_number<<" slot "<<it->slot_number<<" record_id "<<it->record_ID <<' '<<it->record <<'\n';
    cout<<'\n';

    cout<<"slots libres "<<new_page.get_num_free_slot()<<'\n';

    int rid = new_page.insert_record("nuevo registro");
    cout<< new_page.get_record(rid)<<'\n';
    cout<< new_page.get_record(slots[1])<<'\n';
    cout<<"slots libres "<<new_page.get_num_free_slot()<<'\n';

    for(auto it = new_page.begin() ; it!=new_page.end() ; ++it )
        cout<<"pagina "<<it->page_number<<" slot "<<it->slot_number<<" record_id "<<it->record_ID <<' '<<it->record <<'\n';
    cout<<'\n';

    rid = new_page.insert_record("nuevo registro");
    new_page.updated_record(slots[2], "pene");
    cout<<new_page.get_record(slots[2])<<'\n';

    for(auto it = new_page.begin() ; it!=new_page.end() ; ++it )
        cout<<"pagina "<<it->page_number<<" slot "<<it->slot_number<<" record_id "<<it->record_ID <<' '<<it->record <<'\n';
    cout<<'\n';
}
