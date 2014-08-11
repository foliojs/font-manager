var binding = require('./build/Release/fontmanager');

function FontDescriptor() {
  this.postscriptName = null;
  this.family = null;
  this.weight = 400; // (100..900) normal = 400, bold = 700
  this.width = 5;    // (1..9) condensed = 3, normal = 4, expanded = 7
  this.italic = false;
  this.monospace = false;
}

// console.log(binding.getAvailableFonts())

var desc = new FontDescriptor();
// desc.postscriptName = "AvenirNext-UltraLight"
// desc.family = "Helvetica Neue";
desc.family = "Avenir Next";
// desc.style = "Ultra Light";
// desc.italic = true;
// desc.weight = 900;
// desc.width = 3;
// desc.monospace = true;

console.log(binding.findFont(desc));