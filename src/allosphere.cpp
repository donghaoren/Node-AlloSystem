#include "allosphere.h"

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

namespace iv { namespace al {

    class ApplicationImpl : public ::al::OmniApp, public Application {
    public:
        ApplicationImpl() {
            delegate = NULL;

            light.ambient(::al::Color(0.4, 0.4, 0.4, 0.1));
            light.pos(5, 5, 5);
        }

        virtual void setDelegate(Delegate* delegate_) {
            delegate = delegate_;
        }

        static void do_nothing() { }

        virtual void initialize() {
            if (mOmni.activeStereo()) {
                Window::displayMode(Window::displayMode() | Window::STEREO_BUF);
            }

            create();

            if (mOmni.fullScreen()) {
                fullScreen(true);
                cursorHide(true);
            }

            if (!bSlave) {
                if(oscSend().opened()) sendHandshake();
                mAudioIO.start();
            }

            glutIdleFunc(do_nothing);
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

        virtual void shaderUniformf(const char* name, float value) {
            shader().uniform(name, value);
        }

        virtual void shaderUniformi(const char* name, int value) {
            shader().uniform(name, value);
        }

        virtual bool onFrame() {
            if(delegate) delegate->onFrame();
            return OmniApp::onFrame();
        }

        virtual void onDraw(::al::Graphics& g) {
            light();
            if(delegate) delegate->onDraw();
        }

        virtual std::string fragmentCode1() {
            return AL_STRINGIFY(
                uniform float lighting;
                uniform float texture;
                uniform sampler2D texture0;
                varying vec4 color;
                varying vec3 normal, lightDir, eyeVec;
                void main() {

                    vec4 colorMixed;
                    if( texture > 0.0){
                        vec4 textureColor = texture2D(texture0, gl_TexCoord[0].st);
                        colorMixed = mix(color, textureColor, texture);
                    }else{
                        colorMixed = color;
                    }

                    vec4 final_color = colorMixed * gl_LightSource[0].ambient;
                    vec3 N = normalize(normal);
                    vec3 L = lightDir;
                    float lambertTerm = max(dot(N, L), 0.0);
                    final_color += gl_LightSource[0].diffuse * colorMixed * lambertTerm;
                    vec3 E = eyeVec;
                    vec3 R = reflect(-L, N);
                    float spec = pow(max(dot(R, E), 0.0), 0.9 + 1e-20);
                    final_color += gl_LightSource[0].specular * spec;
                    gl_FragColor = mix(colorMixed, final_color, lighting);
                    gl_FragColor.a = 0.5;
                }
            );
        }

        ::al::Light light;

        Delegate* delegate;
    };

    Application* Application::Create() {
        return new ApplicationImpl();
    }

} }
