#ifndef _H_DOCUMENT
#define _H_DOCUMENT

#include <chrono>
#include <memory>
#include <string>
#include <unordered_map>

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
    Document(std::string filepath, std::string file_extension, std::unique_ptr<ContentStrategy> strategy);

    /* fills the concordance from the content of the document */
    void index_document();

    void print_tfidf_scores();
    void insert_tfidf_score(std::pair<std::string, double> tfidf_score);

    /* getter functions */
    double get_tfidf_score(const std::string &term);
    int get_term_frequency(const std::string &term);
    std::unordered_map<std::string, int> get_concordance();
    std::string get_filepath() const;
    std::string get_extension();
    std::string get_file_content_as_string();

    bool contains_term(const std::string &term);
    bool needs_reindexing();

    static std::vector<std::string> clean_word(std::string &word);

   private:
    std::string read_content();

    std::string filepath;
    std::string file_extension;
    std::unique_ptr<ContentStrategy> strategy_;
    std::chrono::system_clock::time_point indexed_at;

    /* every term in the document and a counter for that term */
    std::unordered_map<std::string, int> concordance;

    /* every term in the document and its tfidf score */
    std::unordered_map<std::string, double> tfidf_scores;
};

#endif