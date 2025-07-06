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

        std::vector<std::pair<std::string, double>> query_index(const std::vector<std::string> &input_values);

        int get_document_counter();
        void set_thread_num(int num);

    private:
        /* holds a reference to every document in the index */
        std::vector<std::unique_ptr<Document>> documents;
        std::vector<std::string> stopwords;

        std::string index_path;

        /* relevant for BM25 */
        uint64_t m_total_term_count;
        uint64_t m_avg_doc_length;

        /* threading */
        int thread_num;
        std::mutex mtx;
        std::vector<std::thread> threads;

        void build_document_index(std::string directory);
        void read_stopwords(const std::string &filepath);

        /* BM25 Stuff */
        void set_avg_doc_length();
        double compute_idf(int total_docs, int doc_freq);
        double compute_bm25(int term_freq, int doc_length, double avg_doc_len, double idf, double k1 = 1.2, double b = 0.75);
};

#endif