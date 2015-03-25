var allosphere = require("node_allosphere");
var graphics = require("node_graphics");
var GL = allosphere.OpenGL;

IV = { };
var reCommentContents = /\/\*!?(?:\@preserve)?[ \t]*(?:\r\n|\n)([\s\S]*?)(?:\r\n|\n)[ \t]*\*\//;

IV.multiline = function (fn) {
    if (typeof fn !== 'function') {
        throw new TypeError('Expected a function');
    }

    var match = reCommentContents.exec(fn.toString());

    if (!match) {
        throw new TypeError('Multiline comment missing.');
    }

    return match[1].trim();
};

allosphere.initialize();

var lineShader = null;

var lineShader_GeometryCode = IV.multiline(function() {/*
varying in vec4 colors[2];
varying in vec3 normals[2];
varying out vec4 color;
varying out vec3 line_direction, light_direction, eye_vector;

uniform int line_type;

void bezierCurve(vec3 p1, vec3 p2, vec3 p3, vec3 p4) {
    int tick_count = 50;
    int i;
    for(i = 0; i <= tick_count; i++) {
        float t = float(i) / float(tick_count);
        float t2 = t * t;
        float t3 = t2 * t;
        float k1 = 1 - 3 * t + 3 * t2 - t3;
        float dk1 = -3 + 6 * t - 3 * t2;
        float k2 = 3 * t - 6 * t2 + 3 * t3;
        float dk2 = 3 - 12 * t + 9 * t2;
        float k3 = 3 * t2 - 3 * t3;
        float dk3 = 6 * t - 9 * t2;
        float k4 = t3;
        float dk4 = 3 * t2;
        vec3 p = p1 * k1 + p2 * k2 + p3 * k3 + p4 * k4;
        line_direction = normalize(p1 * dk1 + p2 * dk2 + p3 * dk3 + p4 * dk4);
        gl_Position = omni_render(vec4(p, 1.0f));
        light_direction = normalize(gl_LightSource[0].position.xyz - p);
        eye_vector = normalize(-p);
        EmitVertex();
    }
    EndPrimitive();
}

void line(vec3 p1, vec3 p2) {
    int tick_count = 50;
    int i;
    line_direction = normalize(p2 - p1);
    for(i = 0; i <= tick_count; i++) {
        float t = float(i) / float(tick_count);
        vec3 p = p1 + (p2 - p1) * t;
        gl_Position = omni_render(vec4(p, 1.0f));
        light_direction = normalize(gl_LightSource[0].position.xyz - p);
        eye_vector = normalize(-p);
        EmitVertex();
    }
    EndPrimitive();
}

void main() {
    color = colors[0];
    vec3 p1 = gl_PositionIn[0].xyz;
    vec3 p2 = gl_PositionIn[1].xyz;
    vec3 n1 = normalize(normals[0]);
    vec3 n2 = normalize(normals[1]);
    if(line_type == 0) {
        line(p1, p2);
    } else {
        float s = length(p2 - p1) / 3.0f;
        bezierCurve(p1, p1 + n1 * s, p2 + n2 * s, p2);
    }
}
*/});

var lineShader_VertexCode = IV.multiline(function() {/*
varying vec4 colors;
varying vec3 normals;
varying vec3 light_directions, eye_vectors;

vec4 iv_to_al(in vec4 v) {
    return vec4(v.y, v.z, v.x, v.w);
}

vec3 iv_to_al_3(in vec3 v) {
    return vec3(v.y, v.z, v.x);
}

void main() {
    colors = gl_Color;
    normals = gl_NormalMatrix * iv_to_al_3(gl_Normal);
    vec4 vertex = gl_ModelViewMatrix * iv_to_al(gl_Vertex);
    gl_Position = vertex;
}
*/});
var lineShader_FragmentCode = IV.multiline(function() {/*
uniform float specular_term;
varying vec4 color;
varying vec3 line_direction, light_direction, eye_vector;
void main() {
    vec4 colorMixed = color;
    vec4 final_color = colorMixed * gl_LightSource[0].ambient;
    vec3 T = line_direction; // tangent direction.
    vec3 L = light_direction;
    vec3 LN = normalize(L - T * dot(L, T));
    float lambertTerm = max(dot(LN, L), 0.0);
    final_color += gl_LightSource[0].diffuse * colorMixed * lambertTerm;
    vec3 E = eye_vector;
    vec3 R = reflect(-L, LN);
    float spec = pow(max(dot(R, E), 0.0), specular_term);
    final_color += gl_LightSource[0].specular * spec;
    gl_FragColor = final_color;
}
*/});


var lineShader_begin = function(g, specular) {
    if(!lineShader) lineShader = g.allosphere.shaderCreateWithGeometry(
        lineShader_VertexCode, lineShader_FragmentCode, lineShader_GeometryCode,
        GL.LINES, GL.LINE_STRIP, 50);
    g.allosphere.shaderBegin(lineShader);
    g.allosphere.shaderUniformf("specular_term", specular);
    g.allosphere.shaderUniformi("line_type", 1);
};
var lineShader_end = function(g) {
    g.allosphere.shaderEnd(g.allosphere.shaderDefault());
};

allosphere.onDraw(function(info) {
    var g = {
        allosphere: allosphere,
        GL: GL
    };
    GL.enable(GL.BLEND);
    GL.shadeModel(GL.SMOOTH);
                    GL.hint(GL.LINE_SMOOTH_HINT, GL.NICEST);
                    GL.blendFunc(GL.SRC_ALPHA, GL.ONE_MINUS_SRC_ALPHA);
                    GL.enable(GL.DEPTH_TEST);
                    GL.lightfv(GL.LIGHT0, GL.POSITION, [ 0, 0, 0 ]);
                    GL.lightfv(GL.LIGHT0, GL.AMBIENT, [ 0.2, 0.2, 0.2, 1 ]);
                    GL.lightfv(GL.LIGHT0, GL.DIFFUSE, [ 0.8, 0.8, 0.8, 1 ]);
                    GL.lightfv(GL.LIGHT0, GL.SPECULAR, [ 1, 1, 1, 1 ]);
    GL.blendFunc(GL.SRC_ALPHA, GL.ONE_MINUS_SRC_ALPHA);
    lineShader_begin(g, 10.0);
    GL.begin(GL.LINES);
    GL.color4f(1, 0, 0, 1);
    for(var i = 0; i < 50; i++) {
        var offset = new Date().getTime() / 1000;
        var angle = i / 50 * Math.PI * 2;
        var x1 = Math.sin(angle);
        var y1 = Math.cos(angle);
        var x2 = Math.sin(angle + offset);
        var y2 = Math.cos(angle + offset);
        GL.normal3f(0, -1, 1);
        GL.vertex3f(-3 + x1, 3 + y1, y1);
        GL.normal3f(0, +1, 1);
        GL.vertex3f(-3 + x2, -3 - y2, y2);
    }
    GL.end();
    lineShader_end(g);
});

setInterval(function() {
    allosphere.tick();
}, 10);
