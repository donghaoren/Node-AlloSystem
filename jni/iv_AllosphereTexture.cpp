#include "iv_AllosphereTexture.h"

#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/time.h>
#include <string.h>

#include <stdexcept>

#ifdef __APPLE__
  #include <OpenGL/gl.h>
#else
  #include <GL/gl.h>
#endif

double get_precise_time() {
    timeval t;
    gettimeofday(&t, 0);
    double s = t.tv_sec;
    s += t.tv_usec / 1000000.0;
    return s;
}

class IVAllosphereTextureNative {
public:
    struct Metadata {
        double timestamp;
    };

    IVAllosphereTextureNative(int shm_id_, int sem_id_, int size_) {
        shm_id = shm_id_;
        sem_id = sem_id_;
        size = size_;
        shm_data = (unsigned char*)shmat(shm_id, NULL, 0);
        if((jlong)shm_data < 0) {
            shm_data = NULL;
            return;
        }
        metadata = (Metadata*)shm_data;
        pixels = shm_data + 1024;
    }

    bool writeLock() {
        if(!shm_data) return false;
        sembuf operations[2];
        operations[0].sem_num = 1;  // wait for reads to be zero.
        operations[0].sem_op = 0;
        operations[0].sem_flg = 0;
        operations[1].sem_num = 0;  // increment writes.
        operations[1].sem_op = 1;
        operations[1].sem_flg = 0;
        return (semop(sem_id, operations, 2) == 0);
    }

    bool writeUnlock() {
        if(!shm_data) return false;
        sembuf operations[1];
        operations[0].sem_num = 0;  // decrement writes.
        operations[0].sem_op = -1;
        operations[0].sem_flg = 0;
        return (semop(sem_id, operations, 1) == 0);
    }

    bool upload(int texture_id, int width, int height) {
        if(!writeLock()) return false;
        metadata->timestamp = get_precise_time();
        glBindTexture(GL_TEXTURE_2D, texture_id);
        glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
        fprintf(stderr, "upload: %d %d", texture_id, (int)pixels[0]);
        writeUnlock();
        return true;
    }

    bool upload(void* image, int width, int height) {
        if(!writeLock()) return false;
        metadata->timestamp = get_precise_time();
        memcpy(pixels, image, width * height * 4);
        writeUnlock();
        return true;
    }

    ~IVAllosphereTextureNative() {
    }

    int shm_id, sem_id;
    int size;
    unsigned char* shm_data;

    Metadata* metadata;
    unsigned char* pixels;
};

/*
 * Class:     IVAllosphereTexture
 * Method:    open_shared_memory
 * Signature: (III)J
 */
JNIEXPORT jlong JNICALL Java_iv_AllosphereTexture_open_1shared_1memory
  (JNIEnv *, jclass, jint shm_id, jint sem_id, jint size) {
    IVAllosphereTextureNative *result = new IVAllosphereTextureNative(shm_id, sem_id, size);
    if(!result->shm_data) {
        delete result;
        return 0;
    }
    return (jlong)result;
}

/*
 * Class:     IVAllosphereTexture
 * Method:    close_shared_memory
 * Signature: (J)V
 */
JNIEXPORT void JNICALL Java_iv_AllosphereTexture_close_1shared_1memory
  (JNIEnv *, jclass, jlong ptr) {
    IVAllosphereTextureNative* obj = (IVAllosphereTextureNative*)(ptr);
    delete obj;
}

/*
 * Class:     IVAllosphereTexture
 * Method:    upload_texture
 * Signature: (JIII)V
 */
JNIEXPORT void JNICALL Java_iv_AllosphereTexture_upload_1texture
  (JNIEnv *, jclass, jlong ptr, jint texture_id, jint width, jint height) {
    IVAllosphereTextureNative* obj = (IVAllosphereTextureNative*)(ptr);
    obj->upload(texture_id, width, height);
}

/*
 * Class:     iv_AllosphereTexture
 * Method:    upload_buffer
 * Signature: (JLjava/nio/ByteBuffer;II)V
 */
JNIEXPORT void JNICALL Java_iv_AllosphereTexture_upload_1buffer
  (JNIEnv *env, jclass, jlong ptr, jobject byte_buffer, jint width, jint height) {
    IVAllosphereTextureNative* obj = (IVAllosphereTextureNative*)(ptr);
    obj->upload(env->GetDirectBufferAddress(byte_buffer), width, height);
}
