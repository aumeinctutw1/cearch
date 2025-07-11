#include <functional>
#include <unordered_map>
#include <fstream>
#include <regex>

#include "nlohmann/json.hpp"
#include "Session.h"
#include "Document.h"

using json = nlohmann::json;

Session::Session(tcp::socket socket, Index &idx)
    : m_stream(std::move(socket)), m_idx(idx) {

    /* init the possible routes for this session */
    m_routes = {
        /* POST, returns query results ranked bm25 */
        {"/query", [this]() { return handle_index_query(); }},
        /* POST, TODO: index a document */
        {"/index", [this]() { return handle_index(); }},
        /* GET, return json representation for a specific document, example /document/123 */
        {"/document", [this]() { return handle_document(); }},
        /* GET, return simple statistics from index as json */
        {"/statistics", [this]() { return handle_statistics(); }}

        /* POST, TODO: /filter filter a specific document and return filtered content */
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
    //print_http_request_info(m_request);

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
    /* dynamic rest style route */
    if (target.rfind("/document/", 0) == 0) {
        return m_routes.find("/document")->second();
    }

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

Response Session::make_bad_request(const std::string &message) {
    Response res{http::status::bad_request, 11};
    res.set(http::field::server, "Cearch");
    res.set(http::field::content_type, "application/json");

    json j;
    j["error"] = message;
    res.body() = j.dump();

    return res;
}

Response Session::handle_index() {
    Response res{http::status::not_found, 11};
    res.set(http::field::server, "Cearch");
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
*   TODO: Stream the response on bigger queries? 
*/
Response Session::handle_index_query() {
    Response res{http::status::ok, 11};
    res.set(http::field::server, "Cearch");
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
    std::vector<std::pair<uint64_t, double>> query_result = m_idx.query_index(clean_query);

    json response;
    for(const auto &[docid, score]: query_result) {
        response["results"].push_back({
            {"docid", docid},
            {"score", score}
        });
    }

    res.body() = response.dump();
    return res;
}

Response Session::handle_document() {
    std::string target = std::string(m_request.target());

    if (m_request.method() == http::verb::get) {
        std::regex re("^/document/([0-9]+)$");
        std::smatch match;

        if (std::regex_match(target, match, re)) {
            try {
                uint64_t docid = std::stoull(match[1]);
                auto &doc = m_idx.get_document_by_id(docid);

                /* return the json representation of the doc if found */
                Response res{http::status::ok, 11};
                res.set(http::field::server, "Cearch");
                res.set(http::field::content_type, "application/json");
                res.body() = doc.to_json().dump();
                return res;
            } catch (std::exception &e) {
                std::cerr << "Exception in hanling documents: " << e.what() << std::endl;
                return not_found();
            }
        }
    }
    
    /* if the regex failed or wrong http method is used, return 404 */
    return not_found();
}

/*
*   Return some statistics from the index
*/
Response Session::handle_statistics() {
    Response res{http::status::ok, 11};
    res.set(http::field::server, "Cearch");
    res.set(http::field::content_type, "application/json");

    json j;
    j["Document_count"] = m_idx.get_document_counter();
    j["Total_term_count"] = m_idx.get_total_term_count();
    j["Average_document_length"] = m_idx.get_avg_doc_length();
    res.body() = j.dump();

    return res;
}