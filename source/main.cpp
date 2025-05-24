#include <chrono>
#include <exception>
#include <iostream>

/* boost headers */
#include <boost/asio.hpp>

/* cearch headers */
#include "Index.h"
#include "Server.h"

int main(int argc, const char *argv[]) {
    /*
    *   TODO: Use propper commandline parsing
    */
    if (argc != 5) {
        std::cerr << "Usage: ./cearch <Port> <Directory to index> <directory ";
        std::cerr << "to save index in> <number of threads to use> ";
        std::cerr << std::endl;
        return 1;
    }

    int port = atoi(argv[1]);
    std::string directory = argv[2];
    std::string index_path = argv[3];
    int threads = atoi(argv[4]);

    try {
        boost::asio::io_context io_context;

        /* 
        *   TODO: Make indexing asynchronous?
        *   Right now it blocks here until the indexing is done
        */
        Index idx(directory, index_path, threads);

        std::cout << "Starting cearch server on port: " << port << std::endl;

        /* TODO: Use grpc?*/
        Server server(io_context, port, idx);
        io_context.run();
    } catch (const std::exception &e) {
        std::cerr << "Error in main: " << e.what() << std::endl;
        return 2;
    }

    return 0;
}