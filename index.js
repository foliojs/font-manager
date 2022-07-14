let fontManager;
try {
  fontManager = require('./build/Release/fontmanager');
} catch (releaseNotFoundError) {
  try {
    fontManager = require('./build/Debug/fontmanager');
  } catch (debugNotFoundError) {
    throw new Error('There is no built binary for font-manager');
  }
}

module.exports = {
  findFontSync: (fontDescriptor) => fontManager.findFontSync(fontDescriptor),
  findFont: (fontDescriptor) => fontManager.findFont(fontDescriptor),
  findFontsSync: (fontDescriptor) => fontManager.findFontsSync(fontDescriptor),
  findFonts: (fontDescriptor) => fontManager.findFonts(fontDescriptor),
  getAvailableFontsSync: () => fontManager.getAvailableFontsSync(),
  getAvailableFonts: () => fontManager.getAvailableFonts(),
  substituteFontSync: (postscriptName, text) => fontManager.substituteFontSync(postscriptName, text),
  substituteFont: (postscriptName, text) => fontManager.substituteFont(postscriptName, text)
};
