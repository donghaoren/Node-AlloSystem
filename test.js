var allosphere = require("./build/Release/ivnj_allosphere");
var GL = allosphere.OpenGL;
var canvas = require("./build/Release/ivnj_canvas");

allosphere.initialize();

// This is called before each frame.
allosphere.onFrame(function() {
});

// Draw your stuff with OpenGL.
allosphere.onDraw(function() {
    GL.begin(GL.TRIANGLE_STRIP);
    GL.color3f(1, 1, 1);
    GL.vertex3f(-1,  1, -1);
    GL.vertex3f(-1, -1, -1);
    GL.vertex3f( 1,  1, -1);
    GL.vertex3f( 1, -1, -1);
    GL.end();
});

// Main event loop.
setInterval(function() {
    allosphere.tick();
}, 10)
