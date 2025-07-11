#include <exception>
#include <iostream>

/* boost headers */
#include <boost/asio.hpp>

/* cearch headers */
#include "Index.h"
#include "Server.h"
#include "ContentAddressedStorage.h"

int main(int argc, const char *argv[]) {
    /*
    *   TODO: Use propper commandline parsing
    */
    if (argc != 4) {
        std::cerr << "Usage: ./cearch <query_port> <Directory to index> <directory ";
        std::cerr << "to save index in> ";
        std::cerr << std::endl;
        return 1;
    }

    int query_port = atoi(argv[1]);
    std::string directory = argv[2];
    std::string index_path = argv[3];

    try {
        boost::asio::io_context io_context;

        /* TODO: Make CAS Optional for the index */
        auto cas_storage = std::make_unique<ContentAddressedStorage>(index_path);

        /* 
        *   TODO: Make indexing multithreaded?
        *   TODO: Indexing should be triggered from external sources? Right now it blocks here until the indexing is done
        */
        Index idx(directory, index_path, cas_storage);

        std::cout << "Starting Index and Query Services " << query_port << std::endl;
        Server query_service(io_context, query_port, idx);
        io_context.run();
    } catch (const std::exception &e) {
        std::cerr << "Error in main: " << e.what() << std::endl;
        return 2;
    }

    return 0;
}