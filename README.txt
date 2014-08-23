Dependencies
====

1. AlloSystem libraries: allocore, alloutil
    In Linux, please compile with -fPIC, otherwise you'll get linking problems.
    Install the compiled headers and archives to one of the following locations:
      /usr/local
      /opt/local
      /opt/allosystem

2. Graphics library: Skia
    Get it from https://code.google.com/p/skia/
    Put it here: /opt/build/skia
    Build with:
        ./gyp_skia
        ninja -C out/Release

3. Node.js:
    Have `node` and `node-gyp` in your PATH.

Build
====

1. First build the native shared objects.

    mkdir build_native
    cd build_native
    cmake .. -DCMAKE_INSTALL_PREFIX=../native
    make install -j8
    cd ..

2. Build Nodejs bindings.

    node-gyp configure
    node-gyp build
