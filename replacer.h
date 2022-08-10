#pragma  once
#include <cstdint>
#include <cstddef>
#include <iostream>

class Replacer
{
    public:
        Replacer() = default;
        virtual ~Replacer() = default;
        virtual bool Victim(int32_t *frame_id) = 0;
        virtual void Pin(int32_t frame_id) = 0;
        virtual void Unpin(int32_t frame_id) = 0;
        virtual size_t Size() = 0;
};