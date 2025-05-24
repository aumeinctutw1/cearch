#ifndef _H_INDEX
#define _H_INDEX

#include <exception>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

#include "Document.h"

class Index {
    public:
        Index(std::string directory, std::string index_path, int thread_num);
        ~Index() = default;

        /*
        *   calculates a ranking from tfidf index and returns the documents with
        *   the highest rank, based on the input
        */
        std::vector<std::pair<std::string, double>> query_index(const std::vector<std::string> &input_values);

        int get_document_counter();
        void print_tfidf_index();
        void set_thread_num(int num);

    private:
        /* vector of all Documents in the index */
        std::vector<std::unique_ptr<Document>> documents;

        /* holds the path to the index on the filesystem */
        std::string index_path;
        std::vector<std::string> stopwords;

        /* threading */
        int thread_num;
        std::mutex mtx;
        std::vector<std::thread> threads;

        void build_document_index(std::string directory);
        void build_tfidf_index();
        void read_stopwords(const std::string &filepath);
        /* calculates the inverse_doc_frequency of a term over the whole corpus */
        double inverse_doc_frequency(std::string term, const std::vector<std::unique_ptr<Document>> &corpus);
        void calculate_tfidf_index(int start_index, int end_index);
};

#endif