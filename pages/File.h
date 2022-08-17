#pragma once
#include <fstream>
#include <string>
#include <map>
#include <memory>

#include "../types.h"

namespace DBMS
{
    class FileIterator;
    struct FileHeader
    {
        PaginaId cnt_pages;
        PaginaId used_first;
        PaginaId num_free_pages;
        PaginaId free_first;
        bool operator==(const FileHeader &rhs) const
        {
            return (
                cnt_pages == rhs.cnt_pages &&
                used_first == rhs.used_first &&
                free_first == rhs.free_first &&
                num_free_pages == rhs.num_free_pages);
        }
    };

    class File
    {
    private:
        typedef std::map<std::string, std::shared_ptr<std::fstream>> StreamMap;
        typedef std::map<std::string, int> CountMap;
        static StreamMap open_streams_;
        static CountMap open_counts_;
        std::string filename_;
        std::shared_ptr<std::fstream> stream_;
        friend class FileIterator;

        static std::streampos pagePosition(const PaginaId num_pagina)
        {
            return sizeof(FileHeader) + (num_pagina - 1) * Pagina::SIZE;
        }
        File(const std::string &name, const bool create_new);
        void openIfNeeded(const bool create_new);
        void close();
        Pagina readPage(const PaginaId num_pagina, const bool allow_free) const;
        void writePage(const PaginaId num_pagina, const Pagina &new_page);
        void writePage(const PaginaId num_pagina, const PaginaHeader &header, const Pagina &new_page);
        FileHeader readHeader() const;
        void writeHeader(const FileHeader &header);
        PaginaHeader readPaginaHeader(const PaginaId num_pagina) const;

    public:
        static File create(const std::string &filename_);
        static File open(const std::string &filename_);
        static void remove(const std::string &filename_);
        static bool isOpen(const std::string &filename_);
        static bool exists(const std::string &filename_);
        File(const File &other);
        File &operator=(const File &rhs);
        ~File();
        Pagina allocatePage();
        Pagina readPage(const PaginaId num_pagina) const;
        void writePage(const Pagina &new_page);
        void deletePage(const PaginaId num_pagina);
        const std::string &filename() const { return filename_; }
        FileIterator begin();
        FileIterator end();

    };

    class FileIterator
    {
    private:
        File *file_;
        PaginaId num_pagina_actual_;

    public:
        FileIterator()
            : file_(NULL),
              num_pagina_actual_(Pagina::INVALID_NUMBER)
        {
        }

        FileIterator(const FileIterator &it)
        {
            file_ = it.file_;
            num_pagina_actual_ = it.num_pagina_actual_;
        }

        FileIterator(File *file)
            : file_(file)
        {
            assert(file_ != NULL);
            const FileHeader &header = file_->readHeader();
            num_pagina_actual_ = header.used_first;
        }

        FileIterator(File *file, PaginaId num_pagina)
            : file_(file),
              num_pagina_actual_(num_pagina)
        {
        }
        FileIterator &operator++();

        FileIterator operator++(int);

        bool operator==(const FileIterator &rhs) const;

        bool operator!=(const FileIterator &rhs) const;

        Pagina operator*() const;
    };

    FileIterator &FileIterator::operator++()
    {
        assert(file_ != NULL);
        const PaginaHeader &header = file_->readPaginaHeader(num_pagina_actual_);
        num_pagina_actual_ = header.sig_num_pagina;

        return *this;
    }

    FileIterator FileIterator::operator++(int)
    {
        FileIterator tmp = *this;

        assert(file_ != NULL);
        const PaginaHeader &header = file_->readPaginaHeader(num_pagina_actual_);
        num_pagina_actual_ = header.sig_num_pagina;

        return tmp;
    }

    bool FileIterator::operator==(const FileIterator &rhs) const
    {
        return file_->filename() == rhs.file_->filename() &&
               num_pagina_actual_ == rhs.num_pagina_actual_;
    }

    bool FileIterator::operator!=(const FileIterator &rhs) const
    {
        return (file_->filename() != rhs.file_->filename()) ||
               (num_pagina_actual_ != rhs.num_pagina_actual_);
    }

    Pagina FileIterator::operator*() const
    {
        return file_->readPage(num_pagina_actual_);
    }

    File::StreamMap File::open_streams_;
    File::CountMap File::open_counts_;

    File File::create(const std::string &filename)
    {
        return File(filename, true);
    }

    File File::open(const std::string &filename)
    {
        return File(filename, false);
    }

    void File::remove(const std::string &filename)
    {
        if (!exists(filename))
        {
            std::cerr << "File no encontrado: " << filename << "\n";
            return;
        }
        if (isOpen(filename))
        {
            std::cerr << "Actualmente el file se encuentra abierto: " << filename << "\n";
            return;
        }
        std::remove(filename.c_str());
    }

    bool File::isOpen(const std::string &filename)
    {
        if (!exists(filename))
        {
            return false;
        }
        return open_counts_.find(filename) != open_counts_.end();
    }

    bool File::exists(const std::string &filename)
    {
        std::fstream file(filename);
        if (file)
        {
            file.close();
            return true;
        }

        return false;
    }

    File::File(const File &other)
        : filename_(other.filename_),
          stream_(open_streams_[filename_])
    {
        ++open_counts_[filename_];
    }

    File &File::operator=(const File &rhs)
    {
        close();
        filename_ = rhs.filename_;
        openIfNeeded(false);
        return *this;
    }

    File::~File()
    {
        close();
    }

    Pagina File::allocatePage()
    {
        FileHeader header = readHeader();
        Pagina nueva_pagina;
        Pagina pagina_existente;
        if (header.num_free_pages > 0)
        {
            nueva_pagina = readPage(header.free_first, true );
            nueva_pagina.set_num_pagina(header.free_first);
            header.free_first = nueva_pagina.sig_num_pagina();
            --header.num_free_pages;

            if (header.used_first == Pagina::INVALID_NUMBER || header.used_first > nueva_pagina.num_pagina())
            {
                // O no tiene páginas usadas o el encabezado de la lista usada es una página posterior
                // que el que acabamos de asignar, así que se agrega la nueva página al encabezado.
                if (header.used_first > nueva_pagina.num_pagina())
                {
                    nueva_pagina.set_sig_num_pagina(header.used_first);
                }
                header.used_first = nueva_pagina.num_pagina();
            }
            else
            {
                 // La nueva página se reutiliza desde algún lugar después del principio, por lo que debemos
                // encontrar en qué parte de la lista de usados esta para ​​insertarlo.
                PaginaId sig_num_pagina = Pagina::INVALID_NUMBER;
                for (FileIterator iter = begin(); iter != end(); ++iter)
                {
                    sig_num_pagina = (*iter).sig_num_pagina();
                    if (sig_num_pagina > nueva_pagina.num_pagina() || sig_num_pagina == Pagina::INVALID_NUMBER)
                    {
                        pagina_existente = *iter;
                        break;
                    }
                }
                pagina_existente.set_sig_num_pagina(nueva_pagina.num_pagina());
                nueva_pagina.set_sig_num_pagina(sig_num_pagina);
            }

            assert((header.num_free_pages == 0) == (header.free_first == Pagina::INVALID_NUMBER));
        }
        else
        {
            nueva_pagina.set_num_pagina(header.cnt_pages);
            if (header.used_first == Pagina::INVALID_NUMBER)
            {
                header.used_first = nueva_pagina.num_pagina();
            }
            else
            {

                for (FileIterator iter = begin(); iter != end(); ++iter)
                {
                    if ((*iter).sig_num_pagina() == Pagina::INVALID_NUMBER)
                    {
                        pagina_existente = *iter;
                        break;
                    }
                }
                assert(pagina_existente.isUsed());
                pagina_existente.set_sig_num_pagina(nueva_pagina.num_pagina());
            }
            ++header.cnt_pages;
        }
        writePage(nueva_pagina.num_pagina(), nueva_pagina);
        if (pagina_existente.num_pagina() != Pagina::INVALID_NUMBER)
        {
            writePage(pagina_existente.num_pagina(), pagina_existente);
        }
        writeHeader(header);

        return nueva_pagina;
    }

    Pagina File::readPage(const PaginaId num_pagina) const
    {
        FileHeader header = readHeader();
        if (num_pagina >= header.cnt_pages)
        {
            std::cerr << "Peticion dirigida a pagina invalida.\n";
            std::cerr << "Pagina: " << num_pagina << "\n";
            std::cerr << "File: " << filename_ << "\n";
        }
        return readPage(num_pagina, false);
    }

    Pagina File::readPage(const PaginaId num_pagina, const bool allow_free) const
    {
        Pagina pagina;
        stream_->seekg(pagePosition(num_pagina), std::ios::beg);
        stream_->read(reinterpret_cast<char *>(&pagina.header_), sizeof(pagina.header_));
        stream_->read(reinterpret_cast<char *>(&pagina.data_[0]), Pagina::DATA_SIZE);
        if (!allow_free && !pagina.isUsed())
        {
            std::cerr<< "Peticion dirigida a pagina invalida"
            << " Pagina: " << num_pagina
            << " File:'" << filename_ << "'";
            return pagina;
        }
        return pagina;
    }

    void File::writePage(const Pagina &new_page)
    {
        PaginaHeader header = readPaginaHeader(new_page.num_pagina());
        if (header.num_pagina_actual== Pagina::INVALID_NUMBER)
        {
            std::cerr << "Peticion dirigida a pagina invalida.\n";
            std::cerr << "Pagina: " << new_page.num_pagina() << "\n";
            std::cerr << "File: " << filename_ << "\n";
        }
        const PaginaId sig_num_pagina = header.sig_num_pagina;
        header = new_page.header_;
        header.sig_num_pagina = sig_num_pagina;
        writePage(new_page.num_pagina(), header, new_page);
    }

    void File::deletePage(const PaginaId num_pagina)
    {
        FileHeader header = readHeader();
        Pagina pagina_existente = readPage(num_pagina);
        Pagina pagina_previa;
        // Si esta página es el encabezado de la lista usada, actualice el encabezado para que apunte a
        // la página siguiente en la línea.
        if (num_pagina == header.used_first)
        {
            header.used_first = pagina_existente.sig_num_pagina();
        }
        else
        {
            // Recorre la lista de usados ​​para que podamos actualizar la página que apunta a esta.
            for (FileIterator iter = begin(); iter != end(); ++iter)
            {
                pagina_previa = *iter;
                if (pagina_previa.sig_num_pagina() == pagina_existente.num_pagina())
                {
                    pagina_previa.set_sig_num_pagina(pagina_existente.sig_num_pagina());
                    break;
                }
            }
        }
        // Limpia la página y la agrega al encabezado de la lista libre.
        pagina_existente.initialize();
        pagina_existente.set_sig_num_pagina(header.free_first);
        header.free_first = num_pagina;
        ++header.num_free_pages;
        if (pagina_previa.isUsed())
        {
            writePage(pagina_previa.num_pagina(), pagina_previa);
        }
        writePage(num_pagina, pagina_existente);
        writeHeader(header);
    }

    FileIterator File::begin()
    {
        const FileHeader &header = readHeader();
        return FileIterator(this, header.used_first);
    }

    FileIterator File::end()
    {
        return FileIterator(this, Pagina::INVALID_NUMBER);
    }

    File::File(const std::string &name, const bool create_new) : filename_(name)
    {
        openIfNeeded(create_new);

        if (create_new)
        {
           // El archivo comienza con 1 página (el encabezado).
            FileHeader header = {1 /* cnt_pages */, 0 /* used_first */,
                                 0 /* num_free_pages */, 0 /* free_first */};
            writeHeader(header);
        }
    }

    void File::openIfNeeded(const bool create_new)
    {
        if (open_counts_.find(filename_) != open_counts_.end())
        { // ya existe una entrada
            ++open_counts_[filename_];
            stream_ = open_streams_[filename_];
        }
        else
        {
            std::ios_base::openmode mode =
                std::fstream::in | std::fstream::out | std::fstream::binary;
            const bool already_exists = exists(filename_);
            if (create_new)
            {
                // Error si intentamos sobrescribir un archivo existente.
                if (already_exists)
                {
                    std::cerr << "El file ya existe.\n";
                    std::cerr << "File: " << filename_ << "\n";
                }
                // Los archivos nuevos deben truncarse 
                mode = mode | std::fstream::trunc;
            }
            else
            {
               // Error si intentamos abrir un archivo que no existe.
                if (!already_exists)
                {
                    std::cerr << "El file no ha sido encontrado.\n";
                    std::cerr << "File: " << filename_ << "\n";
                }
            }
            stream_.reset(new std::fstream(filename_, mode));
            open_streams_[filename_] = stream_;
            open_counts_[filename_] = 1;
        }
    }

    void File::close()
    {
        --open_counts_[filename_];
        stream_.reset();
        if (open_counts_[filename_] == 0)
        {
            open_streams_.erase(filename_);
            open_counts_.erase(filename_);
        }
    }

    void File::writePage(const PaginaId num_pagina, const Pagina &new_page)
    {
        writePage(num_pagina, new_page.header_, new_page);
    }

    void File::writePage(const PaginaId num_pagina, const PaginaHeader &header,
                         const Pagina &new_page)
    {
        stream_->seekp(pagePosition(num_pagina), std::ios::beg);
        stream_->write(reinterpret_cast<const char *>(&header), sizeof(header));
        stream_->write(reinterpret_cast<const char *>(&new_page.data_[0]),
                       Pagina::DATA_SIZE);
        stream_->flush();
    }

    FileHeader File::readHeader() const
    {
        FileHeader header;
        stream_->seekg(0 /* pos */, std::ios::beg);
        stream_->read(reinterpret_cast<char *>(&header), sizeof(header));

        return header;
    }

    void File::writeHeader(const FileHeader &header)
    {
        stream_->seekp(0 /* pos */, std::ios::beg);
        stream_->write(reinterpret_cast<const char *>(&header), sizeof(header));
        stream_->flush();
    }

    PaginaHeader File::readPaginaHeader(PaginaId num_pagina) const
    {
        PaginaHeader header;
        stream_->seekg(pagePosition(num_pagina), std::ios::beg);
        stream_->read(reinterpret_cast<char *>(&header), sizeof(header));

        return header;
    }
}