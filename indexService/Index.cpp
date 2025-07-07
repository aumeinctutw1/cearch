#include <iostream>
#include <filesystem>
#include <fstream>
#include <cmath>

#include "Index.h"
#include "DocumentFactory.h"

/*
*   @param directory The directoy which should be crawled and indexed   
*   @param index_path The path in which the index should be stored on filesystem
*   @param threads_used The number of threads which should be used during indexing. Must be >=0   
* 
*   TODO: save and load index to filesystem, using json? 
*   TODO: remove Indexing from the constructor, trigger from outside (http server)
*/
Index::Index(std::string directory, std::string index_path, std::unique_ptr<ContentAddressedStorage> &content_store)
    : index_path(index_path), m_total_term_count(0), m_content_store(std::move(content_store))
{
    /* Check wether a index is present in the filesystem and can be loaded */
    std::string index_filepath = index_path + "/index.json";
    if (is_index_present()) {
        std::cout << "Loading existing index found in: " << index_path << std::endl; 
        load_index_from_file(index_filepath);
    } else {
        std::cout << "Building new Index" << std::endl;

        try {
            read_stopwords("stopwords.txt");
            build_document_index(directory);
            set_avg_doc_length();
            save_index_to_file(index_filepath);
        } catch (std::exception &e) {
            std::cerr << "Caught Exception building index: " << e.what() << std::endl;
        }
    }

    std::cout << "Total documents: " << get_document_counter() << std::endl;
    std::cout << "Total term count: " << m_total_term_count << std::endl;
}

/*
*   Queries the index and returns the result ordered by BM25 ranking
*   returns a sorted by rank ascending vector of pairs <document->filepath, tfidf-rank>
*
*   TODO: timeout based search?
*   TODO: Split index in Buckets/Shards, use threads to search the buckets
*/
std::vector<std::pair<uint64_t, double>> Index::query_index(const std::vector<std::string> &input_values) {
    std::unordered_map<uint64_t, double> score_map;

    for (const auto &term: input_values) {
        int doc_freq = 0;
        std::vector<std::tuple<uint64_t, int, int>> term_data;

        for (const auto &doc: documents) {
            int term_freq = doc->get_term_frequency(term);

            if (term_freq > 0) {
                doc_freq++;
            }
            
            int doc_length = doc->get_total_term_count();
            /* temp store term_freq and doc_length for BM25 */
            term_data.emplace_back(doc->get_docid(), term_freq, doc_length);
        }

        double idf = compute_idf(get_document_counter(), doc_freq);

        for (const auto &[doc, tf, dl]: term_data) {
            double score = compute_bm25(tf, dl, m_avg_doc_length, idf);
            if (score > 0.0) {
                score_map[doc] += score;
            }
        }
    }

    /* move scores into sortable vector */
    std::vector<std::pair<uint64_t, double>> result(score_map.begin(), score_map.end());

    /* Sort the result ascending by rank */
    std::sort(result.begin(), result.end(),
        [](const auto &a, const auto &b) { 
            return a.second > b.second; 
        }
    );

    return result;
}

/*
*   for statistics
*/
int Index::get_document_counter() { return documents.size(); }
int Index::get_total_term_count() { return m_total_term_count;}
int Index::get_avg_doc_length() { return m_avg_doc_length; }

/* read content of a single document and create concordance */
void Index::index_document(std::unique_ptr<Document> &doc) {
    std::string content = doc->get_file_content_as_string();
    /* only the raw content is stored, after filtering via content strategy */
    std::string content_hash = m_content_store->store(content);

    std::unordered_map<std::string, int> concordance;
    std::istringstream iss(content);
    std::string word;
    int total_term_count = 0;

    std::cout << "Indexing doc : " << doc->get_docid() << std::endl;

    while (iss >> word) {
        /* split the word if necessary */
        std::vector<std::string> clean_words = Document::clean_word(word);
        for (const auto &clean_word : clean_words) {
            if (!clean_word.empty() && clean_word.find_first_not_of(' ') != std::string::npos) {
                concordance[clean_word]++;
                total_term_count++;
            } 
        }
    }

    doc->set_concordance(concordance);
    doc->set_total_term_count(total_term_count);
    doc->set_indexed_at(std::chrono::system_clock::now());
    doc->set_content_hash(content_hash);
}

/*
*   Moves trough a directy and try's to read every supported file in it
*   For every supported file in the dir, a Document is created and stored in the document index
*/
void Index::build_document_index(std::string directory) {
    /* check if the param is a directory */
    if (std::filesystem::status(directory).type() == std::filesystem::file_type::directory) {
        std::cout << "Building index of directory: " << directory << std::endl;
        if (std::filesystem::exists(directory)) {
            for (auto const &entry : std::filesystem::recursive_directory_iterator(directory)) {
                std::string filepath = entry.path();
                std::string file_extension = std::filesystem::path(entry.path()).extension();
                try {
                    uint64_t docid = documents.size() + 1;
                    std::unique_ptr<Document> new_doc = DocumentFactory::create_document(docid, filepath, file_extension);
                    index_document(new_doc);
                    m_total_term_count += new_doc->get_total_term_count();
                    documents.push_back(std::move(new_doc));
                } catch (std::exception &e) {
                    std::cerr << "Exception caught reading file: ";
					std::cerr << e.what() << std::endl;
                }
            }
        }
    } else {
        std::cerr << "No directoy given to index" << std::endl;
        throw std::runtime_error("Directory to index not found: " + directory);
    }
}

/*
*   read stopwords from a txt file, save them in vector
*/
void Index::read_stopwords(const std::string &filepath) {
    try {
        std::ifstream file(filepath);
        if (!file.is_open()) {
            throw std::runtime_error("Failed to open stopwords textfile: " + filepath);
        }

        std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

        std::stringstream iss(content);
        std::string word;
        while (iss >> word) {
            stopwords.push_back(word);
        }
    } catch (std::exception &e) {
        std::cerr << "Exception ocurred reading stop words: " << e.what() << std::endl;
        stopwords.clear();
        return;
    }
}

void Index::set_avg_doc_length() {
    m_avg_doc_length = m_total_term_count / get_document_counter();
}

double Index::compute_idf(int total_docs, int doc_freq) {
    return log((total_docs - doc_freq + 0.5) / (doc_freq + 0.5) + 1);
}

double Index::compute_bm25(int term_freq, int doc_length, double avg_doc_len, double idf, double k1, double b) {
    double numerator = term_freq * (k1 + 1);
    double denominator = term_freq + k1 *(1 - b + b* (double)doc_length / avg_doc_len);
    return idf * (numerator / denominator);
}

void Index::save_index_to_file(std::string filepath) {
    nlohmann::json j;
    for (const auto &doc: documents) {
        j.push_back(doc->to_json());
    }
    std::ofstream file(filepath);
    file << j.dump(4);
    write_index_marker();
}

void Index::load_index_from_file(std::string filepath) {
    std::ifstream file(filepath);
    nlohmann::json j;
    file >> j;

    for (const auto &doc_json: j) {
        auto doc = DocumentFactory::from_json(doc_json);
        m_total_term_count += doc->get_total_term_count();
        documents.push_back(std::move(doc));
    }

    set_avg_doc_length();
}

void Index::write_index_marker() {
    std::ofstream(index_path + "/.index_complete").put('\n');
}

bool Index::is_index_present() {
    return std::filesystem::exists(index_path + "/.index_complete");
}