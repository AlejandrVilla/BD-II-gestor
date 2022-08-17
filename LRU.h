#pragma once
#include "replacer.h"
#include <list>
#include <vector>
class LRU : public Replacer
{
public:
    // numero paginas para el LRU
    explicit LRU(size_t num_pages);
    ~LRU();
    // Metodos
    bool Victim(int32_t *frame_id) override;
    void Pin(int32_t frame_id) override;
    void Unpin(int32_t frame_id) override;
    size_t Size() override;

private:
    // Atributos Privados
    bool IsInReplacer(int32_t frame_id);    // 
    std::list<int32_t> w_list{};
    std::vector<std::list<int32_t>::iterator> page_iters;
};

LRU::LRU(size_t num_pages) : page_iters{num_pages} {}

LRU::~LRU() = default;


// Metodo de eliminacion del objeto al que se accedio menos recientemente (back)
bool LRU::Victim(int32_t *frame_id)
{
    // si w_list esta vacia devuelve falso
    if (w_list.empty())
    {
        *frame_id = -1;
        return false;
    }
    //Actualiza el frame_id con la cola w_list y lo elimina
    //Limpia el iterador de paginas en la posicion del frame eliminado
    *frame_id = w_list.back();
    w_list.pop_back();
    page_iters[*frame_id] = std::list<int32_t>::iterator{};
    return true;
}

// Este metodo debe llamarse despues de que una pagina se fije a un frame en Buffer.
// Deberia eliminar el frame que contiene la pagina anclada del replacer
void LRU::Pin(int32_t frame_id)
{
    // Si no esta en el replacer termina
    // Si esta procede a eliminarlo de w_list
    if (!IsInReplacer(frame_id))
    {
        return;
    }
    w_list.erase(page_iters[frame_id]);
    page_iters[frame_id] = std::list<int32_t>::iterator{};
}

// Llamar a este método cuando el sea necesario retirar al frame del Buffer.
// Este metodo debe agregar el frame que contiene la pagina no fijada al replacer
void LRU::Unpin(int32_t frame_id)
{
    // Si ya esta en el replacer termina
    // Si no procede a agregarlo al frente de w_list
    if (IsInReplacer(frame_id))
    {
        return;
    }
    w_list.push_front(frame_id);
    page_iters[frame_id] = w_list.begin();
}

//Retrona la cantidad de frames en el replacer
size_t LRU::Size()
{
    return w_list.size();
}

// Verifica el estado de un frame en el replacer
bool LRU::IsInReplacer(int32_t frame_id)
{
    return page_iters[frame_id] != std::list<int32_t>::iterator{};
}
