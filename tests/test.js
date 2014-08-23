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

var saved = false;
var t0 = new Date().getTime();
setInterval(function() {
    // Update the bitmap image.
    var dt = (new Date().getTime() - t0) / 1000;
    context.clear(255, 255, 255, 0.5);
    paint.setMode(graphics.PAINTMODE_STROKE);
    paint.setStrokeWidth(10);
    context.drawCircle(500, 500, 400, paint);
    paint.setColor(0, 255, 0, 1);
    paint.setTypeface("Arial", graphics.FONTSTYLE_NORMAL);
    paint.setTextSize(120);
    paint.setTextAlign(graphics.TEXTALIGN_CENTER);
    paint.setMode(graphics.PAINTMODE_FILL);
    context.drawText("Hello World", 500, 500 + 100 * Math.sin(dt * 5), paint);
    paint.setTextSize(60);
    context.drawText("t = " + dt, 500, 600 + 100 * Math.sin(dt * 5), paint);
}, 10);

// Main event loop for alloutil.
setInterval(function() {
    allosphere.tick();
}, 10);
