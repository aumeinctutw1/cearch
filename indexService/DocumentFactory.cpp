#include <iostream>

#include "DocumentFactory.h"
#include "PDFContentStrategy.h"
#include "XMLContentStrategy.h"
#include "TextContentStrategy.h"

std::unique_ptr<Document> DocumentFactory::create_document(uint64_t docid, const std::string &filepath, const std::string &extension) {

    if (extension == ".xml" || extension == ".xhtml") {
        return std::make_unique<Document>(docid, filepath, extension, std::make_unique<XMLContentStrategy>());
    }

    if (extension == ".txt") {
        return std::make_unique<Document>(docid, filepath, extension, std::make_unique<TextContentStrategy>());
    }

    if (extension == ".pdf") {
        try {
            return std::make_unique<Document>(docid, filepath, extension, std::make_unique<PDFContentStrategy>());
        } catch (std::exception &e) {
            std::cerr << "Exception caught creating pdf document: " << e.what();
            std::cerr << std::endl;
        }
    }

    throw std::runtime_error(std::string("Document " + filepath + " " + extension + " not supported"));
};

std::unique_ptr<Document> DocumentFactory::from_json(const nlohmann::json &j) {
    uint64_t docid = j.at("docid");
    std::string filepath = j.at("filepath");
    std::string extension = j.at("file_extension");

    std::unique_ptr<ContentStrategy> strategy;
    if (extension == ".xml" || extension == ".xhtml") {
        strategy = std::make_unique<XMLContentStrategy>();
    } else if (extension == ".txt") {
        strategy = std::make_unique<TextContentStrategy>();
    } else if (extension == ".pdf") {
        strategy = std::make_unique<PDFContentStrategy>();
    } else {
        throw std::runtime_error("Unsupported file extension in JSON: " + extension);
    }

    auto doc = std::make_unique<Document>(docid, filepath, extension, std::move(strategy));

    doc->set_concordance(j.at("concordance").get<std::unordered_map<std::string, int>>());
    doc->set_total_term_count(j.at("total_term_count"));
    auto seconds_since_epoch = j.at("indexed_at").get<int64_t>();
    doc->set_indexed_at(std::chrono::system_clock::time_point(std::chrono::seconds(seconds_since_epoch)));

    return doc;
}