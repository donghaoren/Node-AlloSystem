#include <node.h>
#include <node_buffer.h>
#include <v8.h>
#include <uv.h>

#include <string>
#include <vector>
#include <deque>

#include <broadcaster.h>

using namespace v8;
using namespace std;

void do_nothing_free_callback(char* data, void* hint) { }
void post_message_s(uv_async_t *handle, int status);

class Delegate : public Broadcaster::Delegate {
public:

    Delegate() {
        loop = uv_default_loop();
        uv_async_init(loop, &async, post_message_s);
        uv_mutex_init(&mutex);
    }

    void post_message() {
        uv_mutex_lock(&mutex);
        if(!onBroadcastCallback.IsEmpty()) {
            while(!broadcast_queue.empty()) {
                vector<unsigned char>& v = broadcast_queue.front();
                const int argc = 1;
                Handle<Value> argv[argc] = {
                    node::Buffer::New((char*)&v[0], v.size(), do_nothing_free_callback, NULL)->handle_
                };
                onBroadcastCallback->Call(Context::GetCurrent()->Global(), argc, argv);
                broadcast_queue.pop_front();
            }
        }
        broadcast_queue.clear();

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
        if(!onBroadcastCallback.IsEmpty()) {
            onBroadcastCallback.Dispose();
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

    virtual void onBroadcast(const unsigned char* data, size_t length) {
        uv_mutex_lock(&mutex);
        vector<unsigned char> buffer;
        broadcast_queue.push_back(buffer);
        broadcast_queue.back().assign((unsigned char*)data, (unsigned char*)data + length);
        uv_mutex_unlock(&mutex);
        uv_async_send(&async);
    }

    Persistent<Function> onMessageCallback;
    Persistent<Function> onBroadcastCallback;

    uv_async_t async;
    uv_loop_t *loop;
    uv_mutex_t mutex;
    deque<vector<unsigned char> > message_queue;
    deque<vector<unsigned char> > broadcast_queue;
};

Broadcaster* broadcaster = 0;;
Delegate* delegate = 0;

void post_message_s(uv_async_t *handle, int status) {
    delegate->post_message();
}

Handle<Value> EXPORT_start(const Arguments& args) {
    if(broadcaster) return Undefined();
    String::Utf8Value hostname(args[0]);
    String::Utf8Value config_path(args[1]);
    delegate = new Delegate();
    broadcaster = Broadcaster::CreateTCP(std::string(*hostname, *hostname + hostname.length()).c_str(), std::string(*config_path, *config_path + config_path.length()).c_str());
    broadcaster->setDelegate(delegate);
    return Undefined();
}

Handle<Value> EXPORT_stop(const Arguments& args) {
    if(broadcaster) {
        delete broadcaster;
    }
    if(delegate) {
        delete delegate;
    }
    broadcaster = NULL;
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

Handle<Value> EXPORT_onBroadcast(const Arguments& args) {
    if(!delegate)
      return ThrowException(Exception::RangeError(String::New("Must call start() first.")));
    if(args.Length() != 1)
        return ThrowException(Exception::RangeError(String::New("Requires one argument.")));
    if(!args[0]->IsFunction()) {
        return ThrowException(Exception::TypeError(String::New("Callback should be callable.")));
    }
    if(!delegate->onBroadcastCallback.IsEmpty()) {
        delegate->onBroadcastCallback.Dispose();
    }
    delegate->onBroadcastCallback = Persistent<Function>::New(args[0].As<Function>());
    return Undefined();
}

Handle<Value> EXPORT_sendMessage(const Arguments& args) {
    broadcaster->sendMessage(node::Buffer::Data(args[0]), node::Buffer::Length(args[0]));
    return Undefined();
}

Handle<Value> EXPORT_sendBroadcast(const Arguments& args) {
    broadcaster->sendBroadcast(node::Buffer::Data(args[0]), node::Buffer::Length(args[0]));
    return Undefined();
}

Handle<Value> EXPORT_getTime(const Arguments& args) {
    double t = broadcaster->getTime();
    return Number::New(t);
}

void NODE_init(Handle<Object> exports) {
    exports->Set(String::NewSymbol("start"), FunctionTemplate::New(EXPORT_start)->GetFunction());
    exports->Set(String::NewSymbol("stop"), FunctionTemplate::New(EXPORT_stop)->GetFunction());
    exports->Set(String::NewSymbol("getTime"), FunctionTemplate::New(EXPORT_getTime)->GetFunction());
    exports->Set(String::NewSymbol("onMessage"), FunctionTemplate::New(EXPORT_onMessage)->GetFunction());
    exports->Set(String::NewSymbol("onBroadcast"), FunctionTemplate::New(EXPORT_onBroadcast)->GetFunction());
    exports->Set(String::NewSymbol("sendMessage"), FunctionTemplate::New(EXPORT_sendMessage)->GetFunction());
    exports->Set(String::NewSymbol("sendBroadcast"), FunctionTemplate::New(EXPORT_sendBroadcast)->GetFunction());
}

NODE_MODULE(node_broadcaster, NODE_init)
