var fontManager = require("../");
var assert = require("assert");

describe("node-monospace-fonts", function () {
  it("should export some functions", function () {
    assert.equal(typeof fontManager.getMonospaceFonts, "function");
    assert.equal(typeof fontManager.getMonospaceFontsSync, "function");
  });

  function assertFontDescriptor(font) {
    assert.equal(typeof font, "object");
    assert.equal(typeof font.path, "string");
    assert.equal(typeof font.postscriptName, "string");
    assert.equal(typeof font.family, "string");
    assert.equal(typeof font.style, "string");
    assert.equal(typeof font.weight, "number");
    assert.equal(typeof font.width, "number");
    assert.equal(typeof font.italic, "boolean");
    assert.equal(typeof font.monospace, "boolean");
  }

  describe("getMonospaceFonts", function () {
    it("should throw if no callback is provided", function () {
      assert.throws(function () {
        fontManager.getMonospaceFonts();
      }, /Expected a callback/);
    });

    it("should throw if callback is not a function", function () {
      assert.throws(function () {
        fontManager.getMonospaceFonts(2);
      }, /Expected a callback/);
    });

    it("should getMonospaceFonts asynchronously", function (done) {
      var async = false;

      fontManager.getMonospaceFonts(function (fonts) {
        assert(async);
        assert(Array.isArray(fonts));
        assert(fonts.length > 0);
        fonts.forEach(assertFontDescriptor);
        done();
      });

      async = true;
    });
  });

  describe("getMonospaceFontsSync", function () {
    it("should getMonospaceFonts synchronously", function () {
      var fonts = fontManager.getMonospaceFontsSync();
      assert(Array.isArray(fonts));
      assert(fonts.length > 0);
      fonts.forEach(assertFontDescriptor);
      // console.log(fonts);
    });
  });
});
