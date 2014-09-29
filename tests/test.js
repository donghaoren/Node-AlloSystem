var allosphere = require("node_allosphere");
var graphics = require("node_graphics");
var GL = allosphere.OpenGL;

allosphere.initialize();

var s = new graphics.Surface2D(1000, 1000);

function EquirectangularTexture(filename) {
    var image = graphics.loadImageData(require("fs").readFileSync(filename));
    if(image.width() == image.height()) {
        var texture_left = allosphere.textureCreate();
        var texture_right = allosphere.textureCreate();
        allosphere.textureBind(texture_left, 0);
        allosphere.textureSubmit(image.width(), image.height() / 2, image.pixels());
        allosphere.textureBind(texture_right, 0);
        allosphere.textureSubmit(image.width(), image.height() / 2, image.pixels().slice(image.width() * image.height() / 2 * 4));
        this.textures = [ texture_left, texture_right ];
    } else {
        var texture = allosphere.textureCreate();
        allosphere.textureBind(texture, 0);
        allosphere.textureSubmit(image.width(), image.height(), image.pixels());
        this.textures = [ texture, texture ];
    }
}

var context = new graphics.GraphicalContext2D(s);
var paint = context.paint();

function EquirectangularRenderer(allosphere) {
    var vertex_shader = [
    "    void main() {",
    "        vec4 vertex = gl_ModelViewMatrix * gl_Vertex;",
    "        gl_TexCoord[0] = gl_MultiTexCoord0;",
    "        gl_Position = omni_render(vertex);",
    "    }"
    ].join("\n");
    var fragment_shader = [
    "    uniform sampler2D texture0;",
    "    const float PI = 3.141592653589793238462643383;",
    "    void main() {",
    "        vec3 position = -gl_TexCoord[0].zxy;",
    "        float phi = atan(position.y, position.x);",
    "        float theta = atan(position.z, length(position.xy));",
    "        vec2 st;",
    "        st.x = -phi / PI / 2.0 + 0.5;",
    "        st.y = theta / PI + 0.5;",
    "        vec4 textureColor = texture2D(texture0, st);",
    "        gl_FragColor = textureColor;",
    "    }"
    ].join("\n");

    this.shader_id = allosphere.shaderCreate(vertex_shader, fragment_shader);
};

EquirectangularRenderer.prototype.render = function(textures, info) {
    var texture = textures[info.eye > 0 ? 0 : 1];
    allosphere.shaderBegin(this.shader_id);
    allosphere.shaderUniformi("texture0", 2);
    allosphere.shaderUniformf("omni_eye", 0);
    allosphere.textureBind(texture, 2);
    var s = 10;
    var vertices = [  //           7------4      z
      [ +s, +s, +s ], // 0        /|     /|      ^
      [ +s, +s, -s ], // 1       3-|----0 |      + > y
      [ +s, -s, -s ], // 2       | 6----|-5
      [ +s, -s, +s ], // 3       |/     |/
      [ -s, +s, +s ], // 4       2------1
      [ -s, +s, -s ], // 5
      [ -s, -s, -s ], // 6
      [ -s, -s, +s ]  // 7
    ];
    var faces = [
      [ 0, 1, 2, 3 ],
      [ 4, 0, 3, 7 ],
      [ 4, 5, 1, 0 ],
      [ 3, 2, 6, 7 ],
      [ 7, 6, 5, 4 ],
      [ 1, 5, 6, 2 ]
    ];

    GL.begin(GL.QUADS);
    for(var i = 0; i < faces.length; i++) {
        var f = faces[i];
        for(var j = 0; j < 4; j++) {
            var v = vertices[f[j]];
            GL.texCoord3f(v[0], v[1], v[2]);
            GL.normal3f(-v[0], -v[1], -v[2]);
            GL.vertex3f(v[0], v[1], v[2]);
        }
    }
    GL.end();

    allosphere.textureUnbind(texture, 2);
    allosphere.shaderEnd(this.shader_id);
};


var pana_renderer = new EquirectangularRenderer(allosphere);
var pana_texture = new EquirectangularTexture("tests/test2.png");

var frame_time = 0;
allosphere.onFrame(function() {
    frame_time = (new Date().getTime() - t0) / 1000;
});

// Draw your stuff with OpenGL.
allosphere.onDraw(function(info) {
    GL.enable(GL.BLEND);
    GL.blendFunc(GL.SRC_ALPHA, GL.ONE_MINUS_SRC_ALPHA);

    var id = allosphere.shaderDefault();
    allosphere.shaderBegin(id);

    s.bindTexture(2);

    allosphere.shaderUniformf("texture", 1);
    allosphere.shaderUniformi("texture0", 2);
    allosphere.shaderUniformf("lighting", 0);

    var sc = (Math.sin(frame_time) * 0 + 5) / Math.sqrt(2*2+5*5);

    GL.begin(GL.QUADS);
    GL.texCoord2f(0, 0); GL.normal3f(0, 1, 0); GL.vertex3f(  2*sc, -5*sc, -2*sc);
    GL.texCoord2f(0, 1); GL.normal3f(0, 1, 0); GL.vertex3f( -2*sc, -5*sc, -2*sc);
    GL.texCoord2f(1, 1); GL.normal3f(0, 1, 0); GL.vertex3f( -2*sc, -5*sc,  2*sc);
    GL.texCoord2f(1, 0); GL.normal3f(0, 1, 0); GL.vertex3f(  2*sc, -5*sc,  2*sc);
    GL.end();

    s.unbindTexture(2);
    allosphere.shaderEnd(id);

    var t = frame_time;
    //GL.pushMatrix();
    //GL.rotatef(t * 30, 1, 1, 0);
    pana_renderer.render(pana_texture.textures, info);
    //GL.popMatrix();
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
    s.uploadTexture();
}, 10);

// Main event loop for alloutil.
setInterval(function() {
    allosphere.tick();
}, 10);
