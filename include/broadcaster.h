#include <cstdlib>

class Broadcaster {
public:
    class Delegate {
    public:
        // Receive a message, called from a separate thread.
        virtual void onMessage(const unsigned char* data, size_t length) = 0;
        virtual void onBroadcast(const unsigned char* data, size_t length) = 0;
        virtual ~Delegate() { }
    };

    virtual void setDelegate(Delegate* delegate) = 0;

    // Send message to all clients.
    // Only "ROOT" can do that.
    virtual void sendMessage(const void* data, size_t length) = 0;
    virtual void sendBroadcast(const void* data, size_t length) = 0;

    virtual double getTime() = 0;

    virtual ~Broadcaster() { }

    // Create with a configuration file.
    static Broadcaster* CreateTCP(const char* hostname = 0, const char* config_file = 0);
};
