mex -c WrapScienceMode2.h serial_linux.h ScienceMode2.h WrapScienceMode2.cpp serial_linux.cpp ScienceMode2.cpp
!sh build_static_lib.sh
mex sfun_ScienceMode2.c WrapScienceMode2.h serial_linux.h ScienceMode2.h WrapScienceMode2.cpp serial_linux.cpp ScienceMode2.cpp