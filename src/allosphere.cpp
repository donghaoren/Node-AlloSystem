#include "allosphere.h"

#include <alloutil/al_OmniApp.hpp>
#include <alloutil/al_OmniStereo.hpp>


#include <GLUT/glut.h>

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
            glutCheckLoop();
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
