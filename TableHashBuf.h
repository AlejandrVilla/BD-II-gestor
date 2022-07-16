#pragma once

#include "pages/File.h"
#include <memory>
#include <iostream>
#include "buffer.h"

namespace manager {
struct hashetbuck
{
    File *file;
    PageId pageNo;
    FrameId frameNo;
    hashetbuck *next;
};

class TableHashBuf
{
private:
    int HTSIZE;

    hashetbuck **hashbuck;

    int hash(const File *file, const PageId pageNo);

public:
    TableHashBuf(const int htSize); // constructor

    ~TableHashBuf(); // destructor

    void insert(const File *file, const PageId pageNo, const FrameId frameNo);

    void lookup(const File *file, const PageId pageNo, FrameId &frameNo);

    void remove(const File *file, const PageId pageNo);
};

int TableHashBuf::hash(const File *file, const PageId pageNo)
{
    int tmp, value;
    tmp = (long long)file; // conversión del puntero al objeto archivo en un entero
    value = (tmp + pageNo) % HTSIZE;
    return value;
}

TableHashBuf::TableHashBuf(int htSize)
    : HTSIZE(htSize)
{
    // asignar un arreglo de punteros a los hashetbucks
    hashbuck = new hashetbuck *[htSize];
    for (int i = 0; i < HTSIZE; i++)
        hashbuck[i] = NULL;
}

TableHashBuf::~TableHashBuf()
{
    for (int i = 0; i < HTSIZE; i++)
    {
        hashetbuck *tmpBuf = hashbuck[i];
        while (hashbuck[i])
        {
            tmpBuf = hashbuck[i];
            hashbuck[i] = hashbuck[i]->next;
            delete tmpBuf;
        }
    }
    delete[] hashbuck;
}

void TableHashBuf::insert(const File *file, const PageId pageNo, const FrameId frameNo)
{
    int index = hash(file, pageNo);

    hashetbuck *auxBuc = hashbuck[index];
    while (auxBuc)
    {
        if (auxBuc->file == file && auxBuc->pageNo == pageNo)
        {
            std::cerr<<"Entrada correspondiente al valor hash del archivo:" << auxBuc->file->filename() << "page:" << auxBuc->pageNo << "ya está presente en la tabla hash.";           
            return;
        }
        auxBuc = auxBuc->next;
    }

    auxBuc = new hashetbuck;
    if (!auxBuc)
    {
        std::cerr<< "Se ha producido un error en la tabla hash del buffer.";
        return;
    }

    auxBuc->file = (File *)file;
    auxBuc->pageNo = pageNo;
    auxBuc->frameNo = frameNo;
    auxBuc->next = hashbuck[index];
    hashbuck[index] = auxBuc;
}

void TableHashBuf::lookup(const File *file, const PageId pageNo, FrameId &frameNo)
{
    int index = hash(file, pageNo);
    hashetbuck *auxBuc = hashbuck[index];
    while (auxBuc)
    {
        if (auxBuc->file == file && auxBuc->pageNo == pageNo)
        {
            frameNo = auxBuc->frameNo; // retorna frameNo por referencia
            return;
        }
        auxBuc = auxBuc->next;
    }
    std::cerr<< "El valor hash no está presente en la tabla hash del archivo: " << file->filename() << "page: " << pageNo;
}

void TableHashBuf::remove(const File *file, const PageId pageNo)
{

    int index = hash(file, pageNo);
    hashetbuck *auxBuc = hashbuck[index];
    hashetbuck *prevBuc = NULL;

    while (auxBuc)
    {
        if (auxBuc->file == file && auxBuc->pageNo == pageNo)
        {
            if (prevBuc)
                prevBuc->next = auxBuc->next;
            else
                hashbuck[index] = auxBuc->next;

            delete auxBuc;
            return;
        }
        else
        {
            prevBuc = auxBuc;
            auxBuc = auxBuc->next;
        }
    }
    std::cerr<< "El valor hash no está presente en la tabla hash del archivo: " << file->filename() << "page: " << pageNo;
}
}