var allosphere = require("node_allosphere");
var graphics = require("node_graphics");
var shm = require("node_sharedmemory");
var GL = allosphere.OpenGL;

allosphere.initialize();

var buf = new shm.SharedMemory(0x1000, 1000 * 1000 * 4, true);

var s = new graphics.Surface2D(1000, 1000, buf.buffer());

allosphere.onFrame(function() {
    buf.readLock();
    s.uploadTexture();
    buf.readUnlock();
});

// Draw your stuff with OpenGL.
allosphere.onDraw(function() {
    GL.enable(GL.BLEND);
    GL.blendFunc(GL.SRC_ALPHA, GL.ONE_MINUS_SRC_ALPHA);

    s.bindTexture(2);

    allosphere.shaderUniformf("texture", 1);
    allosphere.shaderUniformi("texture0", 2);
    allosphere.shaderUniformf("lighting", 0);


    GL.begin(GL.QUADS);
    GL.texCoord2f(0, 0); GL.normal3f(0, 0, 1); GL.vertex3f(-2,  2, -1.5);
    GL.texCoord2f(0, 1); GL.normal3f(0, 0, 1); GL.vertex3f(-2, -2, -1.5);
    GL.texCoord2f(1, 1); GL.normal3f(0, 0, 1); GL.vertex3f( 2, -2, -1.5);
    GL.texCoord2f(1, 0); GL.normal3f(0, 0, 1); GL.vertex3f( 2,  2, -1.5);
    GL.end();

    GL.begin(GL.QUADS);
    GL.texCoord2f(0, 0); GL.normal3f(0, 0, 1); GL.vertex3f(-1,  1, -1);
    GL.texCoord2f(0, 1); GL.normal3f(0, 0, 1); GL.vertex3f(-1, -1, -1);
    GL.texCoord2f(1, 1); GL.normal3f(0, 0, 1); GL.vertex3f( 1, -1, -1);
    GL.texCoord2f(1, 0); GL.normal3f(0, 0, 1); GL.vertex3f( 1,  1, -1);
    GL.end();

    s.unbindTexture(2);
});

var timer = setInterval(function() {
    allosphere.tick();
}, 10);

process.on('SIGINT', function() {
    clearInterval(timer);
    buf.delete();
});
