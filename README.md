# font-manager

A C++ module for Node.js providing access to the system font catalog.

## Features

* List all available fonts
* Find fonts with specified characteristics
* Font substitution when characters are missing

## Platforms

* Mac OS X 10.5 and later supported via [CoreText](https://developer.apple.com/library/mac/documentation/Carbon/reference/CoreText_Framework_Ref/_index.html)
* Windows Vista SP2 and later supported via [DirectWrite](http://msdn.microsoft.com/en-us/library/windows/desktop/dd368038(v=vs.85).aspx)
* Linux supported via [fontconfig](http://www.freedesktop.org/software/fontconfig)

## API

```javascript
var fontManager = require('font-manager');

// list all available fonts
fontManager.getAvailableFonts();
//=> [{ path: '/path/to/font.ttf', postscriptName: 'name' }, ...]

// find fonts matching a font descriptor (see below for a list of supported fields)
fontManager.findFonts({ family: 'Helvetica' });
//=> [{ path: '/path/to/Helvetica.ttf', postscriptName: 'Helvetica' }, ...]

// find the font with the best match
fontManager.findFont({ family: 'Helvetica Neue', weight: 700 });
//=> { path: '/path/to/Helvetica.ttf', postscriptName: 'Helvetica-Bold' }

// substitute the font with the given postscript name 
// with another font that contains the given characters
fontManager.substituteFont('TimesNewRomanPSMT', '汉字')
//=> { path: '/Library/Fonts/Songti.ttc', postscriptName: 'STSongti-SC-Regular' }
```

### Font Descriptor

Font descriptors are normal JavaScript objects that describe characteristics of
a font.  They are passed to the `findFonts` and `findFont` methods.  The fields
allowed are documented below.

Name             | Type    | Description
---------------- | ------- | -----------
`postscriptName` | string  | The PostScript name of the font (e.g `'Arial-BoldMT'`). This uniquely identities a font in most cases.
`family`         | string  | The font family name (e.g `'Arial'`)
`style`          | string  | The font style name (e.g. `'Bold'`)
`weight`         | number  | The font weight (e.g. `400` for normal weight). Should be a multiple of 100, between 100 and 900. See [below](#weights) for weight documentation.
`width`          | number  | The font width (e.g. `5` for normal width). Should be an integer between 1 and 9. See [below](#widths) for width documentation.
`italic`         | boolean | Whether the font is italic or not.
`monospace`      | boolean | Whether the font is monospace or not.

#### Weights

Value   | Name                     
------- | -------------------------
100     | Thin                     
200     | Ultra Light              
300     | Light                    
**400** | **Normal** (default)     
500     | Medium                   
600     | Semi Bold                
700     | Bold                     
800     | Ultra Bold               
900     | Heavy                    

#### Widths

Value | Name                         
----- | -----------------------------
1     | Ultra Condensed              
2     | Extra Condensed              
3     | Condensed                    
4     | Semi Condensed               
**5** | **Normal** (default)         
6     | Semi Expanded                
7     | Expanded                     
8     | Extra Expanded               
9     | Ultra Expanded               

## License

MIT
