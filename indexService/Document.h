#ifndef _H_DOCUMENT
#define _H_DOCUMENT

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include <chrono>

#include <nlohmann/json.hpp>

#include "ContentStrategy.h"

/* 
*   Uses Strategy Design Pattern to get rid of inheritance
*   For each type of Document a Content Strategy needs to be defined
*   The Content Strategy has to implement the read_content function
*   The type of the Document is defined by its file extension 
*/
class Document {
    public:
        /* TODO: make document independant of the filepath, rather use a title or document name or id */
        Document(uint64_t docid, std::string filepath, std::string file_extension, std::unique_ptr<ContentStrategy> strategy);

        /* fills the concordance from the content of the document */
        void index_document();

        /* setter functions */
        void set_concordance(std::unordered_map<std::string, int> concordance);
        void set_total_term_count(int term_count);
        void set_indexed_at(std::chrono::system_clock::time_point time);

        /* getter functions */
        uint64_t get_docid();
        int get_total_term_count();
        std::unordered_map<std::string, int> get_concordance();
        int get_term_frequency(const std::string &term);
        std::string get_filepath() const;
        std::string get_extension();
        std::string get_file_content_as_string();
        bool contains_term(const std::string &term);
        static std::vector<std::string> clean_word(std::string &word);

        /* JSON Serialization, deserialization is done in DocumentFactory */
        nlohmann::json to_json() const;

    private:
        std::string read_content();

        uint64_t m_docid;
        int m_total_term_count;
        std::string filepath;
        std::string file_extension;
        std::unique_ptr<ContentStrategy> strategy_;
        std::chrono::system_clock::time_point indexed_at;

        /* every term in the document and a counter for that term */
        std::unordered_map<std::string, int> concordance;
};

#endif