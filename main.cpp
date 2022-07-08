#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <cstring>
#include <memory>

#include "pages/page.h"
#include "pages/File.h"
#include "buffer.h"

#define PRINT_ERROR(str)  std::cerr << "On Line No:" << __LINE__ << "\n"; std::cerr << str << "\n"; exit(1);                                        

using namespace siprec;    

const PageId num = 100;
PageId      pid[num], pageno1, pageno2, pageno3, i;
RecordId    rid[num], rid2, rid3;
Page        *page, *page2, *page3;
char        tmpbuf[100];
BufMgr      *bufMgr;
File        *file1ptr, *file2ptr, *file3ptr, *file4ptr, *file5ptr;

void test1();

int main()
{
    // Following code shows how to you File and Page classes

    const std::string &filename = "test.db";
    // Clean up from any previous runs that crashed.

    File::remove(filename);

    {
        // Create a new database file.
        File new_file = File::create(filename);

        // Allocate some pages and put data on them.
        PageId third_page_number;
        for (int i = 0; i < 5; ++i)
        {
            Page new_page = new_file.allocatePage();
            if (i == 3)
            {
                // Keep track of the identifier for the third page so we can read it
                // later.
                third_page_number = new_page.page_number();
            }
            new_page.insertRecord("hello!");
            // Write the page back to the file (with the new data).
            new_file.writePage(new_page);
        }
        

        // Iterate through all pages in the file.
        for (FileIterator iter = new_file.begin() ; iter != new_file.end() ; ++iter)
        {
            //Page it = *iter;
            // Iterate through all records on the page.
            for (PageIterator page_iter = (*iter).begin() ; page_iter != (*iter).end() ; ++page_iter)
            {   
                // indireccion verifica que el slot no este en uso
                std::cout << "Found record: " << *page_iter << " on page " << (*iter).page_number() << "\n";
            }
        }

        // Retrieve the third page and add another record to it.
        Page third_page = new_file.readPage(third_page_number);
        const RecordId &rid = third_page.insertRecord("world!");
        new_file.writePage(third_page);

        // Retrieve the record we just added to the third page.
        std::cout << "Third page has a new record: "
                  << third_page.getRecord(rid) << "\n\n";
    }
    // new_file goes out of scope here, so file is automatically closed.

    // Delete the file since we're done with it.
    File::remove(filename);
}