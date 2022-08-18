#include <cstdlib>

#include "napi.h"

#include "FontDescriptor.h"
#include "ResultSet.h"
#include "FontManagerImpl.h"

// converts a ResultSet to a JavaScript array
Napi::Array collectResults(Napi::Env env, ResultSet *results) {
  Napi::EscapableHandleScope scope(env);
  Napi::Array res = Napi::Array::New(env, results->size());

  int i = 0;
  for (ResultSet::iterator it = results->begin(); it != results->end(); it++) {
    res.Set(i++, (*it)->toJSObject(env));
  }

  delete results;
  return scope.Escape(res).As<Napi::Array>();
}

// converts a FontDescriptor to a JavaScript object
Napi::Value wrapResult(Napi::Env env, FontDescriptor *result) {
  Napi::EscapableHandleScope scope(env);
  if (result == NULL)
    return scope.Escape(env.Null());

  Napi::Object res = result->toJSObject(env);
  delete result;
  return scope.Escape(res);
}

// handles asynchrnous execution for interface functions
// this destroys itself unless we call AsyncWorker::SuppressDestruct()
template<typename _ReturnType>
struct PromiseAsyncWorker : public Napi::AsyncWorker {
  static_assert(std::is_same<_ReturnType, ResultSet>::value || std::is_same<_ReturnType, FontDescriptor>::value, "");

  PromiseAsyncWorker(Napi::Env env, std::function<long(_ReturnType **)> fn) :
    Napi::AsyncWorker(env),
    deferred(Napi::Promise::Deferred::New(env)), fn(fn) {}

  virtual void Execute() override {
    this->result = nullptr;
    long error = this->fn(&this->result);
    if(error) {
      SetError(std::string("Function failed with error code ") + std::to_string(error));
    }
  };

  virtual void OnOK() override {
    if (std::is_same<_ReturnType, ResultSet>::value) {
      deferred.Resolve(collectResults(this->Env(), (ResultSet *)this->result));
    } else if(std::is_same<_ReturnType, FontDescriptor>::value) {
      deferred.Resolve(wrapResult(this->Env(), (FontDescriptor *)this->result));
    }
  };

  virtual void OnError(Napi::Error const &error) override {
    deferred.Reject(error.Value());
  }

  Napi::Promise GetPromise() {
    return deferred.Promise();
  }

private:
  Napi::Promise::Deferred deferred;
  _ReturnType *result;
  std::function<long(_ReturnType **)> fn;
};

// Helper Functions

// simplifies throwing errors in the sync/async functions
template <bool async>
Napi::Value throwError(Napi::Env env, Napi::Error err) {
  if (async) {
    Napi::Promise::Deferred deferred(env);
    deferred.Reject(err.Value());
    return deferred.Promise();
  } else {
    err.ThrowAsJavaScriptException();
    return env.Null();
  }
}

// introduced to prevent name-conflicts
struct FontManagerInterface {
  template<bool async>
  static Napi::Value getAvailableFonts(const Napi::CallbackInfo& info);
  template<bool async>
  static Napi::Value findFonts(const Napi::CallbackInfo& info);
  template<bool async>
  static Napi::Value findFont(const Napi::CallbackInfo& info);
  template<bool async>
  static Napi::Value substituteFont(const Napi::CallbackInfo& info);

  FontManagerInterface() = delete;
};

// -----------------------------------------------------------------------------
// Get Available Fonts
// -----------------------------------------------------------------------------
template<bool async>
Napi::Value FontManagerInterface::getAvailableFonts(const Napi::CallbackInfo& info) {
  Napi::Env const &env = info.Env();
  FontManagerImpl *impl = env.GetInstanceData<FontManagerImpl>();

  if (async) {
    auto worker = new PromiseAsyncWorker<ResultSet>(env, [=](ResultSet **results){
      return impl->getAvailableFonts(results);
    });
    worker->Queue();

    return worker->GetPromise();
  } else {
    ResultSet *results = NULL;
    long error = impl->getAvailableFonts(&results);
    if (error != 0) {
      Napi::Error::New(env).ThrowAsJavaScriptException();
      return env.Null();
    }

    return collectResults(env, results);
  }
}


// -----------------------------------------------------------------------------
// Find Fonts
// -----------------------------------------------------------------------------
template<bool async>
Napi::Value FontManagerInterface::findFonts(const Napi::CallbackInfo& info) {
  Napi::Env const &env = info.Env();
  FontManagerImpl *impl = env.GetInstanceData<FontManagerImpl>();

  if (info.Length() < 1 || !info[0].IsObject()) {
    return throwError<async>(env, Napi::TypeError::New(env, "Expected a font descriptor"));
  }
  Napi::Object desc = info[0].As<Napi::Object>();
  FontDescriptor *descriptor = new FontDescriptor(env, desc);

  if (async) {
    auto worker = new PromiseAsyncWorker<ResultSet>(env, [=](ResultSet **results) {
      long error = impl->findFonts(results, descriptor);
      // need to clean up descriptor somewhere
      delete descriptor;
      return error;
    });
    worker->Queue();

    return worker->GetPromise();
  } else {
    ResultSet *results = NULL;
    long error = impl->findFonts(&results, descriptor);
    if (error != 0) {
      Napi::Error::New(env).ThrowAsJavaScriptException();
      return env.Null();
    }

    Napi::Object res = collectResults(env, results);
    delete descriptor;
    return res;
  }
}

// -----------------------------------------------------------------------------
// Find Font
// -----------------------------------------------------------------------------
template<bool async>
Napi::Value FontManagerInterface::findFont(const Napi::CallbackInfo& info) {
  Napi::Env const &env = info.Env();
  FontManagerImpl *impl = env.GetInstanceData<FontManagerImpl>();

  if (info.Length() < 1 || !info[0].IsObject()) {
    return throwError<async>(env, Napi::TypeError::New(env, "Expected a font descriptor"));
  }
  Napi::Object desc = info[0].As<Napi::Object>();
  FontDescriptor *descriptor = new FontDescriptor(env, desc);

  if (async) {
    auto worker = new PromiseAsyncWorker<FontDescriptor>(env, [=](FontDescriptor **result) {
      long error = impl->findFont(result, descriptor);
      // need to clean up descriptor somewhere
      delete descriptor;
      return error;
    });
    worker->Queue();

    return worker->GetPromise();
  } else {
    FontDescriptor *result = NULL;
    long error = impl->findFont(&result, descriptor);
    if (error != 0) {
      Napi::Error::New(env).ThrowAsJavaScriptException();
      return env.Null();
    }

    Napi::Value res = wrapResult(env, result);
    delete descriptor;
    return res;
  }
}

// -----------------------------------------------------------------------------
// Substitute Fonts
// -----------------------------------------------------------------------------
template<bool async>
Napi::Value FontManagerInterface::substituteFont(const Napi::CallbackInfo& info) {
  Napi::Env const &env = info.Env();
  FontManagerImpl *impl = env.GetInstanceData<FontManagerImpl>();

  if (info.Length() < 1 || !info[0].IsString()) {
    return throwError<async>(env, Napi::TypeError::New(env, "Expected postscript name"));
  }

  if (info.Length() < 2 || !info[1].IsString()) {
    return throwError<async>(env, Napi::TypeError::New(env, "Expected substitution string"));
  }

  std::string postscriptName = info[0].As<Napi::String>();
  std::string substitutionString = info[1].As<Napi::String>();

  if (async) {
    auto worker = new PromiseAsyncWorker<FontDescriptor>(env, [=](FontDescriptor **result) {
      return impl->substituteFont(result, (char *)postscriptName.c_str(), (char *)substitutionString.c_str());
    });
    worker->Queue();

    return worker->GetPromise();
  } else {
    FontDescriptor *result = NULL;
    long error = impl->substituteFont(&result,
      (char *)postscriptName.c_str(), (char *)substitutionString.c_str());
    if (error != 0) {
      Napi::Error::New(env).ThrowAsJavaScriptException();
      return env.Null();
    }

    return wrapResult(env, result);
  }
}

Napi::Object Init(Napi::Env env, Napi::Object exports) {
  env.SetInstanceData<FontManagerImpl>(new FontManagerImpl());

  exports.Set("getAvailableFonts", Napi::Function::New(env, FontManagerInterface::getAvailableFonts<true>));
  exports.Set("getAvailableFontsSync", Napi::Function::New(env, FontManagerInterface::getAvailableFonts<false>));

  exports.Set("findFonts", Napi::Function::New(env, FontManagerInterface::findFonts<true>));
  exports.Set("findFontsSync", Napi::Function::New(env, FontManagerInterface::findFonts<false>));

  exports.Set("findFont", Napi::Function::New(env, FontManagerInterface::findFont<true>));
  exports.Set("findFontSync", Napi::Function::New(env, FontManagerInterface::findFont<false>));

  exports.Set("substituteFont", Napi::Function::New(env, FontManagerInterface::substituteFont<true>));
  exports.Set("substituteFontSync", Napi::Function::New(env, FontManagerInterface::substituteFont<false>));

  return exports;
}

NODE_API_MODULE(fontmanager, Init)
