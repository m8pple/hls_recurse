MSYS:

Create:
   Vivado_HLS\2015.2\msys\var\lib\mingw-get\data
Copy defaults.xml from existing msys installation
to
   Vivado_HLS\2015.2\msys\var\lib\mingw-get\data\defaults.xml

From mingw shell do:
  mingw-get install g++


Add #define __NO_INLINE__ to math.h
http://ehc.ac/p/mingw/bugs/2250/
