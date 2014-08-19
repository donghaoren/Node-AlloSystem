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

            light.ambient(::al::Color(0.4, 0.4, 0.4, 1.0));
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

        ::al::Light light;

        Delegate* delegate;
    };

    Application* Application::Create() {
        return new ApplicationImpl();
    }

} }
