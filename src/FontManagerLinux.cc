#include <fontconfig/fontconfig.h>
#include <node.h>
#include <v8.h>
#include "FontDescriptor.h"
#include "FontManagerResult.h"

Handle<Value> getAvailableFonts(const Arguments& args) {
  HandleScope scope;
    
  FcInit();

  FcConfig *config = FcConfigGetCurrent();
  FcPattern *pat = FcPatternCreate();

  FcObjectSet *os = FcObjectSetBuild(FC_POSTSCRIPT_NAME, FC_FILE, (char *) 0);
  FcFontSet *fs = FcFontList(config, pat, os);
  if (!fs)
    return scope.Close(Null());

  Local<Array> res = Array::New(fs->nfont);
  int count = 0;

  for (int i = 0; i < fs->nfont; i++) {
    FcPattern *font = fs->fonts[i];
    FcChar8 *file;
    FcChar8 *psName;

    if (FcPatternGetString(font, FC_FILE, 0, &file) == FcResultMatch &&
        FcPatternGetString(font, FC_POSTSCRIPT_NAME, 0, &psName) == FcResultMatch) {
      res->Set(count++, createResult((char *)file, (char *)psName));
    }
  }
  
  return scope.Close(res);
}

Handle<Value> findFont(FontDescriptor *desc) {
  return Null();
}