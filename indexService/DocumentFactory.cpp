#include <iostream>

#include "DocumentFactory.h"
#include "PDFContentStrategy.h"
#include "XMLContentStrategy.h"
#include "TextContentStrategy.h"

std::unique_ptr<Document> DocumentFactory::create_document(
    const std::string &filepath, const std::string &extension) {

    if (extension == ".xml" || extension == ".xhtml") {
        return std::make_unique<Document>(filepath, extension, std::make_unique<XMLContentStrategy>());
    }

    if (extension == ".txt") {
        return std::make_unique<Document>(filepath, extension, std::make_unique<TextContentStrategy>());
    }

    if (extension == ".pdf") {
        try {
            return std::make_unique<Document>(filepath, extension, std::make_unique<PDFContentStrategy>());
        } catch (std::exception &e) {
            std::cerr << "Exception caught creating pdf document: " << e.what();
            std::cerr << std::endl;
        }
    }

    throw std::runtime_error(std::string("Document " + filepath + " " + extension + " not supported"));
};