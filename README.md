# font-manager

A C++ module for Node.js providing access to the system font catalog.

## Features

* List all available fonts
* Find fonts with specified characteristics
* Font substitution when characters are missing (TODO)

## Platforms

* Mac OS X 10.5 and later supported via [CoreText](https://developer.apple.com/library/mac/documentation/Carbon/reference/CoreText_Framework_Ref/_index.html)
* Windows Vista SP2 and later supported via [DirectWrite](http://msdn.microsoft.com/en-us/library/windows/desktop/dd368038(v=vs.85).aspx)
* Linux supported via [fontconfig](http://www.freedesktop.org/software/fontconfig)

## API

```javascript
var fontManager = require('font-manager');

// list all available fonts
fontManager.getAvailableFonts(); // => [{ path: '/path/to/font.ttf', postscriptName: 'name' }, ...]

// find fonts with characteristics
var desc = new fontManager.FontDescriptor();
desc.family = "Helvetica Neue";

fontManager.findFonts(desc); // => [{ path: '/path/to/Helvetica.ttf', postscriptName: 'Helvetica-Regular' }, ...]

// find the font with the best match
fontManager.findFont(desc); // => { path: '/path/to/Helvetica.ttf', postscriptName: 'Helvetica-Regular' }
```

## License

MIT
