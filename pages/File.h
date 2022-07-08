#pragma once
#include <fstream>
#include <string>
#include <map>
#include <memory>

#include "../types.h"
// #include "File_Iterator.h"

namespace DB
{
    class FileIterator;
    struct FileHeader
    {
        PageId num_pages;
        PageId first_used_page;
        PageId num_free_pages;
        PageId first_free_page;
        bool operator==(const FileHeader &rhs) const
        {
            return (
                num_pages == rhs.num_pages &&
                first_used_page == rhs.first_used_page &&
                first_free_page == rhs.first_free_page &&
                num_free_pages == rhs.num_free_pages);
        }
    };

    class File
    {
    public:
        static File create(const std::string &filename_);
        static File open(const std::string &filename);
        static void remove(const std::string &filename);
        static bool isOpen(const std::string &filename);
        static bool exists(const std::string &filename);
        File(const File &other);
        File &operator=(const File &rhs);
        ~File();
        Page allocatePage();
        Page readPage(const PageId page_number) const;
        void writePage(const Page &new_page);
        void deletePage(const PageId page_number);
        const std::string &filename() const { return filename_; }
        FileIterator begin();
        FileIterator end();

    private:
        static std::streampos pagePosition(const PageId page_number)
        {
            return sizeof(FileHeader) + (page_number - 1) * Page::SIZE;
        }
        File(const std::string &name, const bool create_new);
        void openIfNeeded(const bool create_new);
        void close();
        Page readPage(const PageId page_number, const bool allow_free) const;
        void writePage(const PageId page_number, const Page &new_page);
        void writePage(const PageId page_number, const PageHeader &header, const Page &new_page);
        FileHeader readHeader() const;
        void writeHeader(const FileHeader &header);
        PageHeader readPageHeader(const PageId page_number) const;
        typedef std::map<std::string, std::shared_ptr<std::fstream>> StreamMap;
        typedef std::map<std::string, int> CountMap;

        static StreamMap open_streams_;
        static CountMap open_counts_;
        std::string filename_;
        std::shared_ptr<std::fstream> stream_;
        friend class FileIterator;
    };

    class FileIterator
    {
    private:
        File *file_;
        PageId current_page_number_;

    public:
        FileIterator()
            : file_(NULL),
              current_page_number_(Page::INVALID_NUMBER)
        {
        }

        FileIterator(const FileIterator &it)
        {
            file_ = it.file_;
            current_page_number_ = it.current_page_number_;
        }

        FileIterator(File *file)
            : file_(file)
        {
            assert(file_ != NULL);
            const FileHeader &header = file_->readHeader();
            current_page_number_ = header.first_used_page;
        }

        FileIterator(File *file, PageId page_number)
            : file_(file),
              current_page_number_(page_number)
        {
        }
        FileIterator &operator++();

        FileIterator operator++(int);

        bool operator==(const FileIterator &rhs) const;

        bool operator!=(const FileIterator &rhs) const;

        Page operator*() const;
    };

    FileIterator &FileIterator::operator++()
    {
        assert(file_ != NULL);
        const PageHeader &header = file_->readPageHeader(current_page_number_);
        current_page_number_ = header.next_page_number;

        return *this;
    }

    FileIterator FileIterator::operator++(int)
    {
        FileIterator tmp = *this;

        assert(file_ != NULL);
        const PageHeader &header = file_->readPageHeader(current_page_number_);
        current_page_number_ = header.next_page_number;

        return tmp;
    }

    bool FileIterator::operator==(const FileIterator &rhs) const
    {
        return file_->filename() == rhs.file_->filename() &&
               current_page_number_ == rhs.current_page_number_;
    }

    bool FileIterator::operator!=(const FileIterator &rhs) const
    {
        return (file_->filename() != rhs.file_->filename()) ||
               (current_page_number_ != rhs.current_page_number_);
    }

    Page FileIterator::operator*() const
    {
        return file_->readPage(current_page_number_);
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

    Page File::allocatePage()
    {
        FileHeader header = readHeader();
        Page new_page;
        Page existing_page;
        if (header.num_free_pages > 0)
        {
            new_page = readPage(header.first_free_page, true /* allow_free */);
            new_page.set_page_number(header.first_free_page);
            header.first_free_page = new_page.next_page_number();
            --header.num_free_pages;

            if (header.first_used_page == Page::INVALID_NUMBER ||
                header.first_used_page > new_page.page_number())
            {
                // Either have no pages used or the head of the used list is a page later
                // than the one we just allocated, so add the new page to the head.
                if (header.first_used_page > new_page.page_number())
                {
                    new_page.set_next_page_number(header.first_used_page);
                }
                header.first_used_page = new_page.page_number();
            }
            else
            {
                // New page is reused from somewhere after the beginning, so we need to
                // find where in the used list to insert it.
                PageId next_page_number = Page::INVALID_NUMBER;
                for (FileIterator iter = begin(); iter != end(); ++iter)
                {
                    next_page_number = (*iter).next_page_number();
                    if (next_page_number > new_page.page_number() ||
                        next_page_number == Page::INVALID_NUMBER)
                    {
                        existing_page = *iter;
                        break;
                    }
                }
                existing_page.set_next_page_number(new_page.page_number());
                new_page.set_next_page_number(next_page_number);
            }

            assert((header.num_free_pages == 0) ==
                   (header.first_free_page == Page::INVALID_NUMBER));
        }
        else
        {
            new_page.set_page_number(header.num_pages);
            if (header.first_used_page == Page::INVALID_NUMBER)
            {
                header.first_used_page = new_page.page_number();
            }
            else
            {

                for (FileIterator iter = begin(); iter != end(); ++iter)
                {
                    if ((*iter).next_page_number() == Page::INVALID_NUMBER)
                    {
                        existing_page = *iter;
                        break;
                    }
                }
                assert(existing_page.isUsed());
                existing_page.set_next_page_number(new_page.page_number());
            }
            ++header.num_pages;
        }
        writePage(new_page.page_number(), new_page);
        if (existing_page.page_number() != Page::INVALID_NUMBER)
        {
            writePage(existing_page.page_number(), existing_page);
        }
        writeHeader(header);

        return new_page;
    }

    Page File::readPage(const PageId page_number) const
    {
        FileHeader header = readHeader();
        if (page_number >= header.num_pages)
        {
            std::cerr << "Peticion dirigida a pagina invalida.\n";
            std::cerr << "Pagina: " << page_number << "\n";
            std::cerr << "File: " << filename_ << "\n";
        }
        return readPage(page_number, false);
    }

    Page File::readPage(const PageId page_number, const bool allow_free) const
    {
        Page page;
        stream_->seekg(pagePosition(page_number), std::ios::beg);
        stream_->read(reinterpret_cast<char *>(&page.header_), sizeof(page.header_));
        stream_->read(reinterpret_cast<char *>(&page.data_[0]), Page::DATA_SIZE);
        if (!allow_free && !page.isUsed())
        {
            std::cerr<< "Request made for an invalid page."
            << " Requested page " << page_number
            << " from file '" << filename_ << "'";
            return page;
        }
        return page;
    }

    void File::writePage(const Page &new_page)
    {
        PageHeader header = readPageHeader(new_page.page_number());
        if (header.current_page_number == Page::INVALID_NUMBER)
        {
            std::cerr << "Peticion dirigida a pagina invalida.\n";
            std::cerr << "Pagina: " << new_page.page_number() << "\n";
            std::cerr << "File: " << filename_ << "\n";
        }
        const PageId next_page_number = header.next_page_number;
        header = new_page.header_;
        header.next_page_number = next_page_number;
        writePage(new_page.page_number(), header, new_page);
    }

    void File::deletePage(const PageId page_number)
    {
        FileHeader header = readHeader();
        Page existing_page = readPage(page_number);
        Page previous_page;
        // If this page is the head of the used list, update the header to point to
        // the next page in line.
        if (page_number == header.first_used_page)
        {
            header.first_used_page = existing_page.next_page_number();
        }
        else
        {
            // Walk the used list so we can update the page that points to this one.
            for (FileIterator iter = begin(); iter != end(); ++iter)
            {
                previous_page = *iter;
                if (previous_page.next_page_number() == existing_page.page_number())
                {
                    previous_page.set_next_page_number(existing_page.next_page_number());
                    break;
                }
            }
        }
        // Clear the page and add it to the head of the free list.
        existing_page.initialize();
        existing_page.set_next_page_number(header.first_free_page);
        header.first_free_page = page_number;
        ++header.num_free_pages;
        if (previous_page.isUsed())
        {
            writePage(previous_page.page_number(), previous_page);
        }
        writePage(page_number, existing_page);
        writeHeader(header);
    }

    FileIterator File::begin()
    {
        const FileHeader &header = readHeader();
        return FileIterator(this, header.first_used_page);
    }

    FileIterator File::end()
    {
        return FileIterator(this, Page::INVALID_NUMBER);
    }

    File::File(const std::string &name, const bool create_new) : filename_(name)
    {
        openIfNeeded(create_new);

        if (create_new)
        {
            // File starts with 1 page (the header).
            FileHeader header = {1 /* num_pages */, 0 /* first_used_page */,
                                 0 /* num_free_pages */, 0 /* first_free_page */};
            writeHeader(header);
        }
    }

    void File::openIfNeeded(const bool create_new)
    {
        if (open_counts_.find(filename_) != open_counts_.end())
        { // exists an entry already
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
                // Error if we try to overwrite an existing file.
                if (already_exists)
                {
                    std::cerr << "El file ya existe.\n";
                    std::cerr << "File: " << filename_ << "\n";
                }
                // New files have to be truncated on open.
                mode = mode | std::fstream::trunc;
            }
            else
            {
                // Error if we try to open a file that doesn't exist.
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

    void File::writePage(const PageId page_number, const Page &new_page)
    {
        writePage(page_number, new_page.header_, new_page);
    }

    void File::writePage(const PageId page_number, const PageHeader &header,
                         const Page &new_page)
    {
        stream_->seekp(pagePosition(page_number), std::ios::beg);
        stream_->write(reinterpret_cast<const char *>(&header), sizeof(header));
        stream_->write(reinterpret_cast<const char *>(&new_page.data_[0]),
                       Page::DATA_SIZE);
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

    PageHeader File::readPageHeader(PageId page_number) const
    {
        PageHeader header;
        stream_->seekg(pagePosition(page_number), std::ios::beg);
        stream_->read(reinterpret_cast<char *>(&header), sizeof(header));

        return header;
    }
}