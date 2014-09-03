#include <node.h>
#include <node_buffer.h>
#include <v8.h>

#include <string>

#include <boardcaster.h>

using namespace v8;

void do_nothing_free_callback(char* data, void* hint) { }

class Delegate : public Boardcaster::Delegate {
public:
    virtual void onMessage(const unsigned char* data, size_t length) {
        if(!onMessageCallback.IsEmpty()) {
            const int argc = 1;
            Handle<Value> argv[argc] = {
                node::Buffer::New((char*)data, length, do_nothing_free_callback, NULL)->handle_
            };
            onMessageCallback->Call(Context::GetCurrent()->Global(), argc, argv);
        }
    }
    Persistent<Function> onMessageCallback;
};

Boardcaster* boardcaster = 0;;
Delegate* delegate = 0;

Handle<Value> EXPORT_start(const Arguments& args) {
    if(boardcaster) return Undefined();
    String::Utf8Value hostname(args[0]);
    String::Utf8Value config_path(args[1]);
    delegate = new Delegate();
    boardcaster = Boardcaster::CreateTCP(std::string(*hostname, *hostname + hostname.length()).c_str(), std::string(*config_path, *config_path + config_path.length()).c_str());
    boardcaster->setDelegate(delegate);
    return Undefined();
}

Handle<Value> EXPORT_onMessage(const Arguments& args) {
    if(!delegate)
      return ThrowException(Exception::RangeError(String::New("Must call start() first.")));
    if(args.Length() != 1)
        return ThrowException(Exception::RangeError(String::New("Requires one argument.")));
    if(!args[0]->IsFunction()) {
        return ThrowException(Exception::TypeError(String::New("Callback should be callable.")));
    }
    if(!delegate->onMessageCallback.IsEmpty()) {
        delegate->onMessageCallback.Dispose();
    }
    delegate->onMessageCallback = Persistent<Function>::New(args[0].As<Function>());
    return Undefined();
}


void NODE_init(Handle<Object> exports) {
    exports->Set(String::NewSymbol("start"), FunctionTemplate::New(EXPORT_start)->GetFunction());
    exports->Set(String::NewSymbol("onMessage"), FunctionTemplate::New(EXPORT_onMessage)->GetFunction());
}

NODE_MODULE(node_boardcaster, NODE_init)
