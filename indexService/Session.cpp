#include <functional>
#include <unordered_map>
#include <fstream>

#include "nlohmann/json.hpp"
#include "Session.h"
#include "Document.h"

using json = nlohmann::json;

Session::Session(tcp::socket socket, Index &idx)
    : m_stream(std::move(socket)), m_idx(idx) {

    /* init the possible routes for this session */
    m_routes = {
        {"/search", [this]() { return handle_search();}},
        {"/index", [this]() { return handle_index();}}
    };
}

Session::~Session() {}

void Session::start() {
    read_request();
}

/* for debugging and logging purpose */
void Session::print_http_request_info(const Request &req) {
    std::cout << "Method: " << req.method_string() << std::endl;
    std::cout << "Target: " << req.target() << std::endl;

    std::cout << "Headers:" << std::endl;
    for (auto const& field : req) {
        std::cout << field.name_string() << ": " << field.value() << std::endl;
    }    
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
    /* print info about the request */
    print_http_request_info(m_request);

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

Response Session::handle_index() {
    Response res{http::status::not_found, 11};
    res.set(http::field::content_type, "text/plain");
    res.body() = "Not implemented yet";
    res.prepare_payload();
    return res;
}

/* 
*   Accepts and Responds with JSON, example query:
*        curl -X POST http://localhost:8080/search \
*        -H "Content-Type: application/json" \
*        -d '{"query": "example search term"}'
*      
*   TODO: Stream the response?
*/
Response Session::handle_search() {
    Response res{http::status::ok, 11};
    res.set(http::field::server, "TFIDF Indexer");
    res.set(http::field::content_type, "application/json");

    std::string query = "";
    std::vector<std::string> clean_query;

    try {
        /* try parsing the json */
        json j = json::parse(m_request.body());

        /* try accessing the query field */
        if (j.contains("query") && j["query"].is_string()) {
            query = j["query"];
            std::cout << "Searching for: " << query << std::endl;
        } else {
            std::cout << "Invalid or missing query field" << std::endl;
            return make_bad_request("Missing or invalid 'query' field in JSON body");
        }
    } catch (const json::parse_error &e) {
        std::cerr << "JSON parse error: " << e.what() << std::endl;
        return make_bad_request("Malformed JSON in request body");
    }

    /* get search terms */
    clean_query = Document::clean_word(query);

    /* search the index */
    std::vector<std::pair<std::string, double>> query_result = m_idx.query_index(clean_query);

    json response;
    for(const auto &[filename, score]: query_result) {
        response["resulsts"].push_back({
            {"filename", filename},
            {"score", score}
        });
    }

    res.body() = response.dump();
    return res;
}

Response Session::make_bad_request(const std::string &message) {
    Response res{http::status::ok, 11};
    res.set(http::field::server, "TFIDF Indexer");
    res.set(http::field::content_type, "application/json");

    json j;
    j["error"] = message;
    res.body() = j.dump();

    return res;
}