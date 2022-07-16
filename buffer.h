#pragma once

#include "pages/File.h"
#include "TableHashBuf.h"
namespace DBMS
{
    class BufManager;

    class BufFrame
    {

        friend class BufManager;

    private:
        File *file;

        PaginaId pageNo;

        FrameId frameNo;

        int num_pin;

        bool dirty;

        bool valid;

        bool refbit;

        void Clear()
        {
            num_pin = 0;
            file = NULL;
            pageNo = Pagina::INVALID_NUMBER;
            dirty = false;
            refbit = false;
            valid = false;
        };

        void Set(File *filePtr, PaginaId pageNum)
        {
            file = filePtr;
            pageNo = pageNum;
            num_pin = 1;
            dirty = false;
            valid = true;
            refbit = true;
        }

        void Print()
        {
            if (file)
            {
                std::cout << "file:" << file->filename() << " ";
                std::cout << "pageNo:" << pageNo << " ";
            }
            else
                std::cout << "file:NULL ";

            std::cout << "valid:" << valid << " ";
            std::cout << "num_pin:" << num_pin << " ";
            std::cout << "dirty:" << dirty << " ";
            std::cout << "refbit:" << refbit << "\n";
        }

        BufFrame()
        {
            Clear();
        }
    };

    struct BufMethodAccess
    {

        int accesses;
        int diskreads;
        int diskwrites;

        void clear()
        {
            accesses = diskreads = diskwrites = 0;
        }

        BufMethodAccess()
        {
            clear();
        }
    };

    class BufManager
    {
    private:
        FrameId clockHand;

        std::uint32_t numBufs;

        TableHashBuf *hashTable;

        BufFrame *BufFrameTable;

        BufMethodAccess bufMethodAccess;

        void advanceClock();

        void allocBuf(FrameId &frame);

    public:
        Pagina *bufPool;

        BufManager(std::uint32_t bufs);

        ~BufManager();

        void readPage(File *file, const PaginaId PageNo, Pagina *&page);

        void unPinPage(File *file, const PaginaId PageNo, const bool dirty);

        void allocPage(File *file, PaginaId &PageNo, Pagina *&page);

        void flushFile(const File *file);

        void disposePage(File *file, const PaginaId PageNo);

        void printSelf();

        BufMethodAccess &getBufMethodAccess()
        {
            return bufMethodAccess;
        }
        void clearBufMethodAccess()
        {
            bufMethodAccess.clear();
        }
    };

    BufManager::BufManager(std::uint32_t bufs)
        : numBufs(bufs)
    {
        BufFrameTable = new BufFrame[bufs];

        for (FrameId i = 0; i < bufs; i++)
        {
            BufFrameTable[i].frameNo = i;
            BufFrameTable[i].valid = false;
        }

        bufPool = new Pagina[bufs];

        int htsize = ((((int)(bufs * 1.2)) * 2) / 2) + 1;
        hashTable = new TableHashBuf(htsize); // asignar la tabla hash del buffer

        clockHand = bufs - 1;
    }

    BufManager::~BufManager()
    {

        for (uint32_t i = 0; i < numBufs; i++)
        {
            BufFrame buf = BufFrameTable[i];
            if (buf.dirty)
            {
                flushFile(buf.file);
            }
        }

        delete[] bufPool;
        delete[] BufFrameTable;
        delete hashTable;
    }

    // Avanzar el reloj a la siguiente trama del buffer pool
    void BufManager::advanceClock()
    {
        clockHand++;
        clockHand = clockHand % numBufs;
    }

    // Allocates a free frame using the clock algorithm; if necessary, writing a dirty page back
    // to disk. Throws BufferExceededException if all buffer frames are pinned.
    void BufManager::allocBuf(FrameId &frame)
    {
        // Comprobar si se ha encontrado la página
        bool frameNF = true;
        // Almacenar el frame que se está comprobando
        BufFrame buf;
        // Contador de bucle. Asegura que el bucle while no llegue al infinito
        uint32_t count = 0;

        while (frameNF && count <= numBufs)
        {
            // Incrementa el reloj
            advanceClock();
            // Incrementa el contador
            count++;
            // Obtenga el frame actual que se va a probar
            buf = BufFrameTable[clockHand];

            if (buf.valid == false)
            {
                frameNF = false;
            }
            else if (buf.refbit == true)
            {
                // debemos cambiar directamente el valor del refbit
                BufFrameTable[clockHand].refbit = false;
            }
            else if (buf.num_pin == 0)
            {
                frameNF = false;
                hashTable->remove(buf.file, buf.pageNo);
                if (buf.dirty)
                {
                    BufFrameTable[clockHand].file->writePage(bufPool[clockHand]);
                    BufFrameTable[clockHand].dirty = false;
                }
                BufFrameTable[clockHand].Clear();
            }
        }

        if (frameNF)
        {
            std::cerr << "Se ha superado la capacidad del buffer.\n";
        }

        frame = buf.frameNo;
    }

    // Lee una página específica. Si la página está en el buffer, la leerá desde allí. En caso contrario
    // la página se cargará en el pool de buffers.
    void BufManager::readPage(File *file, const PaginaId pageNo, Pagina *&page)
    {
        FrameId fno;
        try
        {
            // buscar la página en la tabla de hash
            hashTable->lookup(file, pageNo, fno);
            // si se encuentra, incrementa la cuenta de pines, pone el refbit a 0 y devuelve la página
            BufFrameTable[fno].num_pin++;
            BufFrameTable[fno].refbit = true;
            page = &bufPool[fno];
        }
        catch (std::tuple<std::string, PaginaId> a)
        {
            // Si la página no está en la tabla de hash, significa que hubo una pérdida de búfer, por lo que necesitamos leer del disco y asignar un marco de búfer para colocar la página
            allocBuf(fno);
            bufPool[fno] = file->readPage(pageNo);
            BufFrameTable[fno].Set(file, pageNo);
            page = &bufPool[fno];
            hashTable->insert(file, pageNo, fno);
        }
    }

    // Disminuye el num_pin de la frame que contiene (file, PageNo) y, si dirty == true, pone
    // el dirty bit. Lanza PAGENOTPINNED si la cuenta de pines ya es 0. No hace nada si
    // la página no se encuentra en la búsqueda de la tabla hash.
    void BufManager::unPinPage(File *file, const PaginaId pageNo, const bool dirty)
    {
        FrameId fno;
        // buscar hashtable
        hashTable->lookup(file, pageNo, fno);
        // si esta página ya está desanclada, lanza una excepción
        if (BufFrameTable[fno].num_pin == 0)
        {
            std::cerr << "Esta página no está anclada. file:  "
                      << "pageNotPinned"
                      << "page: " << BufFrameTable[fno].pageNo << "frame: " << fno;
            return;
        }
        // throw PageNotPinnedException("pageNotPinned", BufFrameTable[fno].pageNo, fno);
        else
            BufFrameTable[fno].num_pin--; // Disminuir la cantidad de pines
        if (dirty)
            BufFrameTable[fno].dirty = true; // Establece dirty
    }

    void BufManager::flushFile(const File *file)
    {
        for (uint32_t i = 0; i < numBufs; i++)
        {
            if (BufFrameTable[i].file == file)
            {
                if (!BufFrameTable[i].valid)
                {
                    // Si el frame está no es valido, lanza BadBufferException

                    std::cerr << "Este buffer está mal: " << i;
                    return;
                    // throw BadBufferException(i, BufFrameTable[i].dirty, BufFrameTable[i].valid, BufFrameTable[i].refbit);
                }
                if (BufFrameTable[i].num_pin != 0)
                {
                    // Si el frame está fijado, lanza PagePinnedException
                    std::cerr << "Esta pagina ya está anclada. file:  "
                              << "pagePinned"
                              << "page: " << BufFrameTable[i].pageNo << "frame: " << i;
                    return;
                    // throw PagePinnedException("pagePinned", BufFrameTable[i].pageNo, i);
                }
                if (BufFrameTable[i].dirty)
                {
                    // Si la trama está corrupta, la escribe en el disco, y luego pone el valor de dirty en false
                    BufFrameTable[i].file->writePage(bufPool[i]);
                    BufFrameTable[i].dirty = false;
                }
                // Eliminar la página de la tabla de hash
                hashTable->remove(file, BufFrameTable[i].pageNo);
                // Limpiar buffer frame
                BufFrameTable[i].Clear();
            }
        }
    }

    // Este método devolverá una página recién asignada.
    void BufManager::allocPage(File *file, PaginaId &pageNo, Pagina *&page)
    {
        FrameId frameNum;

        Pagina p1 = file->allocatePage();
        allocBuf(frameNum);
        bufPool[frameNum] = p1;
        hashTable->insert(file, p1.num_pagina(), frameNum);
        BufFrameTable[frameNum].Set(file, p1.num_pagina());
        pageNo = p1.num_pagina();
        page = &bufPool[frameNum];
    }

    // Este método elimina una página concreta del archivo.
    void BufManager::disposePage(File *file, const PaginaId PageNo)
    {
        try
        {
            FrameId frameNum;
            hashTable->lookup(file, PageNo, frameNum);
            BufFrameTable[frameNum].Clear();
            hashTable->remove(file, PageNo);
            file->deletePage(PageNo);
        }
        catch (std::tuple<std::string, PaginaId> a)
        {
            // Hash not found
        }
    }

    void BufManager::printSelf(void)
    {
        BufFrame *tmpbuf;
        int validFrames = 0;

        for (std::uint32_t i = 0; i < numBufs; i++)
        {
            tmpbuf = &(BufFrameTable[i]);
            std::cout << "FrameNo:" << i << " ";
            tmpbuf->Print();

            if (tmpbuf->valid == true)
                validFrames++;
        }

        std::cout << "Número total de Frames Válidos:" << validFrames << "\n";
    }
}