#pragma once
#include <cstdint>
#include <cstddef>
#include <memory>
#include <stdint.h>
#include <assert.h>
#include <string>
#include <sstream>
#include <iostream>

#include "../types.h"
// #include "Page_Iterator.h"
/*Metadata de la pagina e indice hacia la siguiente pagina en el file*/

namespace DB
{
    struct PageHeader
    {
        /*Este es el desplazamiento del primer byte no utilizado después de un slot.*/
        std::uint16_t free_space_lower_bound;
        /*Este es el desplazamiento del último byte no utilizado antes del primer registro de datos.*/
        std::uint16_t free_space_upper_bound;
        SlotId num_slots;           // cantidad de slot para guardar registros
        SlotId num_free_slots;      // cant slots libres
        PageId current_page_number; // numero actual de la pagina
        /*Siguiente pagina en el File*/
        PageId next_page_number; // numero de la sig pag
        /*Sobrecarga operador de comparacion*/
        bool operator==(const PageHeader &rhs) const
        {
            return num_slots == rhs.num_slots &&
                   num_free_slots == rhs.num_free_slots &&
                   current_page_number == rhs.current_page_number &&
                   next_page_number == rhs.next_page_number;
        }
    };

    struct PageSlot
    {
        bool used;                 // slot esta usado o no
        std::uint16_t item_offset; // donde comienza el registro
        std::uint16_t item_length; // tamanio del registro
    };

    class PageIterator;

    class Page
    {
    private:
        PageHeader header_;
        std::string data_;
        void initialize();

        void set_page_number(const PageId new_page_number) // actualiza num pagina
        {
            header_.current_page_number = new_page_number;
        }

        void set_next_page_number(const PageId new_next_page_number) // actualiza num de sig pagina
        {
            header_.next_page_number = new_next_page_number;
        }

        void deleteRecord(const RecordId &record_id, const bool allow_slot_compaction); // elimina registro con/sin compactacion

        PageSlot *getSlot(const SlotId slot_number); // obtiene puntero a un slot

        const PageSlot &getSlot(const SlotId slot_number) const; // obtiene referencia a slot

        SlotId getAvailableSlot(); // devuelve el id de un slot para guardar un registro

        void insertRecordInSlot(const SlotId slot_number, const std::string &record_data); // inserta datos segund el id de slot

        void validateRecordId(const RecordId &record_id) const; // verifica que un registro este en la pagina y que su slot no este en uso

        bool isUsed() const { return page_number() != INVALID_NUMBER; } // si la pagina es usada
        friend class File;
        friend class PageIterator;

    public:
        static const std::size_t SIZE = 8192;                           // tamanio de la pagina
        static const std::size_t DATA_SIZE = SIZE - sizeof(PageHeader); // tamanio de los datos quitan el tamanio del header
        static const PageId INVALID_NUMBER = 0;
        static const SlotId INVALID_SLOT = 0;
        Page();
        RecordId insertRecord(const std::string &record_data);                        // inserta registro
        std::string getRecord(const RecordId &record_id) const;                       // obtiene registro
        void updateRecord(const RecordId &record_id, const std::string &record_data); // actualiza registro
        void deleteRecord(const RecordId &record_id);                                 // elimina registro

        bool hasSpaceForRecord(const std::string &record_data) const; // hay espacio para guardar el registro
        std::uint16_t getFreeSpace() const { return header_.free_space_upper_bound -
                                                    header_.free_space_lower_bound; } // devuelve el espacio libre
        PageId page_number() const { return header_.current_page_number; }            // devuelve el numero actual de pag
        PageId next_page_number() const { return header_.next_page_number; }          // devuelve el num de la sig pagina
        PageIterator begin();
        PageIterator end();
    };

    class PageIterator
    {
    private:
        Page *page_;
        RecordId current_record_;

    public:
        PageIterator() : page_(NULL)
        {
            current_record_.page_number = Page::INVALID_NUMBER;
            current_record_.slot_number = Page::INVALID_SLOT;
        }

        PageIterator(const PageIterator &it)
        {
            page_ = it.page_;
            current_record_ = it.current_record_;
        }

        PageIterator(Page *page)
            : page_(page)
        {
            assert(page_ != NULL);                                        // verifica que el puntero no apunte a NULL
            const SlotId used_slot = getNextUsedSlot(Page::INVALID_SLOT); // obtiene el sig slot usado
            current_record_ = {page_->page_number(), used_slot};          // actualiza a que pagina y slot pertenece el registro actual
        }

        PageIterator(Page *page, const RecordId &record_id)
            : page_(page),
              current_record_(record_id)
        {
        }

        PageIterator &operator++();

        PageIterator operator++(int);

        bool operator==(const PageIterator &rhs) const;

        bool operator!=(const PageIterator &rhs) const;

        std::string operator*() const;

        SlotId getNextUsedSlot(const SlotId start) const;
    };

    PageIterator &PageIterator::operator++() // ++it
    {
        assert(page_ != NULL);                                                 // que no apunte a NULL
        const SlotId used_slot = getNextUsedSlot(current_record_.slot_number); // obtiene el sig slot usado
        current_record_ = {page_->page_number(), used_slot};                   // siguiente registro

        return *this; // devuelve iterator
    }

    PageIterator PageIterator::operator++(int) // it++
    {
        PageIterator tmp = *this; // copy ourselves

        assert(page_ != NULL);                                                 // que no apunte a NULL
        const SlotId used_slot = getNextUsedSlot(current_record_.slot_number); // obtiene el sig slot usado
        current_record_ = {page_->page_number(), used_slot};                   // siguiente registro

        return tmp; // devuelve copia
    }

    bool PageIterator::operator==(const PageIterator &rhs) // compara si es el mismo numero de pagina y este en el mismo registro
        const
    {
        return page_->page_number() == rhs.page_->page_number() && current_record_ == rhs.current_record_;
    }
    bool PageIterator::operator!=(const PageIterator &rhs) // compara si no es el mismo numero de pagina o no este en el mismo registro
        const
    {
        return (page_->page_number() != rhs.page_->page_number()) || (current_record_ != rhs.current_record_);
    }

    std::string PageIterator::operator*() const // indireccion
    {
        return page_->getRecord(current_record_); // devuelve el registro
    }

    SlotId PageIterator::getNextUsedSlot(const SlotId start) const // obtiene el sig slot
    {
        SlotId slot_number = Page::INVALID_SLOT;
        for (SlotId i = start + 1; i <= page_->header_.num_slots; ++i) // recorre todos los slots
        {
            const PageSlot *slot = page_->getSlot(i); // obtiene puntero a slot
            if (slot->used)                           // si el slot esta usado
            {
                slot_number = i; // devuelve el slot
                break;
            }
        }
        return slot_number;
    }

    Page::Page()
    {
        initialize();
    }

    void Page::initialize()
    {
        header_.free_space_lower_bound = 0;
        header_.free_space_upper_bound = DATA_SIZE;
        header_.num_slots = 0;
        header_.num_free_slots = 0;
        header_.current_page_number = INVALID_NUMBER;
        header_.next_page_number = INVALID_NUMBER;
        data_.assign(DATA_SIZE, char());        // inicializa el tamaño del string
    }

    RecordId Page::insertRecord(const std::string &record_data) // inserta registro, devuelve su id
    {
        if (!hasSpaceForRecord(record_data)) // si no hay espacio
        {
            std::cerr << "Espacio insuficiente en la pagina " << page_number()
                      << "para almacenar el registro.  Requerido: " << record_data.length() << " bytes."
                      << " Disponible: " << getFreeSpace() << " bytes.";
            return {0,0};
        }
        const SlotId slot_number = getAvailableSlot(); // obtiene un slot
        insertRecordInSlot(slot_number, record_data);  // inserta registro
        return {page_number(), slot_number};           // devuelve el numero de pag y el numero de slot
    }

    std::string Page::getRecord(const RecordId &record_id) const // devuelve un registro
    {
        validateRecordId(record_id);                             // valida que el num de pagina sea correcto y el slot este en uso
        const PageSlot &slot = getSlot(record_id.slot_number);   // recupera el slot
        return data_.substr(slot.item_offset, slot.item_length); // obtiene el registro
    }

    void Page::updateRecord(const RecordId &record_id, const std::string &record_data)
    {
        validateRecordId(record_id);                                                    // valida que el num de pagina sea correcto y el slot este en uso
        const PageSlot *slot = getSlot(record_id.slot_number);                          // obtiene puntero al slot
        const std::size_t free_space_after_delete = getFreeSpace() + slot->item_length; // obtiene tamanio libre mas el tamanio que ocupa el registro
        if (record_data.length() > free_space_after_delete)                             // si el espacio libre es menor al del nuevo registro
        {
            std::cerr << "Espacio insuficiente en la pagina " << page_number()
                      << "para almacenar el registro.  Requerido: " << record_data.length() << " bytes."
                      << " Disponible: " << free_space_after_delete << " bytes.";
            return;
        }
        /*
          Tenemos que deshabilitar la compactación de slots aquí porque
          vamos a colocar los datos de registro en la misma ranura, y la compactación
          podría eliminar los slots si lo permitimos.
        */
        deleteRecord(record_id, false);                         // False: evita la compactacion de slots
        insertRecordInSlot(record_id.slot_number, record_data); // inserta registro al final
    }

    void Page::deleteRecord(const RecordId &record_id)
    {
        deleteRecord(record_id, true); /*Se permite la compactacion de slots en desuso*/
    }

    void Page::deleteRecord(const RecordId &record_id, const bool allow_slot_compaction) // borra un registro (falta analizar mas)
    {
        validateRecordId(record_id); // valida que el num de pagina sea correcto y el slot este en uso

        PageSlot *slot = getSlot(record_id.slot_number); // Se obtene el slot que contiene el registro

        data_.replace(slot->item_offset, slot->item_length, slot->item_length, '\0'); // Remplaza el registro con caracteres nulos

        // Comprime la data removiendo el vacio dejado por este record (si es necesario)
        std::uint16_t move_offset = slot->item_offset; // inicio del registro
        std::size_t move_bytes = 0;
        for (SlotId i = 1; i <= header_.num_slots; ++i) // recorre todos los slots
        {
            PageSlot *other_slot = getSlot(i);                                   // obtiene nuevo slot
            if (other_slot->used && other_slot->item_offset < slot->item_offset) // si nuevo slot es usado y esta antes del slot
            {
                if (other_slot->item_offset < move_offset) // si nuevo slot esta antes del slot
                {
                    move_offset = other_slot->item_offset; // inicio del slot es ahora nuevo slot
                }
                move_bytes += other_slot->item_length;
                // Actualice el slot para que los otros datos reflejen la próxima ubicación actualizada.
                other_slot->item_offset += slot->item_length;
            }
        }
        if (move_bytes > 0)
        {
            const std::string &data_to_move = data_.substr(move_offset, move_bytes);  // extrae los datos a mover
            data_.replace(move_offset + slot->item_length, move_bytes, data_to_move); // coloca los datos en el string
        }
        header_.free_space_upper_bound += slot->item_length;

        /* Se marca el slot como *LIBRE* */
        slot->used = false;
        slot->item_offset = 0;
        slot->item_length = 0;
        /*Aumenta el numero de slots libres*/
        ++header_.num_free_slots;

        // Compactacion de slots
        if (allow_slot_compaction && record_id.slot_number == header_.num_slots)
        {
            // Ultimo slot en la lista, asi que necesitamos liberar algun slot no usado que esta al final de la lissta de slot
            int num_slots_to_delete = 1; // cantidad de slots a eliminar
            for (SlotId i = 1; i < header_.num_slots; ++i)
            {
                /*Se recorre la lista en reversa, en busca de slots en desuso*/
                const PageSlot *other_slot = getSlot(header_.num_slots - i);
                if (!other_slot->used) // si slot no esta en uso
                {
                    ++num_slots_to_delete; // aumenta la cant de slots a eliminar
                }
                else
                {
                    // Detiene en el primer slot sin usar que encontremos, hasta que no podamos mover slots usados sin afectar los ids registro
                    break;
                }
            }
            /*Actualizacion de la metadata*/
            header_.num_slots -= num_slots_to_delete;
            header_.num_free_slots -= num_slots_to_delete;
            header_.free_space_lower_bound -= sizeof(PageSlot) * num_slots_to_delete;
        }
    }

    bool Page::hasSpaceForRecord(const std::string &record_data) const // hay espacio para guardar el registro
    {
        std::size_t record_size = record_data.length(); /*Se obtiene el tamaño del registro*/
        if (header_.num_free_slots == 0)                // si hay slots libres
        {
            record_size += sizeof(PageSlot); // Se adiciona el tamaño de la metadada del slot, al size del registro
        }
        return record_size <= getFreeSpace(); // si el espacio del registro es menor o igual al espacio libre
    }

    PageSlot *Page::getSlot(const SlotId slot_number) // devuelve un puntero a un pageSlot
    {
        return reinterpret_cast<PageSlot *>(&data_[(slot_number - 1) * sizeof(PageSlot)]); // devuelve un puntero
    }

    const PageSlot &Page::getSlot(const SlotId slot_number) const // devuelve un slot por referencia
    {
        return *reinterpret_cast<const PageSlot *>(&data_[(slot_number - 1) * sizeof(PageSlot)]); // indireccion del slot
    }

    SlotId Page::getAvailableSlot() // devuelve el id de un slot para guardar un registro
    {
        SlotId slot_number = INVALID_SLOT;
        if (header_.num_free_slots > 0) // si hay slots libres
        {
            // Tiene un slot asignado pero no usado que podemos reutilizar
            for (SlotId i = 1; i <= header_.num_slots; ++i) // busca un slot no usado
            {
                const PageSlot *slot = getSlot(i);
                if (!slot->used) // si el slot no es usado
                {
                    // No decrementa el numero libre de slots hasta que alguien coloque data en el slot
                    slot_number = i; // devuelve el numero del slot no usado
                    break;
                }
            }
        }
        else
        {
            slot_number = header_.num_slots + 1; // Tiene que asignar un nuevo slot
            ++header_.num_slots;                 // aumenta la cant de slots
            ++header_.num_free_slots;            // aumenta la cantidad de slots libres
            header_.free_space_lower_bound = sizeof(PageSlot) * header_.num_slots;
        }
        assert(slot_number != INVALID_SLOT);
        /*Casteo a "SlotId" en tiempo de compilacion*/
        return static_cast<SlotId>(slot_number); // devuelve el numero de slot
    }

    void Page::insertRecordInSlot(const SlotId slot_number, const std::string &record_data) // inserta registro
    {
        if (slot_number > header_.num_slots || slot_number == INVALID_SLOT)
        {
            std::cerr << "Intento de acceder a un slot el cual no esta actualmente en uso."
                      << " Pagina: " << page_number() << " Slot: " << slot_number;
            return;
        }
        PageSlot *slot = getSlot(slot_number); // Se obtiene puntero al slot
        if (slot->used)                        // si esta en uso
        {
            std::cerr << "Intento de insertar datos a un slot que esta actualmente en uso."
                      << " Pagina: " << page_number() << " Slot: " << slot_number;
            return;
        }
        const int record_length = record_data.length(); // guarda tamaño del string
        /*Actualiza el slot como "EN USO" y valores correspondientes*/
        slot->used = true;
        slot->item_length = record_length;
        slot->item_offset = header_.free_space_upper_bound - record_length; // donde comienza el registro

        header_.free_space_upper_bound = slot->item_offset; // Se actualiza el espacio LIBRE

        --header_.num_free_slots; // Reduce el numero de slots libres

        data_.replace(slot->item_offset, slot->item_length, record_data); // Se inserta el registro en el string, siguiendo el desplazamiento y tamaño correspondientes
    }


    void Page::validateRecordId(const RecordId &record_id) const // verifica que un registro este en la pagina y que su slot no este en uso
    {
        if (record_id.page_number != page_number()) // si no es el mismo numero de pagina
        {
            std::cerr << "Requerimiento dirigido a registro invalido."
                      << " Registro {pagina=" << record_id.page_number
                      << ", slot=" << record_id.slot_number
                      << "} de pagina " << page_number();
            return;
        }
        const PageSlot &slot = getSlot(record_id.slot_number);
        if (!slot.used) // si no esta en uso
        {
            std::cerr << "Requerimiento dirigido a registro invalido."
                      << " Registro {pagina=" << record_id.page_number
                      << ", slot=" << record_id.slot_number
                      << "} de pagina " << page_number();
            return;
        }
    }

    PageIterator Page::begin()
    {
        return PageIterator(this);
    }

    PageIterator Page::end()
    {
        const RecordId &end_record_id = {page_number(), Page::INVALID_SLOT};
        return PageIterator(this, end_record_id);
    }
}
