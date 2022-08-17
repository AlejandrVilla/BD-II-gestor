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
  Page *page;
  int dirty_bit;
  int pin_count; // cuantos tienen acceso a la pagina
};

// Buffer pool
class Buffer
{
public:
  Buffer(size_t pool_size, File *file);
  bool FlushPageImpl(int page_id);
  Page *NewPageImpl(int *page_id);
  Page *FetchPageImpl(int page_id);
  bool DeletePageImpl(int page_id);
  bool UnpinPageImpl(int page_id, bool is_dirty);
  void FlushAllPagesImpl();
  ~Buffer();
  std::vector<Frame> buffer_pool;

private:
  size_t pool_size_;                        // Tamanio del buffer
  Replacer *replacer_;                      // Clase abstracta (polimorfismo)
  std::list<int> free_list_;                // Lista de indices de frames libres
  std::unordered_map<int, int> page_table_; // Mapa de <idPage , idFrame>
  File *file_;
};

// Constructor con tamanio de buffer_pool y puntero al File
Buffer::Buffer(size_t pool_size, File *file) : pool_size_(pool_size), file_(file)
{
  buffer_pool = std::vector<Frame>(pool_size);
  // Inicializamos el replacer con el algoritmo de remplazamiento elegido
  replacer_ = new Clock(pool_size);
  for (int32_t i = 0; i < pool_size; i++)
  {
    // Inicializamos las listas en 0
    free_list_.emplace_back(static_cast<int>(i));
    buffer_pool[i].dirty_bit = 0;
    buffer_pool[i].pin_count = 0;
  }
}

// Obtener pagina solicitada por id
Page *Buffer::FetchPageImpl(int page_id)
{
  // 1. Buscar en la page_table la página solicitada (P).
  // 1.1 Si P existe, la fija y la devuelve inmediatamente.
  // 1.2 Si P no existe, encuentra una página de reemplazo (R) de la lista libre o del replacer.
  // 2. Si R está dirty, escribirla de nuevo en el disco.
  // 3. Eliminar R de la page_table e insertar P.
  // 4. Actualizar los metadatos de P, leer el contenido de la página desde el disco, y luego devolver un puntero a P.

  // Si la pagina se encuentra ya en el buffer.
  if (page_table_.find(page_id) != page_table_.end())
  {
    // Actualiza metadata de la pagina en el Buffer
    int target = page_table_[page_id]; // devuelve frame_id
    replacer_->Pin(target);
    buffer_pool[target].pin_count++;
    // Retorna el puntero la pagina encontrada
    return buffer_pool[target].page;
  }
  // En caso la pagina solicitada, NO se encuentre en el buffer
  else
  {
    // Seteamos un entero que reciba el indice a utilizar
    int target;
    if (!free_list_.empty())
    {
      // Si existen espacios disponibles
      target = free_list_.front();
      free_list_.pop_front();
      // Target se setea como el espacio encontrado
      replacer_->Unpin(target);
      // Actualizamos los contenidos de la page_table y el buffer_pool
      page_table_.insert({page_id, target});
      buffer_pool[target].pin_count++;
      buffer_pool[target].page = file_->readPage(page_id);
      buffer_pool[target].dirty_bit = 0;
      // Retornamos el puntero a la pagina requerida
      return buffer_pool[target].page;
    }
    else
    { // Sino, utilizamos el algoritmo de remplazamiento
      if (!replacer_->Victim(&target))
      {
        // std::cout << "NULL\n";
        return nullptr;
      }
      int target_page_id = buffer_pool[target].page->page_number(); // page_id de la pagina que se eliminara del buffer
      std::cout << "target_page_id: " << target_page_id << "\n";

      if (buffer_pool[target].dirty_bit)
      {
        if (!FlushPageImpl(target_page_id))
        {
          return nullptr;
        }
      }
      // replacer_->Pin(target);
      buffer_pool[target].pin_count++;
      // Actualizamos los contenidos de la page_table y el buffer_pool
      page_table_.erase(target_page_id); // borra la pagina del buffer
      page_table_.insert({page_id, target});
      buffer_pool[target].page = file_->readPage(page_id); // lee nueva pagina del file
      buffer_pool[target].dirty_bit = 0;
      // Retornamos el puntero a la pagina requerida
      return buffer_pool[target].page;
    }
  }
  return nullptr;
}

bool Buffer::UnpinPageImpl(int page_id, bool is_dirty)
{
  if (page_table_.find(page_id) == page_table_.end())
  {
    return true;
  }
  int target = page_table_[page_id];
  if (buffer_pool[target].pin_count <= 0)
  {
    return false;
  }
  else
  {
    buffer_pool[target].pin_count--;
    buffer_pool[target].dirty_bit |= is_dirty;
    if (buffer_pool[target].pin_count == 0)
    {
      replacer_->Unpin(target);
    }
    return true;
  }

  // void Buffer::FlushAllPagesImpl (const File *file)
  // {
  //     for (uint32_t i = 0; i < numBufs; i++)
  //     {
  //         if (BufFrameTable[i].file == file)
  //         {
  //             if (!BufFrameTable[i].valid)
  //             {
  //                 // Si el frame está no es valido, lanza BadBufferException

  //                 std::cerr << "Este buffer está mal: " << i;
  //                 return;
  //                 // throw BadBufferException(i, BufFrameTable[i].dirty, BufFrameTable[i].valid, BufFrameTable[i].refbit);
  //             }
  //             if (BufFrameTable[i].num_pin != 0)
  //             {
  //                 // Si el frame está fijado, lanza PagePinnedException
  //                 std::cerr << "Esta pagina ya está anclada. file:  "
  //                           << "pagePinned"
  //                           << "page: " << BufFrameTable[i].pageNo << "frame: " << i;
  //                 return;
  //                 // throw PagePinnedException("pagePinned", BufFrameTable[i].pageNo, i);
  //             }
  //             if (BufFrameTable[i].dirty)
  //             {
  //                 // Si la trama está corrupta, la escribe en el disco, y luego pone el valor de dirty en false
  //                 BufFrameTable[i].file->writePage(bufPool[i]);
  //                 BufFrameTable[i].dirty = false;
  //             }
  //             // Eliminar la página de la tabla de hash
  //             hashTable->remove(file, BufFrameTable[i].pageNo);
  //             // Limpiar buffer frame
  //             BufFrameTable[i].Clear();
  //         }
  //     }
  // }

  // // Este método devolverá una página recién asignada.
  // void BufManager::allocPage(File *file, PaginaId &pageNo, Pagina *&page)
  // {
  //     FrameId frameNum;

  //     Pagina p1 = file->allocatePage();
  //     allocBuf(frameNum);
  //     bufPool[frameNum] = p1;
  //     hashTable->insert(file, p1.num_pagina(), frameNum);
  //     BufFrameTable[frameNum].Set(file, p1.num_pagina());
  //     pageNo = p1.num_pagina();
  //     page = &bufPool[frameNum];
  // }

  // // Este método elimina una página concreta del archivo.
  // void BufManager::disposePage(File *file, const PaginaId PageNo)
  // {
  //     try
  //     {
  //         FrameId frameNum;
  //         hashTable->lookup(file, PageNo, frameNum);
  //         BufFrameTable[frameNum].Clear();
  //         hashTable->remove(file, PageNo);
  //         file->deletePage(PageNo);
  //     }
  //     catch (std::tuple<std::string, PaginaId> a)
  //     {
  //         // Hash not found
  //     }
  // }

  // void BufManager::printSelf(void)
  // {
  //     BufFrame *tmpbuf;
  //     int validFrames = 0;

  //     for (std::uint32_t i = 0; i < numBufs; i++)
  //     {
  //         tmpbuf = &(BufFrameTable[i]);
  //         std::cout << "FrameNo:" << i << " ";
  //         tmpbuf->Print();

  //         if (tmpbuf->valid == true)
  //             validFrames++;
  //     }

  //     std::cout << "Número total de Frames Válidos:" << validFrames << "\n";
  // }
}
