var fontManager = require('../');
var assert = require('assert');

// some standard fonts that are likely to be installed on the platform the tests are running on
var standardFont = process.platform === 'linux' ? 'DejaVu Sans' : 'Arial';
var postscriptName = process.platform === 'linux' ? 'DejaVuSans' : 'ArialMT';

describe('font-manager', function() {
  it('should export some functions', function() {
    assert.equal(typeof fontManager.getAvailableFonts, 'function');
    assert.equal(typeof fontManager.getAvailableFontsSync, 'function');
    assert.equal(typeof fontManager.findFonts, 'function');
    assert.equal(typeof fontManager.findFontsSync, 'function');
    assert.equal(typeof fontManager.findFont, 'function');
    assert.equal(typeof fontManager.findFontSync, 'function');
    assert.equal(typeof fontManager.substituteFont, 'function');
    assert.equal(typeof fontManager.substituteFontSync, 'function');
  });
  
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
    it('should throw if no callback is provided', function() {
      assert.throws(function() {
        fontManager.getAvailableFonts();
      }, /Expected a callback/);
    });
    
    it('should throw if callback is not a function', function() {
      assert.throws(function() {
        fontManager.getAvailableFonts(2);
      }, /Expected a callback/);
    });
    
    it('should getAvailableFonts asynchronously', function(done) {
      var async = false;

      fontManager.getAvailableFonts(function(fonts) {
        assert(async); 
        assert(Array.isArray(fonts));
        assert(fonts.length > 0);
        fonts.forEach(assertFontDescriptor);
        done();
      });

      async = true;
    });
  });

  describe('getAvailableFontsSync', function() {
    it('should getAvailableFonts synchronously', function() {
      var fonts = fontManager.getAvailableFontsSync();
      assert(Array.isArray(fonts));
      assert(fonts.length > 0);
      fonts.forEach(assertFontDescriptor);
    });
  });
  
  describe('findFonts', function() {
    it('should throw if no font descriptor is provided', function() {
      assert.throws(function() {
        fontManager.findFonts(function(fonts) {});
      }, /Expected a font descriptor/);
    });
    
    it('should throw if font descriptor is not an object', function() {
      assert.throws(function() {
        fontManager.findFonts(2, function(fonts) {});
      }, /Expected a font descriptor/);
    });
    
    it('should throw if no callback is provided', function() {
      assert.throws(function() {
        fontManager.findFonts({ family: standardFont });
      }, /Expected a callback/);
    });
    
    it('should throw if callback is not a function', function() {
      assert.throws(function() {
        fontManager.findFonts({ family: standardFont }, 2);
      }, /Expected a callback/);
    });
    
    it('should findFonts asynchronously', function(done) {
      var async = false;
    
      fontManager.findFonts({ family: standardFont }, function(fonts) {
        assert(async); 
        assert(Array.isArray(fonts));
        assert(fonts.length > 0);
        fonts.forEach(assertFontDescriptor);
        done();
      });
    
      async = true;
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
  });
  
  describe('findFont', function() {
    it('should throw if no font descriptor is provided', function() {
      assert.throws(function() {
        fontManager.findFont(function(fonts) {});
      }, /Expected a font descriptor/);
    });
    
    it('should throw if font descriptor is not an object', function() {
      assert.throws(function() {
        fontManager.findFont(2, function(fonts) {});
      }, /Expected a font descriptor/);
    });
    
    it('should throw if no callback is provided', function() {
      assert.throws(function() {
        fontManager.findFont({ family: standardFont });
      }, /Expected a callback/);
    });
    
    it('should throw if callback is not a function', function() {
      assert.throws(function() {
        fontManager.findFont({ family: standardFont }, 2);
      }, /Expected a callback/);
    });
    
    it('should findFont asynchronously', function(done) {
      var async = false;
    
      fontManager.findFont({ family: standardFont }, function(font) {
        assert(async); 
        assert.equal(typeof font, 'object');
        assert(!Array.isArray(font));
        assertFontDescriptor(font);
        done();
      });
    
      async = true;
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
  });
  
  describe('substituteFont', function() {
    it('should throw if no postscript name is provided', function() {
      assert.throws(function() {
        fontManager.substituteFont(function(font) {});
      }, /Expected postscript name/);
    });
    
    it('should throw if postscript name is not a string', function() {
      assert.throws(function() {
        fontManager.substituteFont(2, 'hi', function(font) {});
      }, /Expected postscript name/);
    });
    
    it('should throw if no substitution string is provided', function() {
      assert.throws(function() {
        fontManager.substituteFont(postscriptName, function(font) {});
      }, /Expected substitution string/);
    });
    
    it('should throw if substitution string is not a string', function() {
      assert.throws(function() {
        fontManager.substituteFont(postscriptName, 2, function(font) {});
      }, /Expected substitution string/);
    });
    
    it('should throw if no callback is provided', function() {
      assert.throws(function() {
        fontManager.substituteFont(postscriptName, '汉字');
      }, /Expected a callback/);
    });
    
    it('should throw if callback is not a function', function() {
      assert.throws(function() {
        fontManager.substituteFont(postscriptName, '汉字', 52);
      }, /Expected a callback/);
    });
    
    it('should substituteFont asynchronously', function(done) {
      var async = false;
    
      fontManager.substituteFont(postscriptName, '汉字', function(font) {
        assert(async); 
        assert.equal(typeof font, 'object');
        assert(!Array.isArray(font));
        assertFontDescriptor(font);
        assert.notEqual(font.postscriptName, postscriptName);
        done();
      });
    
      async = true;
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
    
    it('should substituteFont asynchronously', function() {
      var font = fontManager.substituteFontSync(postscriptName, '汉字');
      assert.equal(typeof font, 'object');
      assert(!Array.isArray(font));
      assertFontDescriptor(font);
      assert.notEqual(font.postscriptName, postscriptName);
    });
  });
});
