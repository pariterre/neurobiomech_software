%mex  -c sfun_ScienceMode2.c WrapScienceMode2.h serialport.h ScienceMode2.h WrapScienceMode2.cpp serial_windows.cpp ScienceMode2.cpp

%retval = dos('build_static_lib.bat');

mex  sfun_ScienceMode2.c WrapScienceMode2.cpp serial_windows.cpp ScienceMode2.cpp