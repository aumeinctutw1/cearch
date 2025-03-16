#ifndef _H_CONTENTSTRATEGY
#define _H_CONTENTSTRATEGY

#include <string>

class ContentStrategy {
   public:
    virtual ~ContentStrategy() = default;
    virtual std::string read_content(const std::string &filepath) const = 0;
};

#endif