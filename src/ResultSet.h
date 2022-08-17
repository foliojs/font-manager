#ifndef RESULT_SET_H
#define RESULT_SET_H

#include <vector>

#include "FontDescriptor.h"

// This is an owning container
class ResultSet : public std::vector<FontDescriptor *> {
public:
  ResultSet() {}
  ResultSet (const ResultSet&) = delete;
  ResultSet& operator= (const ResultSet&) = delete;
  ResultSet (ResultSet&&) = delete;
  ResultSet& operator= (ResultSet&&) = delete;

  ~ResultSet() {
    for (FontDescriptor *fd : *this) {
      delete fd;
    }
  }
};

#endif