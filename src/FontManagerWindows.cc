#include <node.h>
#include <v8.h>
#include <dwrite.h>
#include "FontDescriptor.h"
#include "FontManagerResult.h"

// throws a JS error when there is some exception in DirectWrite
#define HR(hr) \
  if (FAILED(hr)) ThrowException(Exception::Error(String::New("Font loading error")));

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
    WCHAR *lastPSName = NULL;
    unsigned int lastPSLength = 0;

    // Get the font family.
    HR(collection->GetFontFamily(i, &family));
    fontCount = family->GetFontCount();

    for (int j = 0; j < fontCount; j++) {
      IDWriteFont *font = NULL;
      IDWriteFontFace *face = NULL;
      unsigned int numFiles = 0;
      IDWriteLocalizedStrings *strings = NULL;
      unsigned int psNameLength = 0;
      WCHAR *psName = NULL;

      HR(family->GetFont(j, &font));
      HR(font->CreateFontFace(&face));

      BOOL exists = false;
      HR(font->GetInformationalStrings(
        DWRITE_INFORMATIONAL_STRING_POSTSCRIPT_NAME,
        &strings,
        &exists
      ));

      HR(strings->GetStringLength(0, &psNameLength));
      psName = (WCHAR *) malloc((psNameLength + 1) * sizeof(WCHAR));

      HR(strings->GetString(0, psName, psNameLength + 1));
      HR(face->GetFiles(&numFiles, NULL));

      // ignore duplicates
      if (lastPSName && lastPSLength == psNameLength && memcmp(lastPSName, psName, psNameLength * sizeof(WCHAR)) == 0) {
        continue;
      }

      free(lastPSName);
      lastPSName = psName;
      lastPSLength = psNameLength;

      IDWriteFontFile *files = NULL;
      HR(face->GetFiles(&numFiles, &files));

      for (int k = 0; k < numFiles; k++) {
        IDWriteFontFileLoader *loader = NULL;
        IDWriteLocalFontFileLoader *fileLoader = NULL;
        unsigned int nameLength = 0;
        const void *referenceKey = NULL;
        unsigned int referenceKeySize = 0;
        WCHAR *name = NULL;

        HR(files[k].GetLoader(&loader));

        // check if this is a local font
        HRESULT hr = loader->QueryInterface(__uuidof(IDWriteLocalFontFileLoader), (void **)&fileLoader);
        if (FAILED(hr)) {
          continue;
        }

        HR(files[k].GetReferenceKey(&referenceKey, &referenceKeySize));
        HR(fileLoader->GetFilePathLengthFromKey(referenceKey, referenceKeySize, &nameLength));

        name = (WCHAR *) malloc((nameLength + 1) * sizeof(WCHAR));
        HR(fileLoader->GetFilePathFromKey(referenceKey, referenceKeySize, name, nameLength + 1));

        res->Set(count++, createResult((uint16_t *) name, (uint16_t *) psName));
        free(name);
      }
    }

    free(lastPSName);
  }

  return scope.Close(res);
}

Handle<Value> findFont(FontDescriptor *desc) {
  return Null();
}
