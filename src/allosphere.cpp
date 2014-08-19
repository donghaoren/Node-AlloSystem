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

        virtual bool onFrame() {
            if(delegate) delegate->onFrame();
            return OmniApp::onFrame();
        }

        virtual void onDraw(::al::Graphics& g) {
            if(delegate) delegate->onDraw();
        }

        Delegate* delegate;
    };

    Application* Application::Create() {
        return new ApplicationImpl();
    }

} }
