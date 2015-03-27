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

var sphereShader_GeometryCode = IV.multiline(function() {/*@preserve
varying in vec4 colors[1];
varying in vec3 normals[1];
varying in float radiuses[1];

varying out vec4 color;
varying out float radius;
varying out vec3 center;
varying out vec3 p_prime;

vec3 iv_to_al_3(in vec3 v) {
    return vec3(v.y, v.z, v.x);
}

vec4 iv_to_al(in vec4 v) {
    return vec4(v.y, v.z, v.x, v.w);
}

void main() {
    color = colors[0];
    radius = radiuses[0];
    center = gl_PositionIn[0].xyz;

    int sides = 24;

    float d = length(center);
    if(d <= radius) return;

    float x = radius * radius / d;
    vec3 center_prime = center - center * (x / d);
    float radius_prime = sqrt(radius * radius - x * x);
    radius_prime /= cos(3.1415926535897932 / sides);
    radius_prime *= 1.1;
    vec3 up = vec3(0, 1, 1);
    vec3 ex = normalize(cross(center, up));
    vec3 ey = normalize(cross(ex, center));
    ex *= radius_prime;
    ey *= radius_prime;

    vec3 p0 = center_prime + ex;

    for(int i = 0; i <= sides; i++) {
        float t = float(i) / sides * 3.1415926535897932 * 2;
        vec3 p1 = center_prime + ex * cos(t) + ey * sin(t);

        p_prime = center_prime; gl_Position = omni_render(vec4(p_prime, 1.0)); EmitVertex();
        p_prime = p1; gl_Position = omni_render(vec4(p_prime, 1.0)); EmitVertex();
    }
    EndPrimitive();
}
*/console.log});

var sphereShader_VertexCode = IV.multiline(function() {/*@preserve
varying vec4 colors;
varying vec3 normals;
varying float radiuses;

vec4 iv_to_al(in vec4 v) {
    return vec4(v.y, v.z, v.x, v.w);
}

vec3 iv_to_al_3(in vec3 v) {
    return vec3(v.y, v.z, v.x);
}

void main() {
    colors = gl_Color;
    normals = gl_NormalMatrix * iv_to_al_3(gl_Normal);
    vec4 vertex = gl_ModelViewMatrix * iv_to_al(vec4(gl_Vertex.xyz, 1.0));
    radiuses = gl_Vertex.w;
    gl_Position = vertex;
}
*/console.log});

var sphereShader_FragmentCode = IV.multiline(function() {/*@preserve
uniform float specular_term;
varying vec4 color;
varying float radius;
varying vec3 center;
varying vec3 p_prime;

void main() {
    float qa = dot(p_prime, p_prime);
    float qb = -2.0 * dot(p_prime, center);
    float qc = dot(center, center) - radius * radius;
    float qd = qb * qb - 4.0 * qa * qc;
    if(qd <= 0.0) discard;
    float t = (-qb - sqrt(qd)) / qa / 2.0;

    vec3 p = (p_prime * t);

    vec3 N = normalize(p - center);
    vec3 L = normalize((gl_ModelViewMatrix * (gl_LightSource[0].position.yzxw)).xyz - p);
    vec3 R = reflect(-L, N);

    vec4 colorMixed = color;
    vec4 final_color = colorMixed * (gl_LightSource[0].ambient);

    float lambertTerm = max(dot(N, L), 0.0);
    final_color += gl_LightSource[0].diffuse * colorMixed * lambertTerm;
    vec3 E = normalize(-p);
    float spec = pow(max(dot(R, E), 0.0), specular_term);
    final_color += gl_LightSource[0].specular * spec;
    final_color.a = color.a;
    final_color.rgb *= final_color.a;
    gl_FragColor = final_color;

    vec4 clip_position = omni_project(vec4(p, 1.0));
    vec3 pixel_position;
    pixel_position.xy = clip_position.xy;
    pixel_position.z = -clip_position.w;
    pixel_position = pixel_position * (length(p) / length(pixel_position));
    float z2 = (pixel_position.z * (omni_far + omni_near) + omni_far * omni_near * 2.0f) / (omni_near - omni_far);
    gl_FragDepth = (z2 / -pixel_position.z * 0.5 + 0.5);
}
*/console.log});

var sphereShader_begin = function(g, specular) {
    if(!lineShader) lineShader = g.allosphere.shaderCreateWithGeometry(
        sphereShader_VertexCode, sphereShader_FragmentCode,
        sphereShader_GeometryCode, GL.POINTS, GL.TRIANGLE_STRIP, 50
    );
    g.allosphere.shaderBegin(lineShader);
    g.allosphere.shaderUniformf("specular_term", specular);
};

var sphereShader_end = function(g) {
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

    g.GL.enable(GL.BLEND);
    g.GL.blendFunc(g.GL.ONE, g.GL.ONE_MINUS_SRC_ALPHA);
    sphereShader_begin(g, 40);
    g.GL.begin(g.GL.POINTS);
    g.GL.color4f(1, 1, 1, 1);
    g.GL.vertex4f(-2, 1, 1, 0.7);
    g.GL.vertex4f(-2, 2, 1, 0.7);
    g.GL.end();
    sphereShader_end(g);

    GL.flush();
});

allosphere.setStereoMode("anaglyph_blend");
allosphere.setProjectionMode("perspective");
allosphere.enableWindowNavigation();

// Main event loop for alloutil.
setInterval(function() {
    allosphere.tick();
}, 10);
