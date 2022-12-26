#include "FontDescriptor.h"

char *copyString(const char *input) {
  if (!input)
    return NULL;

  char *str = new char[strlen(input) + 1];
  strcpy(str, input);
  return str;
}

char *getString(Napi::Env env, Napi::Object obj, const char *name) {
  Napi::HandleScope scope(env);
  Napi::Value value;

  if (!obj.Get(name).UnwrapTo(&value) || !value.IsString()) {
    return NULL;
  }

  return copyString(value.As<Napi::String>().Utf8Value().c_str());
}

int getNumber(Napi::Env env, Napi::Object obj, const char *name) {
  Napi::HandleScope scope(env);
  Napi::Value value;

  if (!obj.Get(name).UnwrapTo(&value) || !value.IsNumber()) {
    return 0;
  }

  return value.As<Napi::Number>().Int32Value();
}

bool getBool(Napi::Env env, Napi::Object obj, const char *name) {
  Napi::HandleScope scope(env);
  Napi::Value value;

  if (!obj.Get(name).UnwrapTo(&value) || !value.IsBoolean()) {
    return false;
  }

  return value.As<Napi::Boolean>().Value();
}

FontDescriptor::FontDescriptor(Napi::Env env, Napi::Object obj) {
  path = NULL;
  postscriptName = getString(env, obj, "postscriptName");
  family = getString(env, obj, "family");
  localizedName = getString(env, obj, "localizedName");
  enName = getString(env, obj, "enName");
  style = getString(env, obj, "style");
  weight = (FontWeight) getNumber(env, obj, "weight");
  width = (FontWidth) getNumber(env, obj, "width");
  italic = getBool(env, obj, "italic");
  monospace = getBool(env, obj, "monospace");
}

FontDescriptor::FontDescriptor() {
  path = NULL;
  postscriptName = NULL;
  family = NULL;
  localizedName = NULL;
  enName = NULL;
  style = NULL;
  weight = FontWeightUndefined;
  width = FontWidthUndefined;
  italic = false;
  monospace = false;
}

FontDescriptor::FontDescriptor(
  const char *path,
  const char *postscriptName,
  const char *family,
  const char *localizedName,
  const char *enName,
  const char *style,
  FontWeight weight,
  FontWidth width,
  bool italic,
  bool monospace
) {
  this->path = copyString(path);
  this->postscriptName = copyString(postscriptName);
  this->family = copyString(family);
  this->localizedName = copyString(localizedName);
  this->enName = copyString(enName);
  this->style = copyString(style);
  this->weight = weight;
  this->width = width;
  this->italic = italic;
  this->monospace = monospace;
}

FontDescriptor::FontDescriptor(FontDescriptor *desc) {
  path = copyString(desc->path);
  postscriptName = copyString(desc->postscriptName);
  family = copyString(desc->family);
  localizedName = copyString(desc->localizedName);
  enName = copyString(desc->enName);
  style = copyString(desc->style);
  weight = desc->weight;
  width = desc->width;
  italic = desc->italic;
  monospace = desc->monospace;
}

FontDescriptor::~FontDescriptor() {
  if (path)
    delete path;

  if (postscriptName)
    delete postscriptName;

  if (family)
    delete family;

  if (localizedName)
    delete localizedName;

  if (enName)
    delete enName;

  if (style)
    delete style;

  postscriptName = NULL;
  family = NULL;
  style = NULL;
}

Napi::Object FontDescriptor::toJSObject(Napi::Env env) {
  Napi::EscapableHandleScope scope(env);
  Napi::Object res = Napi::Object::New(env);
  if (path) {
    res.Set(Napi::String::New(env, "path"), Napi::String::New(env, path));
  }

  if (postscriptName) {
    res.Set(Napi::String::New(env, "postscriptName"), Napi::String::New(env, postscriptName));
  }

  if (family) {
    res.Set(Napi::String::New(env, "family"), Napi::String::New(env, family));
  }

  if (localizedName) {
    res.Set(Napi::String::New(env, "localizedName"), Napi::String::New(env, localizedName));
  }

  if (enName) {
    res.Set(Napi::String::New(env, "enName"), Napi::String::New(env, enName));
  }

  if (style) {
    res.Set(Napi::String::New(env, "style"), Napi::String::New(env, style));
  }

  res.Set(Napi::String::New(env, "weight"), Napi::Number::New(env, weight));
  res.Set(Napi::String::New(env, "width"), Napi::Number::New(env, width));
  res.Set(Napi::String::New(env, "italic"), Napi::Boolean::New(env, italic));
  res.Set(Napi::String::New(env, "monospace"), Napi::Boolean::New(env, monospace));
  return scope.Escape(res).As<Napi::Object>();
}
