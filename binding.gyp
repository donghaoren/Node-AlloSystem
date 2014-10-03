{
  "targets": [
    {
      "target_name": "node_allosphere",
      "include_dirs": [
        "native/include"
      ],
      "libraries": [
        "-L../native/lib", "-lnode-allosphere"
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
        "src/node_allosphere.cpp",
        "src/glbind.cpp",
        "src/stream.cpp",
        "src/glubind.cpp"
      ]
    },
    {
      "target_name": "node_graphics",
      "include_dirs": [
        "native/include"
      ],
      "libraries": [
        "-L../native/lib", "-lnode-graphics"
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
      "sources": [ "src/node_graphics.cpp" ]
    },
    {
      "target_name": "node_broadcaster",
      "include_dirs": [
        "native/include"
      ],
      "libraries": [
        "-L../native/lib", "-lnode-broadcaster"
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
      "sources": [ "src/node_broadcaster.cpp" ]
    },
    {
      "include_dirs": [
        "native/include"
      ],
      "target_name": "node_sharedmemory",
      "sources": ["src/node_sharedmemory.cpp"]
    }
  ]
}
