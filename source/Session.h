#ifndef _H_SESSION
#define _H_SESSION

#include <exception>
#include <fstream>
#include <iostream>
#include <memory>
#include <utility>

/* Boost HTTP Stuff*/
#include <boost/asio.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>

#include "Index.h"
#include "Server.h"

namespace beast = boost::beast;  // from <boost/beast.hpp>
namespace http = beast::http;    // from <boost/beast/http.hpp>

class Session : public std::enable_shared_from_this<Session> {
   public:
    Session(boost::asio::ip::tcp::socket socket, Index &idx);
    ~Session();
    void start();

   private:
    void write_response();
    std::string read_html_file(const std::string &file_path);

    boost::asio::ip::tcp::socket socket;
    Index &idx;

    /* http request buffers */
    beast::flat_buffer m_buffer;
    http::request<http::dynamic_body> m_request;
    http::response<http::dynamic_body> m_response;
};

#endif