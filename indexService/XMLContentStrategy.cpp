#include <iostream>
#include <queue>

#include "XMLContentStrategy.h"

/* XML Specific Documents */
XMLContentStrategy::XMLContentStrategy() {}

std::string XMLContentStrategy::read_content(const std::string &filepath) const {
    pugi::xml_document doc;
    std::string file_content;

    if (!doc.load_file(filepath.c_str())) {
        std::cerr << "failed to load xml file" << std::endl;
    }

    traverse_nodes(doc.document_element(), file_content);
    return file_content;
}

void XMLContentStrategy::traverse_nodes(const pugi::xml_node &root_node, std::string &content) const {
    std::queue<pugi::xml_node> node_queue;
    node_queue.push(root_node);

    while (!node_queue.empty()) {
        pugi::xml_node current_node = node_queue.front();
        node_queue.pop();

        /* process the current node */
        content.append(current_node.value());
        content.append(" ");

        for (pugi::xml_node child_node = current_node.first_child(); child_node;
             child_node = child_node.next_sibling()) {
            node_queue.push(child_node);
        }
    }
}