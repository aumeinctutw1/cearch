#ifndef _H_SERVER
#define _H_SERVER

#include <boost/asio.hpp>
#include <iostream>
#include <memory>
#include <utility>

#include "Index.h"
#include "Session.h"

class Server {
   public:
    Server(boost::asio::io_context &io_context, short port, Index &idx);
    void open_acceptor();

   private:
    void do_accept();

    boost::asio::ip::tcp::endpoint endpoint;
    boost::asio::ip::tcp::acceptor acceptor;
    Index &idx;
};

#endif