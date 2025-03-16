#include "PDFContentStrategy.h"

std::string PDFContentStrategy::read_content(const std::string &filepath) const {
    std::string fulltext;

    std::unique_ptr<poppler::document> doc{poppler::document::load_from_file(filepath)};

    if (!doc) {
        throw std::runtime_error("Error: Could not open the PDF file!");
    }

    for (int i = 0; i < doc->pages(); ++i) {
        std::unique_ptr<poppler::page> page(doc->create_page(i));
        if (page) {
            fulltext.append(page->text().to_latin1());
            fulltext.append("\n");
        }
    }

    return fulltext;
}