#ifndef _H_INDEX
#define _H_INDEX

#include <exception>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>
#include <future>

#include "Document.h"
#include "ContentAddressedStorage.h"

class Index {
    public:
        Index(std::string directory, std::string index_path, std::unique_ptr<ContentAddressedStorage> &content_store);
        ~Index() = default;

        std::vector<std::pair<uint64_t, double>> query_index(const std::vector<std::string> &input_values);
        const Document &get_document_by_id(uint64_t docid) const;

        /* TODO: implement a consistency check against the content storage, are hashes from index present in filesystem? */

        int get_document_counter();
        int get_total_term_count();
        int get_avg_doc_length();

    private:
        /* holds a reference to every document in the index */
        std::unordered_map<uint64_t, std::unique_ptr<Document>> documents;
        std::vector<std::string> stopwords;

        std::string index_path;

        /* content storage */
        std::shared_ptr<ContentAddressedStorage> m_content_store;       

        /* relevant for BM25 */
        uint64_t m_total_term_count;
        uint64_t m_avg_doc_length;

        /* Parallelization with future */
        std::vector<std::future<void>> m_futures;
        std::mutex m_index_mutex;
        std::atomic<uint64_t> m_docid_counter{1};

        /* Indexing */
        void index_document(std::unique_ptr<Document> &doc);
        void build_document_index(std::string directory);
        void read_stopwords(const std::string &filepath);

        /* file persistence */
        void write_index_marker();
        bool is_index_present();
        void save_index_to_file(std::string filepath);
        void load_index_from_file(std::string filepath);

        /* BM25 Stuff */
        void set_avg_doc_length();
        double compute_idf(int total_docs, int doc_freq);
        double compute_bm25(int term_freq, int doc_length, double avg_doc_len, double idf, double k1 = 1.2, double b = 0.75);
};

#endif