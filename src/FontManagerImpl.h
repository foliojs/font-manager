#include "FontDescriptor.h"
#include "ResultSet.h"

struct FontManagerImpl {
  FontManagerImpl();
  virtual ~FontManagerImpl();

  FontManagerImpl (const FontManagerImpl&) = delete;
  FontManagerImpl& operator= (const FontManagerImpl&) = delete;
  FontManagerImpl (FontManagerImpl&&) = delete;
  FontManagerImpl& operator= (FontManagerImpl&&) = delete;

  long getAvailableFonts(ResultSet **);
  long findFonts(ResultSet **, FontDescriptor *);
  long findFont(FontDescriptor **, FontDescriptor *);
  long substituteFont(FontDescriptor **, char *, char *);

private:
  void *instance_data;
};