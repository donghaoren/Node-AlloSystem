#include <node.h>
#include <node_buffer.h>
#include <v8.h>

#include "node_sharedmemory.h"

#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>

#include <stdexcept>

using namespace v8;

void NODE_SharedMemory::Init(Handle<Object> exports) {
    Local<FunctionTemplate> tpl = FunctionTemplate::New(New);
    tpl->SetClassName(String::NewSymbol("SharedMemory"));
    tpl->InstanceTemplate()->SetInternalFieldCount(1);

    // Prototype
    tpl->PrototypeTemplate()->Set(
        String::NewSymbol("size"), FunctionTemplate::New(NODE_size)->GetFunction());
    tpl->PrototypeTemplate()->Set(
        String::NewSymbol("shmid"), FunctionTemplate::New(NODE_shmid)->GetFunction());
    tpl->PrototypeTemplate()->Set(
        String::NewSymbol("semid"), FunctionTemplate::New(NODE_semid)->GetFunction());
    tpl->PrototypeTemplate()->Set(
        String::NewSymbol("buffer"), FunctionTemplate::New(NODE_buffer)->GetFunction());
    tpl->PrototypeTemplate()->Set(
        String::NewSymbol("delete"), FunctionTemplate::New(NODE_delete)->GetFunction());
    tpl->PrototypeTemplate()->Set(
        String::NewSymbol("close"), FunctionTemplate::New(NODE_close)->GetFunction());
    tpl->PrototypeTemplate()->Set(
        String::NewSymbol("writeLock"), FunctionTemplate::New(NODE_writeLock)->GetFunction());
    tpl->PrototypeTemplate()->Set(
        String::NewSymbol("writeUnlock"), FunctionTemplate::New(NODE_writeUnlock)->GetFunction());
    tpl->PrototypeTemplate()->Set(
        String::NewSymbol("readLock"), FunctionTemplate::New(NODE_readLock)->GetFunction());
    tpl->PrototypeTemplate()->Set(
        String::NewSymbol("readUnlock"), FunctionTemplate::New(NODE_readUnlock)->GetFunction());

    constructor = Persistent<Function>::New(tpl->GetFunction());

    // Export constructor.
    exports->Set(String::NewSymbol("SharedMemory"), constructor);
}

int shmrm(key_t key) {
    if(key) {
        int id = shmget(key, 0, 0);
        if(id == -1)
            return -1;
        return shmctl(id, IPC_RMID, NULL);
    }
    return -1;
}

int semrm(key_t key) {
    if(key) {
        int id = semget(key, 0, 0);
        if(id == -1)
            return -1;
        return semctl(id, 0, IPC_RMID, NULL);
    }
    return -1;
}

// open(key: int, size: int) -> [ shm_id, sem_id ]
NODE_SharedMemory::NODE_SharedMemory(int key, int size_, bool is_create_) {
    is_create = is_create_;
    shm_data = NULL;
    size = size_;
    if(is_create) {
        // Try to remove first.
        shmrm(key);
        semrm(key);

        shm_id = shmget(key, size, IPC_CREAT | IPC_EXCL | 0666);
        if(shm_id < 0) { perror("shmget"); return; }
        sem_id = semget(key, 2, IPC_CREAT | IPC_EXCL | 0666);
        if(sem_id < 0) { perror("semget"); return; }
        short sarray[2] = { 0, 0 };
        semctl(sem_id, 0, SETALL, sarray);

        shm_data = (unsigned char*)shmat(shm_id, NULL, 0);
    } else {
        shm_id = shmget(key, size, 0666);
        if(shm_id < 0) { perror("shmget"); return; }
        sem_id = semget(key, 2, 0666);
        if(sem_id < 0) { perror("semget"); return; }

        shm_data = (unsigned char*)shmat(shm_id, NULL, 0);
    }
}

NODE_SharedMemory::NODE_SharedMemory(int shmid, int semid, int size_, bool is_create_) {
    is_create = is_create_;
    shm_data = NULL;
    size = size_;
    if(is_create) {
        shm_id = shmget(IPC_PRIVATE, size, IPC_CREAT | 0666);
        if(shm_id < 0) { perror("shmget2"); return; }
        sem_id = semget(IPC_PRIVATE, 2, IPC_CREAT | 0666);
        if(sem_id < 0) { perror("semget2"); return; }
        short sarray[2] = { 0, 0 };
        semctl(sem_id, 0, SETALL, sarray);

        shm_data = (unsigned char*)shmat(shm_id, NULL, 0);
    } else {
        shm_id = shmid;
        sem_id = semid;
        shm_data = (unsigned char*)shmat(shm_id, NULL, 0);
    }
}

NODE_SharedMemory::~NODE_SharedMemory() {
    if(is_create) {
        if(sem_id >= 0) semctl(sem_id, 0, IPC_RMID, 0);
        if(shm_id >= 0) shmctl(shm_id, IPC_RMID, 0);
    }
}

v8::Handle<v8::Value> NODE_SharedMemory::New(const v8::Arguments& args) {
    if(args.IsConstructCall()) {
        if(args.Length() == 3) {
            int key = args[0]->IntegerValue();
            int size = args[1]->IntegerValue();
            bool is_create = args[2]->BooleanValue();
            NODE_SharedMemory* obj = new NODE_SharedMemory(key, size, is_create);
            if(obj->shm_id < 0 || obj->sem_id < 0) {
                return ThrowException(Exception::Error(String::New("SharedMemory: shmget()/semget() failed.")));
            }
            obj->Wrap(args.This());
            return args.This();
        } else if(args.Length() == 4) {
            int shmid = args[0]->IntegerValue();
            int semid = args[1]->IntegerValue();
            int size = args[2]->IntegerValue();
            bool is_create = args[3]->BooleanValue();
            NODE_SharedMemory* obj = new NODE_SharedMemory(shmid, semid, size, is_create);
            if(obj->shm_id < 0 || obj->sem_id < 0) {
                return ThrowException(Exception::Error(String::New("SharedMemory: shmget()/semget() failed.")));
            }
            obj->Wrap(args.This());
            return args.This();
        } else {
            return ThrowException(Exception::Error(String::New("SharedMemory: invalid arguments.")));
        }
    } else {
        // Invoked as plain function `MyObject(...)`, turn into construct call.
        const int argc = 3;
        Local<Value> argv[argc] = { args[0], args[1], args[2] };
        return constructor->NewInstance(argc, argv);
    }
}

v8::Handle<v8::Value> NODE_SharedMemory::NODE_size(const v8::Arguments& args) {
    NODE_SharedMemory* obj = node::ObjectWrap::Unwrap<NODE_SharedMemory>(args.This());
    return Integer::New(obj->size);
}

v8::Handle<v8::Value> NODE_SharedMemory::NODE_shmid(const v8::Arguments& args) {
    NODE_SharedMemory* obj = node::ObjectWrap::Unwrap<NODE_SharedMemory>(args.This());
    return Integer::New(obj->shm_id);
}

v8::Handle<v8::Value> NODE_SharedMemory::NODE_semid(const v8::Arguments& args) {
    NODE_SharedMemory* obj = node::ObjectWrap::Unwrap<NODE_SharedMemory>(args.This());
    return Integer::New(obj->sem_id);
}

void do_nothing_free_callback(char* data, void* hint) { }

v8::Handle<v8::Value> NODE_SharedMemory::NODE_buffer(const v8::Arguments& args) {
    NODE_SharedMemory* obj = node::ObjectWrap::Unwrap<NODE_SharedMemory>(args.This());
    if(obj->shm_data) {
        int start = args[0]->IsUndefined() ? 0 : args[0]->IntegerValue();
        int length = args[1]->IsUndefined() ? obj->size : args[1]->IntegerValue();
        return node::Buffer::New((char*)obj->shm_data + start, length, do_nothing_free_callback, NULL)->handle_;
    } else {
        return ThrowException(Exception::Error(String::New("buffer: shared memory deleted or not opened.")));
    }
}

v8::Handle<v8::Value> NODE_SharedMemory::NODE_delete(const v8::Arguments& args) {
    NODE_SharedMemory* obj = node::ObjectWrap::Unwrap<NODE_SharedMemory>(args.This());
    if(obj->sem_id >= 0) semctl(obj->sem_id, 0, IPC_RMID, 0);
    if(obj->shm_id >= 0) shmctl(obj->shm_id, IPC_RMID, 0);
    obj->shm_id = -1;
    obj->sem_id = -1;
    obj->shm_data = NULL;
    return args.This();
}

v8::Handle<v8::Value> NODE_SharedMemory::NODE_close(const v8::Arguments& args) {
    NODE_SharedMemory* obj = node::ObjectWrap::Unwrap<NODE_SharedMemory>(args.This());
    shmdt(obj->shm_data);
    return args.This();
}

v8::Handle<v8::Value> NODE_SharedMemory::NODE_writeLock(const v8::Arguments& args) {
    NODE_SharedMemory* obj = node::ObjectWrap::Unwrap<NODE_SharedMemory>(args.This());
    sembuf operations[2];
    operations[0].sem_num = 1;  // wait for reads to be zero.
    operations[0].sem_op = 0;
    operations[0].sem_flg = 0;
    operations[1].sem_num = 0;  // increment writes.
    operations[1].sem_op = 1;
    operations[1].sem_flg = 0;
    if(semop(obj->sem_id, operations, 2) == 0) return args.This();
    return ThrowException(Exception::Error(String::New("writeLock: semop() failed.")));
}

v8::Handle<v8::Value> NODE_SharedMemory::NODE_writeUnlock(const v8::Arguments& args) {
    NODE_SharedMemory* obj = node::ObjectWrap::Unwrap<NODE_SharedMemory>(args.This());
    sembuf operations[1];
    operations[0].sem_num = 0;  // decrement writes.
    operations[0].sem_op = -1;
    operations[0].sem_flg = 0;
    if(semop(obj->sem_id, operations, 1) == 0) return args.This();
    return ThrowException(Exception::Error(String::New("writeUnlock: semop() failed.")));
}

v8::Handle<v8::Value> NODE_SharedMemory::NODE_readLock(const v8::Arguments& args) {
    NODE_SharedMemory* obj = node::ObjectWrap::Unwrap<NODE_SharedMemory>(args.This());
    sembuf operations[2];
    operations[0].sem_num = 0;  // wait for writes to be zero.
    operations[0].sem_op = 0;
    operations[0].sem_flg = 0;
    operations[1].sem_num = 1;  // increment reads.
    operations[1].sem_op = 1;
    operations[1].sem_flg = 0;
    if(semop(obj->sem_id, operations, 2) == 0) return args.This();
    return ThrowException(Exception::Error(String::New("readUnlock: semop() failed.")));
}

v8::Handle<v8::Value> NODE_SharedMemory::NODE_readUnlock(const v8::Arguments& args) {
    NODE_SharedMemory* obj = node::ObjectWrap::Unwrap<NODE_SharedMemory>(args.This());
    sembuf operations[1];
    operations[0].sem_num = 1;  // decrement reads.
    operations[0].sem_op = -1;
    operations[0].sem_flg = 0;
    if(semop(obj->sem_id, operations, 1) == 0) return args.This();
    return ThrowException(Exception::Error(String::New("readUnlock: semop() failed.")));
}

v8::Persistent<v8::Function> NODE_SharedMemory::constructor;

void NODE_init(Handle<Object> exports) {
    NODE_SharedMemory::Init(exports);
}
NODE_MODULE(node_sharedmemory, NODE_init)
