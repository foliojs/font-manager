#include <Foundation/Foundation.h>
#include <CoreText/CoreText.h>

#include "FontManagerImpl.h"
#include "FontDescriptor.h"
#include "ResultSet.h"

// get localized string
NSString* CFStringToUtf8String(CFStringRef str)
{
  if(str == NULL){
    return NULL;
  }
  CFIndex max = CFStringGetMaximumSizeForEncoding(CFStringGetLength(str), kCFStringEncodingUTF8) + 1;
  char* buffer = (char*)malloc(max);
  if (CFStringGetCString(str, buffer, max, kCFStringEncodingUTF8)) {
      return [NSString stringWithUTF8String: buffer];;
  }
  free(buffer);
  return NULL;
}

NSString* getLocalizedAttribute(CTFontDescriptorRef ref, CFStringRef attr, CFStringRef *localized) {
  CFTypeRef value = CTFontDescriptorCopyLocalizedAttribute(ref, attr, localized);
  if(value && (CFStringRef)value) {
    return CFStringToUtf8String((CFStringRef)value);
  }
  return (NSString *) CTFontDescriptorCopyAttribute(ref, attr);
}

// converts a CoreText weight (-1 to +1) to a standard weight (100 to 900)
static int convertWeight(float weight) {
  if (weight <= -0.8f)
    return 100;
  else if (weight <= -0.6f)
    return 200;
  else if (weight <= -0.4f)
    return 300;
  else if (weight <= 0.0f)
    return 400;
  else if (weight <= 0.25f)
    return 500;
  else if (weight <= 0.35f)
    return 600;
  else if (weight <= 0.4f)
    return 700;
  else if (weight <= 0.6f)
    return 800;
  else
    return 900;
}

// converts a CoreText width (-1 to +1) to a standard width (1 to 9)
static int convertWidth(float unit) {
  if (unit < 0) {
    return 1 + (1 + unit) * 4;
  } else {
    return 5 + unit * 4;
  }
}

FontDescriptor *getMaybeFontDescriptor(const char *path, const char *postscriptName, const char *family, const char *style,
  FontWeight weight, FontWidth width, bool italic, bool monospace) {
  if (path == NULL || postscriptName == NULL || family == NULL || style == NULL) {
    return NULL;
  }

  return new FontDescriptor(
    path,
    postscriptName,
    family,
    style,
    weight,
    width,
    italic,
    monospace
  );
}

long createFontDescriptor(FontDescriptor **res, CTFontDescriptorRef descriptor) {
  NSURL *url = (NSURL *) CTFontDescriptorCopyAttribute(descriptor, kCTFontURLAttribute);
  NSString *psName = (NSString *) CTFontDescriptorCopyAttribute(descriptor, kCTFontNameAttribute);
  NSString *family = (NSString *) CTFontDescriptorCopyAttribute(descriptor, kCTFontFamilyNameAttribute);
  NSString *localizedName = getLocalizedAttribute(descriptor, kCTFontFamilyNameAttribute, NULL);
  NSString *enName = (NSString *) CTFontDescriptorCopyAttribute(descriptor, kCTFontFamilyNameAttribute);
  NSString *style = (NSString *) CTFontDescriptorCopyAttribute(descriptor, kCTFontStyleNameAttribute);

  NSDictionary *traits = (NSDictionary *) CTFontDescriptorCopyAttribute(descriptor, kCTFontTraitsAttribute);
  NSNumber *weightVal = traits[(id)kCTFontWeightTrait];
  FontWeight weight = (FontWeight) convertWeight([weightVal floatValue]);

  NSNumber *widthVal = traits[(id)kCTFontWidthTrait];
  FontWidth width = (FontWidth) convertWidth([widthVal floatValue]);

  NSNumber *symbolicTraitsVal = traits[(id)kCTFontSymbolicTrait];
  unsigned int symbolicTraits = [symbolicTraitsVal unsignedIntValue];

  *res = getMaybeFontDescriptor(
    [[url path] UTF8String],
    [psName UTF8String],
    [family UTF8String],
    [localizedName UTF8String],
    [enName UTF8String],
    [style UTF8String],
    weight,
    width,
    (symbolicTraits & kCTFontItalicTrait) != 0,
    (symbolicTraits & kCTFontMonoSpaceTrait) != 0
  );

  [url release];
  [psName release];
  [family release];
  [localizedName release];
  [enName release];
  [style release];
  [traits release];
  return *res == NULL ? -1 : 0;
}

struct DarwinInstanceData {
  CTFontCollectionRef collection = NULL;
};

FontManagerImpl::FontManagerImpl() : instance_data(new DarwinInstanceData()) {}
FontManagerImpl::~FontManagerImpl() {
  delete static_cast<DarwinInstanceData *>(instance_data);
}

long FontManagerImpl::getAvailableFonts(ResultSet **resultSet) {
  // cache font collection for fast use in future calls
  DarwinInstanceData *instance_data = static_cast<DarwinInstanceData *>(this->instance_data);
  if (instance_data->collection == NULL)
    instance_data->collection = CTFontCollectionCreateFromAvailableFonts(NULL);
  NSArray *matches = (NSArray *) CTFontCollectionCreateMatchingFontDescriptors(instance_data->collection);

  *resultSet = new ResultSet;
  for (id m in matches) {
    CTFontDescriptorRef match = (CTFontDescriptorRef) m;
    FontDescriptor *fontDescriptor;
    long error = createFontDescriptor(&fontDescriptor, match);
    if (error == 0) {
      (*resultSet)->push_back(fontDescriptor);
    }
  }

  [matches release];
  return 0;
}

// helper to square a value
static inline int sqr(int value) {
  return value * value;
}

CTFontDescriptorRef getFontDescriptor(FontDescriptor *desc) {
  // build a dictionary of font attributes
  NSMutableDictionary *attrs = [NSMutableDictionary dictionary];
  CTFontSymbolicTraits symbolicTraits = 0;

  if (desc->postscriptName) {
    NSString *postscriptName = [NSString stringWithUTF8String:desc->postscriptName];
    attrs[(id)kCTFontNameAttribute] = postscriptName;
  }

  if (desc->family) {
    NSString *family = [NSString stringWithUTF8String:desc->family];
    attrs[(id)kCTFontFamilyNameAttribute] = family;
  }

  // localizedName は CTFontDescriptor には不要

  if (desc->style) {
    NSString *style = [NSString stringWithUTF8String:desc->style];
    attrs[(id)kCTFontStyleNameAttribute] = style;
  }

  // build symbolic traits
  if (desc->italic)
    symbolicTraits |= kCTFontItalicTrait;

  if (desc->weight == FontWeightBold)
    symbolicTraits |= kCTFontBoldTrait;

  if (desc->monospace)
    symbolicTraits |= kCTFontMonoSpaceTrait;

  if (desc->width == FontWidthCondensed)
    symbolicTraits |= kCTFontCondensedTrait;

  if (desc->width == FontWidthExpanded)
    symbolicTraits |= kCTFontExpandedTrait;

  if (symbolicTraits) {
    NSDictionary *traits = @{(id)kCTFontSymbolicTrait:[NSNumber numberWithUnsignedInt:symbolicTraits]};
    attrs[(id)kCTFontTraitsAttribute] = traits;
  }

  // create a font descriptor and search for matches
  return CTFontDescriptorCreateWithAttributes((CFDictionaryRef) attrs);
}

int metricForMatch(CTFontDescriptorRef match, FontDescriptor *desc) {
  NSDictionary *dict = (NSDictionary *)CTFontDescriptorCopyAttribute(match, kCTFontTraitsAttribute);

  bool italic = ([dict[(id)kCTFontSymbolicTrait] unsignedIntValue] & kCTFontItalicTrait);

  // normalize everything to base-900
  int metric = 0;
  if (desc->weight)
    metric += sqr(convertWeight([dict[(id)kCTFontWeightTrait] floatValue]) - desc->weight);

  if (desc->width)
    metric += sqr((convertWidth([dict[(id)kCTFontWidthTrait] floatValue]) - desc->width) * 100);

  metric += sqr((italic != desc->italic) * 900);

  [dict release];
  return metric;
}

long FontManagerImpl::findFonts(ResultSet** fonts, FontDescriptor *desc) {
  CTFontDescriptorRef descriptor = getFontDescriptor(desc);
  NSArray *matches = (NSArray *) CTFontDescriptorCreateMatchingFontDescriptors(descriptor, NULL);

  NSArray *sorted = [matches sortedArrayUsingComparator:^NSComparisonResult(id a, id b) {
    int ma = metricForMatch((CTFontDescriptorRef) a, desc);
    int mb = metricForMatch((CTFontDescriptorRef) b, desc);
    return ma < mb ? NSOrderedAscending : ma > mb ? NSOrderedDescending : NSOrderedSame;
  }];

  *fonts = new ResultSet;
  for (id m in sorted) {
    CTFontDescriptorRef match = (CTFontDescriptorRef) m;
    int mb = metricForMatch((CTFontDescriptorRef) m, desc);

    if (mb < 10000) {
      FontDescriptor *fontDescriptor;
      long error = createFontDescriptor(&fontDescriptor, match);
      if (error == 0) {
        (*fonts)->push_back(fontDescriptor);
      }
    }
  }

  CFRelease(descriptor);
  [matches release];
  return 0;
}

CTFontDescriptorRef findBest(FontDescriptor *desc, NSArray *matches) {
  // find the closest match for width and weight attributes
  CTFontDescriptorRef best = NULL;
  int bestMetric = INT_MAX;

  for (id m in matches) {
    int metric = metricForMatch((CTFontDescriptorRef) m, desc);

    if (metric < bestMetric) {
      bestMetric = metric;
      best = (CTFontDescriptorRef) m;
    }

    // break if this is an exact match
    if (metric == 0)
      break;
  }

  return best;
}

long FontManagerImpl::findFont(FontDescriptor **foundFont, FontDescriptor *desc) {
  CTFontDescriptorRef descriptor = getFontDescriptor(desc);
  NSArray *matches = (NSArray *) CTFontDescriptorCreateMatchingFontDescriptors(descriptor, NULL);

  // if there was no match, try again but only try to match traits
  if ([matches count] == 0) {
    [matches release];
    NSSet *set = [NSSet setWithObjects:(id)kCTFontTraitsAttribute, nil];
    matches = (NSArray *) CTFontDescriptorCreateMatchingFontDescriptors(descriptor, (CFSetRef) set);
  }

  // find the closest match for width and weight attributes
  CTFontDescriptorRef best = findBest(desc, matches);

  long error = -1;
  // if we found a match, generate and return a URL for it
  if (best) {
    error = createFontDescriptor(foundFont, best);
  }

  [matches release];
  CFRelease(descriptor);
  return error;
}

long FontManagerImpl::substituteFont(FontDescriptor **res, char *postscriptName, char *string) {
  // create a font descriptor to find the font by its postscript name
  // we don't use CTFontCreateWithName because that supports font
  // names other than the postscript name but prints warnings.
  NSString *ps = [NSString stringWithUTF8String:postscriptName];
  NSDictionary *attrs = @{(id)kCTFontNameAttribute: ps};
  CTFontDescriptorRef descriptor = CTFontDescriptorCreateWithAttributes((CFDictionaryRef) attrs);
  CTFontRef font = CTFontCreateWithFontDescriptor(descriptor, 12.0, NULL);

  // find a substitute font that support the given characters
  NSString *str = [NSString stringWithUTF8String:string];
  CTFontRef substituteFont = CTFontCreateForString(font, (CFStringRef) str, CFRangeMake(0, [str length]));
  CTFontDescriptorRef substituteDescriptor = CTFontCopyFontDescriptor(substituteFont);

  // finally, create and return a result object for this substitute font
  long error = createFontDescriptor(res, substituteDescriptor);

  CFRelease(font);
  CFRelease(substituteFont);
  CFRelease(substituteDescriptor);

  return error;
}
