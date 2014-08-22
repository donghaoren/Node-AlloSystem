var allosphere = require("node_allosphere");
var graphics = require("node_graphics");
var GL = allosphere.OpenGL;

allosphere.initialize();

var s = new graphics.Surface2D(1000, 1000);

var context = new graphics.GraphicalContext2D(s);
var paint = context.paint();

allosphere.onFrame(function() {
    s.uploadTexture();
});

// Draw your stuff with OpenGL.
allosphere.onDraw(function() {
    s.bindTexture(2);

    allosphere.shaderUniformf("texture", 1.0);
    allosphere.shaderUniformi("texture0", 2);
    allosphere.shaderUniformf("lighting", 0.1);

    GL.begin(GL.QUADS);
    GL.texCoord2f(0, 0); GL.normal3f(0, 0, 1); GL.vertex3f(-1,  1, -1);
    GL.texCoord2f(0, 1); GL.normal3f(0, 0, 1); GL.vertex3f(-1, -1, -1);
    GL.texCoord2f(1, 1); GL.normal3f(0, 0, 1); GL.vertex3f( 1, -1, -1);
    GL.texCoord2f(1, 0); GL.normal3f(0, 0, 1); GL.vertex3f( 1,  1, -1);
    GL.end();

    GL.begin(GL.QUADS);
    GL.texCoord2f(1, 0); GL.normal3f(0, 0, 1); GL.vertex3f(-1,  1, 1);
    GL.texCoord2f(1, 1); GL.normal3f(0, 0, 1); GL.vertex3f(-1, -1, 1);
    GL.texCoord2f(0, 1); GL.normal3f(0, 0, 1); GL.vertex3f( 1, -1, 1);
    GL.texCoord2f(0, 0); GL.normal3f(0, 0, 1); GL.vertex3f( 1,  1, 1);
    GL.end();

    s.unbindTexture(2);
});

var saved = false;
var t0 = new Date().getTime();
setInterval(function() {
    // Update the bitmap image.
    var dt = (new Date().getTime() - t0) / 1000;
    context.clear(255, 255, 255, 1);
    context.drawLine(0, 0, 1000, 1000, paint);
    paint.setTypeface("Arial", graphics.FONTSTYLE_NORMAL);
    paint.setTextSize(120);
    paint.setTextAlign(graphics.TEXTALIGN_CENTER);
    paint.setMode(graphics.PAINTMODE_FILL);
    context.drawText("Hello World", 500, 500 + 100 * Math.sin(dt * 5), paint);
    paint.setTextSize(60);
    context.drawText("t = " + dt, 500, 600 + 100 * Math.sin(dt * 5), paint);
    if(!saved) s.save("test.png");
    saved = true;
}, 10);

// Main event loop for alloutil.
setInterval(function() {
    allosphere.tick();
}, 10);
