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
*/
Index::Index(std::string directory, std::string index_path, int threads_used)
    : index_path(index_path), thread_num(threads_used), m_total_term_count(0)
{
    if (threads_used == 0 || threads_used < 0) {
        std::cerr << "Threads used is set to 0 or smaller than 0, setting 1" << std::endl;
        set_thread_num(1);
    }
    
    const auto processor_count = std::thread::hardware_concurrency();
    if (processor_count == 0) {
        std::cerr << "Processor count cant be determined" << std::endl;
    }
    std::cout << "Processor count: " << processor_count << " used threads: " << threads_used << std::endl;

    try {
        read_stopwords("stopwords.txt");
        build_document_index(directory);
        set_avg_doc_length();
    } catch (std::exception &e) {
        std::cerr << "Caught Exception building index: " << e.what() << std::endl;
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
std::vector<std::pair<std::string, double>> Index::query_index(const std::vector<std::string> &input_values) {
    std::unordered_map<std::string, double> score_map;

    for (const auto &term: input_values) {
        std::cout << "Searching term: " << term << std::endl;

        int doc_freq = 0;
        std::vector<std::tuple<const std::string, int, int>> term_data;

        for (const auto &doc: documents) {
            int term_freq = doc->get_term_frequency(term);

            if (term_freq > 0) {
                doc_freq++;
            }
            
            int doc_length = doc->get_total_term_count();
            /* temp store term_freq and doc_length for BM25 */
            term_data.emplace_back(doc->get_filepath(), term_freq, doc_length);
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
    std::vector<std::pair<std::string, double>> result(score_map.begin(), score_map.end());

    /* Sort the result ascending by rank */
    std::sort(result.begin(), result.end(),
        [](const auto &a, const auto &b) { 
            return a.second > b.second; 
        }
    );

    return result;
}

/*
*   returns the number of documents in the index
*/
int Index::get_document_counter() { return documents.size(); }

void Index::set_thread_num(int num) {
    if (num < 0 || num == 0) {
        std::cerr  << "Threads num cant be 0 or smaller than 0, not setting thread num" << std::endl;
        return;
    }

    this->thread_num = num;
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
                    std::unique_ptr<Document> new_doc = DocumentFactory::create_document(filepath, file_extension);
                    new_doc->index_document();
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