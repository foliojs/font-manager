#ifndef FONT_DESCRIPTOR_H
#define FONT_DESCRIPTOR_H
#include <node.h>
#include <v8.h>
#include <nan.h>
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

  FontDescriptor(FontDescriptor *desc) {
    path = copyString(desc->path);
    postscriptName = copyString(desc->postscriptName);
    family = copyString(desc->family);
    style = copyString(desc->style);
    weight = desc->weight;
    width = desc->width;
    italic = desc->italic;
    monospace = desc->monospace;
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
    Nan::EscapableHandleScope scope;
    Local<Object> res = Nan::New<Object>();

    if (path) {
      Nan::Set(res, Nan::New<String>("path").ToLocalChecked(), Nan::New<String>(path).ToLocalChecked());
    }
    
    if (postscriptName) {
      Nan::Set(res, Nan::New<String>("postscriptName").ToLocalChecked(), Nan::New<String>(postscriptName).ToLocalChecked());
    }
    
    if (family) {
      Nan::Set(res, Nan::New<String>("family").ToLocalChecked(), Nan::New<String>(family).ToLocalChecked());
    }
    
    if (style) {
      Nan::Set(res, Nan::New<String>("style").ToLocalChecked(), Nan::New<String>(style).ToLocalChecked());
    }
    
    Nan::Set(res, Nan::New<String>("weight").ToLocalChecked(), Nan::New<Number>(weight));
    Nan::Set(res, Nan::New<String>("width").ToLocalChecked(), Nan::New<Number>(width));
    Nan::Set(res, Nan::New<String>("italic").ToLocalChecked(), Nan::New<v8::Boolean>(italic));
    Nan::Set(res, Nan::New<String>("monospace").ToLocalChecked(), Nan::New<v8::Boolean>(monospace));
    return scope.Escape(res);
  }

private:
  char *copyString(const char *input) {
    if (!input)
      return NULL;

    char *str = new char[strlen(input) + 1];
    strcpy(str, input);
    return str;
  }

  char *getString(Local<Object> obj, const char *name) {
    Nan::HandleScope scope;
    MaybeLocal<Value> value = Nan::Get(obj, Nan::New<String>(name).ToLocalChecked());

    if (!value.IsEmpty() && value.ToLocalChecked()->IsString()) {
      return copyString(*Nan::Utf8String(value.ToLocalChecked()));
    }

    return NULL;
  }

  int getNumber(Local<Object> obj, const char *name) {
    Nan::HandleScope scope;
    MaybeLocal<Value> value = Nan::Get(obj, Nan::New<String>(name).ToLocalChecked());

    if (!value.IsEmpty() && value.ToLocalChecked()->IsNumber()) {
      return value.ToLocalChecked()->Int32Value(Nan::GetCurrentContext()).FromJust();
    }

    return 0;
  }

  bool getBool(Local<Object> obj, const char *name) {
    Nan::HandleScope scope;
    MaybeLocal<Value> value = Nan::Get(obj, Nan::New<String>(name).ToLocalChecked());

    if (!value.IsEmpty() && value.ToLocalChecked()->IsBoolean()) {
      return value.ToLocalChecked()->BooleanValue((Nan::GetCurrentContext())->GetIsolate());
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
