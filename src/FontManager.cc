#include <stdlib.h>
#include <node.h>
#include <uv.h>
#include <v8.h>
#include "FontDescriptor.h"

using namespace v8;

// these functions are implemented by the platform
ResultSet *getAvailableFonts();
ResultSet *findFonts(FontDescriptor *);
FontDescriptor *findFont(FontDescriptor *);
FontDescriptor *substituteFont(char *, char *);

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

// converts a FontDescriptor to a JavaScript object
Handle<Value> wrapResult(FontDescriptor *result) {
  if (result == NULL)
    return Null();
  
  Local<Object> res = result->toJSObject();
  delete result;
  return res;
}

// holds data about an operation that will be 
// performed on a background thread
struct AsyncRequest {
  uv_work_t work;
  FontDescriptor *desc;           // used by findFont and findFonts
  char *postscriptName;           // used by substituteFont
  char *substitutionString;       // ditto
  FontDescriptor *result;         // for functions with a single result
  ResultSet *results;             // for functions with multiple results
  Persistent<Function> callback;  // the actual JS callback to call when we are done
  
  AsyncRequest(Local<Value> v) {
    work.data = (void *)this;
    callback = Persistent<Function>::New(Local<Function>::Cast(v));
    desc = NULL;
    postscriptName = NULL;
    substitutionString = NULL;
    result = NULL;
    results = NULL;
  }
  
  ~AsyncRequest() {
    callback.Dispose();
    
    if (desc)
      delete desc;
    
    if (postscriptName)
      delete postscriptName;
    
    if (substitutionString)
      delete substitutionString;
    
    // result/results deleted by wrapResult/collectResults respectively
  }
};

// calls the JavaScript callback for a request
void asyncCallback(uv_work_t *work, int status) {
  AsyncRequest *req = (AsyncRequest *) work->data;
  Handle<Value> args[1];
  
  if (req->results) {
    args[0] = collectResults(req->results);
  } else if (req->result) {
    args[0] = wrapResult(req->result);
  } else {
    args[0] = Null();
  }
  
  req->callback->Call(Context::GetCurrent()->Global(), 1, args);
  delete req;
}

void getAvailableFontsAsync(uv_work_t *work) {
  AsyncRequest *req = (AsyncRequest *) work->data;
  req->results = getAvailableFonts();
}

template<bool async>
Handle<Value> getAvailableFonts(const Arguments& args) {
  HandleScope scope;
  
  if (async) {
    if (args.Length() < 1 || !args[0]->IsFunction()) {
      ThrowException(Exception::TypeError(String::New("Expected a callback")));
      return scope.Close(Undefined());
    }
    
    AsyncRequest *req = new AsyncRequest(args[0]);
    uv_queue_work(uv_default_loop(), &req->work, getAvailableFontsAsync, asyncCallback);
    
    return scope.Close(Undefined());
  } else {
    return scope.Close(collectResults(getAvailableFonts()));
  }
}

void findFontsAsync(uv_work_t *work) {
  AsyncRequest *req = (AsyncRequest *) work->data;
  req->results = findFonts(req->desc);
}

template<bool async>
Handle<Value> findFonts(const Arguments& args) {
  HandleScope scope;
  
  if (args.Length() < 1 || !args[0]->IsObject() || args[0]->IsFunction()) {
    ThrowException(Exception::TypeError(String::New("Expected a font descriptor")));
    return scope.Close(Undefined());
  }
  
  Local<Object> desc = Local<Object>::Cast(args[0]);
  FontDescriptor *descriptor = new FontDescriptor(desc);
  
  if (async) {
    if (args.Length() < 2 || !args[1]->IsFunction()) {
      ThrowException(Exception::TypeError(String::New("Expected a callback")));
      return scope.Close(Undefined());
    }
    
    AsyncRequest *req = new AsyncRequest(args[1]);
    req->desc = descriptor;
    uv_queue_work(uv_default_loop(), &req->work, findFontsAsync, asyncCallback);
    
    return scope.Close(Undefined());
  } else {
    Local<Object> res = collectResults(findFonts(descriptor));
    delete descriptor;
    return scope.Close(res);
  }
}

void findFontAsync(uv_work_t *work) {
  AsyncRequest *req = (AsyncRequest *) work->data;
  req->result = findFont(req->desc);
}

template<bool async>
Handle<Value> findFont(const Arguments& args) {  
  HandleScope scope;
  
  if (args.Length() < 1 || !args[0]->IsObject() || args[0]->IsFunction()) {
    ThrowException(Exception::TypeError(String::New("Expected a font descriptor")));
    return scope.Close(Undefined());
  }

  Local<Object> desc = Local<Object>::Cast(args[0]);
  FontDescriptor *descriptor = new FontDescriptor(desc);
  
  if (async) {
    if (args.Length() < 2 || !args[1]->IsFunction()) {
      ThrowException(Exception::TypeError(String::New("Expected a callback")));
      return scope.Close(Undefined());
    }
    
    AsyncRequest *req = new AsyncRequest(args[1]);
    req->desc = descriptor;
    uv_queue_work(uv_default_loop(), &req->work, findFontAsync, asyncCallback);
    
    return scope.Close(Undefined());
  } else {
    Handle<Value> res = wrapResult(findFont(descriptor));
    delete descriptor;
    return scope.Close(res);
  }
}

void substituteFontAsync(uv_work_t *work) {
  AsyncRequest *req = (AsyncRequest *) work->data;
  req->result = substituteFont(req->postscriptName, req->substitutionString);
}

template<bool async>
Handle<Value> substituteFont(const Arguments& args) {
  HandleScope scope;
  
  if (args.Length() < 1 || !args[0]->IsString()) {
    ThrowException(Exception::TypeError(String::New("Expected postscript name")));
    return scope.Close(Undefined());
  }
  
  if (args.Length() < 2 || !args[1]->IsString()) {
    ThrowException(Exception::TypeError(String::New("Expected substitution string")));
    return scope.Close(Undefined());
  }
  
  v8::String::Utf8Value postscriptName(args[0]);
  v8::String::Utf8Value substitutionString(args[1]);
    
  if (async) {
    if (args.Length() < 3 || !args[2]->IsFunction()) {
      ThrowException(Exception::TypeError(String::New("Expected a callback")));
      return scope.Close(Undefined());
    }
    
    // copy the strings since the JS garbage collector might run before the async request is finished
    char *ps = new char[postscriptName.length() + 1];
    strcpy(ps, *postscriptName);
  
    char *sub = new char[substitutionString.length() + 1];
    strcpy(sub, *substitutionString);
    
    AsyncRequest *req = new AsyncRequest(args[2]);
    req->postscriptName = ps;
    req->substitutionString = sub;
    uv_queue_work(uv_default_loop(), &req->work, substituteFontAsync, asyncCallback);
    
    return scope.Close(Undefined());
  } else {
    return scope.Close(wrapResult(substituteFont(*postscriptName, *substitutionString)));
  }
}

void init(Handle<Object> exports) {
  NODE_SET_METHOD(exports, "getAvailableFonts", getAvailableFonts<true>);
  NODE_SET_METHOD(exports, "getAvailableFontsSync", getAvailableFonts<false>);
  NODE_SET_METHOD(exports, "findFonts", findFonts<true>);
  NODE_SET_METHOD(exports, "findFontsSync", findFonts<false>);
  NODE_SET_METHOD(exports, "findFont", findFont<true>);
  NODE_SET_METHOD(exports, "findFontSync", findFont<false>);
  NODE_SET_METHOD(exports, "substituteFont", substituteFont<true>);
  NODE_SET_METHOD(exports, "substituteFontSync", substituteFont<false>);
}

NODE_MODULE(fontmanager, init)
