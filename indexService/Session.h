#ifndef _H_SESSION
#define _H_SESSION

#include <string>

/* Boost HTTP Stuff*/
#include <boost/asio.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>

#include "Index.h"
#include "Server.h"

namespace beast = boost::beast;  // from <boost/beast.hpp>
namespace http = beast::http;    // from <boost/beast/http.hpp>
namespace asio = boost::asio;

using tcp = asio::ip::tcp;

using Request = http::request<http::string_body>;
using Response = http::response<http::string_body>;

class Session : public std::enable_shared_from_this<Session> {
    public:
        explicit Session(tcp::socket socket, Index &idx);
        ~Session();
        void start();

    private:
        /* when searching we need to access the index */
        Index &m_idx;

        beast::tcp_stream m_stream;

        /* possible http routes */
        std::unordered_map<std::string, std::function<Response()>> m_routes;

        /* http request buffers */
        beast::flat_buffer m_buffer;
        Request m_request;
        Response m_response;

        void print_http_request_info(const Request &req);

        /* Request handles */
        void read_request();
        void handle_request();
        Response route_request(const std::string &target);
        Response handle_search();
        Response handle_index();
        Response handle_statistics();
        Response not_found();
        Response make_bad_request(const std::string &message);
};

#endif