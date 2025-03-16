#ifndef _TEXTDOCUMENT_H
#define _TEXTDOCUMENT_H

#include "ContentStrategy.h"

class TextContentStrategy: public ContentStrategy{
   public:
    TextContentStrategy();
    std::string read_content(const std::string &filepath) const override;

   private:
};

#endif 