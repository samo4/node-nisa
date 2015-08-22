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
          "target_name" : "makecallback", 
          "sources"     : [ "cpp/makecallback.cpp", "cpp/visa.cpp" ], 
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
          "target_name" : "makecallback", 
          "sources"     : [ "cpp/makecallback.cpp", "cpp/visa.cpp" ],
          "include_dirs": [ "/Library/Frameworks/Visa.framework/Headers" ], 
          "link_settings": {
              "library_dirs": ['/Library/Frameworks/Visa.framework/VISA'],
              "libraries": ['visa64']
            },
          "xcode_settings": {
              'OTHER_LDFLAGS': [ '-framework CoreFoundation -framework IOKit' ]
            }
       }]
     ]
    }
   ]
}


