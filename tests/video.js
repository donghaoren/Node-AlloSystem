var allosphere = require("node_allosphere");
var graphics = require("node_graphics");
var GL = allosphere.OpenGL;

allosphere.initialize();

function EquirectangularVideoTexture(filename, mode) {
    if(!mode) mode = "left-right";
    this.video = new graphics.VideoSurface2D(filename);
    this.mode = mode;
    if(this.mode == "mono") {
        var texture = allosphere.textureCreate();
        this.textures = [ texture, texture ];
    } else {
        var texture_left = allosphere.textureCreate();
        var texture_right = allosphere.textureCreate();
        this.textures = [ texture_left, texture_right ];
    }
    this.nextFrame();
}

EquirectangularVideoTexture.prototype.nextFrame = function() {
    this.video.nextFrame();
    if(this.mode == "mono") {
        console.log("upload.", this.video.width(), this.video.height());
        allosphere.textureBind(this.textures[0], 0);
        allosphere.textureSubmit(this.video.width(), this.video.height(), this.video.pixels());
    }
    if(this.mode == "top-down") {
        allosphere.textureBind(this.textures[0], 0);
        allosphere.textureSubmit(this.video.width(), this.video.height() / 2, this.video.pixels());
        allosphere.textureBind(this.textures[1], 0);
        allosphere.textureSubmit(this.video.width(), this.video.height() / 2, this.video.pixels().slice(this.video.width() * this.video.height() / 2 * 4));
    }

    if(this.mode == "left-right") {
        GL.pixelStorei(GL.UNPACK_ROW_LENGTH, this.video.width());
        allosphere.textureBind(this.textures[0], 0);
        allosphere.textureSubmit(this.video.width() / 2, this.video.height(), this.video.pixels());
        allosphere.textureBind(this.textures[1], 0);
        allosphere.textureSubmit(this.video.width() / 2, this.video.height(), this.video.pixels().slice(this.video.width() / 2 * 4));
        GL.pixelStorei(GL.UNPACK_ROW_LENGTH, 0);
    }
};

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
    allosphere.textureBind(texture, 2)
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

    allosphere.textureUnbind(texture, 2)
    allosphere.shaderEnd(this.shader_id);
};

var video = new EquirectangularVideoTexture("a.mp4");

function go() {
    video.nextFrame();
    setTimeout(go, 1);
}
go();


var pana_renderer = new EquirectangularRenderer(allosphere);

var frame_time = 0;
var t0 = new Date().getTime();
allosphere.onFrame(function() {
    frame_time = (new Date().getTime() - t0) / 1000;
});

// Draw your stuff with OpenGL.
allosphere.onDraw(function(info) {
    GL.enable(GL.BLEND);
    GL.blendFunc(GL.SRC_ALPHA, GL.ONE_MINUS_SRC_ALPHA);


    var t = frame_time;
    //GL.pushMatrix();
    //GL.rotatef(t * 30, 1, 1, 0);
    pana_renderer.render(video.textures, info);
    //GL.popMatrix();
});

// Main event loop for alloutil.
setInterval(function() {
    allosphere.tick();
}, 10);
