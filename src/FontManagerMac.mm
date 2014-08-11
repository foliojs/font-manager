#include <Foundation/Foundation.h>
#include <CoreText/CoreText.h>
#include <node.h>
#include <v8.h>
#include "FontDescriptor.h"

v8::Handle<v8::Value> getAvailableFonts(const v8::Arguments& args) {
  v8::HandleScope scope;
    
  NSArray *urls = (NSArray *) CTFontManagerCopyAvailableFontURLs();
  v8::Local<v8::Array> res = v8::Array::New([urls count]);
  
  int i = 0;
  for (NSURL *url in urls) {
    NSString *str = [url absoluteString];
    const char *data = [str cStringUsingEncoding:NSUTF8StringEncoding];
    res->Set(i++, v8::String::New(data));
    [str release];
  }
  
  [urls release];
  return scope.Close(res);
}

// converts a Core Text weight (-1 to +1) to a standard weight (100 to 900)
static int convertWeight(float unit) {
  if (unit < 0) {
    return 100 + (1 + unit) * 300;
  } else {
    return 400 + unit * 500;
  }
}

// converts a Core Text width (-1 to +1) to a standard width (1 to 9)
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

const char *findFont(FontDescriptor *desc) {
  char *data = NULL;
  
  // build a dictionary of font attributes
  NSMutableDictionary *attrs = [NSMutableDictionary dictionary];
  CTFontSymbolicTraits symbolicTraits = 0;
  
  if (desc->postscriptName) {
    NSString *postscriptName = [[NSString alloc] initWithCString:desc->postscriptName encoding:NSUTF8StringEncoding];
    attrs[(id)kCTFontNameAttribute] = postscriptName;
  }
  
  if (desc->family) {
    NSString *family = [[NSString alloc] initWithCString:desc->family encoding:NSUTF8StringEncoding];
    attrs[(id)kCTFontFamilyNameAttribute] = family;
  }
  
  if (desc->style) {
    NSString *style = [[NSString alloc] initWithCString:desc->style encoding:NSUTF8StringEncoding];
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
  NSArray *matches = (NSArray *) CTFontDescriptorCreateMatchingFontDescriptors(descriptor, NULL);
  
  // find the closest match for width and weight attributes
  CTFontDescriptorRef best = NULL;
  int bestMetric = INT_MAX;
  
  for (id m in matches) {
    CTFontDescriptorRef match = (CTFontDescriptorRef) m;
    NSDictionary *dict = (NSDictionary *)CTFontDescriptorCopyAttribute(match, kCTFontTraitsAttribute);
    
    int weight = convertWeight([dict[(id)kCTFontWeightTrait] floatValue]);
    int width = convertWidth([dict[(id)kCTFontWidthTrait] floatValue]);
    bool italic = ([dict[(id)kCTFontSymbolicTrait] unsignedIntValue] & kCTFontItalicTrait) == 1;
        
    // normalize everything to base-900
    int metric = sqr(weight - desc->weight) + 
                 sqr((width - desc->width) * 100) + 
                 sqr((italic != desc->italic) * 900);
    
    if (metric < bestMetric) {
      bestMetric = metric;
      best = match;
    }
    
    [dict release];
    
    // break if this is an exact match
    if (metric == 0)
      break;
  }
      
  // if we found a match, generate and return a URL for it
  if (best) {    
    NSURL *url = (NSURL *)CTFontDescriptorCopyAttribute(best, kCTFontURLAttribute);
    NSString *ps = (NSString *)CTFontDescriptorCopyAttribute(best, kCTFontNameAttribute);
    NSString *str = [NSString stringWithFormat:@"%@#postscript-name=%@", [url absoluteString], ps];
    NSInteger len = [str maximumLengthOfBytesUsingEncoding:NSUTF8StringEncoding] + 1;

    data = (char *) malloc(len);
    [str getCString:data maxLength:len encoding:NSUTF8StringEncoding];

    [url release];
    [str release];
    [ps release];
    [matches release];
  }
  
  CFRelease(descriptor);  
  return data;
}
