#pragma once

#include "pages/File.h"
#include <memory>
#include <iostream>
#include "buffer.h"
#include "bufHashTbl.h"

namespace siprec{
struct hashBucket
{
    File *file;

    PageId pageNo;

    FrameId frameNo;

    hashBucket *next;
};

class BufHashTbl
{
private:
    int HTSIZE;

    hashBucket **ht;

    int hash(const File *file, const PageId pageNo);

public:
    BufHashTbl(const int htSize); // constructor

    ~BufHashTbl(); // destructor

    void insert(const File *file, const PageId pageNo, const FrameId frameNo);

    void lookup(const File *file, const PageId pageNo, FrameId &frameNo);

    void remove(const File *file, const PageId pageNo);
};

int BufHashTbl::hash(const File *file, const PageId pageNo)
{
    int tmp, value;
    tmp = (long long)file; // cast of pointer to the file object to an integer
    value = (tmp + pageNo) % HTSIZE;
    return value;
}

BufHashTbl::BufHashTbl(int htSize)
    : HTSIZE(htSize)
{
    // allocate an array of pointers to hashBuckets
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

void BufHashTbl::insert(const File *file, const PageId pageNo, const FrameId frameNo)
{
    int index = hash(file, pageNo);

    hashBucket *tmpBuc = ht[index];
    while (tmpBuc)
    {
        if (tmpBuc->file == file && tmpBuc->pageNo == pageNo)
        {
            std::cerr<<"Entry corresponding to the hash value of file:" << tmpBuc->file->filename() << "page:" << tmpBuc->pageNo << "is already present in the hash table.";           
            return;
        }
        tmpBuc = tmpBuc->next;
    }

    tmpBuc = new hashBucket;
    if (!tmpBuc)
    {
        std::cerr<< "Error occurred in buffer hash table.";
        return;
    }

    tmpBuc->file = (File *)file;
    tmpBuc->pageNo = pageNo;
    tmpBuc->frameNo = frameNo;
    tmpBuc->next = ht[index];
    ht[index] = tmpBuc;
}

void BufHashTbl::lookup(const File *file, const PageId pageNo, FrameId &frameNo)
{
    int index = hash(file, pageNo);
    hashBucket *tmpBuc = ht[index];
    while (tmpBuc)
    {
        if (tmpBuc->file == file && tmpBuc->pageNo == pageNo)
        {
            frameNo = tmpBuc->frameNo; // return frameNo by reference
            return;
        }
        tmpBuc = tmpBuc->next;
    }
    std::cerr<< "The hash value is not present in the hash table for file: " << file->filename() << "page: " << pageNo;
}

void BufHashTbl::remove(const File *file, const PageId pageNo)
{

    int index = hash(file, pageNo);
    hashBucket *tmpBuc = ht[index];
    hashBucket *prevBuc = NULL;

    while (tmpBuc)
    {
        if (tmpBuc->file == file && tmpBuc->pageNo == pageNo)
        {
            if (prevBuc)
                prevBuc->next = tmpBuc->next;
            else
                ht[index] = tmpBuc->next;

            delete tmpBuc;
            return;
        }
        else
        {
            prevBuc = tmpBuc;
            tmpBuc = tmpBuc->next;
        }
    }
    std::cerr<< "The hash value is not present in the hash table for file: " << file->filename() << "page: " << pageNo;
}
}