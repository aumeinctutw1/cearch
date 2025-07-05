#ifndef _H_DOCUMENTFACTORY
#define _H_DOCUMENTFACTORY

#include <memory>

#include "Document.h"

/* 
*   a Factory which returns Document objects depending on a file extension 
*   throws Exception if the file extension is not implemented
*/
class DocumentFactory {
   public:
    static std::unique_ptr<Document> create_document(
        const std::string &filepath, const std::string &extension);
};

#endif