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

FontManagerResult *resultFromFont(IDWriteFont *font) {
  FontManagerResult *res = NULL;
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

      res = new FontManagerResult((uint16_t *) name, (uint16_t *) psName);
      free(name);
    }
  }

  free(psName);
  return res;
}

ResultSet *getAvailableFonts(const Arguments& args) {
  ResultSet *res = new ResultSet();
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
      res->push_back(resultFromFont(font));
    }
  }

  return res;
}

WCHAR *utf8ToUtf16(char *input) {
  unsigned int len = MultiByteToWideChar(CP_UTF8, 0, input, -1, NULL, 0);
  WCHAR *output = new WCHAR[len];
  MultiByteToWideChar(CP_UTF8, 0, input, -1, output, len);
  return output;
}

IDWriteFontList *findFontsByFamily(IDWriteFontCollection *collection, FontDescriptor *desc) {
  WCHAR *family = utf8ToUtf16(desc->family);

  unsigned int index;
  BOOL exists;
  HR(collection->FindFamilyName(family, &index, &exists));
  delete family;

  if (exists) {
    IDWriteFontFamily *family = NULL;
    HR(collection->GetFontFamily(index, &family));

    IDWriteFontList *fontList = NULL;
    HR(family->GetMatchingFonts(
      desc->weight ? (DWRITE_FONT_WEIGHT) desc->weight : DWRITE_FONT_WEIGHT_NORMAL,
      desc->width  ? (DWRITE_FONT_STRETCH) desc->width : DWRITE_FONT_STRETCH_UNDEFINED,
      desc->italic ? DWRITE_FONT_STYLE_ITALIC : DWRITE_FONT_STYLE_NORMAL,
      &fontList
    ));

    return fontList;
  }

  return NULL;
}

IDWriteFont *findFontByPostscriptName(IDWriteFontCollection *collection, FontDescriptor *desc) {
  WCHAR *postscriptName = utf8ToUtf16(desc->postscriptName);

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

ResultSet *findFonts(FontDescriptor *desc) {
  ResultSet *res = new ResultSet();
  
  IDWriteFactory *factory = NULL;
  HR(DWriteCreateFactory(
    DWRITE_FACTORY_TYPE_SHARED,
    __uuidof(IDWriteFactory),
    reinterpret_cast<IUnknown**>(&factory)
  ));

  // Get the system font collection.
  IDWriteFontCollection *collection = NULL;
  HR(factory->GetSystemFontCollection(&collection));

  if (desc->family) {
    IDWriteFontList *fonts = findFontsByFamily(collection, desc);
    int fontCount = fonts ? fonts->GetFontCount() : 0;

    for (int j = 0; j < fontCount; j++) {
      IDWriteFont *font = NULL;
      HR(fonts->GetFont(j, &font));
      res->push_back(resultFromFont(font));
    }

    return res;
  } else if (desc->postscriptName) {
    IDWriteFont *font = findFontByPostscriptName(collection, desc);
    res->push_back(resultFromFont(font));
    return res;
  }

  return res;
}

FontManagerResult *findFont(FontDescriptor *desc) {
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
    IDWriteFontList *fonts = findFontsByFamily(collection, desc);
    if (fonts && fonts->GetFontCount() > 0)
      fonts->GetFont(0, &font);
  } else if (desc->postscriptName) {
    font = findFontByPostscriptName(collection, desc);
  }

  if (font) {
    return resultFromFont(font);
  }

  return NULL;
}

// custom text renderer used to determine the fallback font for a given char
class FontFallbackRenderer : public IDWriteTextRenderer {
public:
  IDWriteFontCollection *systemFonts;
  IDWriteFont *font;
  unsigned long refCount;

  FontFallbackRenderer(IDWriteFontCollection *collection) {
    refCount = 0;
    collection->AddRef();
    systemFonts = collection;
    font = NULL;
  }

  ~FontFallbackRenderer() {
    if (systemFonts)
      systemFonts->Release();

    if (font)
      font->Release();
  }

  // IDWriteTextRenderer methods
  IFACEMETHOD(DrawGlyphRun)(
      void *clientDrawingContext,
      FLOAT baselineOriginX,
      FLOAT baselineOriginY,
      DWRITE_MEASURING_MODE measuringMode,
      DWRITE_GLYPH_RUN const *glyphRun,
      DWRITE_GLYPH_RUN_DESCRIPTION const *glyphRunDescription,
      IUnknown *clientDrawingEffect) {

    // save the font that was actually rendered
    return systemFonts->GetFontFromFontFace(glyphRun->fontFace, &font);
  }

  IFACEMETHOD(DrawUnderline)(
      void *clientDrawingContext,
      FLOAT baselineOriginX,
      FLOAT baselineOriginY,
      DWRITE_UNDERLINE const *underline,
      IUnknown *clientDrawingEffect) {
    return E_NOTIMPL;
  }


  IFACEMETHOD(DrawStrikethrough)(
      void *clientDrawingContext,
      FLOAT baselineOriginX,
      FLOAT baselineOriginY,
      DWRITE_STRIKETHROUGH const *strikethrough,
      IUnknown *clientDrawingEffect) {
    return E_NOTIMPL;
  }


  IFACEMETHOD(DrawInlineObject)(
      void *clientDrawingContext,
      FLOAT originX,
      FLOAT originY,
      IDWriteInlineObject *inlineObject,
      BOOL isSideways,
      BOOL isRightToLeft,
      IUnknown *clientDrawingEffect) {
    return E_NOTIMPL;
  }

  // IDWritePixelSnapping methods
  IFACEMETHOD(IsPixelSnappingDisabled)(void *clientDrawingContext, BOOL *isDisabled) {
    *isDisabled = FALSE;
    return S_OK;
  }

  IFACEMETHOD(GetCurrentTransform)(void *clientDrawingContext, DWRITE_MATRIX *transform) {
    const DWRITE_MATRIX ident = {1.0, 0.0, 0.0, 1.0, 0.0, 0.0};
    *transform = ident;
    return S_OK;
  }

  IFACEMETHOD(GetPixelsPerDip)(void *clientDrawingContext, FLOAT *pixelsPerDip) {
    *pixelsPerDip = 1.0f;
    return S_OK;
  }

  // IUnknown methods
  IFACEMETHOD_(unsigned long, AddRef)() {
    return InterlockedIncrement(&refCount);
  }

  IFACEMETHOD_(unsigned long,  Release)() {
    unsigned long newCount = InterlockedDecrement(&refCount);
    if (newCount == 0) {
      delete this;
      return 0;
    }

    return newCount;
  }

  IFACEMETHOD(QueryInterface)(IID const& riid, void **ppvObject) {
    if (__uuidof(IDWriteTextRenderer) == riid) {
      *ppvObject = this;
    } else if (__uuidof(IDWritePixelSnapping) == riid) {
      *ppvObject = this;
    } else if (__uuidof(IUnknown) == riid) {
      *ppvObject = this;
    } else {
      *ppvObject = nullptr;
      return E_FAIL;
    }

    this->AddRef();
    return S_OK;
  }
};

FontManagerResult *substituteFont(char *postscriptName, char *string) {
  FontManagerResult *res = NULL;

  IDWriteFactory *factory = NULL;
  HR(DWriteCreateFactory(
    DWRITE_FACTORY_TYPE_SHARED,
    __uuidof(IDWriteFactory),
    reinterpret_cast<IUnknown**>(&factory)
  ));

  // Get the system font collection.
  IDWriteFontCollection *collection = NULL;
  HR(factory->GetSystemFontCollection(&collection));

  // find the font for the given postscript name
  FontDescriptor *desc = new FontDescriptor();
  desc->postscriptName = postscriptName;
  IDWriteFont *font = findFontByPostscriptName(collection, desc);

  if (font) {
    // get the font family name
    IDWriteFontFamily *family = NULL;
    HR(font->GetFontFamily(&family));

    IDWriteLocalizedStrings *names = NULL;
    HR(family->GetFamilyNames(&names));

    unsigned int length = 0;
    HR(names->GetStringLength(0, &length));
    WCHAR *familyName = new WCHAR[length + 1];
    HR(names->GetString(0, familyName, length + 1));

    // convert utf8 string for substitution to utf16
    WCHAR *str = utf8ToUtf16(string);

    // create a text format
    IDWriteTextFormat *format = NULL;
    HR(factory->CreateTextFormat(
      familyName,
      collection,
      font->GetWeight(),
      font->GetStyle(),
      font->GetStretch(),
      12.0,
      L"en-us",
      &format
    ));

    // create a text layout for the substitution string
    IDWriteTextLayout *layout = NULL;
    HR(factory->CreateTextLayout(
      str,
      wcslen(str),
      format,
      100.0,
      100.0,
      &layout
    ));

    // render it using a custom renderer that saves the physical font being used
    FontFallbackRenderer *renderer = new FontFallbackRenderer(collection);
    HR(layout->Draw(NULL, renderer, 100.0, 100.0));

    // if we found something, create a result object
    if (renderer->font) {
      res = resultFromFont(renderer->font);
    }

    // free all the things
    delete renderer;
    layout->Release();
    format->Release();
    delete str;
    delete familyName;
    names->Release();
    family->Release();
    font->Release();
  }

  desc->postscriptName = NULL;
  delete desc;
  collection->Release();
  factory->Release();

  return res;
}
