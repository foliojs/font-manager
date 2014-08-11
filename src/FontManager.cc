#include <node.h>
#include <v8.h>
#include "FontDescriptor.h"

using namespace v8;

Handle<Value> getAvailableFonts(const Arguments&);
char *findFont(FontDescriptor *);

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
  char *url = findFont(descriptor);
  
  if (url) {
    Local<String> str = String::New(url);
    free(url);
    return scope.Close(str);
  }
  
  return scope.Close(Null());
}

void init(Handle<Object> exports) {
  exports->Set(String::NewSymbol("getAvailableFonts"), FunctionTemplate::New(getAvailableFonts)->GetFunction());
  exports->Set(String::NewSymbol("findFont"), FunctionTemplate::New(findFont)->GetFunction());
}

NODE_MODULE(fontmanager, init)
