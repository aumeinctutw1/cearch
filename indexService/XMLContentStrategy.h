#ifndef _H_XMLCONTENTSTRATEGY
#define _H_XMLCONTENTSTRATEGY

#include "ContentStrategy.h"

/* XML Parsing */
#include <pugixml.hpp>

class XMLContentStrategy : public ContentStrategy {
   public:
    XMLContentStrategy();
    std::string read_content(const std::string &filepath) const;

   private:
    /* helper function to traverse every node in a xml file */
    void traverse_nodes(const pugi::xml_node &root_node, std::string &content) const;
};

#endif