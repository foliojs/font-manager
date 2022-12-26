var fontManager = require('../');
var assert = require('assert');

// some standard fonts that are likely to be installed on the platform the tests are running on
var standardFont = process.platform === 'linux' ? 'Liberation Sans' : 'Arial';
var postscriptName = process.platform === 'linux' ? 'LiberationSans' : 'ArialMT';

describe('font-manager', function() {
  function assertFontDescriptor(font) {
    assert.equal(typeof font, 'object');
    assert.equal(typeof font.path, 'string');
    assert.equal(typeof font.postscriptName, 'string');
    assert.equal(typeof font.family, 'string');
    assert.equal(typeof font.style, 'string');
    assert.equal(typeof font.weight, 'number');
    assert.equal(typeof font.width, 'number');
    assert.equal(typeof font.italic, 'boolean');
    assert.equal(typeof font.monospace, 'boolean');
  }

  describe('getAvailableFonts', function() {
    it('should getAvailableFonts asynchronously', function() {
      // CI on windows takes forever to finish this test.
      if (process.platform === 'win32') {
        this.timeout(60 * 1000);
      }

      return fontManager.getAvailableFonts()
        .then(function(fonts) {
          assert(Array.isArray(fonts));
          assert(fonts.length > 0);
          fonts.forEach(assertFontDescriptor);
        });
    });
  });

  describe('getAvailableFontsSync', function() {
    it('should getAvailableFonts synchronously', function() {
      // CI on windows takes forever to finish this test.
      if (process.platform === 'win32') {
        this.timeout(60 * 1000);
      }

      var fonts = fontManager.getAvailableFontsSync();
      assert(Array.isArray(fonts));
      assert(fonts.length > 0);
      fonts.forEach(assertFontDescriptor);
    });
  });

  describe('findFonts', function() {
    it('should throw if no font descriptor is provided', function() {
      return fontManager.findFonts()
        .then(
          () => { throw new Error('Should have failed'); },
          (error) => assert.deepEqual(new TypeError('Expected a font descriptor'), error)
        );
    });

    it('should throw if font descriptor is not an object', function() {
      return fontManager.findFonts(2)
        .then(
          () => { throw new Error('Should have failed'); },
          (error) => assert.deepEqual(new TypeError('Expected a font descriptor'), error)
        );
    });

    it('should findFonts asynchronously', function() {
      return fontManager.findFonts({ family: standardFont })
        .then(function(fonts) {
          assert(Array.isArray(fonts));
          assert(fonts.length > 0);
          fonts.forEach(assertFontDescriptor);
        });
    });

    it('should find fonts by postscriptName', function() {
      return fontManager.findFonts({ postscriptName: postscriptName })
        .then(function(fonts) {
          assert(Array.isArray(fonts));
          assert.equal(fonts.length, 1);
          fonts.forEach(assertFontDescriptor);
          assert.equal(fonts[0].postscriptName, postscriptName);
          assert.equal(fonts[0].family, standardFont);
        });
    });

    it('should find fonts by family and style', function() {
      return fontManager.findFonts({ family: standardFont, style: 'Bold' })
        .then(function(fonts) {
          assert(Array.isArray(fonts));
          assert.equal(fonts.length, 1);
          fonts.forEach(assertFontDescriptor);
          assert.equal(fonts[0].family, standardFont);
          assert.equal(fonts[0].style, 'Bold');
          assert.equal(fonts[0].weight, 700);
        });
    });

    it('should find fonts by weight', function() {
      return fontManager.findFonts({ family: standardFont, weight: 700 })
        .then(function(fonts) {
          assert(Array.isArray(fonts));
          assert(fonts.length > 0);
          fonts.forEach(assertFontDescriptor);
          fonts.forEach(function(font) {
            assert.equal(font.weight, 700);
          });
        });
    });

    it('should find italic fonts', function() {
      return fontManager.findFonts({ family: standardFont, italic: true })
        .then(function(fonts) {
          assert(Array.isArray(fonts));
          assert(fonts.length > 0);
          fonts.forEach(assertFontDescriptor);
          fonts.forEach(function(font) {
            assert.equal(font.italic, true);
          });
        });
    });

    it('should find italic and bold fonts', function() {
      return fontManager.findFonts({ family: standardFont, italic: true, weight: 700 })
        .then(function(fonts) {
          assert(Array.isArray(fonts));
          assert(fonts.length > 0);
          fonts.forEach(assertFontDescriptor);
          fonts.forEach(function(font) {
            assert.equal(font.italic, true);
            assert.equal(font.weight, 700);
          });
        });
    });

    it('should return an empty array for nonexistent family', function() {
      return fontManager.findFonts({ family: '' + Date.now() })
        .then(function(fonts) {
          assert(Array.isArray(fonts));
          assert.equal(fonts.length, 0);
        });
    });

    it('should return an empty array for nonexistent postscriptName', function() {
      return fontManager.findFonts({ postscriptName: '' + Date.now() })
        .then(function(fonts) {
          assert(Array.isArray(fonts));
          assert.equal(fonts.length, 0);
        });
    });

    it('should return many fonts for empty font descriptor', function() {
      return fontManager.findFonts({})
        .then(function(fonts) {
          assert(Array.isArray(fonts));
          assert(fonts.length > 0);
          fonts.forEach(assertFontDescriptor);
        });
    });
  });

  describe('findFontsSync', function() {
    it('should throw if no font descriptor is provided', function() {
      assert.throws(function() {
        fontManager.findFontsSync();
      }, /Expected a font descriptor/);
    });

    it('should throw if font descriptor is not an object', function() {
      assert.throws(function() {
        fontManager.findFontsSync(2);
      }, /Expected a font descriptor/);
    });

    it('should findFonts synchronously', function() {
      var fonts = fontManager.findFontsSync({ family: standardFont });
      assert(Array.isArray(fonts));
      assert(fonts.length > 0);
      fonts.forEach(assertFontDescriptor);
    });

    it('should find fonts by postscriptName', function() {
      var fonts = fontManager.findFontsSync({ postscriptName: postscriptName });
      assert(Array.isArray(fonts));
      assert.equal(fonts.length, 1);
      fonts.forEach(assertFontDescriptor);
      assert.equal(fonts[0].postscriptName, postscriptName);
      assert.equal(fonts[0].family, standardFont);
    });

    it('should find fonts by family and style', function() {
      var fonts = fontManager.findFontsSync({ family: standardFont, style: 'Bold' });
      assert(Array.isArray(fonts));
      assert.equal(fonts.length, 1);
      fonts.forEach(assertFontDescriptor);
      assert.equal(fonts[0].family, standardFont);
      assert.equal(fonts[0].style, 'Bold');
      assert.equal(fonts[0].weight, 700);
    });

    it('should find fonts by weight', function() {
      var fonts = fontManager.findFontsSync({ family: standardFont, weight: 700 });
      assert(Array.isArray(fonts));
      assert(fonts.length > 0);
      fonts.forEach(assertFontDescriptor);
      assert.equal(fonts[0].weight, 700);
    });

    it('should find italic fonts', function() {
      var fonts = fontManager.findFontsSync({ family: standardFont, italic: true });
      assert(Array.isArray(fonts));
      assert(fonts.length > 0);
      fonts.forEach(assertFontDescriptor);
      assert.equal(fonts[0].italic, true);
    });

    it('should find italic and bold fonts', function() {
      var fonts = fontManager.findFontsSync({ family: standardFont, italic: true, weight: 700 });
      assert(Array.isArray(fonts));
      assert(fonts.length > 0);
      fonts.forEach(assertFontDescriptor);
      assert.equal(fonts[0].italic, true);
      assert.equal(fonts[0].weight, 700);
    });

    it('should return an empty array for nonexistent family', function() {
      var fonts = fontManager.findFontsSync({ family: '' + Date.now() });
      assert(Array.isArray(fonts));
      assert.equal(fonts.length, 0);
    });

    it('should return an empty array for nonexistent postscriptName', function() {
      var fonts = fontManager.findFontsSync({ postscriptName: '' + Date.now() });
      assert(Array.isArray(fonts));
      assert.equal(fonts.length, 0);
    });

    it('should return many fonts for empty font descriptor', function() {
      var fonts = fontManager.findFontsSync({});
      assert(Array.isArray(fonts));
      assert(fonts.length > 0);
      fonts.forEach(assertFontDescriptor);
    });
  });

  describe('findFont', function() {
    it('should throw if no font descriptor is provided', function() {
      return fontManager.findFont()
        .then(
          () => { throw new Error('Should have failed'); },
          (error) => assert.deepEqual(new TypeError('Expected a font descriptor'), error)
        );
    });

    it('should throw if font descriptor is not an object', function() {
      return fontManager.findFont(2)
        .then(
          () => { throw new Error('Should have failed'); },
          (error) => assert.deepEqual(new TypeError('Expected a font descriptor'), error)
        );
    });

    it('should findFont asynchronously', function() {
      return fontManager.findFont({ family: standardFont })
        .then(function(font) {
          assert.equal(typeof font, 'object');
          assert(!Array.isArray(font));
          assertFontDescriptor(font);
          assert.equal(font.family, standardFont);
        });
    });

    it('should find font by postscriptName', function() {
      return fontManager.findFont({ postscriptName: postscriptName })
        .then(function(font) {
          assertFontDescriptor(font);
          assert.equal(font.postscriptName, postscriptName);
          assert.equal(font.family, standardFont);
        });
    });

    it('should find font by family and style', function() {
      return fontManager.findFont({ family: standardFont, style: 'Bold' })
        .then(function(font) {
          assertFontDescriptor(font);
          assert.equal(font.family, standardFont);
          assert.equal(font.style, 'Bold');
          assert.equal(font.weight, 700);
        });
    });

    it('should find font by weight', function() {
      return fontManager.findFont({ family: standardFont, weight: 700 })
        .then(function(font) {
          assertFontDescriptor(font);
          assert.equal(font.weight, 700);
        });
    });

    it('should find italic font', function() {
      return fontManager.findFont({ family: standardFont, italic: true })
        .then(function(font) {
          assertFontDescriptor(font);
          assert.equal(font.italic, true);
        });
    });

    it('should find bold italic font', function() {
      return fontManager.findFont({ family: standardFont, italic: true, weight: 700 })
        .then(function(font) {
          assertFontDescriptor(font);
          assert.equal(font.italic, true);
          assert.equal(font.weight, 700);
        });
    });

    it('should return a fallback font for nonexistent family', function() {
      return fontManager.findFont({ family: '' + Date.now() })
        .then(assertFontDescriptor);
    });

    it('should return a fallback font for nonexistent postscriptName', function() {
      return fontManager.findFont({ postscriptName: '' + Date.now() })
        .then(assertFontDescriptor)
    });

    it('should return a fallback font matching traits as best as possible', function() {
      return fontManager.findFont({ family: '' + Date.now(), weight: 700 })
        .then(function(font) {
          assertFontDescriptor(font);
          assert.equal(font.weight, 700);
        });
    });

    it('should return a font for empty font descriptor', function() {
      return fontManager.findFont({})
        .then(assertFontDescriptor);
    });
  });

  describe('findFontSync', function() {
    it('should throw if no font descriptor is provided', function() {
      assert.throws(function() {
        fontManager.findFontSync();
      }, /Expected a font descriptor/);
    });

    it('should throw if font descriptor is not an object', function() {
      assert.throws(function() {
        fontManager.findFontSync(2);
      }, /Expected a font descriptor/);
    });

    it('should findFonts synchronously', function() {
      var font = fontManager.findFontSync({ family: standardFont });
      assert.equal(typeof font, 'object');
      assert(!Array.isArray(font));
      assertFontDescriptor(font);
    });

    it('should find font by postscriptName', function() {
      var font = fontManager.findFontSync({ postscriptName: postscriptName });
      assertFontDescriptor(font);
      assert.equal(font.postscriptName, postscriptName);
      assert.equal(font.family, standardFont);
    });

    it('should find font by family and style', function() {
      var font = fontManager.findFontSync({ family: standardFont, style: 'Bold' });
      assertFontDescriptor(font);
      assert.equal(font.family, standardFont);
      assert.equal(font.style, 'Bold');
      assert.equal(font.weight, 700);
    });

    it('should find font by weight', function() {
      var font = fontManager.findFontSync({ family: standardFont, weight: 700 });
      assertFontDescriptor(font);
      assert.equal(font.weight, 700);
    });

    it('should find italic font', function() {
      var font = fontManager.findFontSync({ family: standardFont, italic: true });
      assertFontDescriptor(font);
      assert.equal(font.italic, true);
    });

    it('should find bold italic font', function() {
      var font = fontManager.findFontSync({ family: standardFont, italic: true, weight: 700 });
      assertFontDescriptor(font);
      assert.equal(font.italic, true);
      assert.equal(font.weight, 700);
    });

    it('should return a fallback font for nonexistent family', function() {
      var font = fontManager.findFontSync({ family: '' + Date.now() });
      assertFontDescriptor(font);
    });

    it('should return a fallback font for nonexistent postscriptName', function() {
      var font = fontManager.findFontSync({ postscriptName: '' + Date.now() });
      assertFontDescriptor(font);
    });

    it('should return a fallback font matching traits as best as possible', function() {
      var font = fontManager.findFontSync({ family: '' + Date.now(), weight: 700 });
      assertFontDescriptor(font);
      assert.equal(font.weight, 700);
    });

    it('should return a font for empty font descriptor', function() {
      var font = fontManager.findFontSync({});
      assertFontDescriptor(font);
    });
  });

  describe('substituteFont', function() {
    it('should throw if no postscript name is provided', function() {
      return fontManager.substituteFont()
        .then(
          () => { throw new Error('Should have failed'); },
          (error) => assert.deepEqual(new TypeError('Expected postscript name'), error)
        );
    });

    it('should throw if postscript name is not a string', function() {
      return fontManager.substituteFont(2, 'hi')
        .then(
          () => { throw new Error('Should have failed'); },
          (error) => assert.deepEqual(new TypeError('Expected postscript name'), error)
        );
    });

    it('should throw if no substitution string is provided', function() {
      return fontManager.substituteFont(postscriptName)
        .then(
          () => { throw new Error('Should have failed'); },
          (error) => assert.deepEqual(new TypeError('Expected substitution string'), error)
        );
    });

    it('should throw if substitution string is not a string', function() {
      return fontManager.substituteFont(postscriptName, 2)
        .then(
          () => { throw new Error('Should have failed'); },
          (error) => assert.deepEqual(new TypeError('Expected substitution string'), error)
        );
    });

    it('should substituteFont asynchronously', function() {
      return fontManager.substituteFont(postscriptName, '汉字')
        .then(function(font) {
          assert.equal(typeof font, 'object');
          assert(!Array.isArray(font));
          assertFontDescriptor(font);
          assert.notEqual(font.postscriptName, postscriptName);
        });
    });

    it('should return the same font if it already contains the requested characters', function() {
      return fontManager.substituteFont(postscriptName, 'hi')
        .then(function(font) {
          assertFontDescriptor(font);
          assert.equal(font.postscriptName, postscriptName);
        });
    });

    it('should return a default font if no font exists for the given postscriptName', function() {
      return fontManager.substituteFont('' + Date.now(), '汉字')
        .then(assertFontDescriptor);
    });
  });

  describe('substituteFontSync', function() {
    it('should throw if no postscript name is provided', function() {
      assert.throws(function() {
        fontManager.substituteFontSync();
      }, /Expected postscript name/);
    });

    it('should throw if postscript name is not a string', function() {
      assert.throws(function() {
        fontManager.substituteFontSync(2, 'hi');
      }, /Expected postscript name/);
    });

    it('should throw if no substitution string is provided', function() {
      assert.throws(function() {
        fontManager.substituteFontSync(postscriptName);
      }, /Expected substitution string/);
    });

    it('should throw if substitution string is not a string', function() {
      assert.throws(function() {
        fontManager.substituteFontSync(postscriptName, 2);
      }, /Expected substitution string/);
    });

    it('should substituteFont synchronously', function() {
      var font = fontManager.substituteFontSync(postscriptName, '汉字');
      assert.equal(typeof font, 'object');
      assert(!Array.isArray(font));
      assertFontDescriptor(font);
      assert.notEqual(font.postscriptName, postscriptName);
    });

    it('should return the same font if it already contains the requested characters', function() {
      var font = fontManager.substituteFontSync(postscriptName, 'hi');
      assertFontDescriptor(font);
      assert.equal(font.postscriptName, postscriptName);
    });

    it('should return a default font if no font exists for the given postscriptName', function() {
      var font = fontManager.substituteFontSync('' + Date.now(), '汉字');
      assertFontDescriptor(font);
    });
  });
});
