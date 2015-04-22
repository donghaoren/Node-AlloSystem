#include "allosphere.h"
#include <map>
#include <alloutil/al_OmniApp.hpp>
#include <alloutil/al_OmniStereo.hpp>

#ifdef PLATFORM_MACOSX
  #include <OpenGL/OpenGL.h>
  #include <GLUT/glut.h>
#endif
#ifdef PLATFORM_LINUX
  #include <GL/glut.h>
  #include <GL/freeglut_ext.h>
#endif

#include <allovolume/omnistereo_renderer.h>

using namespace ::al;

#define str(x) #x

#pragma mark GLSL
static const char * vGeneric = AL_STRINGIFY(
    varying vec2 T;
    void main(void) {
        // pass through the texture coordinate (normalized pixel):
        T = vec2(gl_MultiTexCoord0);
        gl_Position = vec4(T*2.-1., 0, 1);
    }
);

#pragma mark Cube GLSL
static const char * fCube = AL_STRINGIFY(
    uniform sampler2D pixelMap;
    uniform sampler2D alphaMap;
    uniform sampler2D volumeFront;
    uniform sampler2D volumeBack;
    uniform samplerCube cubeMap;

    varying vec2 T;

    vec4 blend(vec4 src, vec4 dst) {
        vec4 result;
        result.a = src.a + dst.a * (1.0 - src.a);
        result.rgb = (src.rgb * src.a + dst.rgb * dst.a * (1.0 - src.a)) / result.a;
        if(result.a == 0.0) result.rgb = vec3(0.0, 0.0, 0.0);
        return result;
    }

    void main (void){
        // ray location (calibration space):
        vec3 v = normalize(texture2D(pixelMap, T).rgb);

        // index into cubemap:
        vec4 color = textureCube(cubeMap, v);
        // Volume colors are non-premultiplied.
        vec4 vf = texture2D(volumeFront, T);
        vec4 vb = texture2D(volumeBack, T);
        color = blend(vf, blend(color, vb));
        // //color = vf + vb;
        //color = blend(vf, color);

        vec3 rgb = color.rgb * color.a * texture2D(alphaMap, T).rgb;
        gl_FragColor = vec4(rgb, 1.0);
    }
);

namespace iv { namespace al {

    // class ApplicationImpl : public ::al::OmniApp, public Application {
    // public:
    //     ApplicationImpl() {
    //         delegate = NULL;
    //     }

    //     virtual void setDelegate(Delegate* delegate_) {
    //         delegate = delegate_;
    //     }

    //     static void do_nothing() { }

    //     virtual void initialize() {
    //         if (mOmni.activeStereo()) {
    //             Window::displayMode(Window::displayMode() | Window::STEREO_BUF);
    //         }

    //         create();

    //         if (mOmni.fullScreen()) {
    //             fullScreen(true);
    //             cursorHide(true);
    //         }

    //         if (!bSlave) {
    //             if(oscSend().opened()) sendHandshake();
    //             mAudioIO.start();
    //         }

    //         glutIdleFunc(do_nothing);
    //     }

    //     virtual void tick() {
    //     #ifdef PLATFORM_LINUX
    //         glutMainLoopEvent();
    //     #endif
    //     #ifdef PLATFORM_MACOSX
    //         glutCheckLoop();
    //     #endif
    //         ::al::Main::get().tick();
    //     }

    //     virtual void shaderUniformf(const char* name, float value) {
    //         shader().uniform(name, value);
    //     }

    //     virtual void shaderUniformi(const char* name, int value) {
    //         shader().uniform(name, value);
    //     }

    //     virtual void setLens(const Lens& lens_) {
    //         lens().eyeSep(lens_.eye_separation);
    //     }
    //     virtual void setPose(const Pose& pose_) {
    //         ::al::Pose& pose = mNav;
    //         pose.pos().x = pose_.position.x;
    //         pose.pos().y = pose_.position.y;
    //         pose.pos().z = pose_.position.z;
    //         pose.quat().x = pose_.rotation.v.x;
    //         pose.quat().y = pose_.rotation.v.y;
    //         pose.quat().z = pose_.rotation.v.z;
    //         pose.quat().w = pose_.rotation.w;
    //     }

    //     virtual double eye() {
    //         return mOmni.eye();
    //     }

    //     virtual bool onFrame() {
    //         if(delegate) delegate->onFrame();
    //         return OmniApp::onFrame();
    //     }

    //     virtual void onDraw(::al::Graphics& g) {
    //         if(delegate) delegate->onDraw();
    //     }

    //     Delegate* delegate;
    // };

    class ApplicationImpl : public Window, public FPS, public OmniStereo::Drawable, public Application, public osc::PacketHandler, public allovolume::OmnistereoRenderer::Delegate {
    public:

        ApplicationImpl() : mNavControl(mNav) {
            mLens.near(0.01).far(500).eyeSep(0.1);
            mNav.smooth(0.8);
            mRadius = 5;

            mOSCRecv = NULL;
            mNavSpeed = 1;
            mNavTurnSpeed = 0.02;

            window_navigation_enabled = false;

            Window::dimensions(Window::Dim(800, 400));
            Window::title("AllosphereNodejsApplication");
            Window::fps(60);
            Window::displayMode(Window::DEFAULT_BUF);

            Window::append(mStdControls);

            mOmni.configure("", Socket::hostName());
            if(mOmni.activeStereo()) {
                mOmni.mode(OmniStereo::ACTIVE).stereo(true);
            } else {
                mOmni.mode(OmniStereo::MONO).stereo(false);
            }

            mNextShaderID = 100;
            mNextTextureID = 100;

            allovolume_renderer = NULL;
        }

        virtual void setProjectionMode(ProjectionMode mode) {
            if(mode == PERSPECTIVE) {
                if(mOmni.mode() == OmniStereo::DUAL) {
                    mOmni.configure(OmniStereo::NOBLEND).configure(OmniStereo::RECT, (float)width() / height() / 2.0f);
                } else {
                    mOmni.configure(OmniStereo::NOBLEND).configure(OmniStereo::RECT, (float)width() / height());
                }
            } else if(mode == FISHEYE) {
                mOmni.configure(OmniStereo::NOBLEND).configure(OmniStereo::FISHEYE);
            }
        }

        virtual void setStereoMode(StereoMode mode) {
            if(mode == MONO) {
                mOmni.mode(OmniStereo::MONO).stereo(false);
            }
            if(mode == ANAGLYPH) {
                mOmni.mode(OmniStereo::ANAGLYPH).stereo(true);
            }
            if(mode == ANAGLYPH_BLEND) {
                mOmni.mode(OmniStereo::ANAGLYPH_BLEND).stereo(true);
            }
            if(mode == DUAL) {
                mOmni.mode(OmniStereo::DUAL).stereo(true);
            }
        }

        virtual void enableWindowNavigation() {
            Window::append(mNavControl);
            window_navigation_enabled = true;
        }

        virtual bool onCreate() {
            mOmni.onCreate();
            Shader cubeV, cubeF;
            cubeV.source(vGeneric, Shader::VERTEX).compile();
            cubeF.source(fCube, Shader::FRAGMENT).compile();
            mCubeProgram.attach(cubeV).attach(cubeF);
            mCubeProgram.link(false);   // false means do not validate
            // set uniforms before validating to prevent validation error
            mCubeProgram.begin();
                mCubeProgram.uniform("cubeMap", 0);
                mCubeProgram.uniform("pixelMap", 1);
                mCubeProgram.uniform("alphaMap", 2);
                mCubeProgram.uniform("volumeBack", 3);
                mCubeProgram.uniform("volumeFront", 4);
            mCubeProgram.end();
            mCubeProgram.validate();
            cubeV.printLog();
            cubeF.printLog();
            mCubeProgram.printLog();

            mDefaultShaderID = shaderCreate(default_vertex_code().c_str(), default_fragment_code().c_str());
            shaderBegin(mDefaultShaderID);
            shaderEnd(mDefaultShaderID);

            mOmni.clearColor() = ::al::Color(0, 0, 0, 0);

            if(delegate) delegate->onCreate();
        }

        virtual void launchAlloVolume() {
            allovolume_renderer = allovolume::OmnistereoRenderer::CreateWithYAMLConfig("/Users/donghao/Documents/Projects/AlloVolumeRendering/build/allovolume.yaml");
            allovolume_renderer->setDelegate(this);
        }

        virtual void onPresent() {
            printf("onPresent()\n");
        }

        virtual bool onFrame() {
            FPS::onFrame();
            while(mOSCRecv && mOSCRecv->recv());
            if(window_navigation_enabled) mNav.step();
            Viewport vp(width(), height());
            if(delegate) delegate->onFrame();
            mOmni.capture(*this, mLens, mNav);
            if(allovolume_renderer && allovolume_renderer->isReady()) {
                allovolume_renderer->loadDepthCubemap(mOmni.textureDepthLeft(), mOmni.textureDepthRight(), mLens.near(), mLens.far());

                allovolume_renderer->uploadTextures();

                glViewport(0, 0, width(), height());
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                for (int i = 0; i < mOmni.numProjections(); i++) {
                    OmniStereo::Projection& p = mOmni.projection(i);
                    Viewport& v = p.viewport();
                    Viewport viewport(
                        vp.l + v.l * vp.w,
                        vp.b + v.b * vp.h,
                        v.w * vp.w,
                        v.h * vp.h
                    );
                    gl.viewport(viewport);
                    //gl.clear(Graphics::COLOR_BUFFER_BIT | Graphics::DEPTH_BUFFER_BIT);
                    p.blend().bind(2);
                    p.warp().bind(1);

                    gl.error("OmniStereo cube draw begin");

                    mCubeProgram.begin();

                    gl.error("OmniStereo cube drawStereo begin");

                    allovolume::OmnistereoRenderer::Textures volume_front_back = allovolume_renderer->getTextures(i, 0);
                    glActiveTexture(GL_TEXTURE3);
                    glEnable(GL_TEXTURE_2D);
                    glBindTexture(GL_TEXTURE_2D, volume_front_back.back);
                    glActiveTexture(GL_TEXTURE4);
                    glEnable(GL_TEXTURE_2D);
                    glBindTexture(GL_TEXTURE_2D, volume_front_back.front);

                    glActiveTexture(GL_TEXTURE0);
                    glEnable(GL_TEXTURE_CUBE_MAP);

                    mOmni.drawStereo<&OmniStereo::drawEye>(mLens, mNav, viewport);

                    gl.error("OmniStereo cube drawStereo end");

                    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
                    glDisable(GL_TEXTURE_CUBE_MAP);

                    glActiveTexture(GL_TEXTURE3);
                    glBindTexture(GL_TEXTURE_2D, 0);
                    glActiveTexture(GL_TEXTURE4);
                    glBindTexture(GL_TEXTURE_2D, 0);

                    mCubeProgram.end();
                    gl.error("OmniStereo cube draw end");

                    p.blend().unbind(2);
                    p.warp().unbind(1);
                }
            } else {
                mOmni.draw(mLens, mNav, vp);
            }
            return true;
        }

        virtual void onDrawOmni(OmniStereo& omni) {
            DrawInfo info;
            info.eye = omni.eye();
            glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
            if(delegate) delegate->onDraw(info);
        }

        static void do_nothing() { }

        virtual void initialize() {
            if(mOmni.activeStereo()) {
                Window::displayMode(Window::displayMode() | Window::STEREO_BUF);
            }

            create();

            if(mOmni.fullScreen()) {
                fullScreen(true);
                cursorHide(true);
            }

            glutIdleFunc(do_nothing);

            std::printf("OpenGL Version: %s\n%s\n",
                glGetString(GL_RENDERER),  // e.g. Intel HD Graphics 3000 OpenGL Engine
                glGetString(GL_VERSION)    // e.g. 3.2 INTEL-8.0.61
            );
        }

        virtual void setLens(const Lens& lens_) {
            mLens.eyeSep(lens_.eye_separation);
            mRadius = lens_.focal_distance;
        }

        virtual void setPose(const Pose& pose_) {
            mNav.pos().z = pose_.position.x;
            mNav.pos().x = pose_.position.y;
            mNav.pos().y = pose_.position.z;
            mNav.quat().z = pose_.rotation.v.x;
            mNav.quat().x = pose_.rotation.v.y;
            mNav.quat().y = pose_.rotation.v.z;
            mNav.quat().w = pose_.rotation.w;
        }

        virtual void setDelegate(Application::Delegate* delegate_) {
            delegate = delegate_;
        }

        virtual void setSize(int width, int height) {
            Window::dimensions(Window::Dim(width, height));
        }

        virtual void screenCapture(int x, int y, int width, int height, void* data) {
            glReadBuffer(GL_FRONT);
            glReadPixels(x, y, width, height, GL_RGBA, GL_UNSIGNED_BYTE, data);
            // Flip the image.
            unsigned char* datab = (unsigned char*)data;
            for(int iy = 0; iy < height / 2; iy++) {
                unsigned char* row1 = datab + iy * width * 4;
                unsigned char* row2 = datab + (height - 1 - iy) * width * 4;
                for(int ix = 0; ix < width * 4; ix++) {
                    unsigned char tmp = row2[ix];
                    row2[ix] = row1[ix];
                    row1[ix] = tmp;
                }
            }
        }

        virtual void tick() {
        #ifdef PLATFORM_LINUX
            glutMainLoopEvent();
        #endif
        #ifdef PLATFORM_MACOSX
            glutCheckLoop();
        #endif
            ::al::Main::get().tick();
        }

        std::string per_vertex_glsl() { return str(
            uniform float omni_eye;
            uniform int omni_face;
            uniform float omni_radius;
            uniform float omni_near;
            uniform float omni_far;
            vec3 omni_displace(in vec3 vertex) {
                float l = length(vertex.xyz);
                vec3 vn = normalize(vertex.xyz);
                // Precise formula.
                float displacement = omni_eye *
                  (omni_radius * omni_radius -
                     sqrt(l * l * omni_radius * omni_radius +
                          omni_eye * omni_eye * (omni_radius * omni_radius - l * l))) /
                  (omni_radius * omni_radius - omni_eye * omni_eye);
                // Approximation.
                // float displacement = omni_eye * (1.0 - 1.0 / omni_radius);

                // omni-stereo effect (in eyespace XZ plane)
                // cross-product with up vector also ensures stereo fades out at Y
                // poles
                // v.xyz -= omni_eye * cross(vn, vec3(0, 1, 0));
                // simplified:
                vertex.xz += vec2(displacement * vn.z, displacement * -vn.x);
                return vertex;
            }
            vec4 omni_project(in vec4 vertex) {
                if (omni_face == 0) {
                  vertex.xyz = vec3(-vertex.z, -vertex.y, -vertex.x);
                }
                    // GL_TEXTURE_CUBE_MAP_NEGATIVE_X
                    else if (omni_face == 1) {
                  vertex.xyz = vec3(vertex.z, -vertex.y, vertex.x);
                }
                    // GL_TEXTURE_CUBE_MAP_POSITIVE_Y
                    else if (omni_face == 2) {
                  vertex.xyz = vec3(vertex.x, vertex.z, -vertex.y);
                }
                    // GL_TEXTURE_CUBE_MAP_NEGATIVE_Y
                    else if (omni_face == 3) {
                  vertex.xyz = vec3(vertex.x, -vertex.z, vertex.y);
                }
                    // GL_TEXTURE_CUBE_MAP_POSITIVE_Z
                    else if (omni_face == 4) {
                  vertex.xyz = vec3(vertex.x, -vertex.y, -vertex.z);
                }
                    // GL_TEXTURE_CUBE_MAP_NEGATIVE_Z
                    else {
                  vertex.xyz = vec3(-vertex.x, -vertex.y, vertex.z);
                }
                // convert into screen-space:
                // simplified perspective projection since fovy = 90 and aspect = 1
                vertex.zw = vec2((vertex.z * (omni_far + omni_near) +
                                  vertex.w * omni_far * omni_near * 2.) /
                                     (omni_near - omni_far),
                                 -vertex.z);
                return vertex;
            }
            vec4 omni_render(in vec4 vertex) {
                vertex.xyz = omni_displace(vertex.xyz);
                return omni_project(vertex);
            }
        ); }

        virtual int shaderCreate(const char* vertex_code, const char* fragment_code) {
            std::string version_line = "#version 120\n";
            ShaderProgram* shader = new ShaderProgram();
            Shader vert, frag;
            vert.source(version_line + per_vertex_glsl() + vertex_code, Shader::VERTEX).compile();
            vert.printLog();
            frag.source(version_line + per_vertex_glsl() + fragment_code, Shader::FRAGMENT).compile();
            frag.printLog();
            shader->attach(vert).attach(frag).link();
            shader->printLog();
            int id = mNextShaderID;
            mNextShaderID += 1;
            mShaders[id] = shader;
            return id;
        }

        virtual int shaderCreate(const char* vertex_code, const char* fragment_code,
            const char* geometry_code, int geometry_in_primitive, int geometry_out_primitive, int geometry_out_vertices) {
            std::string version_line = "#version 120\n";
            ShaderProgram* shader = new ShaderProgram();
            Shader vert, frag, geom;
            vert.source(version_line + per_vertex_glsl() + vertex_code, Shader::VERTEX).compile();
            vert.printLog();
            ::al::Graphics gl;
            gl.error("vert");
            frag.source(version_line + per_vertex_glsl() + fragment_code, Shader::FRAGMENT).compile();
            frag.printLog();
            gl.error("frag");
            geom.source(version_line + "#extension GL_EXT_geometry_shader4: enable\n" + per_vertex_glsl() + geometry_code, Shader::GEOMETRY).compile();
            geom.printLog();
            gl.error("geom");
            shader->setGeometryInputPrimitive((Graphics::Primitive)geometry_in_primitive);
            shader->setGeometryOutputPrimitive((Graphics::Primitive)geometry_out_primitive);
            shader->setGeometryOutputVertices(geometry_out_vertices);
            gl.error("set");
            shader->attach(geom).attach(vert).attach(frag);
            shader->link();
            shader->printLog();
            gl.error("link");
            int id = mNextShaderID;
            mNextShaderID += 1;
            mShaders[id] = shader;
            return id;
        }

        virtual int shaderDefault() {
            return mDefaultShaderID;
        }

        virtual void shaderBegin(int id) {
            mCurrentShader = mShaders[id];
            mCurrentShader->begin();
            mOmni.uniforms(*mCurrentShader);
            mCurrentShader->uniform("omni_radius", mRadius);
        }

        virtual void shaderEnd(int id) {
            if(mCurrentShader) {
                mCurrentShader->end();
                mCurrentShader = NULL;
            }
        }

        virtual void shaderDelete(int id) {
            if(mShaders[id])
                delete mShaders[id];
            mShaders[id] = NULL;
        }

        // Other functions to call.
        virtual void shaderUniformf(const char* name, float value) {
            mCurrentShader->uniform(name, value);
        }
        virtual void shaderUniform2f(const char* name, float value1, float value2) {
            mCurrentShader->uniform(name, value1, value2);
        }
        virtual void shaderUniform3f(const char* name, float value1, float value2, float value3) {
            mCurrentShader->uniform(name, value1, value2, value3);
        }
        virtual void shaderUniform4f(const char* name, float value1, float value2, float value3, float value4) {
            mCurrentShader->uniform(name, value1, value2, value3, value4);
        }

        virtual void shaderUniformi(const char* name, int value) {
            mCurrentShader->uniform(name, value);
        }

        virtual int textureCreate() {
            Texture* texture = new Texture();
            int id = mNextTextureID;
            mNextTextureID += 1;
            mTextures[id] = texture;
            return id;
        }

        virtual void textureBind(int id, int target) {
            mCurrentTexture = mTextures[id];
            mCurrentTexture->bind(target);
        }

        virtual void textureSubmit(int width, int height, void* rgba) {
            mCurrentTexture->width(width).height(height).submit(rgba);
        }

        virtual void textureUnbind(int id, int target) {
            mCurrentTexture->unbind(target);
            mCurrentTexture = NULL;
        }

        virtual void textureDelete(int id) {
            if(mTextures[id])
                delete mTextures[id];
            mTextures[id] = NULL;
        }

        inline std::string default_vertex_code() {
            return AL_STRINGIFY(
                varying vec4 color;
                varying vec3 normal, light_direction, eye_vector;
                vec4 iv_to_al(in vec4 v) {
                    return vec4(v.y, v.z, v.x, v.w);
                }
                vec3 iv_to_al_3(in vec3 v) {
                    return vec3(v.y, v.z, v.x);
                }
                void main() {
                    color = gl_Color;
                    vec4 vertex = gl_ModelViewMatrix * iv_to_al(gl_Vertex);
                    normal = gl_NormalMatrix * iv_to_al_3(gl_Normal);
                    vec3 V = vertex.xyz;
                    eye_vector = normalize(-V);
                    light_direction = normalize(vec3(iv_to_al_3(gl_LightSource[0].position.xyz) - V));
                    gl_TexCoord[0] = gl_MultiTexCoord0;
                    gl_Position = omni_render(vertex);
                }
            );
        }

        inline std::string default_fragment_code() {
            return AL_STRINGIFY(
                uniform float lighting;
                uniform float texture;
                uniform sampler2D texture0;
                varying vec4 color;
                varying vec3 normal, light_direction, eye_vector;
                void main() {
                    vec4 colorMixed;
                    if(texture > 0.0) {
                      vec4 textureColor = texture2D(texture0, gl_TexCoord[0].st);
                      colorMixed = mix(color, textureColor, texture);
                    } else {
                      colorMixed = color;
                    }
                    vec4 final_color = colorMixed * gl_LightSource[0].ambient;
                    vec3 N = normalize(normal);
                    vec3 L = light_direction;
                    float lambertTerm = max(dot(N, L), 0.0);
                    final_color += gl_LightSource[0].diffuse * colorMixed * lambertTerm;
                    vec3 E = eye_vector;
                    vec3 R = reflect(-L, N);
                    float spec = pow(max(dot(R, E), 0.0), 0.9 + 1e-20);
                    final_color += gl_LightSource[0].specular * spec;
                    gl_FragColor = mix(colorMixed, final_color, lighting);
                }
            );
        }

        ::al::Lens mLens;
        float mRadius;
        ::al::Graphics mGraphics;
        ::al::Nav mNav;
        ::al::NavInputControl mNavControl;
        bool window_navigation_enabled;

        OmniStereo mOmni;

        Application::Delegate* delegate;

        StandardWindowKeyControls mStdControls;

        std::map<int, ShaderProgram*> mShaders;
        int mNextShaderID;
        ShaderProgram* mCurrentShader;
        int mDefaultShaderID;

        std::map<int, Texture*> mTextures;
        int mNextTextureID;
        Texture* mCurrentTexture;

        allovolume::OmnistereoRenderer* allovolume_renderer;

        void onMessage(osc::Message& m) {
          float x;
          if (m.addressPattern() == "/mx") {
            m >> x;
            mNav.moveR(-x * mNavSpeed);

          } else if (m.addressPattern() == "/my") {
            m >> x;
            mNav.moveU(x * mNavSpeed);

          } else if (m.addressPattern() == "/mz") {
            m >> x;
            mNav.moveF(x * mNavSpeed);

          } else if (m.addressPattern() == "/tx") {
            m >> x;
            mNav.spinR(x * -mNavTurnSpeed);

          } else if (m.addressPattern() == "/ty") {
            m >> x;
            mNav.spinU(x * mNavTurnSpeed);

          } else if (m.addressPattern() == "/tz") {
            m >> x;
            mNav.spinF(x * -mNavTurnSpeed);

          } else if (m.addressPattern() == "/home") {
            mNav.home();

          } else if (m.addressPattern() == "/halt") {
            mNav.halt();
          }
        }
        osc::Recv* mOSCRecv;
        double mNavSpeed, mNavTurnSpeed;
        virtual void enableOSCNavigation() {
            mOSCRecv = new osc::Recv(12001);
            mOSCRecv->bufferSize(32000);
            mOSCRecv->handler(*this);
        }
        Graphics gl;
        ShaderProgram mCubeProgram;
    };

    Application* Application::Create() {
        return new ApplicationImpl();
    }

} }
