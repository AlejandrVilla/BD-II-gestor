#pragma once 
#include "page.h"
#include "replacer.h"
#include "Clock.h"
#include "LRU.h"
#include "MRU.h"
#include "file.h"
#include <unordered_map>
#include <iostream>

// Frame del Buffer
struct Frame
{
    Page    *page;
    int dirty_bit;
    int pin_count;      // cuantos tienen acceso a la pagina
};

// Buffer pool
class Buffer
{
    public:
        Buffer(size_t pool_size, File *file);
        bool  FlushPageImpl (int page_id);
        Page* NewPageImpl   (int *page_id);
        Page* FetchPageImpl (int page_id);
        bool  DeletePageImpl(int page_id);
        bool  UnpinPageImpl (int page_id, bool is_dirty);
        void  FlushAllPagesImpl();
        ~Buffer();
        std::vector<Frame> buffer_pool;
    private:
        size_t pool_size_;                          // Tamanio del buffer    
        Replacer *replacer_;                        // Clase abstracta (polimorfismo)
        std::list<int> free_list_;                  // Lista de indices de frames libres
        std::unordered_map<int, int> page_table_;   // Mapa de <idPage , idFrame>
        File *file_;
};

//Constructor con tamanio de buffer_pool y puntero al File
Buffer::Buffer(size_t pool_size, File *file) : pool_size_(pool_size), file_(file)
{
    buffer_pool = std::vector<Frame>(pool_size);
    //Inicializamos el replacer con el algoritmo de remplazamiento elegido 
    replacer_ = new Clock(pool_size);
    for (int32_t i = 0; i < pool_size; i++)
    {
        //Inicializamos las listas en 0
        free_list_.emplace_back(static_cast<int>(i));
        buffer_pool[i].dirty_bit = 0;
        buffer_pool[i].pin_count = 0;
    }
}

//Obtener pagina solicitada por id
Page* Buffer::FetchPageImpl(int page_id) 
{
    // 1. Buscar en la page_table la página solicitada (P).
    // 1.1 Si P existe, la fija y la devuelve inmediatamente.
    // 1.2 Si P no existe, encuentra una página de reemplazo (R) de la lista libre o del replacer.
    // 2. Si R está dirty, escribirla de nuevo en el disco.
    // 3. Eliminar R de la page_table e insertar P.
    // 4. Actualizar los metadatos de P, leer el contenido de la página desde el disco, y luego devolver un puntero a P.

    //Si la pagina se encuentra ya en el buffer.
    if(page_table_.find(page_id)!= page_table_.end())
    {
        //Actualiza metadata de la pagina en el Buffer
        int target = page_table_[page_id];      // devuelve frame_id
        replacer_->Pin(target);
        buffer_pool[target].pin_count++;
        //Retorna el puntero la pagina encontrada
        return buffer_pool[target].page;
    }
    //En caso la pagina solicitada, NO se encuentre en el buffer
    else
    {
        //Seteamos un entero que reciba el indice a utilizar
        int target;
        if(!free_list_.empty())
        {
        //Si existen espacios disponibles
            target = free_list_.front();
            free_list_.pop_front();
            //Target se setea como el espacio encontrado
            replacer_->Unpin(target);
            //Actualizamos los contenidos de la page_table y el buffer_pool
            page_table_.insert({page_id, target});
            buffer_pool[target].pin_count++;
            buffer_pool[target].page = file_->readPage(page_id);
            buffer_pool[target].dirty_bit = 0;
            //Retornamos el puntero a la pagina requerida 
            return buffer_pool[target].page;
        }
        else
        {//Sino, utilizamos el algoritmo de remplazamiento
            if(!replacer_->Victim(&target)) 
            {
                //std::cout << "NULL\n";
                return nullptr;
            }
            int target_page_id = buffer_pool[target].page->page_number(); // page_id de la pagina que se eliminara del buffer
            std::cout << "target_page_id: " << target_page_id << "\n"; 
            
            if(buffer_pool[target].dirty_bit)
            {
                if(!FlushPageImpl(target_page_id)) 
                {
                    return nullptr;
                }
            }
            //replacer_->Pin(target);
            buffer_pool[target].pin_count++;
            //Actualizamos los contenidos de la page_table y el buffer_pool
            page_table_.erase(target_page_id);          // borra la pagina del buffer
            page_table_.insert({page_id, target});
            buffer_pool[target].page = file_->readPage(page_id);    // lee nueva pagina del file
            buffer_pool[target].dirty_bit = 0;
            //Retornamos el puntero a la pagina requerida 
            return buffer_pool[target].page;
        }
        
    }
    return nullptr;
}

bool Buffer::UnpinPageImpl(int page_id, bool is_dirty)
{
    if(page_table_.find(page_id) == page_table_.end())
    {
        return true;
    }
    int target = page_table_[page_id];
    if(buffer_pool[target].pin_count <= 0)
    {
        return false;
    }
    else 
    {
        buffer_pool[target].pin_count--;
        buffer_pool[target].dirty_bit |= is_dirty;
        if(buffer_pool[target].pin_count == 0)
        {
            replacer_->Unpin(target);
        }
        return true;
    }
}

bool Buffer::FlushPageImpl(int page_id) 
{
    if(page_id == -1)
        return false;
    if(page_table_.find(page_id) == page_table_.end())
        return false;
    else 
    {
        int target = page_table_[page_id];
        if(!buffer_pool[target].dirty_bit)
            return true;
        file_->write_page(page_id, buffer_pool[target].page);
        buffer_pool[target].dirty_bit = 0;
        return true;
    }
}

Page *Buffer::NewPageImpl(int *page_id) 
{
    // 1. Si todas las páginas del bufferpool están fijadas, devuelve nullptr.
    // 2. Escoge una página víctima P de la lista libre o del replacer. Siempre se elige primero de la lista libre.
    // 3. Actualizar los metadatos de P, poner a cero la memoria y añadir P a la page_table.
    // 4. Establecer el parámetro de salida del ID de la página. Devuelve un puntero a P.
    int free_frame = free_list_.front();
    if(!free_list_.empty())
    {
        free_list_.pop_front();
        *page_id = file_->allocatePage();
        page_table_[*page_id] = free_frame;
        replacer_->Pin(free_frame);
        buffer_pool[free_frame].page        = new Page(*page_id);
        buffer_pool[free_frame].pin_count   = 1;
        buffer_pool[free_frame].dirty_bit   = false;

        //std::cout << "Numero asig: " << *page_id << "\n";
        return buffer_pool[free_frame].page;
    }
    else
    {
        int victim;
        if(!replacer_->Victim(&victim)) 
            return nullptr;

        if(buffer_pool[victim].dirty_bit)
            if(!FlushPageImpl(buffer_pool[victim].page->page_number()))
                return nullptr;

        int victim_page_id = buffer_pool[victim].page->page_number();
        page_table_.erase(victim_page_id);
        *page_id = file_->allocatePage();
        page_table_[*page_id] = victim;
        replacer_->Pin(victim);
        buffer_pool[free_frame].page = new Page(*page_id);
        buffer_pool[victim].pin_count = 1;
        buffer_pool[victim].dirty_bit = false;

        return buffer_pool[victim].page;
    }
}

bool Buffer::DeletePageImpl(int page_id) {
    // 1. Buscar en la page_table la página solicitada (P).
    // 1. Si P no existe, devuelve true.
    // 2. Si P existe, pero tiene un número de pines distinto de cero, devuelve false. 
    //    Alguien está usando la página.
    // 3. En caso contrario, P puede ser eliminado. 
    //    Elimina P de la page_table, restablece sus metadatos y la devuelve a la lista de libres.
    if(page_table_.find(page_id) == page_table_.end())
        return true;
    else
    {
        int target = page_table_[page_id];
        if(buffer_pool[target].pin_count != 0) 
            return false;
        else
        { 
            page_table_.erase(page_id);
            buffer_pool[target].page = nullptr;
            buffer_pool[target].dirty_bit = false;
            free_list_.push_back(target);
            return true;
        }
    }
    return false;
}

void Buffer::FlushAllPagesImpl() 
{
    for(size_t i = 0; i < pool_size_; ++i)
        if(buffer_pool[i].dirty_bit)
            if(!FlushPageImpl(buffer_pool[i].page->page_number())){}
}


Buffer::~Buffer(){
    delete replacer_;
}