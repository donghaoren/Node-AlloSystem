#ifndef IV_STREAM_H
#define IV_STREAM_H

#include "common.h"

IVNS_BEGIN

class ByteStream {
public:
    enum SeekOrigin {
        BEGIN, END, CURRENT
    };
    virtual size_t read(void* buffer, size_t length);
    virtual size_t write(const void* buffer, size_t length);
    virtual void seek(SeekOrigin origin, std::ptrdiff_t pos);
    virtual size_t position();
    virtual void flush();
    virtual bool canRead();
    virtual bool canWrite();
    virtual bool canSeek();
    virtual ~ByteStream() { }

    // mode = "r" / "w" / "a"
    static ByteStream* OpenFile(const char* path, const char* mode);
    static ByteStream* OpenFile(const std::string& path, const std::string& mode) {
        return OpenFile(path.c_str(), mode.c_str());
    }
};

class io_error : public std::exception { };

std::string get_file_contents(const std::string& filename);

IVNS_END

#endif
