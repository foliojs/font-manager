#include <stdlib.h>
#include <node.h>
#include <v8.h>
#include "FontDescriptor.h"
#include "FontManagerResult.h"

using namespace v8;

// these functions are implemented by the platform
ResultSet *getAvailableFonts();
ResultSet *findFonts(FontDescriptor *);
FontManagerResult *findFont(FontDescriptor *);
FontManagerResult *substituteFont(char *, char *);

// converts a ResultSet to a JavaScript array
Local<Array> collectResults(ResultSet *results) {
  Local<Array> res = Array::New(results->size());
    
  int i = 0;
  for (ResultSet::iterator it = results->begin(); it != results->end(); it++) {
    res->Set(i++, (*it)->toJSObject());
  }
  
  delete results;
  return res;
}

// converts a FontManagerResult to a JavaScript object
Local<Object> wrapResult(FontManagerResult *result) {
  Local<Object> res = result->toJSObject();
  delete result;
  return res;
}

Handle<Value> getAvailableFonts(const Arguments& args) {
  HandleScope scope;
  return scope.Close(collectResults(getAvailableFonts()));
}

Handle<Value> findFonts(const Arguments& args) {
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
  return scope.Close(collectResults(findFonts(descriptor)));
}

Handle<Value> findFont(const Arguments& args) {  
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
  return scope.Close(wrapResult(findFont(descriptor)));
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
  
  return scope.Close(wrapResult(substituteFont(*postscriptName, *subtitutionString)));
}

void init(Handle<Object> exports) {
  NODE_SET_METHOD(exports, "getAvailableFonts", getAvailableFonts);
  NODE_SET_METHOD(exports, "findFonts", findFonts);
  NODE_SET_METHOD(exports, "findFont", findFont);
  NODE_SET_METHOD(exports, "substituteFont", substituteFont);
}

NODE_MODULE(fontmanager, init)
