#ifndef RESULT_SET_H
#define RESULT_SET_H

#include <vector>

#include "FontDescriptor.h"

class ResultSet : public std::vector<FontDescriptor *> {
public:
  ~ResultSet() {
    for (FontDescriptor *fd : *this) {
      delete fd;
    }
  }
};

#endif