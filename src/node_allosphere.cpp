#include <node.h>
#include <v8.h>

#include <string>

#include <allosphere.h>
#include "glbind.h"

using namespace v8;

class MyApplicationClass : public iv::al::Application::Delegate {
public:
    MyApplicationClass() {
        app = iv::al::Application::Create();
        app->setDelegate(this);
    }

    virtual void onFrame() {
        if(!onFrameCallback.IsEmpty()) {
            onFrameCallback->Call(Context::GetCurrent()->Global(), 0, NULL);
        }
    }

    virtual void onDraw() {
        if(!onDrawCallback.IsEmpty()) {
            onDrawCallback->Call(Context::GetCurrent()->Global(), 0, NULL);
        }
    }

    ~MyApplicationClass() {
        if(!onFrameCallback.IsEmpty()) {
            onFrameCallback.Dispose();
        }
        if(!onDrawCallback.IsEmpty()) {
            onDrawCallback.Dispose();
        }
        delete app;
    }

    iv::al::Application* app;

    Persistent<Function> onFrameCallback;
    Persistent<Function> onDrawCallback;

    GlFactory factory;
};

MyApplicationClass* application = 0;
GlFactory* gl_factory;

Handle<Value> EXPORT_initialize(const Arguments& args) {
  if(application) return Undefined();
  application = new MyApplicationClass;
  application->app->initialize();
  return Undefined();
}

Handle<Value> EXPORT_tick(const Arguments& args) {
  if(!application)
      return ThrowException(Exception::RangeError(String::New("Must call initialize() first.")));
  application->app->tick();
  return Undefined();
}

Handle<Value> EXPORT_onFrame(const Arguments& args) {
    if(!application)
      return ThrowException(Exception::RangeError(String::New("Must call initialize() first.")));
    if(args.Length() != 1)
        return ThrowException(Exception::RangeError(String::New("Requires one argument.")));
    if(!args[0]->IsFunction()) {
        return ThrowException(Exception::TypeError(String::New("Callback should be callable.")));
    }
    if(!application->onFrameCallback.IsEmpty()) {
        application->onFrameCallback.Dispose();
    }
    application->onFrameCallback = Persistent<Function>::New(args[0].As<Function>());
    return Undefined();
}

Handle<Value> EXPORT_onDraw(const Arguments& args) {
    if(!application)
      return ThrowException(Exception::RangeError(String::New("Must call initialize() first.")));
    if(args.Length() != 1)
        return ThrowException(Exception::RangeError(String::New("Requires one argument.")));
    if(!args[0]->IsFunction()) {
        return ThrowException(Exception::TypeError(String::New("Callback should be callable.")));
    }
    if(!application->onDrawCallback.IsEmpty()) {
        application->onDrawCallback.Dispose();
    }
    application->onDrawCallback = Persistent<Function>::New(args[0].As<Function>());
    return Undefined();
}

Handle<Value> EXPORT_shaderUniformi(const Arguments& args) {
    String::Utf8Value name(args[0]);
    std::string name_(*name, *name + name.length());
    application->app->shaderUniformi(name_.c_str(), args[1]->IntegerValue());
    return Undefined();
}
Handle<Value> EXPORT_shaderUniformf(const Arguments& args) {
  String::Utf8Value name(args[0]);
    std::string name_(*name, *name + name.length());
    application->app->shaderUniformf(name_.c_str(), args[1]->NumberValue());
    return Undefined();
}

void NODE_init(Handle<Object> exports) {
  exports->Set(String::NewSymbol("initialize"), FunctionTemplate::New(EXPORT_initialize)->GetFunction());
  exports->Set(String::NewSymbol("tick"), FunctionTemplate::New(EXPORT_tick)->GetFunction());
  exports->Set(String::NewSymbol("onFrame"), FunctionTemplate::New(EXPORT_onFrame)->GetFunction());
  exports->Set(String::NewSymbol("onDraw"), FunctionTemplate::New(EXPORT_onDraw)->GetFunction());
  exports->Set(String::NewSymbol("shaderUniformi"), FunctionTemplate::New(EXPORT_shaderUniformi)->GetFunction());
  exports->Set(String::NewSymbol("shaderUniformf"), FunctionTemplate::New(EXPORT_shaderUniformf)->GetFunction());
  gl_factory = new GlFactory();
  exports->Set(String::NewSymbol("OpenGL"), gl_factory->createGl()->NewInstance());
}

NODE_MODULE(node_allosphere, NODE_init)
