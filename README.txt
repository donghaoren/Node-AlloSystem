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
    Checkout the commit: 7f8c54cefefb855bb0d85d09ce5282ba7e9e352a
    Change one thing:
        In include/config/SkUserConfig.h, comment out these lines:
         #ifdef SK_SAMPLES_FOR_X
             #define SK_R32_SHIFT    16
             #define SK_G32_SHIFT    8
             #define SK_B32_SHIFT    0
             #define SK_A32_SHIFT    24
         #endif
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
