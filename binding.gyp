{
  "targets": [
    {
      "target_name": "ivnj_allosphere",
      "include_dirs": [
        "native/include"
      ],
      "libraries": [
        "-L../native/lib", "-livnj-allosphere"
      ],
      "cflags_cc": [
        "-std=c++11"
      ],
      'conditions': [
        [ 'OS=="mac"', {
          'xcode_settings': {
            'OTHER_CPLUSPLUSFLAGS' : ['-std=c++11']
          },
        } ],
      ],
      "sources": [
        "src/ivnj_allosphere.cpp",
        "src/glbind.cpp",
        "src/glubind.cpp"
      ]
    },
    {
      "target_name": "ivnj_canvas",
      "include_dirs": [
        "native/include"
      ],
      "libraries": [
        "-L../native/lib", "-livnj-canvas"
      ],
      "cflags_cc": [
        "-std=c++11"
      ],
      'conditions': [
        [ 'OS=="mac"', {
          'xcode_settings': {
            'OTHER_CPLUSPLUSFLAGS' : ['-std=c++11']
          },
        } ],
      ],
      "sources": [ "src/ivnj_canvas.cpp" ]
    }
  ]
}
