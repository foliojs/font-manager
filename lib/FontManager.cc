#include <stdlib.h>
#include <node.h>
#include <uv.h>
#include <v8.h>
#include <nan.h>
#include "FontDescriptor.h"

using namespace v8;

// these functions are implemented by the platform
ResultSet *getMonospaceFonts();

// converts a ResultSet to a JavaScript array
Local<Array> collectResults(ResultSet *results) {
  Nan::EscapableHandleScope scope;
  Local<Array> res = Nan::New<Array>(results->size());

  int i = 0;
  for (ResultSet::iterator it = results->begin(); it != results->end(); it++) {
    Nan::Set(res, i++, (*it)->toJSObject());
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
  Local<Value> info[1];

  if (req->results) {
    info[0] = collectResults(req->results);
  } else if (req->result) {
    info[0] = wrapResult(req->result);
  } else {
    info[0] = Nan::Null();
  }

  req->callback->Call(1, info, &async);
  delete req;
}

void getMonospaceFontsAsync(uv_work_t *work) {
  AsyncRequest *req = (AsyncRequest *) work->data;
  req-> results = getMonospaceFonts();
}

template<bool async>
NAN_METHOD(getMonospaceFonts) {
  if (async) {
    if (info.Length() < 1 || !info[0]->IsFunction())
      return Nan::ThrowTypeError("Expected a callback");

    AsyncRequest *req = new AsyncRequest(info[0]);
    uv_queue_work(uv_default_loop(), &req->work, getMonospaceFontsAsync, (uv_after_work_cb) asyncCallback);

    return;
  } else {
    info.GetReturnValue().Set(collectResults(getMonospaceFonts()));
  }
}

NAN_MODULE_INIT(Init) {
  Nan::Export(target, "getMonospaceFonts", getMonospaceFonts<true>);
  Nan::Export(target, "getMonospaceFontsSync", getMonospaceFonts<false>);
}

NODE_MODULE(fontmanager, Init)
