#include <filesystem>
#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#include <algorithm>

#include "Document.h"

/* Base Document Class */
Document::Document(uint64_t docid, std::string filepath, std::string file_extension, std::unique_ptr<ContentStrategy> strategy)
    :m_docid(docid), filepath(filepath), file_extension(file_extension), strategy_(std::move(strategy))
{
}

void Document::set_concordance(std::unordered_map<std::string, int> concordance_) {
    concordance = std::move(concordance_);
}

void Document::set_total_term_count(int term_count) {
    m_total_term_count = term_count;
}

void Document::set_indexed_at(std::chrono::system_clock::time_point time) {
    indexed_at = time;
}

/* concordence contains every term in the document and its counter */
std::unordered_map<std::string, int> Document::get_concordance() {
    return concordance;
}

uint64_t Document::get_docid() {
    return m_docid;
}

int Document::get_total_term_count() {
    return m_total_term_count;
}

std::string Document::get_filepath() const { return filepath; }

std::string Document::get_extension() { return file_extension; }

/* number of times, a word occurs in a given document */
int Document::get_term_frequency(const std::string &term) {
    if (concordance.find(term) != concordance.end()) {
        return concordance.at(term);
    }

    return 0;
}

std::string Document::get_file_content_as_string() {
    std::string file_content = read_content();
    return file_content;
}

bool Document::contains_term(const std::string &term) {
    if (concordance.find(term) != concordance.end()) {
        return true;
    }
    return false;
}

void Document::index_document() {
    std::string word;
    std::istringstream iss(read_content());

    std::cout << "Indexing doc : " << this->get_filepath() << std::endl;

    while (iss >> word) {
        /* split the word if necessary */
        std::vector<std::string> clean_words = clean_word(word);
        for (auto &clean_word : clean_words) {
            if (clean_word.find_first_not_of(' ') != std::string::npos) {
                if (concordance.find(clean_word) != concordance.end()) {
                    concordance[clean_word]++;
                } else {
                    concordance[clean_word] = 1;
                }
                m_total_term_count++;
            }
        }
    }

    indexed_at = std::chrono::system_clock::now();

    /* TODO: Update in Filesystem? */
}

std::string Document::read_content() {
    return strategy_->read_content(filepath);
}

/*
*   TODO: detect word that end with 's, remove the 's
*/
std::vector<std::string> Document::clean_word(std::string &word) {
    std::vector<std::string> clean_words;

    /* transform every word to lower case letters */
    std::transform(word.begin(), word.end(), word.begin(),
                   [](unsigned char c) { return std::tolower(c); });

    /* keep only alphabetic characters, and split words by non alpha numeric chars */
    std::string current_word;
    for (unsigned char c: word) {
        if (std::isalpha(c)) {
            current_word += c;
        } else {
            clean_words.push_back(current_word);
            current_word.clear();
        }
    }

    /* if the clean string is not empty, add to result */
    if (!current_word.empty()) {
        clean_words.push_back(current_word);
    }

    return clean_words;
}

nlohmann::json Document::to_json() const {
    return {
        {"docid", m_docid},
        {"filepath", filepath},
        {"file_extension", file_extension},
        {"total_term_count", m_total_term_count},
        {"concordance", concordance},
        {"indexed_at", std::chrono::duration_cast<std::chrono::seconds>(
            indexed_at.time_since_epoch()).count()}
    };
}