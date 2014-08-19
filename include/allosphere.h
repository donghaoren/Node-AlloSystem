// Header for native binding functions.

namespace iv {
    namespace al {
        class Application {
        public:
            class Delegate {
            public:
                virtual void onFrame() = 0;
                virtual void onDraw() = 0;

                virtual ~Delegate() { };
            };

            virtual void setDelegate(Delegate*) = 0;
            virtual void initialize() = 0;
            virtual void tick() = 0;

            // Other functions to call.
            virtual void shaderUniformf(const char* name, float value) = 0;
            virtual void shaderUniformi(const char* name, int value) = 0;

            virtual ~Application() { };

            static Application* Create();
        };
    }
}
