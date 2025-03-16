#include <chrono>
#include <exception>
#include <iostream>

/* boost headers */
#include <boost/asio.hpp>

/* cearch headers */
#include "Index.h"
#include "Server.h"
#include "Timer.h"

int main(int argc, const char *argv[]) {
    /* read configuration from command line */
    if (argc != 6) {
        std::cerr << "Usage: ./cearch <Port> <Directory to index> <directory ";
        std::cerr << "to save index in> <number of threads to use>";
        std::cerr << "<timer in seconds for reindexing>";
        std::cerr << std::endl;
        return 1;
    }

    int port = atoi(argv[1]);
    std::string directory = argv[2];
    std::string index_path = argv[3];
    int threads = atoi(argv[4]);
    int tick = atoi(argv[5]);

    try {
        /* create io context */
        boost::asio::io_context io_context;

        /* 
        *   init the index and timer 
        *   TODO: Make indexing asynchronous
        */
        Index idx(directory, index_path, threads);
        Timer timer(tick, io_context, idx);

        std::cout << "Starting cearch server on port: " << port << std::endl;
        Server server(io_context, port, idx);
        io_context.run();
    } catch (const std::exception &e) {
        std::cerr << "Error in main: " << e.what() << std::endl;
        return 2;
    }

    return 0;
}