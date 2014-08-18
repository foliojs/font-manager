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

// find fonts with characteristics
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

## License

MIT
