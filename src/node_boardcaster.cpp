#include <node.h>
#include <node_buffer.h>
#include <v8.h>
#include <uv.h>

#include <string>
#include <vector>
#include <deque>

#include <boardcaster.h>

using namespace v8;
using namespace std;

void do_nothing_free_callback(char* data, void* hint) { }
void post_message_s(uv_async_t *handle, int status);

class Delegate : public Boardcaster::Delegate {
public:

    Delegate() {
        loop = uv_default_loop();
        uv_async_init(loop, &async, post_message_s);
        uv_mutex_init(&mutex);
    }

    void post_message() {
        uv_mutex_lock(&mutex);
        if(!onMessageCallback.IsEmpty()) {
            while(!message_queue.empty()) {
                vector<unsigned char>& v = message_queue.front();
                const int argc = 1;
                Handle<Value> argv[argc] = {
                    node::Buffer::New((char*)&v[0], v.size(), do_nothing_free_callback, NULL)->handle_
                };
                onMessageCallback->Call(Context::GetCurrent()->Global(), argc, argv);
                message_queue.pop_front();
            }
        }
        message_queue.clear();
        uv_mutex_unlock(&mutex);
    }

    ~Delegate() {
        uv_unref((uv_handle_t*)&async);
        uv_mutex_destroy(&mutex);
        if(!onMessageCallback.IsEmpty()) {
            onMessageCallback.Dispose();
        }
    }

    virtual void onMessage(const unsigned char* data, size_t length) {
        uv_mutex_lock(&mutex);
        vector<unsigned char> buffer;
        message_queue.push_back(buffer);
        message_queue.back().assign((unsigned char*)data, (unsigned char*)data + length);
        uv_mutex_unlock(&mutex);
        uv_async_send(&async);
    }
    Persistent<Function> onMessageCallback;

    uv_async_t async;
    uv_loop_t *loop;
    uv_mutex_t mutex;
    deque<vector<unsigned char> > message_queue;
};

Boardcaster* boardcaster = 0;;
Delegate* delegate = 0;

void post_message_s(uv_async_t *handle, int status) {
    delegate->post_message();
}

Handle<Value> EXPORT_start(const Arguments& args) {
    if(boardcaster) return Undefined();
    String::Utf8Value hostname(args[0]);
    String::Utf8Value config_path(args[1]);
    delegate = new Delegate();
    boardcaster = Boardcaster::CreateTCP(std::string(*hostname, *hostname + hostname.length()).c_str(), std::string(*config_path, *config_path + config_path.length()).c_str());
    boardcaster->setDelegate(delegate);
    return Undefined();
}

Handle<Value> EXPORT_stop(const Arguments& args) {
    if(boardcaster) {
        delete boardcaster;
    }
    if(delegate) {
        delete delegate;
    }
    boardcaster = NULL;
    delegate = NULL;
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
    exports->Set(String::NewSymbol("stop"), FunctionTemplate::New(EXPORT_stop)->GetFunction());
    exports->Set(String::NewSymbol("onMessage"), FunctionTemplate::New(EXPORT_onMessage)->GetFunction());
}

NODE_MODULE(node_boardcaster, NODE_init)
