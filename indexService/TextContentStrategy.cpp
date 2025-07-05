#include <fstream>

#include "TextContentStrategy.h"

TextContentStrategy::TextContentStrategy() {}

std::string TextContentStrategy::read_content(const std::string &filepath) const {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file: " + filepath);
    }

    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    return content;
}