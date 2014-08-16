#include <Foundation/Foundation.h>
#include <CoreText/CoreText.h>
#include "FontDescriptor.h"
#include "FontManagerResult.h"

ResultSet *getAvailableFonts() {
  NSArray *urls = (NSArray *) CTFontManagerCopyAvailableFontURLs();
  ResultSet *results = new ResultSet();
  
  for (NSURL *url in urls) {
    NSString *path = [url path];
    NSString *psName = [[url fragment] stringByReplacingOccurrencesOfString:@"postscript-name=" withString:@""];
    results->push_back(new FontManagerResult([path UTF8String], [psName UTF8String]));
  }
  
  [urls release];
  return results;
}

// converts a CoreText weight (-1 to +1) to a standard weight (100 to 900)
static int convertWeight(float unit) {
  if (unit < 0) {
    return 100 + (1 + unit) * 300;
  } else {
    return 400 + unit * 500;
  }
}

// converts a CoreText width (-1 to +1) to a standard width (1 to 9)
static int convertWidth(float unit) {
  if (unit < 0) {
    return 1 + (1 + unit) * 4;
  } else {
    return 5 + unit * 4;
  }
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
  CTFontDescriptorRef descriptor = CTFontDescriptorCreateWithAttributes((CFDictionaryRef) attrs);
  [attrs release];
  
  return descriptor;
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

ResultSet *findFonts(FontDescriptor *desc) {
  CTFontDescriptorRef descriptor = getFontDescriptor(desc);
  NSArray *matches = (NSArray *) CTFontDescriptorCreateMatchingFontDescriptors(descriptor, NULL);
  ResultSet *results = new ResultSet();
  
  NSArray *sorted = [matches sortedArrayUsingComparator:^NSComparisonResult(id a, id b) {
    int ma = metricForMatch((CTFontDescriptorRef) a, desc);
    int mb = metricForMatch((CTFontDescriptorRef) b, desc);
    return ma < mb ? NSOrderedAscending : ma > mb ? NSOrderedDescending : NSOrderedSame;
  }];
  
  for (id m in sorted) {
    CTFontDescriptorRef match = (CTFontDescriptorRef) m;
    int mb = metricForMatch((CTFontDescriptorRef) m, desc);
    
    if (mb < 10000) {
      NSURL *url = (NSURL *) CTFontDescriptorCopyAttribute(match, kCTFontURLAttribute);
      NSString *ps = (NSString *) CTFontDescriptorCopyAttribute(match, kCTFontNameAttribute);
      results->push_back(new FontManagerResult([[url path] UTF8String], [ps UTF8String]));
      [url release];
      [ps release];
    }
  }
  
  [sorted release];
  [matches release];
  return results;
}

FontManagerResult *findFont(FontDescriptor *desc) {  
  FontManagerResult *res = NULL;
  CTFontDescriptorRef descriptor = getFontDescriptor(desc);
  NSArray *matches = (NSArray *) CTFontDescriptorCreateMatchingFontDescriptors(descriptor, NULL);
  
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
      
  // if we found a match, generate and return a URL for it
  if (best) {    
    NSURL *url = (NSURL *) CTFontDescriptorCopyAttribute(best, kCTFontURLAttribute);
    NSString *ps = (NSString *) CTFontDescriptorCopyAttribute(best, kCTFontNameAttribute);
    res = new FontManagerResult([[url path] UTF8String], [ps UTF8String]);

    [url release];
    [ps release];
    [matches release];
  }
  
  CFRelease(descriptor);
  return res;
}

FontManagerResult *substituteFont(char *postscriptName, char *string) {
  FontManagerResult *res = NULL;
  
  // create a font descriptor to find the font by its postscript name
  // we don't use CTFontCreateWithName because that will return a best
  // match even if the font doesn't actually exist.
  NSString *ps = [NSString stringWithUTF8String:postscriptName];
  NSDictionary *attrs = @{(id)kCTFontNameAttribute: ps};
  CTFontDescriptorRef descriptor = CTFontDescriptorCreateWithAttributes((CFDictionaryRef) attrs);
  [attrs release];
  
  // find a match
  CTFontDescriptorRef match = CTFontDescriptorCreateMatchingFontDescriptor(descriptor, NULL);
  CFRelease(descriptor);
  
  if (match) {
    // copy the font descriptor for this match and create a substitute font matching the given string
    CTFontRef font = CTFontCreateWithFontDescriptor(match, 12.0, NULL);
    NSString *str = [NSString stringWithUTF8String:string];
    CTFontRef substituteFont = CTFontCreateForString(font, (CFStringRef) str, CFRangeMake(0, [str length]));
    CTFontDescriptorRef substituteDescriptor = CTFontCopyFontDescriptor(substituteFont);
    
    // finally, create and return a result object for this substitute font
    NSURL *url = (NSURL *) CTFontDescriptorCopyAttribute(substituteDescriptor, kCTFontURLAttribute);
    NSString *ps = (NSString *) CTFontDescriptorCopyAttribute(substituteDescriptor, kCTFontNameAttribute);
    res = new FontManagerResult([[url path] UTF8String], [ps UTF8String]);
    
    CFRelease(font);
    [str release];
    CFRelease(substituteFont);
    CFRelease(substituteDescriptor);
    [url release];
    [ps release];
  }
  
  return res;
}
