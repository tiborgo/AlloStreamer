{
  "targets": [
    {
      "target_name": "allounity_receiver",
      "include_dirs": [
        "..",
        "<!(node -e \"require('nan')\")"
      ],
      "libraries": [
        "-L ../../build/AlloReceiver", "-lAlloReceiver"
      ],
      "cflags!": [ "-fno-exceptions", "-fno-rtti" ],
      "cflags_cc!": [ "-fno-exceptions", "-fno-rtti" ],
      "cflags_cc": [
        "-std=c++11"
      ],
      'conditions': [
        [ 'OS=="mac"', {
          'xcode_settings': {
            'OTHER_CPLUSPLUSFLAGS' : ['-std=c++11'],
            'GCC_ENABLE_CPP_EXCEPTIONS': 'YES',
            'GCC_ENABLE_CPP_RTTI': 'YES'
          },
        } ],
      ],
      "sources": [
        "src/node_allounity_receiver.cpp"
      ]
    }
  ]
}
