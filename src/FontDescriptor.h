#ifndef FONT_DESCRIPTOR_H
#define FONT_DESCRIPTOR_H

#include "napi.h"


enum FontWeight {
  FontWeightUndefined   = 0,
  FontWeightThin        = 100,
  FontWeightUltraLight  = 200,
  FontWeightLight       = 300,
  FontWeightNormal      = 400,
  FontWeightMedium      = 500,
  FontWeightSemiBold    = 600,
  FontWeightBold        = 700,
  FontWeightUltraBold   = 800,
  FontWeightHeavy       = 900
};

enum FontWidth {
  FontWidthUndefined      = 0,
  FontWidthUltraCondensed = 1,
  FontWidthExtraCondensed = 2,
  FontWidthCondensed      = 3,
  FontWidthSemiCondensed  = 4,
  FontWidthNormal         = 5,
  FontWidthSemiExpanded   = 6,
  FontWidthExpanded       = 7,
  FontWidthExtraExpanded  = 8,
  FontWidthUltraExpanded  = 9
};

struct FontDescriptor {
public:
  const char *path;
  const char *postscriptName;
  const char *family;
  const char *localizedName;
  const char *enName;
  const char *style;
  FontWeight weight;
  FontWidth width;
  bool italic;
  bool monospace;

  FontDescriptor();
  FontDescriptor (const FontDescriptor&) = delete;
  FontDescriptor& operator= (const FontDescriptor&) = delete;
  FontDescriptor (FontDescriptor&&) = delete;
  FontDescriptor& operator= (FontDescriptor&&) = delete;

  FontDescriptor(FontDescriptor *desc);
  FontDescriptor(Napi::Env env, Napi::Object obj);
  FontDescriptor(
    const char *path,
    const char *postscriptName,
    const char *family,
    const char *localizedName,
    const char *enName,
    const char *style,
    FontWeight weight,
    FontWidth width,
    bool italic,
    bool monospace
  );

  virtual ~FontDescriptor();

  Napi::Object toJSObject(Napi::Env env);
};

#endif
