{
  "targets": [
    {
      "target_name": "system-fonts",
      "sources": [ "lib/FontManager.cc" ],
      "include_dirs" : [
        "<!(node -e \"require('nan')\")"
      ],
      "conditions": [
        ['OS=="mac"', {
          "sources": ["lib/FontManagerMac.mm"],
          "link_settings": {
            "libraries": ["CoreText.framework", "Foundation.framework"]
          }
        }],
        ['OS=="win"', {
          "sources": ["lib/FontManagerWindows.cc"],
          "link_settings": {
            "libraries": ["Dwrite.lib"]
          }
        }],
        ['OS=="linux"', {
          "sources": ["lib/FontManagerLinux.cc"],
          "link_settings": {
            "libraries": ["-lfontconfig"]
          }
        }]
      ]
    }
  ]
}
