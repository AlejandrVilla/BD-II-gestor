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
/*Metadata de la pagina e indice hacia la siguiente pagina en el file*/

namespace DBMS
{
    struct PaginaHeader
    {
        /*Este es el desplazamiento del primer byte no utilizado después de un slot.*/
        std::uint16_t free_space_lower_bound;
        /*Este es el desplazamiento del último byte no utilizado antes del primer registro de datos.*/
        std::uint16_t free_space_upper_bound;
        SlotId num_slots;           // cantidad de slot para guardar registros
        SlotId num_slots_libres;      // cant slots libres
        PaginaId num_pagina_actual; // numero actual de la pagina
        /*Siguiente pagina en el File*/
        PaginaId sig_num_pagina; // numero de la sig pag
        /*Sobrecarga operador de comparacion*/
        bool operator==(const PaginaHeader &rhs) const
        {
            return num_slots == rhs.num_slots &&
                   num_slots_libres == rhs.num_slots_libres &&
                   num_pagina_actual == rhs.num_pagina_actual &&
                   sig_num_pagina == rhs.sig_num_pagina;
        }
    };

    struct PaginaSlot
    {
        bool used;                 // slot esta usado o no
        std::uint16_t item_offset; // donde comienza el registro
        std::uint16_t item_length; // tamanio del registro
    };

    class PaginaIterator;

    class Pagina
    {
    private:
        PaginaHeader header_;
        std::string data_;
        void initialize();

        void set_num_pagina(const PaginaId nuevo_num_pagina) // actualiza num pagina
        {
            header_.num_pagina_actual = nuevo_num_pagina;
        }

        void set_sig_num_pagina(const PaginaId nuevo_num_pagina_sig) // actualiza num de sig pagina
        {
            header_.sig_num_pagina = nuevo_num_pagina_sig;
        }

        void deleteRecord(const RecordId &record_id, const bool allow_slot_compaction); // elimina registro con/sin compactacion
        PaginaSlot *getSlot(const SlotId num_slot); // obtiene puntero a un slot
        const PaginaSlot &getSlot(const SlotId num_slot) const; // obtiene referencia a slot
        SlotId getAvailableSlot(); // devuelve el id de un slot para guardar un registro
        void insertRecordInSlot(const SlotId num_slot, const std::string &record_data); // inserta datos segund el id de slot
        void validateRecordId(const RecordId &record_id) const; // verifica que un registro este en la pagina y que su slot no este en uso
        bool isUsed() const { return num_pagina() != INVALID_NUMBER; } // si la pagina es usada
        friend class File;
        friend class PaginaIterator;

    public:
        static const std::size_t SIZE = 8192;                           // tamanio de la pagina
        static const std::size_t DATA_SIZE = SIZE - sizeof(PaginaHeader); // tamanio de los datos quitan el tamanio del header
        static const PaginaId INVALID_NUMBER = 0;
        static const SlotId INVALID_SLOT = 0;
        Pagina();
        RecordId insertRecord(const std::string &record_data);                        // inserta registro
        std::string getRecord(const RecordId &record_id) const;                       // obtiene registro
        void updateRecord(const RecordId &record_id, const std::string &record_data); // actualiza registro
        void deleteRecord(const RecordId &record_id);                                 // elimina registro
        bool hasSpaceForRecord(const std::string &record_data) const; // hay espacio para guardar el registro
        std::uint16_t getFreeSpace() const { return header_.free_space_upper_bound - header_.free_space_lower_bound; } // devuelve el espacio libre
        PaginaId num_pagina() const { return header_.num_pagina_actual; }            // devuelve el numero actual de pag
        PaginaId sig_num_pagina() const { return header_.sig_num_pagina; }          // devuelve el num de la sig pagina
        PaginaIterator begin();
        PaginaIterator end();
    };

    class PaginaIterator
    {
    private:
        Pagina *pagina_;
        RecordId current_record_;

    public:
        PaginaIterator() : pagina_(NULL)
        {
            current_record_.num_pagina = Pagina::INVALID_NUMBER;
            current_record_.num_slot = Pagina::INVALID_SLOT;
        }
        PaginaIterator(const PaginaIterator &it)
        {
            pagina_ = it.pagina_;
            current_record_ = it.current_record_;
        }
        PaginaIterator(Pagina *pagina)
            : pagina_(pagina)
        {
            assert(pagina_ != NULL);                                        // verifica que el puntero no apunte a NULL
            const SlotId used_slot = getNextUsedSlot(Pagina::INVALID_SLOT); // obtiene el sig slot usado
            current_record_ = {pagina_->num_pagina(), used_slot};          // actualiza a que pagina y slot pertenece el registro actual
        }
        PaginaIterator(Pagina *pagina, const RecordId &record_id): pagina_(pagina), current_record_(record_id){}
        PaginaIterator &operator++();
        PaginaIterator operator++(int);
        bool operator==(const PaginaIterator &rhs) const;
        bool operator!=(const PaginaIterator &rhs) const;
        std::string operator*() const;
        SlotId getNextUsedSlot(const SlotId start) const;
    };

    PaginaIterator &PaginaIterator::operator++() // ++it
    {
        assert(pagina_ != NULL);                                                 // que no apunte a NULL
        const SlotId used_slot = getNextUsedSlot(current_record_.num_slot); // obtiene el sig slot usado
        current_record_ = {pagina_->num_pagina(), used_slot};                   // siguiente registro
        return *this; // devuelve iterator
    }

    PaginaIterator PaginaIterator::operator++(int) // it++
    {
        PaginaIterator tmp = *this; // copy ourselves
        assert(pagina_ != NULL);                                                 // que no apunte a NULL
        const SlotId used_slot = getNextUsedSlot(current_record_.num_slot); // obtiene el sig slot usado
        current_record_ = {pagina_->num_pagina(), used_slot};                   // siguiente registro
        return tmp; // devuelve copia
    }

    bool PaginaIterator::operator==(const PaginaIterator &rhs) const // compara si es el mismo numero de pagina y este en el mismo registro
    {
        return pagina_->num_pagina() == rhs.pagina_->num_pagina() && current_record_ == rhs.current_record_;
    }

    bool PaginaIterator::operator!=(const PaginaIterator &rhs) const// compara si no es el mismo numero de pagina o no este en el mismo registro
    {
        return (pagina_->num_pagina() != rhs.pagina_->num_pagina()) || (current_record_ != rhs.current_record_);
    }

    std::string PaginaIterator::operator*() const // indireccion
    {
        return pagina_->getRecord(current_record_); // devuelve el registro
    }

    SlotId PaginaIterator::getNextUsedSlot(const SlotId start) const // obtiene el sig slot
    {
        SlotId num_slot = Pagina::INVALID_SLOT;
        for (SlotId i = start + 1; i <= pagina_->header_.num_slots; ++i) // recorre todos los slots
        {
            const PaginaSlot *slot = pagina_->getSlot(i); // obtiene puntero a slot
            if (slot->used)                           // si el slot esta usado
            {
                num_slot = i; // devuelve el slot
                break;
            }
        }
        return num_slot;
    }

    Pagina::Pagina(){initialize();}

    void Pagina::initialize()
    {
        header_.free_space_lower_bound = 0;
        header_.free_space_upper_bound = DATA_SIZE;
        header_.num_slots = 0;
        header_.num_slots_libres = 0;
        header_.num_pagina_actual = INVALID_NUMBER;
        header_.sig_num_pagina = INVALID_NUMBER;
        data_.assign(DATA_SIZE, char());        // inicializa el tamaño del string
    }

    RecordId Pagina::insertRecord(const std::string &record_data) // inserta registro, devuelve su id
    {
        if (!hasSpaceForRecord(record_data)) // si no hay espacio
        {
            std::cerr << "Espacio insuficiente en la pagina " << num_pagina()
                      << "para almacenar el registro.  Requerido: " << record_data.length() << " bytes."
                      << " Disponible: " << getFreeSpace() << " bytes.";
            return {0,0};
        }
        const SlotId num_slot = getAvailableSlot(); // obtiene un slot
        insertRecordInSlot(num_slot, record_data);  // inserta registro
        return {num_pagina(), num_slot};           // devuelve el numero de pag y el numero de slot
    }

    std::string Pagina::getRecord(const RecordId &record_id) const // devuelve un registro
    {
        // validateRecordId(record_id);                             // valida que el num de pagina sea correcto y el slot este en uso
        const PaginaSlot &slot = getSlot(record_id.num_slot);   // recupera el slot
        return data_;
        // return data_.substr(slot.item_offset, slot.item_length); // obtiene el registro
    }

    void Pagina::updateRecord(const RecordId &record_id, const std::string &record_data)
    {
        validateRecordId(record_id);                                                    // valida que el num de pagina sea correcto y el slot este en uso
        const PaginaSlot *slot = getSlot(record_id.num_slot);                          // obtiene puntero al slot
        const std::size_t free_space_after_delete = getFreeSpace() + slot->item_length; // obtiene tamanio libre mas el tamanio que ocupa el registro
        if (record_data.length() > free_space_after_delete)                             // si el espacio libre es menor al del nuevo registro
        {
            std::cerr << "Espacio insuficiente en la pagina " << num_pagina()
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
        insertRecordInSlot(record_id.num_slot, record_data); // inserta registro al final
    }

    void Pagina::deleteRecord(const RecordId &record_id)
    {
        deleteRecord(record_id, true); /*Se permite la compactacion de slots en desuso*/
    }

    void Pagina::deleteRecord(const RecordId &record_id, const bool allow_slot_compaction) // borra un registro (falta analizar mas)
    {
        validateRecordId(record_id); // valida que el num de pagina sea correcto y el slot este en uso

        PaginaSlot *slot = getSlot(record_id.num_slot); // Se obtene el slot que contiene el registro

        data_.replace(slot->item_offset, slot->item_length, slot->item_length, '\0'); // Remplaza el registro con caracteres nulos

        // Comprime la data removiendo el vacio dejado por este record (si es necesario)
        std::uint16_t move_offset = slot->item_offset; // inicio del registro
        std::size_t move_bytes = 0;
        for (SlotId i = 1; i <= header_.num_slots; ++i) // recorre todos los slots
        {
            PaginaSlot *otro_slot = getSlot(i);                                   // obtiene nuevo slot
            if (otro_slot->used && otro_slot->item_offset < slot->item_offset) // si nuevo slot es usado y esta antes del slot
            {
                if (otro_slot->item_offset < move_offset) // si nuevo slot esta antes del slot
                {
                    move_offset = otro_slot->item_offset; // inicio del slot es ahora nuevo slot
                }
                move_bytes += otro_slot->item_length;
                // Actualice el slot para que los otros datos reflejen la próxima ubicación actualizada.
                otro_slot->item_offset += slot->item_length;
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
        ++header_.num_slots_libres;

        // Compactacion de slots
        if (allow_slot_compaction && record_id.num_slot == header_.num_slots)
        {
            // Ultimo slot en la lista, asi que necesitamos liberar algun slot no usado que esta al final de la lissta de slot
            int num_slots_borrar = 1; // cantidad de slots a eliminar
            for (SlotId i = 1; i < header_.num_slots; ++i)
            {
                /*Se recorre la lista en reversa, en busca de slots en desuso*/
                const PaginaSlot *otro_slot = getSlot(header_.num_slots - i);
                if (!otro_slot->used) // si slot no esta en uso
                {
                    ++num_slots_borrar; // aumenta la cant de slots a eliminar
                }
                else
                {
                    // Detiene en el primer slot sin usar que encontremos, hasta que no podamos mover slots usados sin afectar los ids registro
                    break;
                }
            }
            /*Actualizacion de la metadata*/
            header_.num_slots -= num_slots_borrar;
            header_.num_slots_libres -= num_slots_borrar;
            header_.free_space_lower_bound -= sizeof(PaginaSlot) * num_slots_borrar;
        }
    }

    bool Pagina::hasSpaceForRecord(const std::string &record_data) const // hay espacio para guardar el registro
    {
        std::size_t record_size = record_data.length(); /*Se obtiene el tamaño del registro*/
        if (header_.num_slots_libres == 0)                // si hay slots libres
        {
            record_size += sizeof(PaginaSlot); // Se adiciona el tamaño de la metadada del slot, al size del registro
        }
        return record_size <= getFreeSpace(); // si el espacio del registro es menor o igual al espacio libre
    }

    PaginaSlot *Pagina::getSlot(const SlotId num_slot) // devuelve un puntero a un PaginaSlot
    {
        return reinterpret_cast<PaginaSlot *>(&data_[(num_slot - 1) * sizeof(PaginaSlot)]); // devuelve un puntero
    }

    const PaginaSlot &Pagina::getSlot(const SlotId num_slot) const // devuelve un slot por referencia
    {
        return *reinterpret_cast<const PaginaSlot *>(&data_[(num_slot - 1) * sizeof(PaginaSlot)]); // indireccion del slot
    }

    SlotId Pagina::getAvailableSlot() // devuelve el id de un slot para guardar un registro
    {
        SlotId num_slot = INVALID_SLOT;
        if (header_.num_slots_libres > 0) // si hay slots libres
        {
            // Tiene un slot asignado pero no usado que podemos reutilizar
            for (SlotId i = 1; i <= header_.num_slots; ++i) // busca un slot no usado
            {
                const PaginaSlot *slot = getSlot(i);
                if (!slot->used) // si el slot no es usado
                {
                    // No decrementa el numero libre de slots hasta que alguien coloque data en el slot
                    num_slot = i; // devuelve el numero del slot no usado
                    break;
                }
            }
        }
        else
        {
            num_slot = header_.num_slots + 1; // Tiene que asignar un nuevo slot
            ++header_.num_slots;                 // aumenta la cant de slots
            ++header_.num_slots_libres;            // aumenta la cantidad de slots libres
            header_.free_space_lower_bound = sizeof(PaginaSlot) * header_.num_slots;
        }
        assert(num_slot != INVALID_SLOT);
        /*Casteo a "SlotId" en tiempo de compilacion*/
        return static_cast<SlotId>(num_slot); // devuelve el numero de slot
    }

    void Pagina::insertRecordInSlot(const SlotId num_slot, const std::string &record_data) // inserta registro
    {
        if (num_slot > header_.num_slots || num_slot == INVALID_SLOT)
        {
            std::cerr << "Intento de acceder a un slot el cual no esta actualmente en uso."
                      << " Pagina: " << num_pagina() << " Slot: " << num_slot;
            return;
        }
        PaginaSlot *slot = getSlot(num_slot); // Se obtiene puntero al slot
        if (slot->used)                        // si esta en uso
        {
            std::cerr << "Intento de insertar datos a un slot que esta actualmente en uso."
                      << " Pagina: " << num_pagina() << " Slot: " << num_slot;
            return;
        }
        const int record_length = record_data.length(); // guarda tamaño del string
        /*Actualiza el slot como "EN USO" y valores correspondientes*/
        slot->used = true;
        slot->item_length = record_length;
        slot->item_offset = header_.free_space_upper_bound - record_length; // donde comienza el registro

        header_.free_space_upper_bound = slot->item_offset; // Se actualiza el espacio LIBRE

        --header_.num_slots_libres; // Reduce el numero de slots libres

        data_.replace(slot->item_offset, slot->item_length, record_data); // Se inserta el registro en el string, siguiendo el desplazamiento y tamaño correspondientes
    }


    void Pagina::validateRecordId(const RecordId &record_id) const // verifica que un registro este en la pagina y que su slot no este en uso
    {
        if (record_id.num_pagina != num_pagina()) // si no es el mismo numero de pagina
        {
            std::cerr << "Requerimiento dirigido a registro invalido."
                      << " Registro {pagina=" << record_id.num_pagina
                      << ", slot=" << record_id.num_slot
                      << "} de pagina " << num_pagina();
            return;
        }
        const PaginaSlot &slot = getSlot(record_id.num_slot);
        if (!slot.used) // si no esta en uso
        {
            std::cerr << "Requerimiento dirigido a registro invalido."
                      << " Registro {pagina=" << record_id.num_pagina
                      << ", slot=" << record_id.num_slot
                      << "} de pagina " << num_pagina();
            return;
        }
    }

    PaginaIterator Pagina::begin()
    {
        return PaginaIterator(this);
    }

    PaginaIterator Pagina::end()
    {
        const RecordId &end_record_id = {num_pagina(), Pagina::INVALID_SLOT};
        return PaginaIterator(this, end_record_id);
    }
}
