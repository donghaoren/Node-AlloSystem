var allosphere = require("node_allosphere");
var graphics = require("node_graphics");
var GL = allosphere.OpenGL;

allosphere.initialize();

// Draw your stuff with OpenGL.
allosphere.onDraw(function(info) {
    GL.enable(GL.BLEND);
    GL.blendFunc(GL.SRC_ALPHA, GL.ONE_MINUS_SRC_ALPHA);

    var id = allosphere.shaderDefault();
    allosphere.shaderBegin(id);
    allosphere.shaderUniformf("lighting", 1);

    var draw_cube = function(x, y, z, scale) {
        GL.begin(GL.QUADS);
        GL.normal3f(+1, 0, 0);
        GL.vertex3f(x + scale, y + scale, z + scale);
        GL.vertex3f(x + scale, y - scale, z + scale);
        GL.vertex3f(x + scale, y - scale, z - scale);
        GL.vertex3f(x + scale, y + scale, z - scale);
        GL.normal3f(-1, 0, 0);
        GL.vertex3f(x - scale, y + scale, z + scale);
        GL.vertex3f(x - scale, y + scale, z - scale);
        GL.vertex3f(x - scale, y - scale, z - scale);
        GL.vertex3f(x - scale, y - scale, z + scale);

        GL.normal3f(0, +1, 0);
        GL.vertex3f(x + scale, y + scale, z + scale);
        GL.vertex3f(x + scale, y + scale, z - scale);
        GL.vertex3f(x - scale, y + scale, z - scale);
        GL.vertex3f(x - scale, y + scale, z + scale);
        GL.normal3f(0, -1, 0);
        GL.vertex3f(x + scale, y - scale, z + scale);
        GL.vertex3f(x - scale, y - scale, z + scale);
        GL.vertex3f(x - scale, y - scale, z - scale);
        GL.vertex3f(x + scale, y - scale, z - scale);

        GL.normal3f(0, 0, +1);
        GL.vertex3f(x + scale, y + scale, z + scale);
        GL.vertex3f(x - scale, y + scale, z + scale);
        GL.vertex3f(x - scale, y - scale, z + scale);
        GL.vertex3f(x + scale, y - scale, z + scale);
        GL.normal3f(0, 0, -1);
        GL.vertex3f(x + scale, y + scale, z - scale);
        GL.vertex3f(x + scale, y - scale, z - scale);
        GL.vertex3f(x - scale, y - scale, z - scale);
        GL.vertex3f(x - scale, y + scale, z - scale);
        GL.end(GL.QUADS);
    };

    var step = 2;
    var scale = 0.05;
    var count = 5;
    for(var xt = -count; xt <= count; xt++)
    for(var yt = -count; yt <= count; yt++)
    for(var zt = -count; zt <= count; zt++) {
        if(xt == 0 && yt == 0 && zt == 0) continue;
        draw_cube(xt * step, yt * step, zt * step, scale);
    }

    allosphere.shaderEnd(id);
});

// Main event loop for alloutil.
setInterval(function() {
    allosphere.tick();
}, 10);
