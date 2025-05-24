#include "Session.h"

using boost::asio::ip::tcp;

Session::Session(tcp::socket socket, Index &idx)
    : socket(std::move(socket)), idx(idx) {}

Session::~Session() {}

void Session::start() {
    /* start reading from the socket */
    http::read(socket, m_buffer, m_request);
    write_response();
}

void Session::write_response() {
    /* check the buffer and respond */
    m_response.version(m_request.version());
    m_response.result(http::status::ok);
    m_response.set(http::field::server, "Boost Beast");
    m_response.set(http::field::content_type, "text/html");
    std::string html_body = read_html_file("web/index.html");

    /* handle post request, aka. calculate the tfidf and display a result */
    std::vector<std::string> input_values;

    if (m_request.method() == http::verb::post) {
        std::string input_value;
        if (m_request.body().size() > 0) {
            /* parse the input field, assumong its name is "input-text" */
            std::string request_body =
                beast::buffers_to_string(m_request.body().data());
            std::cout << "Body: " << request_body << std::endl;
            size_t start_pos = request_body.find("input-text=");
            if (start_pos != std::string::npos) {
                /* move past the name to get the value */
                start_pos += std::strlen("input-text=");
                size_t end_pos = request_body.find("&", start_pos);
                input_value =
                    request_body.substr(start_pos, end_pos - start_pos);
            }
        }

        /* extract every single word from input value */
        input_values = Document::clean_word(input_value);

        /* retrieve the result from the index */
        std::vector<std::pair<std::string, double>> result;
        if (!input_values.empty()) {
            result = idx.query_index(input_values);
        }

        /* insert result into index.html */
        size_t pos = html_body.find("<table>") + strlen("<table>");
        if (pos != std::string::npos) {
            std::ostringstream oss;
            for (auto &i : result) {
                oss << "<tr><td>" << i.first << " => " << i.second
                    << "</td></tr>";
            }
            std::string result_table = oss.str();
            html_body.insert(pos, result_table);
        } else {
            std::cerr << "Couldnt find table in html body, no results shown";
        }
    }

    /* send the response with the result */
    beast::ostream(m_response.body()) << html_body;
    m_response.prepare_payload();
    http::write(socket, m_response);
}

std::string Session::read_html_file(const std::string &file_path) {
    std::ifstream file(file_path);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open HTML file: " + file_path);
    }

    std::string html_content((std::istreambuf_iterator<char>(file)),
                             std::istreambuf_iterator<char>());
    return html_content;
}