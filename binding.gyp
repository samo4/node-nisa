{
  "target_defaults":
    {
        "cflags" : ["-Wall", "-Wextra", "-Wno-unused-parameter"],
        "include_dirs": [
          '<!(node -e "require(\'nan\')")'
        ]
    },
    
	"targets": [
	{
    "conditions": 
    [
        ['OS=="win"', {
          "target_name" : "nisa", 
          "sources"     : [ "cpp/nisa.cpp", "cpp/nisa_win.cpp", "cpp/nisa_static_helpers.cpp" ], 
          "include_dirs": [
                'C:\Program Files\IVI Foundation\VISA\Win64\Include'
              ], 
          "link_settings": {
              "library_dirs": ['C:\Program Files\IVI Foundation\VISA\Win64\Lib_x64\msc'],
              "libraries": ['visa64'],
            },
            'msvs_settings': {
                'VCCLCompilerTool': {
                  'ExceptionHandling': '2',
                  'DisableSpecificWarnings': [ '4530', '4506' ],
                },
        }
       }],
       ['OS=="mac"', {
          "target_name" : "nisa", 
          "sources"     : [ "cpp/nisa.cpp", "cpp/nisa_win.cpp" ], 
          "include_dirs": [ "/Library/Frameworks/VISA.framework/Headers" ],
          "link_settings": {
              "library_dirs": ['/Library/Frameworks/VISA.framework'],
              "libraries": ['visa64']
            },
          "xcode_settings": {
              'OTHER_LDFLAGS': [ '-framework CoreFoundation -framework IOKit' ],
              'OTHER_CFLAGS': [
                "-std=c++11"
              ]
            }
       }]
     ]
    }
   ]
}


