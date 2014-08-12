#include <node.h>
#include <v8.h>
#include <dwrite.h>
#include "FontDescriptor.h"
#include "FontManagerResult.h"

// throws a JS error when there is some exception in DirectWrite
#define HR(hr) \
  if (FAILED(hr)) ThrowException(Exception::Error(String::New("Font loading error")));

// gets the postscript name for a font
WCHAR *getPostscriptName(IDWriteFont *font) {
  IDWriteLocalizedStrings *strings = NULL;
  unsigned int psNameLength = 0;
  WCHAR *psName = NULL;

  BOOL exists = false;
  HR(font->GetInformationalStrings(
    DWRITE_INFORMATIONAL_STRING_POSTSCRIPT_NAME,
    &strings,
    &exists
  ));

  HR(strings->GetStringLength(0, &psNameLength));
  psName = new WCHAR[psNameLength + 1];

  HR(strings->GetString(0, psName, psNameLength + 1));
  return psName;
}

Handle<Value> resultFromFont(IDWriteFont *font) {
  Handle<Value> res;
  IDWriteFontFace *face = NULL;
  unsigned int numFiles = 0;
  WCHAR *psName = getPostscriptName(font);

  HR(font->CreateFontFace(&face));

  // get the font files from this font face
  IDWriteFontFile *files = NULL;
  HR(face->GetFiles(&numFiles, NULL));
  HR(face->GetFiles(&numFiles, &files));

  // return the first one
  if (numFiles > 0) {
    IDWriteFontFileLoader *loader = NULL;
    IDWriteLocalFontFileLoader *fileLoader = NULL;
    unsigned int nameLength = 0;
    const void *referenceKey = NULL;
    unsigned int referenceKeySize = 0;
    WCHAR *name = NULL;

    HR(files[0].GetLoader(&loader));

    // check if this is a local font file
    HRESULT hr = loader->QueryInterface(__uuidof(IDWriteLocalFontFileLoader), (void **)&fileLoader);
    if (SUCCEEDED(hr)) {
      // get the file path
      HR(files[0].GetReferenceKey(&referenceKey, &referenceKeySize));
      HR(fileLoader->GetFilePathLengthFromKey(referenceKey, referenceKeySize, &nameLength));

      name = (WCHAR *) malloc((nameLength + 1) * sizeof(WCHAR));
      HR(fileLoader->GetFilePathFromKey(referenceKey, referenceKeySize, name, nameLength + 1));

      res = createResult((uint16_t *) name, (uint16_t *) psName);
      free(name);
    } else {
      res = Null();
    }
  } else {
    res = Null();
  }

  free(psName);
  return res;
}

Handle<Value> getAvailableFonts(const Arguments& args) {
  HandleScope scope;
  Local<Array> res = Array::New(0);
  int count = 0;

  IDWriteFactory *factory = NULL;
  HR(DWriteCreateFactory(
    DWRITE_FACTORY_TYPE_SHARED,
    __uuidof(IDWriteFactory),
    reinterpret_cast<IUnknown**>(&factory)
  ));

  // Get the system font collection.
  IDWriteFontCollection *collection = NULL;
  HR(factory->GetSystemFontCollection(&collection));

  // Get the number of font families in the collection.
  int familyCount = collection->GetFontFamilyCount();

  for (int i = 0; i < familyCount; i++) {
    IDWriteFontFamily *family = NULL;
    int fontCount = 0;

    // Get the font family.
    HR(collection->GetFontFamily(i, &family));
    fontCount = family->GetFontCount();

    for (int j = 0; j < fontCount; j++) {
      IDWriteFont *font = NULL;
      HR(family->GetFont(j, &font));
      res->Set(count++, resultFromFont(font));
    }
  }

  return scope.Close(res);
}

IDWriteFont *findFontByFamily(IDWriteFontCollection *collection, FontDescriptor *desc) {
  int size = strlen(desc->family) + 1;
  wchar_t *family = new wchar_t[size];
  size_t convertedChars = 0;
  mbstowcs_s(&convertedChars, family, size, desc->family, _TRUNCATE);

  unsigned int index;
  BOOL exists;
  HR(collection->FindFamilyName(family, &index, &exists));

  delete family;

  if (exists) {
    IDWriteFontFamily *family = NULL;
    HR(collection->GetFontFamily(index, &family));

    // IDWriteFontList *fontList = NULL;
    IDWriteFont *font = NULL;
    HR(family->GetFirstMatchingFont(
      desc->weight ? (DWRITE_FONT_WEIGHT) desc->weight : DWRITE_FONT_WEIGHT_NORMAL,
      desc->width  ? (DWRITE_FONT_STRETCH) desc->width : DWRITE_FONT_STRETCH_UNDEFINED,
      desc->italic ? DWRITE_FONT_STYLE_ITALIC : DWRITE_FONT_STYLE_NORMAL,
      &font
    ));

    return font;
  }

  return NULL;
}

IDWriteFont *findFontByPostscriptName(IDWriteFontCollection *collection, FontDescriptor *desc) {
  int size = strlen(desc->postscriptName) + 1;
  wchar_t *postscriptName = new wchar_t[size];
  size_t convertedChars = 0;
  mbstowcs_s(&convertedChars, postscriptName, size, desc->postscriptName, _TRUNCATE);

  // Get the number of font families in the collection.
  int familyCount = collection->GetFontFamilyCount();

  for (int i = 0; i < familyCount; i++) {
    IDWriteFontFamily *family = NULL;
    int fontCount = 0;

    // Get the font family.
    HR(collection->GetFontFamily(i, &family));
    fontCount = family->GetFontCount();

    for (int j = 0; j < fontCount; j++) {
      IDWriteFont *font = NULL;
      HR(family->GetFont(j, &font));

      WCHAR *psName = getPostscriptName(font);
      if (wcscmp(psName, postscriptName) == 0) {
        delete postscriptName;
        delete psName;
        return font;
      }

      delete psName;
    }
  }

  delete postscriptName;
  return NULL;
}

Handle<Value> findFont(FontDescriptor *desc) {
  IDWriteFactory *factory = NULL;
  HR(DWriteCreateFactory(
    DWRITE_FACTORY_TYPE_SHARED,
    __uuidof(IDWriteFactory),
    reinterpret_cast<IUnknown**>(&factory)
  ));

  // Get the system font collection.
  IDWriteFontCollection *collection = NULL;
  HR(factory->GetSystemFontCollection(&collection));

  IDWriteFont *font = NULL;
  if (desc->family) {
    font = findFontByFamily(collection, desc);
  } else if (desc->postscriptName) {
    font = findFontByPostscriptName(collection, desc);
  }

  if (font) {
    return resultFromFont(font);
  }

  return Null();
}
