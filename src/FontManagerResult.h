#ifndef FONT_MANAGER_RESULT_H
#define FONT_MANAGER_RESULT_H
#include <v8.h>
#include <vector>
#include <string>

using namespace v8;

class FontManagerResult {
public:
  FontManagerResult(const char *path, const char *postscriptName) {
    this->path = path;
    this->postscriptName = postscriptName;
  }
  
  Local<Object> toJSObject() {
    Local<Object> res = Object::New();
    res->Set(String::NewSymbol("path"), String::New(path.c_str()));
    res->Set(String::NewSymbol("postscriptName"), String::New(postscriptName.c_str()));
    return res;
  }
  
private:
  std::string path;
  std::string postscriptName;
};

class ResultSet : public std::vector<FontManagerResult *> {
public:
  ~ResultSet() {
    for (ResultSet::iterator it = this->begin(); it != this->end(); it++) {
      delete *it;
    }
  }
};

#endif
