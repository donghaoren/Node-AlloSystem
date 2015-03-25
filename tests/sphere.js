var allosphere = require("node_allosphere");
var graphics = require("node_graphics");
var GL = allosphere.OpenGL;

allosphere.initialize();

var lineShader = null;

IV = { };
(function() {
    var multiline_regex = /\/\*!?(?:\@preserve)?[ \t]*(?:\r\n|\n)([\s\S]*?)(?:\r\n|\n)\s*\*\//;
    IV.multiline = function(fn) {
        var match = multiline_regex.exec(fn.toString());
        if(!match) {
            throw new TypeError('Multiline comment missing.');
        }
        return match[1];
    };
})();

var lineShader_GeometryCode = IV.multiline(function() {/*@preserve
varying in vec4 colors[2];
varying in vec3 normals[2];
varying out vec4 color;
varying out vec3 line_direction, light_direction, eye_vector;
varying float specular_boost;

uniform int line_type;
uniform float curveness;

vec3 iv_to_al_3(in vec3 v) {
    return vec3(v.y, v.z, v.x);
}

vec4 iv_to_al(in vec4 v) {
    return vec4(v.y, v.z, v.x, v.w);
}

void bezierCurve(vec3 p1, vec3 p2, vec3 p3, vec3 p4) {
    int tick_count = 40;
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
        light_direction = normalize((gl_ModelViewMatrix * iv_to_al(gl_LightSource[0].position)).xyz - p);
        eye_vector = normalize(-p);
        specular_boost = max(max(0.0, 1.0 - 5.0 * t), max(0.0, 1.0 - 5.0 * (1.0 - t)));
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
        light_direction = normalize((gl_ModelViewMatrix * iv_to_al(gl_LightSource[0].position)).xyz - p);
        eye_vector = normalize(-p);
        specular_boost = max(max(0.0, 1.0 - 5.0 * t), max(0.0, 1.0 - 5.0 * (1.0 - t)));
        EmitVertex();
    }
    EndPrimitive();
}

void line2(vec3 p1, vec3 p2) {
    int tick_count = 10;
    int i;
    line_direction = normalize(p2 - p1);
    for(i = 0; i <= tick_count; i++) {
        float t = float(i) / float(tick_count);
        vec3 p = p1 + (p2 - p1) * t;
        gl_Position = omni_render(vec4(p, 1.0f));
        light_direction = normalize((gl_ModelViewMatrix * iv_to_al(gl_LightSource[0].position)).xyz - p);
        eye_vector = normalize(-p);
        EmitVertex();
    }
    EndPrimitive();
}

void bezierCurve2(vec3 p1, vec3 p2, vec3 p3, vec3 p4) {
    line2(p1, p2);
    line2(p2, p3);
    line2(p3, p4);
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
        float scale = curveness / 6.0f;
        float s = length(p2 - p1) * scale;
        vec3 d = p2 - p1;
        d = vec3(0, 0, 0);
        bezierCurve(p1, p1 + n1 * s + d * abs(scale), p2 + n2 * s - d * abs(scale), p2);
    }
}
*/console.log});

var lineShader_VertexCode = IV.multiline(function() {/*@preserve
varying vec4 colors;
varying vec3 normals;

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
*/console.log});

var lineShader_FragmentCode = IV.multiline(function() {/*@preserve
uniform float specular_term;
varying vec4 color;
varying vec3 line_direction, light_direction, eye_vector;
varying float specular_boost;
void main() {
    vec4 colorMixed = color;
    vec4 final_color = colorMixed * (gl_LightSource[0].ambient);
    vec3 T = normalize(line_direction); // tangent direction.
    vec3 L = normalize(light_direction);
    vec3 LN = normalize(L - T * dot(L, T));
    float lambertTerm = max(dot(LN, L), 0.0);
    final_color += gl_LightSource[0].diffuse * colorMixed * lambertTerm;
    vec3 E = eye_vector;
    vec3 R = reflect(-L, LN);
    float spec = pow(max(dot(R, E), 0.0), specular_term) + specular_boost;
    final_color += gl_LightSource[0].specular * spec;
    final_color.rgb *= final_color.a;
    gl_FragColor = final_color;
}
*/console.log});

var lineShader_begin = function(g, specular, line_type, curveness) {
    if(!lineShader) lineShader = g.allosphere.shaderCreateWithGeometry(
        lineShader_VertexCode, lineShader_FragmentCode,
        lineShader_GeometryCode, GL.LINES, GL.LINE_STRIP, 50
    );
    g.allosphere.shaderBegin(lineShader);
    g.allosphere.shaderUniformf("specular_term", specular);
    g.allosphere.shaderUniformi("line_type", line_type);
    g.allosphere.shaderUniformf("curveness", curveness);
};

var lineShader_end = function(g) {
    g.allosphere.shaderEnd(lineShader);
};

function draw_sphere(x, y, z, r) {
}


allosphere.onDraw(function(info) {
    GL.shadeModel(GL.SMOOTH);
    GL.lightfv(GL.LIGHT0, GL.POSITION, [ 0, 0, 0, 1 ]);
    GL.lightfv(GL.LIGHT0, GL.AMBIENT, [ 0.3, 0.3, 0.3, 1 ]);
    GL.lightfv(GL.LIGHT0, GL.DIFFUSE, [ 0.7, 0.7, 0.7, 1 ]);
    GL.lightfv(GL.LIGHT0, GL.SPECULAR, [ 1, 1, 1, 1 ]);
    var g = { allosphere: allosphere, GL: GL };

    g.GL.blendFunc(g.GL.ONE, g.GL.ONE_MINUS_SRC_ALPHA);
    g.GL.lineWidth(3);
    g.GL.enable(g.GL.LINE_SMOOTH);
    g.GL.hint(g.GL.LINE_SMOOTH_HINT, g.GL.NICEST);
    lineShader_begin(g, 10, 1, 2);
    g.GL.begin(g.GL.LINES);
    g.GL.color4f(1, 1, 1, 1);
    g.GL.normal3f(0, 0, 1);
    g.GL.vertex3f(0, 5, 0);
    g.GL.normal3f(0, 0, 1);
    g.GL.vertex3f(0, -5, 0);
    g.GL.end();
    lineShader_end(g);

    GL.flush();
});

allosphere.setProjectionMode("perspective");
        allosphere.setStereoMode("anaglyph");
        allosphere.enableWindowNavigation();

// Main event loop for alloutil.
setInterval(function() {
    allosphere.tick();
}, 10);
