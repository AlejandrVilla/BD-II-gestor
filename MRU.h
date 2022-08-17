#pragma once
#include "replacer.h"
#include <list>
#include <vector>

class MRU : public Replacer
{
public:
    // Tamanio
    explicit MRU(size_t num_pages);
    // Metodos
    ~MRU();
    bool Victim(int32_t *frame_id) override;
    void Pin(int32_t frame_id) override;
    void Unpin(int32_t frame_id) override;
    size_t Size() override;

private:
    // Privados
    bool IsInReplacer(int32_t frame_id);
    std::list<int32_t> w_list{};
    std::vector<std::list<int32_t>::iterator> page_iters;
};

MRU::MRU(size_t num_pages) {}

MRU::~MRU() = default;


// Metodo de eliminacion del objeto al que se accedio menos recientemente (front)
bool MRU::Victim(int32_t *frame_id)
{
    //Si w_list esta vacia retorna false
    if (w_list.empty())
    {
        *frame_id = -1;
        return false;
    }
    // Guarda el puntero al frame del frente
    // Elimina el puntero de las paginas
    *frame_id = w_list.front();
    w_list.pop_front();
    page_iters[*frame_id] = std::list<int32_t>::iterator{};
    return true;
}

// Este metodo debe llamarse despues de que una pagina se fije a un frame en Buffer.
// Deberia eliminar el frame que contiene la pagina anclada del replacer
void MRU::Pin(int32_t frame_id)
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
void MRU::Unpin(int32_t frame_id)
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
size_t MRU::Size()
{
    return w_list.size();
}

// Retorna el estado en el replacer
bool MRU::IsInReplacer(int32_t frame_id)
{
    return page_iters[frame_id] != std::list<int32_t>::iterator{};
}
