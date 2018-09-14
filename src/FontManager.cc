#include <stdlib.h>
#include <node.h>
#include <uv.h>
#include <v8.h>
#include <nan.h>
#include "FontDescriptor.h"

using namespace v8;

// these functions are implemented by the platform
long getAvailableFonts(ResultSet **);
long findFonts(ResultSet **, FontDescriptor *);
long findFont(FontDescriptor **, FontDescriptor *);
long substituteFont(FontDescriptor **, char *, char *);

// converts a ResultSet to a JavaScript array
Local<Array> collectResults(ResultSet *results) {
  Nan::EscapableHandleScope scope;
  Local<Array> res = Nan::New<Array>(results->size());

  int i = 0;
  for (ResultSet::iterator it = results->begin(); it != results->end(); it++) {
    res->Set(i++, (*it)->toJSObject());
  }

  delete results;
  return scope.Escape(res);
}

// converts a FontDescriptor to a JavaScript object
Local<Value> wrapResult(FontDescriptor *result) {
  Nan::EscapableHandleScope scope;
  if (result == NULL)
    return scope.Escape(Nan::Null());

  Local<Object> res = result->toJSObject();
  delete result;
  return scope.Escape(res);
}

// holds data about an operation that will be
// performed on a background thread
struct AsyncRequest {
  uv_work_t work;
  FontDescriptor *desc;     // used by findFont and findFonts
  char *postscriptName;     // used by substituteFont
  char *substitutionString; // ditto
  FontDescriptor *result;   // for functions with a single result
  ResultSet *results;       // for functions with multiple results
  long error;               // result/results is defined if error == 0
  Nan::Callback *callback;  // the actual JS callback to call when we are done

  AsyncRequest(Local<Value> v) {
    work.data = (void *)this;
    callback = new Nan::Callback(v.As<Function>());
    desc = NULL;
    postscriptName = NULL;
    substitutionString = NULL;
    result = NULL;
    results = NULL;
  }

  ~AsyncRequest() {
    delete callback;

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
void asyncCallback(uv_work_t *work) {
  Nan::HandleScope scope;
  AsyncRequest *req = (AsyncRequest *) work->data;
  Nan::AsyncResource async("asyncCallback");
  Local<Value> info[2] = {Nan::Null(), Nan::Null()};

  if (req->error == 0) {
    if (req->results) {
      info[1] = collectResults(req->results);
    } else if (req->result) {
      info[1] = wrapResult(req->result);
    }
  } else {
    info[0] = Nan::ErrnoException(req->error);
  }

  req->callback->Call(2, info, &async);
  delete req;
}

void getAvailableFontsAsync(uv_work_t *work) {
  AsyncRequest *req = (AsyncRequest *) work->data;
  req->error = getAvailableFonts(&req->results);
}

template<bool async>
NAN_METHOD(getAvailableFonts) {
  if (async) {
    if (info.Length() < 1 || !info[0]->IsFunction())
      return Nan::ThrowTypeError("Expected a callback");

    AsyncRequest *req = new AsyncRequest(info[0]);
    uv_queue_work(uv_default_loop(), &req->work, getAvailableFontsAsync, (uv_after_work_cb) asyncCallback);

    return;
  } else {
    ResultSet *results = NULL;
    long error = getAvailableFonts(&results);
    if (error != 0) {
      return Nan::ThrowError(Nan::ErrnoException(error));
    }

    info.GetReturnValue().Set(collectResults(results));
  }
}

void findFontsAsync(uv_work_t *work) {
  AsyncRequest *req = (AsyncRequest *) work->data;
  req->error = findFonts(&req->results, req->desc);
}

template<bool async>
NAN_METHOD(findFonts) {
  if (info.Length() < 1 || !info[0]->IsObject() || info[0]->IsFunction())
    return Nan::ThrowTypeError("Expected a font descriptor");

  Local<Object> desc = info[0].As<Object>();
  FontDescriptor *descriptor = new FontDescriptor(desc);

  if (async) {
    if (info.Length() < 2 || !info[1]->IsFunction())
      return Nan::ThrowTypeError("Expected a callback");

    AsyncRequest *req = new AsyncRequest(info[1]);
    req->desc = descriptor;
    uv_queue_work(uv_default_loop(), &req->work, findFontsAsync, (uv_after_work_cb) asyncCallback);

    return;
  } else {
    ResultSet *results = NULL;
    long error = findFonts(&results, descriptor);
    if (error != 0) {
      return Nan::ThrowError(Nan::ErrnoException(error));
    }

    Local<Object> res = collectResults(results);
    delete descriptor;
    info.GetReturnValue().Set(res);
  }
}

void findFontAsync(uv_work_t *work) {
  AsyncRequest *req = (AsyncRequest *) work->data;
  req->error = findFont(&req->result, req->desc);
}

template<bool async>
NAN_METHOD(findFont) {
  if (info.Length() < 1 || !info[0]->IsObject() || info[0]->IsFunction())
    return Nan::ThrowTypeError("Expected a font descriptor");

  Local<Object> desc = info[0].As<Object>();
  FontDescriptor *descriptor = new FontDescriptor(desc);

  if (async) {
    if (info.Length() < 2 || !info[1]->IsFunction())
      return Nan::ThrowTypeError("Expected a callback");

    AsyncRequest *req = new AsyncRequest(info[1]);
    req->desc = descriptor;
    uv_queue_work(uv_default_loop(), &req->work, findFontAsync, (uv_after_work_cb) asyncCallback);

    return;
  } else {
    FontDescriptor *result = NULL;
    long error = findFont(&result, descriptor);
    if (error != 0) {
      return Nan::ThrowError(Nan::ErrnoException(error));
    }

    Local<Value> res = wrapResult(result);
    delete descriptor;
    info.GetReturnValue().Set(res);
  }
}

void substituteFontAsync(uv_work_t *work) {
  AsyncRequest *req = (AsyncRequest *) work->data;
  req->error = substituteFont(&req->result, req->postscriptName, req->substitutionString);
}

template<bool async>
NAN_METHOD(substituteFont) {
  if (info.Length() < 1 || !info[0]->IsString())
    return Nan::ThrowTypeError("Expected postscript name");

  if (info.Length() < 2 || !info[1]->IsString())
    return Nan::ThrowTypeError("Expected substitution string");

  Nan::Utf8String postscriptName(info[0]);
  Nan::Utf8String substitutionString(info[1]);

  if (async) {
    if (info.Length() < 3 || !info[2]->IsFunction())
      return Nan::ThrowTypeError("Expected a callback");

    // copy the strings since the JS garbage collector might run before the async request is finished
    char *ps = new char[postscriptName.length() + 1];
    strcpy(ps, *postscriptName);

    char *sub = new char[substitutionString.length() + 1];
    strcpy(sub, *substitutionString);

    AsyncRequest *req = new AsyncRequest(info[2]);
    req->postscriptName = ps;
    req->substitutionString = sub;
    uv_queue_work(uv_default_loop(), &req->work, substituteFontAsync, (uv_after_work_cb) asyncCallback);

    return;
  } else {
    FontDescriptor *result = NULL;
    long error = substituteFont(&result, *postscriptName, *substitutionString);
    if (error != 0) {
      return Nan::ThrowError(Nan::ErrnoException(error));
    }

    info.GetReturnValue().Set(wrapResult(result));
  }
}

NAN_MODULE_INIT(Init) {
  Nan::Export(target, "getAvailableFonts", getAvailableFonts<true>);
  Nan::Export(target, "getAvailableFontsSync", getAvailableFonts<false>);
  Nan::Export(target, "findFonts", findFonts<true>);
  Nan::Export(target, "findFontsSync", findFonts<false>);
  Nan::Export(target, "findFont", findFont<true>);
  Nan::Export(target, "findFontSync", findFont<false>);
  Nan::Export(target, "substituteFont", substituteFont<true>);
  Nan::Export(target, "substituteFontSync", substituteFont<false>);
}

NODE_MODULE(fontmanager, Init)
