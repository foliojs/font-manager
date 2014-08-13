#include <stdlib.h>
#include <node.h>
#include <v8.h>
#include "FontDescriptor.h"

using namespace v8;

// these functions are implemented by the platform
Handle<Value> getAvailableFonts(const Arguments&);
Handle<Value> findFonts(FontDescriptor *);
Handle<Value> findFont(FontDescriptor *);
Handle<Value> substituteFont(char *, char *);

Handle<Value> findFontFn(const Arguments& args, Handle<Value> (*fn)(FontDescriptor *)) {
  HandleScope scope;
  
  if (args.Length() < 1) {
    ThrowException(Exception::TypeError(String::New("Missing font descriptor")));
    return scope.Close(Undefined());
  }
  
  if (!args[0]->IsObject()) {
    ThrowException(Exception::TypeError(String::New("Not a font descriptor")));
    return scope.Close(Undefined());
  }

  Local<Object> desc = Local<Object>::Cast(args[0]);
  FontDescriptor *descriptor = new FontDescriptor(desc);
  return scope.Close(fn(descriptor));
}

Handle<Value> findFonts(const Arguments& args) {
  return findFontFn(args, findFonts);
}

Handle<Value> findFont(const Arguments& args) {
  return findFontFn(args, findFont);
}

Handle<Value> substituteFont(const Arguments& args) {
  HandleScope scope;
  
  if (args.Length() < 2) {
    ThrowException(Exception::TypeError(String::New("Wrong number of arguments")));
    return scope.Close(Undefined());
  }
  
  if (!args[0]->IsString()) {
    ThrowException(Exception::TypeError(String::New("Expected postscript name")));
    return scope.Close(Undefined());
  }
  
  if (!args[1]->IsString()) {
    ThrowException(Exception::TypeError(String::New("Expected substitution string")));
    return scope.Close(Undefined());
  }
  
  v8::String::Utf8Value postscriptName(args[0]);
  v8::String::Utf8Value subtitutionString(args[1]);
  
  return scope.Close(substituteFont(*postscriptName, *subtitutionString));
}

void init(Handle<Object> exports) {
  exports->Set(String::NewSymbol("getAvailableFonts"), FunctionTemplate::New(getAvailableFonts)->GetFunction());
  exports->Set(String::NewSymbol("findFonts"), FunctionTemplate::New(findFonts)->GetFunction());
  exports->Set(String::NewSymbol("findFont"), FunctionTemplate::New(findFont)->GetFunction());
  exports->Set(String::NewSymbol("substituteFont"), FunctionTemplate::New(substituteFont)->GetFunction());
}

NODE_MODULE(fontmanager, init)
