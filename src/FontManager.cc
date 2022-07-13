#include <cstdlib>
#include <functional>

#include "napi.h"
#include "FontDescriptor.h"

using namespace Napi;

using namespace std::placeholders;

// these functions are implemented by the platform
long getAvailableFonts(ResultSet **);
long findFonts(ResultSet **, FontDescriptor *);
long findFont(FontDescriptor **, FontDescriptor *);
long substituteFont(FontDescriptor **, char *, char *);

// converts a ResultSet to a JavaScript array
Napi::Array collectResults(Napi::Env env, ResultSet *results) {
  Napi::EscapableHandleScope scope(env);
  Napi::Array res = Napi::Array::New(env, results->size());

  int i = 0;
  for (ResultSet::iterator it = results->begin(); it != results->end(); it++) {
    (res).Set(i++, (*it)->toJSObject(env));
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


struct PromiseAsyncWorker : public Napi::AsyncWorker {
  PromiseAsyncWorker(Napi::Env env) :
    Napi::AsyncWorker(env),
    result(env.Null()), deferred(Napi::Promise::Deferred::New(env)) {}

  void Execute() = 0;

  void OnOK() override {
    deferred.Resolve(result);
  }

  void OnError(Napi::Error const &error) override {
    deferred.Reject(error.Value());
  }

  Napi::Promise GetPromise() {
    return deferred.Promise();
  }

protected:
  Napi::Value result;

private:
  Napi::Promise::Deferred deferred;
};

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

struct GetAvailableFontsAsyncWorker : public PromiseAsyncWorker {
  GetAvailableFontsAsyncWorker(Napi::Env env, std::function<long(ResultSet **)> fn) :
    PromiseAsyncWorker(env),
    fn(fn) {}

  void Execute() {
    ResultSet *results = nullptr;
    long error = this->fn(&results);

    if(error) {
      this->SetError(""); // TODO: need an error to set here. I think originally it was just a number
      return;
    }

    this->result = collectResults(this->Env(), results);
  }

private:
  std::function<long(ResultSet **)> fn;
};

template<bool async>
Napi::Value FontManagerInterface::getAvailableFonts(const Napi::CallbackInfo& info) {
  Napi::Env const &env = info.Env();

  if constexpr (async) {
    // this destroys itself unless we call AsyncWorker::SuppressDestruct()
    auto worker = new GetAvailableFontsAsyncWorker(env, ::getAvailableFonts);
    worker->Queue();

    return worker->GetPromise();
  } else {
    ResultSet *results = NULL;
    long error = ::getAvailableFonts(&results);
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

struct FindFontsAsyncWorker : public PromiseAsyncWorker {
  FindFontsAsyncWorker(Napi::Env env, std::function<long(ResultSet **)> fn) :
    PromiseAsyncWorker(env),
    fn(fn) {}

  void Execute() {
    ResultSet *results = nullptr;
    long error = this->fn(&results);

    if(error) {
      this->SetError(""); // TODO: need an error to set here. I think originally it was just a number
      return;
    }

    this->result = collectResults(this->Env(), results);
  }

private:
  std::function<long(ResultSet **)> fn;
};

template<bool async>
Napi::Value FontManagerInterface::findFonts(const Napi::CallbackInfo& info) {
  Napi::Env const &env = info.Env();

  if (info.Length() < 1 || !info[0].IsObject() || info[0].IsFunction()) {
    Napi::TypeError::New(env, "Expected a font descriptor").ThrowAsJavaScriptException();
    return env.Null();
  }

  Napi::Object desc = info[0].As<Napi::Object>();
  FontDescriptor *descriptor = new FontDescriptor(env, desc);

  if constexpr (async) {
    // this destroys itself unless we call AsyncWorker::SuppressDestruct()
    auto worker = new FindFontsAsyncWorker(env, std::bind(::findFonts, _1, descriptor));
    worker->Queue();

    return worker->GetPromise();
  } else {
    ResultSet *results = NULL;
    long error = ::findFonts(&results, descriptor);
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

struct FindFontAsyncWorker : public PromiseAsyncWorker {
  FindFontAsyncWorker(Napi::Env env, std::function<long(FontDescriptor **)> fn) :
    PromiseAsyncWorker(env),
    fn(fn) {}

  void Execute() {
    FontDescriptor *result = nullptr;
    long error = this->fn(&result);

    if(error) {
      this->SetError(""); // TODO: need an error to set here. I think originally it was just a number
      return;
    }

    this->result = wrapResult(this->Env(), result);
  }

private:
  std::function<long(FontDescriptor **)> fn;
};

template<bool async>
Napi::Value FontManagerInterface::findFont(const Napi::CallbackInfo& info) {
  Napi::Env const &env = info.Env();

  if (info.Length() < 1 || !info[0].IsObject() || info[0].IsFunction()) {
    Napi::TypeError::New(env, "Expected a font descriptor").ThrowAsJavaScriptException();
    return env.Null();
  }

  Napi::Object desc = info[0].As<Napi::Object>();
  FontDescriptor *descriptor = new FontDescriptor(env, desc);

  if constexpr (async) {
    // this destroys itself unless we call AsyncWorker::SuppressDestruct()
    auto worker = new FindFontAsyncWorker(env, std::bind(::findFont, _1, descriptor));
    worker->Queue();

    return worker->GetPromise();
  } else {
    FontDescriptor *result = NULL;
    long error = ::findFont(&result, descriptor);
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

struct SubstitueFontAsyncWorker : public PromiseAsyncWorker {
  SubstitueFontAsyncWorker(Napi::Env env, std::function<long(FontDescriptor **)> fn) :
    PromiseAsyncWorker(env),
    fn(fn) {}

  void Execute() {
    FontDescriptor *result = nullptr;
    long error = this->fn(&result);

    if(error) {
      this->SetError(""); // TODO: need an error to set here. I think originally it was just a number
      return;
    }

    this->result = wrapResult(this->Env(), result);
  }

private:
  std::function<long(FontDescriptor **)> fn;
};

template<bool async>
Napi::Value FontManagerInterface::substituteFont(const Napi::CallbackInfo& info) {
  Napi::Env const &env = info.Env();

  if (info.Length() < 1 || !info[0].IsString()) {
    Napi::TypeError::New(env, "Expected postscript name").ThrowAsJavaScriptException();
    return env.Null();
  }

  if (info.Length() < 2 || !info[1].IsString()) {
    Napi::TypeError::New(env, "Expected substitution string").ThrowAsJavaScriptException();
    return env.Null();
  }

  std::string postscriptName = info[0].As<Napi::String>();
  std::string substitutionString = info[1].As<Napi::String>();

  if constexpr (async) {
    // this destroys itself unless we call AsyncWorker::SuppressDestruct()
    auto worker = new SubstitueFontAsyncWorker(env,
      std::bind(::substituteFont, _1, (char *)postscriptName.c_str(), (char *)substitutionString.c_str()));
    worker->Queue();

    return worker->GetPromise();
  } else {
    FontDescriptor *result = NULL;
    long error = ::substituteFont(&result,
      (char *)postscriptName.c_str(), (char *)substitutionString.c_str());
    if (error != 0) {
      Napi::Error::New(env).ThrowAsJavaScriptException();
      return env.Null();
    }

    return wrapResult(env, result);
  }
}

Napi::Object Init(Napi::Env env, Napi::Object exports) {
  exports.Set("getAvailableFonts",
    Napi::Function::New(env, FontManagerInterface::getAvailableFonts<true>));
  exports.Set("getAvailableFontsSync",
    Napi::Function::New(env, FontManagerInterface::getAvailableFonts<false>));

  exports.Set("findFonts",
    Napi::Function::New(env, FontManagerInterface::findFonts<true>));
  exports.Set("findFontsSync",
    Napi::Function::New(env, FontManagerInterface::findFonts<false>));

  exports.Set("findFont",
    Napi::Function::New(env, FontManagerInterface::findFont<true>));
  exports.Set("findFontSync",
    Napi::Function::New(env, FontManagerInterface::findFont<false>));

  exports.Set("substituteFont",
    Napi::Function::New(env, FontManagerInterface::substituteFont<true>));
  exports.Set("substituteFontSync",
    Napi::Function::New(env, FontManagerInterface::substituteFont<false>));

  return exports;
}

NODE_API_MODULE(fontmanager, Init)
