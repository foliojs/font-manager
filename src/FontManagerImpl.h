#include "FontDescriptor.h"
#include "ResultSet.h"

struct FontManagerImpl {
  FontManagerImpl();
  ~FontManagerImpl();

  long getAvailableFonts(ResultSet **);
  long findFonts(ResultSet **, FontDescriptor *);
  long findFont(FontDescriptor **, FontDescriptor *);
  long substituteFont(FontDescriptor **, char *, char *);

private:
  void *instance_data;
};