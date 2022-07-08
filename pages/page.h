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
        SlotId num_slots;
        SlotId num_free_slots;
        PageId current_page_number;
        /*Siguiente pagina en el File*/
        PageId next_page_number;
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
        bool used;
        std::uint16_t item_offset;
        std::uint16_t item_length;
    };

    class PageIterator;

    class Page
    {
    private:
        PageHeader header_;
        std::string data_;

        void initialize();

        void set_page_number(const PageId new_page_number)
        {
            header_.current_page_number = new_page_number;
        }

        void set_next_page_number(const PageId new_next_page_number)
        {
            header_.next_page_number = new_next_page_number;
        }

        void deleteRecord(const RecordId &record_id, const bool allow_slot_compaction);

        PageSlot *getSlot(const SlotId slot_number);

        const PageSlot &getSlot(const SlotId slot_number) const;

        SlotId getAvailableSlot();

        void insertRecordInSlot(const SlotId slot_number, const std::string &record_data);

        void validateRecordId(const RecordId &record_id) const;

        bool isUsed() const { return page_number() != INVALID_NUMBER; }

        friend class File;
        friend class PageIterator;

    public:
        static const std::size_t SIZE = 8192;
        static const std::size_t DATA_SIZE = SIZE - sizeof(PageHeader);
        static const PageId INVALID_NUMBER = 0;
        static const SlotId INVALID_SLOT = 0;
        Page();
        RecordId insertRecord(const std::string &record_data);
        std::string getRecord(const RecordId &record_id) const;
        void updateRecord(const RecordId &record_id, const std::string &record_data);
        void deleteRecord(const RecordId &record_id);

        bool hasSpaceForRecord(const std::string &record_data) const;
        std::uint16_t getFreeSpace() const { return header_.free_space_upper_bound -
                                                    header_.free_space_lower_bound; }
        PageId page_number() const { return header_.current_page_number; }
        PageId next_page_number() const { return header_.next_page_number; }
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
            assert(page_ != NULL);
            const SlotId used_slot = getNextUsedSlot(Page::INVALID_SLOT);
            current_record_ = {page_->page_number(), used_slot};
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

    PageIterator &PageIterator::operator++()
    {
        assert(page_ != NULL);
        const SlotId used_slot = getNextUsedSlot(current_record_.slot_number);
        current_record_ = {page_->page_number(), used_slot};

        return *this;
    }

    PageIterator PageIterator::operator++(int)
    {
        PageIterator tmp = *this; // copy ourselves

        assert(page_ != NULL);
        const SlotId used_slot = getNextUsedSlot(current_record_.slot_number);
        current_record_ = {page_->page_number(), used_slot};

        return tmp;
    }

    bool PageIterator::operator==(const PageIterator &rhs)
        const
    {
        return page_->page_number() == rhs.page_->page_number() &&
               current_record_ == rhs.current_record_;
    }
    bool PageIterator::operator!=(const PageIterator &rhs)
        const
    {
        return (page_->page_number() != rhs.page_->page_number()) ||
               (current_record_ != rhs.current_record_);
    }

    std::string PageIterator::operator*() const
    {
        return page_->getRecord(current_record_);
    }

    SlotId PageIterator::getNextUsedSlot(const SlotId start) const
    {
        SlotId slot_number = Page::INVALID_SLOT;
        for (SlotId i = start + 1; i <= page_->header_.num_slots; ++i)
        {
            const PageSlot *slot = page_->getSlot(i);
            if (slot->used)
            {
                slot_number = i;
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
        data_.assign(DATA_SIZE, char());
    }

    RecordId Page::insertRecord(const std::string &record_data)
    {
        if (!hasSpaceForRecord(record_data))
        {
            std::cerr << "Espacio insuficiente en la pagina " << page_number()
                      << "para almacenar el registro.  Requerido: " << record_data.length() << " bytes."
                      << " Disponible: " << getFreeSpace() << " bytes.";
            return {0, 0};
        }
        const SlotId slot_number = getAvailableSlot();
        insertRecordInSlot(slot_number, record_data);
        return {page_number(), slot_number};
    }

    std::string Page::getRecord(const RecordId &record_id) const
    {
        validateRecordId(record_id);
        const PageSlot &slot = getSlot(record_id.slot_number);
        return data_.substr(slot.item_offset, slot.item_length);
    }

    void Page::updateRecord(const RecordId &record_id, const std::string &record_data)
    {
        validateRecordId(record_id);
        const PageSlot *slot = getSlot(record_id.slot_number);
        const std::size_t free_space_after_delete = getFreeSpace() + slot->item_length;
        if (record_data.length() > free_space_after_delete)
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
        deleteRecord(record_id, false); /*False: evita la compactacion de slots*/
        insertRecordInSlot(record_id.slot_number, record_data);
    }

    void Page::deleteRecord(const RecordId &record_id)
    {
        deleteRecord(record_id, true); /*Se permite la compactacion de slots en desuso*/
    }

    void Page::deleteRecord(const RecordId &record_id, const bool allow_slot_compaction)
    {
        validateRecordId(record_id);
        /*Se obtene el slot que contiene el registro*/
        PageSlot *slot = getSlot(record_id.slot_number);
        /*Remplaza el registro con caracteres nulos*/
        data_.replace(slot->item_offset, slot->item_length, slot->item_length, '\0');

        // Comprime la data removiendo el vacio dejado por este record (si es necesario)
        std::uint16_t move_offset = slot->item_offset;
        std::size_t move_bytes = 0;
        for (SlotId i = 1; i <= header_.num_slots; ++i)
        {
            PageSlot *other_slot = getSlot(i);
            if (other_slot->used && other_slot->item_offset < slot->item_offset)
            {
                if (other_slot->item_offset < move_offset)
                {
                    move_offset = other_slot->item_offset;
                }
                move_bytes += other_slot->item_length;
                // Actualice el slot para que los otros datos reflejen la próxima ubicación actualizada.
                other_slot->item_offset += slot->item_length;
            }
        }
        if (move_bytes > 0)
        {
            const std::string &data_to_move = data_.substr(move_offset, move_bytes);
            data_.replace(move_offset + slot->item_length, move_bytes, data_to_move);
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
            /*Si se trata del ultimo slot de la lista*/
            int num_slots_to_delete = 1;
            for (SlotId i = 1; i < header_.num_slots; ++i)
            {
                /*Se recorre la lista en reversa, en busca de slots en desuso*/
                const PageSlot *other_slot = getSlot(header_.num_slots - i);
                if (!other_slot->used)
                {
                    ++num_slots_to_delete;
                }
                else
                {
                    // Detiene en el primer slot sin usar que encontremos, hasta que no podamos mover slots usados sin afectar los ids registro
                    /*Se detiene al 1er slot usado encontrado, asi no alteramos la indexacion*/
                    break;
                }
            }
            /*Actualizacion de la metadata*/
            header_.num_slots -= num_slots_to_delete;
            header_.num_free_slots -= num_slots_to_delete;
            header_.free_space_lower_bound -= sizeof(PageSlot) * num_slots_to_delete;
        }
    }

    bool Page::hasSpaceForRecord(const std::string &record_data) const
    {
        /*Se obtiene el tamaño del registro*/
        std::size_t record_size = record_data.length();
        if (header_.num_free_slots == 0)
        {
            /*Si ya no hay slots libres*/
            record_size += sizeof(PageSlot); // Se adiciona el tamaño de la metadada del slot, al size del registro
        }
        return record_size <= getFreeSpace();
    }

    PageSlot *Page::getSlot(const SlotId slot_number)
    {
        /**/
        return reinterpret_cast<PageSlot *>(
            &data_[(slot_number - 1) * sizeof(PageSlot)]);
    }

    const PageSlot &Page::getSlot(const SlotId slot_number) const
    {
        return *reinterpret_cast<const PageSlot *>(
            &data_[(slot_number - 1) * sizeof(PageSlot)]);
    }

    SlotId Page::getAvailableSlot()
    {
        SlotId slot_number = INVALID_SLOT;
        if (header_.num_free_slots > 0)
        {
            // Tiene un slot asignado pero no usado que podemos reutilizar
            for (SlotId i = 1; i <= header_.num_slots; ++i)
            {
                const PageSlot *slot = getSlot(i);
                if (!slot->used)
                {
                    // No decrementa el numero libre de slots hasta que alguien coloque data en el slot
                    slot_number = i;
                    break;
                }
            }
        }
        else
        {
            // Tiene que asignar un nuevo slot
            slot_number = header_.num_slots + 1;
            ++header_.num_slots;
            ++header_.num_free_slots;
            header_.free_space_lower_bound = sizeof(PageSlot) * header_.num_slots;
        }
        assert(slot_number != INVALID_SLOT);
        /*Casteo a "SlotId" en tiempo de compilacion*/
        return static_cast<SlotId>(slot_number);
    }

    void Page::insertRecordInSlot(const SlotId slot_number, const std::string &record_data)
    {
        if (slot_number > header_.num_slots ||
            slot_number == INVALID_SLOT)
        {
            std::cerr << "Intento de acceder a un slot el cual no esta actualmente en uso."
                      << " Pagina: " << page_number() << " Slot: " << slot_number;
            return;
        }
        /*Se obtiene el slot*/
        PageSlot *slot = getSlot(slot_number);
        if (slot->used)
        {
            std::cerr << "Intento de insertar datos a un slot que esta actualmente en uso."
                      << " Pagina: " << page_number() << " Slot: " << slot_number;
            return;
        }
        const int record_length = record_data.length();
        /*Actualiza el slot como "EN USO" y valores correspondientes*/
        slot->used = true;
        slot->item_length = record_length;
        slot->item_offset = header_.free_space_upper_bound - record_length;
        /*Se actualiza el espacio LIBRE*/
        header_.free_space_upper_bound = slot->item_offset;
        /*Reduce el numero de slots libres*/
        --header_.num_free_slots;
        /*Se inserta el registro en el string, siguiendo el desplazamiento y tamaño correspondientes*/
        data_.replace(slot->item_offset, slot->item_length, record_data);
    }

    void Page::validateRecordId(const RecordId &record_id) const
    {
        if (record_id.page_number != page_number())
        {
            std::cerr << "Requerimiento dirigido a registro invalido."
                      << " Registro {pagina=" << record_id.page_number
                      << ", slot=" << record_id.slot_number
                      << "} de pagina " << page_number();
            return;
        }
        const PageSlot &slot = getSlot(record_id.slot_number);
        if (!slot.used)
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
