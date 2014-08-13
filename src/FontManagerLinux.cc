#include <fontconfig/fontconfig.h>
#include <node.h>
#include <v8.h>
#include "FontDescriptor.h"
#include "FontManagerResult.h"

Local<Array> arrayFromFontSet(FcFontSet *fs) {
  if (!fs)
    return Array::New(0);

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

  return res;
}

Handle<Value> getAvailableFonts(const Arguments& args) {
  HandleScope scope;
    
  FcInit();

  FcPattern *pattern = FcPatternCreate();
  FcObjectSet *os = FcObjectSetBuild(FC_POSTSCRIPT_NAME, FC_FILE, NULL);
  FcFontSet *fs = FcFontList(NULL, pattern, os);
  Local<Array> res = arrayFromFontSet(fs);
  
  FcPatternDestroy(pattern);
  FcObjectSetDestroy(os);
  FcFontSetDestroy(fs);

  return scope.Close(res);
}

int convertWeight(FontWeight weight) {
  switch (weight) {
    case FontWeightThin:
      return FC_WEIGHT_THIN;
    case FontWeightUltraLight:
      return FC_WEIGHT_ULTRALIGHT;
    case FontWeightLight:
      return FC_WEIGHT_LIGHT;
    case FontWeightNormal:
      return FC_WEIGHT_REGULAR;
    case FontWeightMedium:
      return FC_WEIGHT_MEDIUM;
    case FontWeightSemiBold:
      return FC_WEIGHT_SEMIBOLD;
    case FontWeightBold:
      return FC_WEIGHT_BOLD;
    case FontWeightUltraBold:
      return FC_WEIGHT_EXTRABOLD;
    case FontWeightHeavy:
      return FC_WEIGHT_ULTRABLACK;
    default:
      return FC_WEIGHT_REGULAR;
  }
}

int convertWidth(FontWidth width) {
  switch (width) {
    case FontWidthUltraCondensed:
      return FC_WIDTH_ULTRACONDENSED;
    case FontWidthExtraCondensed:
      return FC_WIDTH_EXTRACONDENSED;
    case FontWidthCondensed:
      return FC_WIDTH_CONDENSED;
    case FontWidthSemiCondensed:
      return FC_WIDTH_SEMICONDENSED;
    case FontWidthNormal:
      return FC_WIDTH_NORMAL;
    case FontWidthSemiExpanded:
      return  FC_WIDTH_SEMIEXPANDED;
    case FontWidthExpanded:
      return FC_WIDTH_EXPANDED;
    case FontWidthExtraExpanded:
      return FC_WIDTH_EXTRAEXPANDED;
    case FontWidthUltraExpanded:
      return FC_WIDTH_ULTRAEXPANDED;
    default:
      return FC_WIDTH_NORMAL;
  }
}

FcPattern *createPattern(FontDescriptor *desc) {
  FcInit();
  FcPattern *pattern = FcPatternCreate();

  if (desc->postscriptName)
    FcPatternAddString(pattern, FC_POSTSCRIPT_NAME, (FcChar8 *) desc->postscriptName);

  if (desc->family)
    FcPatternAddString(pattern, FC_FAMILY, (FcChar8 *) desc->family);

  if (desc->style)
    FcPatternAddString(pattern, FC_STYLE, (FcChar8 *) desc->style);

  if (desc->italic)
    FcPatternAddInteger(pattern, FC_SLANT, FC_SLANT_ITALIC);

  if (desc->weight)
    FcPatternAddInteger(pattern, FC_WEIGHT, convertWeight(desc->weight));

  if (desc->width)
    FcPatternAddInteger(pattern, FC_WIDTH, convertWidth(desc->width));

  if (desc->monospace)
    FcPatternAddInteger(pattern, FC_SPACING, FC_MONO);

  return pattern;
}

Handle<Value> findFonts(FontDescriptor *desc) {
  FcPattern *pattern = createPattern(desc);
  FcObjectSet *os = FcObjectSetBuild(FC_POSTSCRIPT_NAME, FC_FILE, NULL);
  FcFontSet *fs = FcFontList(NULL, pattern, os);

  Local<Array> res = arrayFromFontSet(fs);

  FcFontSetDestroy(fs);
  FcPatternDestroy(pattern);
  FcObjectSetDestroy(os);

  return res;
}

Handle<Value> findFont(FontDescriptor *desc) {
  FcPattern *pattern = createPattern(desc);
  FcConfigSubstitute(NULL, pattern, FcMatchPattern);
  FcDefaultSubstitute(pattern);

  FcResult result;
  FcPattern *font = FcFontMatch(NULL, pattern, &result);

  FcChar8 *file;
  FcChar8 *psName;
  Handle<Value> res;

  if (FcPatternGetString(font, FC_FILE, 0, &file) == FcResultMatch &&
      FcPatternGetString(font, FC_POSTSCRIPT_NAME, 0, &psName) == FcResultMatch) {
    res = createResult((char *)file, (char *)psName);
  } else {
    res = Null();
  }

  FcPatternDestroy(pattern);
  FcPatternDestroy(font);

  return res;
}

Handle<Value> substituteFont(char *postscriptName, char *string) {
  FcInit();

  // create a pattern with the postscript name
  FcPattern* pattern = FcPatternCreate();
  FcPatternAddString(pattern, FC_POSTSCRIPT_NAME, (FcChar8 *) postscriptName);

  // create a charset with each character in the string
  FcCharSet* charset = FcCharSetCreate();
  int len = strlen(string);

  for (int i = 0; i < len;) {
    FcChar32 c;
    i += FcUtf8ToUcs4((FcChar8 *)string + i, &c, len - i);
    FcCharSetAddChar(charset, c);
  }

  FcPatternAddCharSet(pattern, FC_CHARSET, charset);
  FcCharSetDestroy(charset);

  FcConfigSubstitute(0, pattern, FcMatchPattern);
  FcDefaultSubstitute(pattern);

  // find the best match font
  FcResult result;
  FcPattern *font = FcFontMatch(NULL, pattern, &result);

  FcChar8 *file;
  FcChar8 *psName;
  Handle<Value> res;

  // create a result object if we found a match
  if (FcPatternGetString(font, FC_FILE, 0, &file) == FcResultMatch &&
      FcPatternGetString(font, FC_POSTSCRIPT_NAME, 0, &psName) == FcResultMatch) {
    res = createResult((char *)file, (char *)psName);
  } else {
    res = Null();
  }

  FcPatternDestroy(pattern);
  FcPatternDestroy(font);

  return res;
}