#ifndef FONT_DESCRIPTOR_H
#define FONT_DESCRIPTOR_H
#include <node.h>
#include <v8.h>
#include <stdlib.h>
#include <string.h>

using namespace v8;

enum FontWeight {
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
  char *postscriptName;
  char *family;
  char *style;
  FontWeight weight;
  FontWidth width;
  bool italic;
  bool monospace;
  
  FontDescriptor(Local<Object> obj) {
    postscriptName = getString(obj, "postscriptName");
    family = getString(obj, "family");
    style = getString(obj, "style");
    weight = (FontWeight) getNumber(obj, "weight");
    width = (FontWidth) getNumber(obj, "width");
    italic = getBool(obj, "italic");
    monospace = getBool(obj, "monospace");
  }

  FontDescriptor() {
    postscriptName = NULL;
    family = NULL;
    style = NULL;
  }
  
  ~FontDescriptor() {
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
  
private:
  char *getString(Local<Object> obj, const char *name) {
    Local<Value> value = obj->Get(String::New(name));
    
    if (value->IsString()) {
      v8::String::Utf8Value string(value);
      char *str = new char[string.length() + 1];
      strcpy(str, *string);
      return str;
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

#endif
