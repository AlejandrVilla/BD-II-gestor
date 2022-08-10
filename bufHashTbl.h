#pragma once
#include "file.h"



struct hashBucket       // celda de la tabla hash
{
    File *file;
    int32_t pageNo;
    int32_t frameNo;
    hashBucket *next;
};

class BufHashTbl
{
private:
    int HTSIZE;
    hashBucket **ht;
    int hash(const File *file, const int32_t pageNo);

public:
    BufHashTbl(const int htSize); // constructor
    ~BufHashTbl(); // destructor
    void insert(const File *file, const int32_t pageNo, const int32_t frameNo);
    void lookup(const File *file, const int32_t pageNo, int32_t &frameNo);
    void remove(const File *file, const int32_t pageNo);
};

int BufHashTbl::hash(const File *file, const int32_t pageNo)
{
    int tmp, value;
    tmp = (long long)file; // conversion del puntero a file, a puntero long long
    value = (tmp + pageNo) % HTSIZE;
    return value;
}

BufHashTbl::BufHashTbl(int htSize)
    : HTSIZE(htSize)
{   
    // Se asigna un arreglo de punteros a hashBuckets
    ht = new hashBucket *[htSize];
    for (int i = 0; i < HTSIZE; i++)
        ht[i] = NULL;
}

BufHashTbl::~BufHashTbl()
{
    for (int i = 0; i < HTSIZE; i++)
    {
        hashBucket *tmpBuf = ht[i];
        while (ht[i])
        {
            tmpBuf = ht[i];
            ht[i] = ht[i]->next;
            delete tmpBuf;
        }
    }
    delete[] ht;
}

void BufHashTbl::insert(const File *file, const int32_t pageNo, const int32_t frameNo)
{
    int index = hash(file, pageNo);

    hashBucket *tmpBuc = ht[index];
    while (tmpBuc)
    {
        if (tmpBuc->file == file && tmpBuc->pageNo == pageNo)
        {
            std::cerr<<"La entrada que corresponde al valor hash del file ya está presente en la tabla hash\n";           
            return;
        }
        tmpBuc = tmpBuc->next;
    }

    tmpBuc = new hashBucket;
    if (!tmpBuc)
    {
        std::cerr<< "Ocurrió un error en la tabla hash.\n";
        return;
    }

    tmpBuc->file = (File *)file;
    tmpBuc->pageNo = pageNo;
    tmpBuc->frameNo = frameNo;
    tmpBuc->next = ht[index];       // desplaza celda
    ht[index] = tmpBuc;             // inserta celda
}

void BufHashTbl::lookup(const File *file, const int32_t pageNo, int32_t &frameNo)
{
    int index = hash(file, pageNo);
    hashBucket *tmpBuc = ht[index];
    while (tmpBuc)
    {
        if (tmpBuc->file == file && tmpBuc->pageNo == pageNo)       // busca a traves de la lista en una celda
        {
            frameNo = tmpBuc->frameNo; // retorna frameNo por referencia
            return;
        }
        tmpBuc = tmpBuc->next;
    }
    std::cerr<< "El valor no está presente en la tabla hash, file\n";
}

void BufHashTbl::remove(const File *file, const int32_t pageNo)
{

    int index = hash(file, pageNo);
    hashBucket *tmpBuc = ht[index];
    hashBucket *prevBuc = NULL;

    while (tmpBuc)
    {
        if (tmpBuc->file == file && tmpBuc->pageNo == pageNo)   // encuentra
        {
            if (prevBuc)                            // si prev tiene algo
                prevBuc->next = tmpBuc->next;       // prev->nex guarda el siguiente  tmpBuc
            else
                ht[index] = tmpBuc->next;           // primer elemento es el siguiente a tmpBuc

            delete tmpBuc;  // elimina tmpBuc
            return;
        }
        else
        {
            prevBuc = tmpBuc;       // guarda actual
            tmpBuc = tmpBuc->next;  // avanza
        }
    }
    std::cerr<< "El valor hash(key) no esta presente el la tabla hash, file\n";