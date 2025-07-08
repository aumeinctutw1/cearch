#ifndef _H_DOCUMENTFACTORY
#define _H_DOCUMENTFACTORY

#include <memory>
#include <nlohmann/json.hpp>

#include "Document.h"

/* 
*   a Factory which returns Document objects depending on a file extension 
*   throws Exception if the file extension is not implemented
*   TODO: Add OpenOffice (and MS Office?) Support
*/
class DocumentFactory {
   public:
    static std::unique_ptr<Document> create_document(uint64_t docid, const std::string &filepath, const std::string &extension);
    static std::unique_ptr<Document> from_json(const nlohmann::json &j);
};

#endif