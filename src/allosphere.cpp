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

using namespace ::al;

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

    class ApplicationImpl : public Window, public FPS, public OmniStereo::Drawable, public Application {
    public:

        ApplicationImpl() {
            mLens.near(0.01).far(40).eyeSep(0.03);
            mNav.smooth(0.8);

            Window::dimensions(Window::Dim(800, 400));
            Window::title("AllosphereNodejsApplication");
            Window::fps(60);
            Window::displayMode(Window::DEFAULT_BUF);

            Window::append(mStdControls);

            mOmni.configure("", Socket::hostName());
            if(mOmni.activeStereo()) {
                mOmni.mode(OmniStereo::ACTIVE).stereo(true);
            }

            mNextShaderID = 100;
            mNextTextureID = 100;
        }

        virtual bool onCreate() {
            mOmni.onCreate();

            mDefaultShaderID = shaderCreate(default_vertex_code().c_str(), default_fragment_code().c_str());
            shaderBegin(mDefaultShaderID);
            shaderEnd(mDefaultShaderID);

            if(delegate) delegate->onCreate();
        }

        virtual bool onFrame() {
            FPS::onFrame();
            Viewport vp(width(), height());
            if(delegate) delegate->onFrame();
            mOmni.onFrame(*this, mLens, mNav, vp);
            return true;
        }

        virtual void onDrawOmni(OmniStereo& omni) {
            DrawInfo info;
            info.eye = omni.eye();
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
        }

        virtual void setLens(const Lens& lens_) {
            mLens.eyeSep(lens_.eye_separation);
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

        virtual void setDelegate(Delegate* delegate_) {
            delegate = delegate_;
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

        virtual int shaderCreate(const char* vertex_code, const char* fragment_code) {
            ShaderProgram* shader = new ShaderProgram();
            Shader vert, frag;
            vert.source(OmniStereo::glsl() + vertex_code, Shader::VERTEX).compile();
            vert.printLog();
            frag.source(fragment_code, Shader::FRAGMENT).compile();
            frag.printLog();
            shader->attach(vert).attach(frag).link();
            shader->printLog();
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
                    light_direction = normalize(vec3(gl_LightSource[0].position.xyz - V));
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
        ::al::Graphics mGraphics;
        ::al::Nav mNav;

        OmniStereo mOmni;

        Delegate* delegate;

        StandardWindowKeyControls mStdControls;

        std::map<int, ShaderProgram*> mShaders;
        int mNextShaderID;
        ShaderProgram* mCurrentShader;
        int mDefaultShaderID;

        std::map<int, Texture*> mTextures;
        int mNextTextureID;
        Texture* mCurrentTexture;

    };

    Application* Application::Create() {
        return new ApplicationImpl();
    }

} }
