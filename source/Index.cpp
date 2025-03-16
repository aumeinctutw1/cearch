#include <iostream>
#include <filesystem>
#include <fstream>
#include <cmath>

#include "Index.h"
#include "DocumentFactory.h"

/*
 *  The directory is the directory which is read and indexed, the index_path is
 *  TODO: save index to filesystem, json?
 */
Index::Index(std::string directory, std::string index_path, int threads_used)
    : index_path(index_path), thread_num(threads_used) {

    const auto processor_count = std::thread::hardware_concurrency();
    if (processor_count == 0) {
        std::cerr << "Processor count cant be determined" << std::endl;
    }
    std::cout << "Processor count: " << processor_count << " used threads: " << threads_used << std::endl;

    /* start building the index */
    try {
        read_stopwords("stopwords.txt");
        build_document_index(directory);
        build_tfidf_index();
    } catch (std::exception &e) {
        std::cerr << "Caught Exception building index: " << e.what() << std::endl;
    }

    /* statistics */
    std::cout << "Documents: " << get_document_counter() << std::endl;
}

/*
 *  queries the index and returns the result ordered by tfidf ranking
 *  returns a sorted vector of pairs <filepath, rank>
 */
std::vector<std::pair<std::string, double>> Index::queryIndex(
    const std::vector<std::string> &input_values) {
    std::vector<std::pair<std::string, double>> result;
    /* loop over every document in the index */
    for (auto &document : documents) {
        double rank = 0.0;
        for (auto &input : input_values) {
            try {
                rank = document->get_tfidf_score(input);
            } catch (std::exception &e) {
                std::cerr << "Error getting tfidf rank of term: " << input;
                std::cerr << " in Document: " << document->get_filepath();
				std::cerr << " " << e.what() << std::endl;
            }
        }

        if (rank == 0.0) {
            continue;
        }

        result.push_back(std::make_pair(document->get_filepath(), rank));
    }

    /* Sort the result ascending by rank */
    std::sort(result.begin(), result.end(),
        [](const auto &a, const auto &b) { return a.second > b.second; });

    return result;
}

/*
 * returns the number of documents in the index
 */
int Index::get_document_counter() { return documents.size(); }

/*
 *   Moves trough a directy and try's to read every supported file in it
 *   For every supported file in the dir, a Document is created
 */
void Index::build_document_index(std::string directory) {
    /* if the param is a directory */
    if (std::filesystem::status(directory).type() == std::filesystem::file_type::directory) {
        std::cout << "Building index of directory: " << directory << std::endl;
        if (std::filesystem::exists(directory)) {
            for (auto const &entry : std::filesystem::recursive_directory_iterator(directory)) {
                std::string filepath = entry.path();
                std::string file_extension = std::filesystem::path(entry.path()).extension();

                try {
                    std::unique_ptr<Document> new_doc = DocumentFactory::create_document(filepath, file_extension);
                    new_doc->index_document();
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
 * Uses threads to read every file in the document index
 * and calculate its tfidf score per every word in the doucments
 */
void Index::build_tfidf_index() {
    std::cout << "Running build tfidf index" << std::endl;
    const auto start{std::chrono::steady_clock::now()};

    int files_per_thread = get_document_counter() / thread_num;
    for (int i = 0; i < thread_num; ++i) {
        int start_index = i * files_per_thread;
        int end_index = (i == thread_num) ? get_document_counter(): (i + 1) * files_per_thread;
        threads.emplace_back([this, start_index, end_index]() {
            this->calculate_tfidf_index(start_index, end_index);
        });
    }

    for (std::thread &thread : threads) {
        thread.join();
    }

    const auto end{std::chrono::steady_clock::now()};
    const std::chrono::duration<double> elapsed_seconds{end - start};
    std::cout << "Building tfidf index took: " << elapsed_seconds.count() << "seconds" << std::endl;
}

/*
 * calculates the tfidf for every word of every document in the
 * document index, saves the calculated values per word in a hashmap,
 * the values are accessed by filepath,
 * needs the start and end index because of threads
 * the tfidf index itself is protected by a mutex
 * To be run the build document index has to be complete
 */
void Index::calculate_tfidf_index(int start_index, int end_index) {
    for (int i = start_index; i < end_index; ++i) {
        for (auto &term : documents.at(i)->get_concordance()) {
            /* skip stop words */
            if (std::find(stopwords.begin(), stopwords.end(), term.first) !=
                stopwords.end()) {
                continue;
            }
            /* caclulating the tfidf */
            double tfidf = documents.at(i)->get_term_frequency(term.first) * inverse_doc_frequency(term.first, documents);
            documents.at(i)->insert_tfidf_score({term.first, tfidf});
        }
    }
}

/*
 * Calculates the idf for a certain term over the whole index
 */
double Index::inverse_doc_frequency(std::string term, const std::vector<std::unique_ptr<Document>> &corpus) {
    int term_count = 0;
    int n = corpus.size();

    for (auto &doc : corpus) {
        if (doc->contains_term(term)) {
            term_count++;
        }
    }

    if (term_count == 0) {
        return 0.0;
    }

    return std::log10((double)n / (double)term_count);
}

/*
 * Used to be run in specific intervall, to rebuild the index
 * checks the Document index and reindexes changed documents,
 * if a documents changed, the index also has to be rebuild
 */
void Index::rebuild_index() {
    bool reindex_tfidf = false;
    for (auto &document : documents) {
        if (document->needs_reindexing()) {
            document->index_document();
            reindex_tfidf = true;
        }
    }
    /*
     * if one document is changed the tfidf index needs to be rebuild
     * and stored on the filesystem again
     */
    if (reindex_tfidf) {
        build_tfidf_index();
    }
}

void Index::run_reindexing() {
    std::cout << "Start reindexing" << std::endl;
    rebuild_index();
}

/*
 *   read stopwords from a txt file
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

void Index::print_tfidf_index() {
    std::cout << "Printing tfidf_index" << std::endl;
    for (const auto &document: documents) {
        document->print_tfidf_scores();
    }
}