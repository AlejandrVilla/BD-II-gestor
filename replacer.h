#pragma  once
#include <cstdint>
#include <cstddef>
#include <iostream>

class Replacer
{
    public:
        // Define los metodos virtuales
        Replacer() = default;
        virtual ~Replacer() = default;
        virtual bool Victim(int32_t *frame_id) = 0;     // Metodo de eleccion de victima
        virtual void Pin(int32_t frame_id) = 0;         // Pin frame 
        virtual void Unpin(int32_t frame_id) = 0;       // Unpin frame
        virtual size_t Size() = 0;                      // tamanio de buffer
};
