#ifndef FONT_MANAGER_RESULT_H
#define FONT_MANAGER_RESULT_H
#include <v8.h>

using namespace v8;
using namespace node;

Local<Object> createResult(const char *path, const char *postscriptName) {
  Local<Object> res = Object::New();

  res->Set(String::NewSymbol("path"), String::New(path));
  res->Set(String::NewSymbol("postscriptName"), String::New(postscriptName));

  return res;
}

Local<Object> createResult(const uint16_t *path, const uint16_t *postscriptName) {
  Local<Object> res = Object::New();

  res->Set(String::NewSymbol("path"), String::New(path));
  res->Set(String::NewSymbol("postscriptName"), String::New(postscriptName));

  return res;
}

#endif
