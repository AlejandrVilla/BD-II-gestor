#pragma once
#include <cstdint>

// uint32_t -> unsigned int 32 bits
// uint16_t -> unsigned short 16 bits
// size_t -> unsigned long long 64 bits

typedef std::uint32_t PageId;       
typedef std::uint16_t SlotId;
typedef std::uint32_t FrameId;
struct RecordId {
    PageId page_number; // a que pag pertenece
    SlotId slot_number; // a que slot pertenece
    bool operator==(const RecordId& rhs) const {
        return page_number == rhs.page_number && slot_number == rhs.slot_number;
    }
    bool operator!=(const RecordId& rhs) const {
        return (page_number != rhs.page_number) || (slot_number != rhs.slot_number);
    }
};
