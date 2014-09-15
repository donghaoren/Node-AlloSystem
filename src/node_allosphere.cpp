#include <node.h>
#include <node_buffer.h>
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

    virtual void onDraw(const iv::al::DrawInfo& info) {
        if(!onDrawCallback.IsEmpty()) {
            HandleScope scope;
            const int argc = 1;
            Local<Object> info_obj = Object::New();
            Local<Value> argv[argc] = { Local<Value>::New(info_obj) };
            info_obj->Set(String::NewSymbol("eye"), Number::New(info.eye));
            onDrawCallback->Call(Context::GetCurrent()->Global(), argc, argv);
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

Handle<Value> EXPORT_shaderCreate(const Arguments& args) {
    String::Utf8Value vert(args[0]);
    std::string vert_(*vert, *vert + vert.length());
    String::Utf8Value frag(args[1]);
    std::string frag_(*frag, *frag + frag.length());
    int index = application->app->shaderCreate(vert_.c_str(), frag_.c_str());
    return Integer::New(index);
}

Handle<Value> EXPORT_shaderDelete(const Arguments& args) {
    application->app->shaderDelete(args[0]->IntegerValue());
    return Undefined();
}

Handle<Value> EXPORT_shaderDefault(const Arguments& args) {
    return Integer::New(application->app->shaderDefault());
}

Handle<Value> EXPORT_shaderBegin(const Arguments& args) {
    application->app->shaderBegin(args[0]->IntegerValue());
    return Undefined();
}

Handle<Value> EXPORT_shaderEnd(const Arguments& args) {
    application->app->shaderEnd(args[0]->IntegerValue());
    return Undefined();
}

Handle<Value> EXPORT_textureCreate(const Arguments& args) {
    int id = application->app->textureCreate();
    return Integer::New(id);
}

Handle<Value> EXPORT_textureDelete(const Arguments& args) {
    application->app->textureDelete(args[0]->IntegerValue());
    return Undefined();
}

Handle<Value> EXPORT_textureBind(const Arguments& args) {
    application->app->textureBind(args[0]->IntegerValue(), args[1]->IntegerValue());
    return Undefined();
}

Handle<Value> EXPORT_textureUnbind(const Arguments& args) {
    application->app->textureUnbind(args[0]->IntegerValue(), args[1]->IntegerValue());
    return Undefined();
}

Handle<Value> EXPORT_textureSubmit(const Arguments& args) {
    application->app->textureSubmit(args[0]->IntegerValue(), args[1]->IntegerValue(), node::Buffer::Data(args[2]));
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
  exports->Set(String::NewSymbol("shaderCreate"), FunctionTemplate::New(EXPORT_shaderCreate)->GetFunction());
  exports->Set(String::NewSymbol("shaderDelete"), FunctionTemplate::New(EXPORT_shaderDelete)->GetFunction());
  exports->Set(String::NewSymbol("shaderDefault"), FunctionTemplate::New(EXPORT_shaderDefault)->GetFunction());
  exports->Set(String::NewSymbol("shaderBegin"), FunctionTemplate::New(EXPORT_shaderBegin)->GetFunction());
  exports->Set(String::NewSymbol("shaderEnd"), FunctionTemplate::New(EXPORT_shaderEnd)->GetFunction());
  exports->Set(String::NewSymbol("textureCreate"), FunctionTemplate::New(EXPORT_textureCreate)->GetFunction());
  exports->Set(String::NewSymbol("textureDelete"), FunctionTemplate::New(EXPORT_textureDelete)->GetFunction());
  exports->Set(String::NewSymbol("textureBind"), FunctionTemplate::New(EXPORT_textureBind)->GetFunction());
  exports->Set(String::NewSymbol("textureSubmit"), FunctionTemplate::New(EXPORT_textureSubmit)->GetFunction());
  exports->Set(String::NewSymbol("textureUnbind"), FunctionTemplate::New(EXPORT_textureUnbind)->GetFunction());
  exports->Set(String::NewSymbol("shaderUniformi"), FunctionTemplate::New(EXPORT_shaderUniformi)->GetFunction());
  exports->Set(String::NewSymbol("shaderUniformf"), FunctionTemplate::New(EXPORT_shaderUniformf)->GetFunction());
  gl_factory = new GlFactory();
  exports->Set(String::NewSymbol("OpenGL"), gl_factory->createGl()->NewInstance());
}

NODE_MODULE(node_allosphere, NODE_init)
