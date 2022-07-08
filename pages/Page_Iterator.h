
#pragma once
#ifndef _PAGE_IT_H_
#define _PAGE_IT_H_

#include <cassert>
//#include "file.h"
#include "page.h"
#include "../types.h"

class Page;

class PageIterator
{

private:
  Page *page_;

  RecordId current_record_;

public:
  PageIterator() : page_(NULL)
  {
    current_record_ = {Page::INVALID_NUMBER, Page::INVALID_SLOT};
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

  inline PageIterator &operator++() // ++it
  {
    assert(page_ != NULL);                                                 // que no apunte a NULL
    const SlotId used_slot = getNextUsedSlot(current_record_.slot_number); // obtiene el sig slot usado
    current_record_ = {page_->page_number(), used_slot};                   // siguiente registro

    return *this; // devuelve iterator
  }

  inline PageIterator operator++(int) // it++
  {
    PageIterator tmp = *this; // copy ourselves

    assert(page_ != NULL);                                                 // que no apunte a NULL
    const SlotId used_slot = getNextUsedSlot(current_record_.slot_number); // obtiene el sig slot usado
    current_record_ = {page_->page_number(), used_slot};                   // siguiente registro

    return tmp; // devuelve copia
  }

  inline bool operator==(const PageIterator &rhs) const // compara si es el mismo numero de pagina y este en el mismo registro
  {
    return page_->page_number() == rhs.page_->page_number() && current_record_ == rhs.current_record_;
  }

  inline bool operator!=(const PageIterator &rhs) const // compara si no es el mismo numero de pagina o no este en el mismo registro
  {
    return (page_->page_number() != rhs.page_->page_number()) ||
           (current_record_ != rhs.current_record_);
  }

  inline std::string operator*() const // indireccion
  {
    return page_->getRecord(current_record_); // devuelve el registro
  }

  SlotId getNextUsedSlot(const SlotId start) const // obtiene el sig slot
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
};

PageIterator Page::begin()
{
  return PageIterator(this);
}

PageIterator Page::end()
{
  const RecordId &end_record_id = {page_number(), Page::INVALID_SLOT};
  return PageIterator(this, end_record_id);
}

#endif
