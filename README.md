Introduction
====

Experimental Nodejs bindings for AlloSystem: https://github.com/AlloSphere-Research-Group/AlloSystem

Mainly designed for integrating iVisDesigner (https://github.com/donghaoren/iVisDesigner) to the allosphere.

Features:

- OmniStereo application framework.
- OpenGL binding (adopted from https://github.com/paddybyers/node-gl)
- Skia binding for 2D graphics.
- FFmpeg binding for basic video playback.
- IPC semaphores and shared memory.
- TCP-based reliable message boardcasting and UDP-based boardcasting.

Dependencies
====

1. AlloSystem libraries: allocore, alloutil

    Install the compiled headers and static libraries to one of the following locations:
    
    * /usr/local
    * /opt/local
    * /opt/allosystem
    
    In Linux, please compile with -fPIC, otherwise you'll get linking problems.

2. Graphics library: Skia

    1. Download Skia source code from https://code.google.com/p/skia/
    
    2. Put them here: /opt/build/skia
    
    3. Checkout the commit: 7f8c54cefefb855bb0d85d09ce5282ba7e9e352a
    
    4. In include/config/SkUserConfig.h, comment out these lines:
    
            #ifdef SK_SAMPLES_FOR_X
                #define SK_R32_SHIFT    16
                #define SK_G32_SHIFT    8
                #define SK_B32_SHIFT    0
                #define SK_A32_SHIFT    24
            #endif
        
    5. Build with:
        ./gyp_skia
        ninja -C out/Release

3. Node.js:

    Have `node` and `node-gyp` in your PATH.

Build
====

        make rebuild
        make deploy
        
