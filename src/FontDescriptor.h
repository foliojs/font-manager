#ifndef FONT_DESCRIPTOR_H
#define FONT_DESCRIPTOR_H
#include <node.h>
#include <v8.h>
#include <stdlib.h>
#include <string.h>
#include <vector>

using namespace v8;

enum FontWeight {
  FontWeightUndefined   = 0,
  FontWeightThin        = 100,
  FontWeightUltraLight  = 200,
  FontWeightLight       = 300,
  FontWeightNormal      = 400,
  FontWeightMedium      = 500,
  FontWeightSemiBold    = 600,
  FontWeightBold        = 700,
  FontWeightUltraBold   = 800,
  FontWeightHeavy       = 900
};

enum FontWidth {
  FontWidthUndefined      = 0,
  FontWidthUltraCondensed = 1,
  FontWidthExtraCondensed = 2,
  FontWidthCondensed      = 3,
  FontWidthSemiCondensed  = 4,
  FontWidthNormal         = 5,
  FontWidthSemiExpanded   = 6,
  FontWidthExpanded       = 7,
  FontWidthExtraExpanded  = 8,
  FontWidthUltraExpanded  = 9
};

struct FontDescriptor {
public:
  const char *path;
  const char *postscriptName;
  const char *family;
  const char *style;
  FontWeight weight;
  FontWidth width;
  bool italic;
  bool monospace;
  
  FontDescriptor(Local<Object> obj) {
    path = NULL;
    postscriptName = getString(obj, "postscriptName");
    family = getString(obj, "family");
    style = getString(obj, "style");
    weight = (FontWeight) getNumber(obj, "weight");
    width = (FontWidth) getNumber(obj, "width");
    italic = getBool(obj, "italic");
    monospace = getBool(obj, "monospace");
  }

  FontDescriptor() {
    path = NULL;
    postscriptName = NULL;
    family = NULL;
    style = NULL;
    weight = FontWeightUndefined;
    width = FontWidthUndefined;
    italic = false;
    monospace = false;
  }
  
  FontDescriptor(const char *path, const char *postscriptName, const char *family, const char *style, 
                 FontWeight weight, FontWidth width, bool italic, bool monospace) {
    this->path = copyString(path);
    this->postscriptName = copyString(postscriptName);
    this->family = copyString(family);
    this->style = copyString(style);
    this->weight = weight;
    this->width = width;
    this->italic = italic;
    this->monospace = monospace;
  }
  
  ~FontDescriptor() {
    if (path)
      delete path;
    
    if (postscriptName)
      delete postscriptName;
    
    if (family)
      delete family;
    
    if (style)
      delete style;
    
    postscriptName = NULL;
    family = NULL;
    style = NULL;
  }
  
  Local<Object> toJSObject() {
    Local<Object> res = Object::New();
    res->Set(String::NewSymbol("path"), String::New(path));
    res->Set(String::NewSymbol("postscriptName"), String::New(postscriptName));
    res->Set(String::NewSymbol("family"), String::New(family));
    res->Set(String::NewSymbol("style"), String::New(style));
    res->Set(String::NewSymbol("weight"), Uint32::New(weight));
    res->Set(String::NewSymbol("width"), Uint32::New(width));
    res->Set(String::NewSymbol("italic"), v8::Boolean::New(italic));
    res->Set(String::NewSymbol("monospace"), v8::Boolean::New(monospace));
    return res;
  }
  
private:
  char *copyString(const char *input) {
    char *str = new char[strlen(input) + 1];
    strcpy(str, input);
    return str;
  }
  
  char *getString(Local<Object> obj, const char *name) {
    Local<Value> value = obj->Get(String::New(name));
    
    if (value->IsString()) {
      v8::String::Utf8Value string(value);
      return copyString(*string);
    }
  
    return NULL;
  }
  
  int getNumber(Local<Object> obj, const char *name) {
    Local<Value> value = obj->Get(String::New(name));
    
    if (value->IsNumber()) {
      return value->Int32Value();
    }
    
    return 0;
  }
  
  bool getBool(Local<Object> obj, const char *name) {
    Local<Value> value = obj->Get(String::New(name));
    
    if (value->IsBoolean()) {
      return value->BooleanValue();
    }
    
    return false;
  }
};

class ResultSet : public std::vector<FontDescriptor *> {
public:
  ~ResultSet() {
    for (ResultSet::iterator it = this->begin(); it != this->end(); it++) {
      delete *it;
    }
  }
};

#endif
