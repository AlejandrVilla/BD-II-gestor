#pragma once

#include "pages/File.h"
#include "bufHashTbl.h"
namespace siprec{
class BufMgr;

class BufDesc
{

    friend class BufMgr;

private:
    File *file;

    PageId pageNo;

    FrameId frameNo;

    int pinCnt;

    bool dirty;

    bool valid;

    bool refbit;

    void Clear()
    {
        pinCnt = 0;
        file = NULL;
        pageNo = Page::INVALID_NUMBER;
        dirty = false;
        refbit = false;
        valid = false;
    };

    void Set(File *filePtr, PageId pageNum)
    {
        file = filePtr;
        pageNo = pageNum;
        pinCnt = 1;
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
        std::cout << "pinCnt:" << pinCnt << " ";
        std::cout << "dirty:" << dirty << " ";
        std::cout << "refbit:" << refbit << "\n";
    }

    BufDesc()
    {
        Clear();
    }
};

struct BufStats
{

    int accesses;

    int diskreads;

    int diskwrites;

    void clear()
    {
        accesses = diskreads = diskwrites = 0;
    }

    BufStats()
    {
        clear();
    }
};

class BufMgr
{
private:
    FrameId clockHand;

    std::uint32_t numBufs;

    BufHashTbl *hashTable;

    BufDesc *bufDescTable;

    BufStats bufStats;

    void advanceClock();

    void allocBuf(FrameId &frame);

public:
    Page *bufPool;

    BufMgr(std::uint32_t bufs);

    ~BufMgr();

    void readPage(File *file, const PageId PageNo, Page *&page);

    void unPinPage(File *file, const PageId PageNo, const bool dirty);

    void allocPage(File *file, PageId &PageNo, Page *&page);

    void flushFile(const File *file);

    void disposePage(File *file, const PageId PageNo);

    void printSelf();

    BufStats &getBufStats()
    {
        return bufStats;
    }
    void clearBufStats()
    {
        bufStats.clear();
    }
};

BufMgr::BufMgr(std::uint32_t bufs)
    : numBufs(bufs)
{
    bufDescTable = new BufDesc[bufs];

    for (FrameId i = 0; i < bufs; i++)
    {
        bufDescTable[i].frameNo = i;
        bufDescTable[i].valid = false;
    }

    bufPool = new Page[bufs];

    int htsize = ((((int)(bufs * 1.2)) * 2) / 2) + 1;
    hashTable = new BufHashTbl(htsize); // asignar la tabla hash del buffer

    clockHand = bufs - 1;
}

BufMgr::~BufMgr()
{

    for (uint32_t i = 0; i < numBufs; i++)
    {
        BufDesc buf = bufDescTable[i];
        if (buf.dirty)
        {
            flushFile(buf.file);
        }
    }

    delete[] bufPool;
    delete[] bufDescTable;
    delete hashTable;
}

// Avanzar el reloj a la siguiente trama del buffer pool
void BufMgr::advanceClock()
{
    clockHand++;
    clockHand = clockHand % numBufs;
}

// Allocates a free frame using the clock algorithm; if necessary, writing a dirty page back
// to disk. Throws BufferExceededException if all buffer frames are pinned.
void BufMgr::allocBuf(FrameId &frame)
{
    // Comprobar si se ha encontrado la página
    bool frameNF = true;
    // Almacenar el frame que se está comprobando
    BufDesc buf;
    // Contador de bucle. Asegura que el bucle while no llegue al infinito
    uint32_t count = 0;

    while (frameNF && count <= numBufs)
    {
        // Incrementa el reloj
        advanceClock();
        // Incrementa el contador
        count++;
        // Obtenga el frame actual que se va a probar
        buf = bufDescTable[clockHand];

        if (buf.valid == false)
        {
            frameNF = false;
        }
        else if (buf.refbit == true)
        {
            // debemos cambiar directamente el valor del refbit
            bufDescTable[clockHand].refbit = false;
        }
        else if (buf.pinCnt == 0)
        {
            frameNF = false;
            hashTable->remove(buf.file, buf.pageNo);
            if (buf.dirty)
            {
                bufDescTable[clockHand].file->writePage(bufPool[clockHand]);
                bufDescTable[clockHand].dirty = false;
            }
            bufDescTable[clockHand].Clear();
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
void BufMgr::readPage(File *file, const PageId pageNo, Page *&page)
{
    FrameId fno;
    try
    {
        // buscar la página en la tabla de hash
        hashTable->lookup(file, pageNo, fno);
        // si se encuentra, incrementa la cuenta de pines, pone el refbit a 0 y devuelve la página
        bufDescTable[fno].pinCnt++;
        bufDescTable[fno].refbit = true;
        page = &bufPool[fno];
    }
    catch (std::tuple<std::string,PageId> a)
    {
        // Si la página no está en la tabla de hash, significa que hubo una pérdida de búfer, por lo que necesitamos leer del disco y asignar un marco de búfer para colocar la página
        allocBuf(fno);
        bufPool[fno] = file->readPage(pageNo);
        bufDescTable[fno].Set(file, pageNo);
        page = &bufPool[fno];
        hashTable->insert(file, pageNo, fno);
    }
}

// Disminuye el pinCnt de la frame que contiene (file, PageNo) y, si dirty == true, pone
// el dirty bit. Lanza PAGENOTPINNED si la cuenta de pines ya es 0. No hace nada si
// la página no se encuentra en la búsqueda de la tabla hash.
void BufMgr::unPinPage(File *file, const PageId pageNo, const bool dirty)
{
    FrameId fno;
    // buscar hashtable
    hashTable->lookup(file, pageNo, fno);
    // si esta página ya está desanclada, lanza una excepción
    if (bufDescTable[fno].pinCnt == 0)
    {
        std::cerr << "Esta página no está anclada. file:  " << "pageNotPinned" << "page: " << bufDescTable[fno].pageNo << "frame: " << fno;
        return;
    }
        //throw PageNotPinnedException("pageNotPinned", bufDescTable[fno].pageNo, fno);
    else
        bufDescTable[fno].pinCnt--; // Disminuir la cantidad de pines
    if (dirty)
        bufDescTable[fno].dirty = true; // Establece dirty
}

void BufMgr::flushFile(const File *file)
{
    for (uint32_t i = 0; i < numBufs; i++)
    {
        if (bufDescTable[i].file == file)
        {
            if (!bufDescTable[i].valid)
            {
                // Si el frame está no es valido, lanza BadBufferException

                std::cerr << "Este buffer está mal: " << i;
                return;
                //throw BadBufferException(i, bufDescTable[i].dirty, bufDescTable[i].valid, bufDescTable[i].refbit);
            }
            if (bufDescTable[i].pinCnt != 0)
            {
                // Si el frame está fijado, lanza PagePinnedException
                std::cerr << "Esta pagina ya está anclada. file:  " << "pagePinned" << "page: " << bufDescTable[i].pageNo << "frame: " << i;
                return;
                //throw PagePinnedException("pagePinned", bufDescTable[i].pageNo, i);
            }
            if (bufDescTable[i].dirty)
            {
                // Si la trama está corrupta, la escribe en el disco, y luego pone el valor de dirty en false
                bufDescTable[i].file->writePage(bufPool[i]);
                bufDescTable[i].dirty = false;
            }
            // Eliminar la página de la tabla de hash
            hashTable->remove(file, bufDescTable[i].pageNo);
            // Limpiar buffer frame
            bufDescTable[i].Clear();
        }
    }
}

// Este método devolverá una página recién asignada.
void BufMgr::allocPage(File *file, PageId &pageNo, Page *&page)
{
    FrameId frameNum;

    Page p1 = file->allocatePage();
    allocBuf(frameNum);
    bufPool[frameNum] = p1;
    hashTable->insert(file, p1.page_number(), frameNum);
    bufDescTable[frameNum].Set(file, p1.page_number());
    pageNo = p1.page_number();
    page = &bufPool[frameNum];
}

// Este método elimina una página concreta del archivo.
void BufMgr::disposePage(File *file, const PageId PageNo)
{
    try
    {
        FrameId frameNum;
        hashTable->lookup(file, PageNo, frameNum);
        bufDescTable[frameNum].Clear();
        hashTable->remove(file, PageNo);
        file->deletePage(PageNo);
    }
    catch (std::tuple<std::string,PageId> a)
    {
        // Hash not found
    }
}

void BufMgr::printSelf(void)
{
    BufDesc *tmpbuf;
    int validFrames = 0;

    for (std::uint32_t i = 0; i < numBufs; i++)
    {
        tmpbuf = &(bufDescTable[i]);
        std::cout << "FrameNo:" << i << " ";
        tmpbuf->Print();

        if (tmpbuf->valid == true)
            validFrames++;
    }

    std::cout << "Número total de Frames Válidos:" << validFrames << "\n";
}
}