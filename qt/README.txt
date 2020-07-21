README
------

This is the Qt port of Stash, which is still under development, but is somewhat useable,
although it's missing some features compared to the native Mac version (the original
UI from 10 years ago).

It builds with Qt 4.8 and 5.12 on Linux, and might build/run on other platforms as well,
but my focus here is predominantly Linux, at least initially for my purposes.


Building
--------

The build uses CMake, and needs a C++11 compiler.

To build on Linux, make a new build directory in this directory,
cd into it, then run:

cmake ../ -DCMAKE_BUILD_TYPE=Release

then:

make -j


TODO (to match Mac version):
----------------------------

* Import functionality
* Export functionality
* Graph functionality
* Check UTF-8 strings work everywhere

