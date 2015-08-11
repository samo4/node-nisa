{
  "target_defaults":
    {
        "cflags" : ["-Wall", "-Wextra", "-Wno-unused-parameter"],
        'include_dirs': [
          '<!(node -e "require(\'nan\')")'
        ]
    },
    
	"targets": [
	{
        "target_name" : "makecallback"
      , "sources"     : [ "cpp/makecallback.cpp" ]
    }
   ]
}


