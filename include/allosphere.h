// Header for native binding functions.
#include "math/quaternion.hpp"

namespace iv {
    namespace al {
        class Application;

        struct Lens {
            double eye_separation;
            double focal_distance;
        };

        struct Pose {
            Vector3d position;
            Quaterniond rotation;
        };

        struct DrawInfo {
            double eye;  // which eye.
        };

        enum ProjectionMode {
            PERSPECTIVE = 0,
            FISHEYE = 1
        };

        enum StereoMode {
            MONO = 0,
            ANAGLYPH = 1,
            ANAGLYPH_BLEND = 2,
            DUAL = 3
        };

        class Application {
        public:
            class Delegate {
            public:
                virtual void onFrame() { };
                virtual void onDraw(const DrawInfo& info) { };
                virtual void onCreate() { };

                virtual ~Delegate() { };
            };

            virtual void setLens(const Lens& lens) = 0;
            virtual void setPose(const Pose& pose) = 0;
            virtual void setProjectionMode(ProjectionMode mode) = 0;
            virtual void setStereoMode(StereoMode mode) = 0;
            virtual void enableWindowNavigation() = 0;
            virtual void enableOSCNavigation() = 0;

            virtual void setDelegate(Delegate*) = 0;
            virtual void initialize() = 0;
            virtual void tick() = 0;

            virtual int shaderCreate(const char* vertex, const char* fragment) = 0;
            virtual int shaderCreate(const char* vertex, const char* fragment, const char* geometry_code, int geometry_in_primitive, int geometry_out_primitive, int geometry_out_vertices) = 0;
            virtual void shaderDelete(int id) = 0;
            virtual int shaderDefault() = 0;
            virtual void shaderBegin(int id) = 0;
            virtual void shaderEnd(int id) = 0;

            virtual int textureCreate() = 0;
            virtual void textureDelete(int id) = 0;
            virtual void textureBind(int id, int target) = 0;
            virtual void textureSubmit(int width, int height, void* rgba) = 0;
            virtual void textureUnbind(int id, int target) = 0;

            // Other functions to call.
            virtual void shaderUniformf(const char* name, float) = 0;
            virtual void shaderUniform2f(const char* name, float, float) = 0;
            virtual void shaderUniform3f(const char* name, float, float, float) = 0;
            virtual void shaderUniform4f(const char* name, float, float, float, float) = 0;
            virtual void shaderUniformi(const char* name, int value) = 0;

            virtual ~Application() { };

            static Application* Create();
        };
    }
}
