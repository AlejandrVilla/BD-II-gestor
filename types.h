#pragma once
#include <cstdint>

// uint32_t -> unsigned int 32 bits
// uint16_t -> unsigned short 16 bits
// size_t -> unsigned long long 64 bits

typedef std::uint32_t PaginaId;       
typedef std::uint16_t SlotId;
typedef std::uint32_t FrameId;
struct RecordId {
    PaginaId num_pagina; // a que pag pertenece
    SlotId num_slot; // a que slot pertenece
    bool operator==(const RecordId& rhs) const {
        return num_pagina == rhs.num_pagina && num_slot == rhs.num_slot;
    }
    bool operator!=(const RecordId& rhs) const {
        return (num_pagina != rhs.num_pagina) || (num_slot != rhs.num_slot);
    }
};
