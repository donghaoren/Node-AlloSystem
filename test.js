var allosphere = require("node_allosphere");
var GL = allosphere.OpenGL;
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
}, 10);

var graphics = require("node_graphics");

var s = new graphics.Surface2D(1000, 1000);
console.log(s.width());

var context = new graphics.GraphicalContext2D(s);
var paint = context.paint();


context.clear(255, 255, 255, 1);
context.drawLine(0, 0, 1000, 1000, paint);
paint.setTypeface("Arial", graphics.FONTSTYLE_NORMAL);
paint.setTextSize(120);
paint.setTextAlign(graphics.TEXTALIGN_CENTER);
paint.setMode(graphics.PAINTMODE_FILL);
context.drawText("Hello World", 500, 500, paint);

//s.save("test.png");
s.uploadTexture();

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

    s.unbindTexture(2);
});
