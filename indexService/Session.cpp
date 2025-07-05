#include <functional>
#include <unordered_map>
#include <fstream>

#include "Session.h"

Session::Session(tcp::socket socket, Index &idx)
    : m_stream(std::move(socket)), m_idx(idx) {
        /* init the possible routes for this session */
        m_routes = {
            {"/search", [this]() { return handle_search();}}
        };
    }

Session::~Session() {}

void Session::start() {
    read_request();
}

void Session::read_request() {
    auto self = shared_from_this();
    http::async_read(m_stream, m_buffer, m_request,
        [self](beast::error_code ec, std::size_t bytes_transferred) {
            boost::ignore_unused(bytes_transferred);
            if (!ec) {
                self->handle_request();
            } else {
                std::cerr << "ERROR: reading http request: " << ec.message() << std::endl; 
            }
        }
    );
}

void Session::handle_request() {
    /* get the response from a handle */
    m_response = route_request(m_request.target());

    /* send the response */
    auto self = shared_from_this();
    std::cout << "INFO writing response" << std::endl;
    http::async_write(m_stream, m_response, 
        [self](boost::beast::error_code ec, std::size_t) {
            if (ec) {
                std::cerr << "ERROR: writing http response: " << ec.message() << std::endl; 
            } else {
                std::cout << "INFO: writing request done, socket shutdown" << std::endl;
                beast::error_code shutdown_ec;
                self->m_stream.socket().shutdown(tcp::socket::shutdown_send, shutdown_ec);
                if (shutdown_ec) {
                    std::cerr << "ERROR: shutdown of socket failed: " << shutdown_ec.message() << std::endl;
                }
            }
        }
    );
}

Response Session::route_request(const std::string &target) {
    auto it = m_routes.find(target);
    if (it != m_routes.end()) {
        return it->second();
    } else {
        return not_found();
    }
}

Response Session::not_found() {
    Response res{http::status::not_found, 11};
    res.set(http::field::content_type, "text/plain");
    res.body() = "404 Not Found";
    res.prepare_payload();
    return res;
}

/* 
*   TODO: should return json body
*   which contains the results
*   The request should also be json then
*/
Response Session::handle_search() {
    Response res{http::status::ok, m_request.version()};
    res.version(m_request.version());
    res.result(http::status::ok);
    res.set(http::field::server, "Boost Beast");
    res.set(http::field::content_type, "text/html");
    std::string html_body = read_html_file("web/index.html");

    std::vector<std::string> input_values;

    std::string input_value;
    if (m_request.body().size() > 0) {
        /* parse the input field, assumong its name is "input-text" */
        std::string request_body = m_request.body();
        std::cout << "Body: " << request_body << std::endl;
        size_t start_pos = request_body.find("input-text=");
        if (start_pos != std::string::npos) {
            /* move past the name to get the value */
            start_pos += std::strlen("input-text=");
            size_t end_pos = request_body.find("&", start_pos);
            input_value = request_body.substr(start_pos, end_pos - start_pos);
        }
    }

    /* extract every single word from input value */
    input_values = Document::clean_word(input_value);

    /* retrieve the result from the index */
    std::vector<std::pair<std::string, double>> result;
    if (!input_values.empty()) {
        result = m_idx.query_index(input_values);
    }

    /* insert result into index.html */
    size_t pos = html_body.find("<table>") + strlen("<table>");
    if (pos != std::string::npos) {
        std::ostringstream oss;
        for (auto &i : result) {
            oss << "<tr><td>" << i.first << " => " << i.second << "</td></tr>";
        }
        std::string result_table = oss.str();
        html_body.insert(pos, result_table);
    } else {
        std::cerr << "Couldnt find table in html body, no results shown";
    }

    return res;
}


std::string Session::read_html_file(const std::string &file_path) {
    std::ifstream file(file_path);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open HTML file: " + file_path);
    }

    std::string html_content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    return html_content;
}