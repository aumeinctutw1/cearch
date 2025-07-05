#ifndef _H_PDFCONTENTSTRATEGY
#define _H_PDFCONTENTSTRATEGY

#include "ContentStrategy.h"

#include <poppler/cpp/poppler-document.h>
#include <poppler/cpp/poppler-page.h>

class PDFContentStrategy: public ContentStrategy {
   public:
    explicit PDFContentStrategy() {};

    /*
    *   Actual implementation of the read content function for PDF's
    *   const &document -> function cant change the document passed in
    *   const -> after a function, const means the function cant change any Data members,
    *   of the class it belongs to (PDFContentStrategy)
    */
    std::string read_content(const std::string &filepath) const;
};

#endif