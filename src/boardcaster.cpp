#include "boardcaster.h"

#include <string>
#include <vector>
#include <map>
#include <boost/asio.hpp>
#include <boost/thread.hpp>

#include <boost/algorithm/string/case_conv.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/algorithm/string/split.hpp>

#include <fstream>

using namespace std;
using boost::asio::ip::tcp;

namespace {

    struct Host {
        string ip;
        int port;
        bool isRoot;
        string upstream;

        Host() { }
        Host(const string& ip_, int port_, bool isRoot_, const string& upstream_ = "")
        : ip(ip_), port(port_), isRoot(isRoot_), upstream(upstream_) { }
    };

    struct Config {
        string hostname;
        map<string, Host> hosts;

        void test(const char* hostname_) {
            hosts["gr01"] = Host("127.0.0.1", 60110, true);
            hosts["gr02"] = Host("127.0.0.1", 60111, false, "gr01");
            hosts["gr03"] = Host("127.0.0.1", 60112, false, "gr01");
            hostname = hostname_;
        }

        void read(const char* file) {
            std::ifstream stream(file);
            if(!stream) throw std::invalid_argument("Error reading configuration file: not found.");

            std::string line;
            while(std::getline(stream, line)) {
                boost::trim_if(line, boost::is_any_of("\t "));
                // Reject comment and blank lines.
                if(line.size() == 0 || line[0] == '#') {
                    continue;
                }
                vector<string> args;
                boost::split(args, line, boost::is_any_of("\t "), boost::token_compress_on);
                if(args.size() == 0) continue;
                boost::to_lower(args[0]);

                if(args[0] == "host") {
                    Host host;
                    string name = args[1];
                    host.ip = args[2];
                    host.port = atoi(args[3].c_str());
                    if(args[4] == "root") host.isRoot = true;
                    else host.isRoot = false;
                    if(args[4] == "upstream") host.upstream = args[5];
                    hosts[name] = host;
                } else {
                    throw std::invalid_argument("Error reading configuration file: invalid command '" + args[0] + "'.");
                }
            }
        }
    };

    struct MessageHeader {
        int length;
    };

    class Boardcaster_Impl_TCP : public Boardcaster {
    public:

        class Connection : public boost::enable_shared_from_this<Connection> {
        public:
            MessageHeader header;

            Connection(boost::asio::io_service& io_service, Boardcaster_Impl_TCP* boardcaster_)
            : socket(io_service) {
                buffer_size = 16;
                boardcaster = boardcaster_;
            }

            typedef boost::shared_ptr<Connection> pointer;

            static pointer create(boost::asio::io_service& io_service, Boardcaster_Impl_TCP* boardcaster) {
                return pointer(new Connection(io_service, boardcaster));
            }

            void start() {
                boardcaster->addConnection(shared_from_this());
            }

            void sendMessage(const void* data, size_t length) {
                boost::asio::async_write(socket,
                    boost::asio::buffer(data, length),
                    boost::bind(&Connection::handleWrite, shared_from_this(),
                        boost::asio::placeholders::error
                    )
                );
            }

            void handleWrite(const boost::system::error_code& error) {
                if(error) {
                    boardcaster->removeConnection(shared_from_this());
                    return;
                }
            }

            ~Connection() { printf("Closed!\n"); }

            int buffer_size;
            tcp::socket socket;
            Boardcaster_Impl_TCP* boardcaster;
        };

        Boardcaster_Impl_TCP(const char* hostname = 0, const char* config_file = 0) : upstream(io_service) {
            if(!config_file) {
                config.test(hostname);
            } else {
                config.read(config_file);
                config.hostname = hostname;
            }

            acceptor.reset(new tcp::acceptor(io_service, tcp::endpoint(tcp::v4(), config.hosts[config.hostname].port)));
            startAccept();
            connectUpstream();
            thread = boost::thread(boost::bind(&Boardcaster_Impl_TCP::ioThread, this));
        }

        void connectUpstream() {
            if(config.hosts[config.hostname].isRoot) return;
            tcp::resolver resolver(io_service);
            Host upstream_info = config.hosts[config.hosts[config.hostname].upstream];
            tcp::resolver::query query(upstream_info.ip, boost::to_string(upstream_info.port));
            tcp::resolver::iterator iterator = resolver.resolve(query);
            printf("%s: Connect to upstream %s:%d.\n", config.hostname.c_str(), upstream_info.ip.c_str(), upstream_info.port);
            boost::asio::async_connect(upstream, iterator,
                boost::bind(&Boardcaster_Impl_TCP::handleUpstreamConnect, this,
                boost::asio::placeholders::error)
            );
        }

        void handleUpstreamConnect(const boost::system::error_code& error) {
            if(error) {
                connectUpstream();
            } else {
                readUpstreamMessage();
            }
        }

        void readUpstreamMessage() {
            boost::asio::async_read(upstream,
                boost::asio::buffer(&message_header, sizeof(MessageHeader)),
                boost::bind(&Boardcaster_Impl_TCP::handleReadHeader, this,
                    boost::asio::placeholders::error
                )
            );
        }

        void handleReadHeader(const boost::system::error_code& error) {
            if(error) {
                connectUpstream();
                return;
            }
            message_data.resize(message_header.length);
            boost::asio::async_read(upstream,
                boost::asio::buffer(message_data),
                boost::bind(&Boardcaster_Impl_TCP::handleReadData, this,
                    boost::asio::placeholders::error
                )
            );
        }

        void handleReadData(const boost::system::error_code& error) {
            if(error) {
                connectUpstream();
                return;
            }
            sendMessage(&message_data[0], message_header.length);
            if(delegate) {
                delegate->onMessage(&message_data[0], message_header.length);
            }
            readUpstreamMessage();
        }

        virtual void setDelegate(Delegate* delegate_) {
            delegate = delegate_;
        }

        // Send message to all clients.
        // Only "ROOT" can do that.
        virtual void sendMessage(const void* data, size_t length) {
            for(set<Connection::pointer>::iterator it = connections.begin(); it != connections.end(); it++) {
                MessageHeader header;
                header.length = length;
                (*it)->sendMessage(&header, sizeof(MessageHeader));
                (*it)->sendMessage(data, length);
            }
        }

        void addConnection(Connection::pointer pointer) {
            connections.insert(pointer);
            printf("Got Connection!\n");
        }

        void removeConnection(Connection::pointer pointer) {
            connections.erase(pointer);
            printf("Lost Connection!\n");
        }

        void startAccept() {
            Connection::pointer new_connection = Connection::create(io_service, this);
            acceptor->async_accept(new_connection->socket,
                boost::bind(&Boardcaster_Impl_TCP::handleAccept, this, new_connection,
                  boost::asio::placeholders::error));
        }

        void handleAccept(Connection::pointer new_connection, const boost::system::error_code& error) {
            if(!error) {
                new_connection->start();
                startAccept();
            } else {
            }
        }

        virtual ~Boardcaster_Impl_TCP() {
            io_service.stop();
        }

        virtual void ioThread() {
            io_service.run();
        }

        Config config;

        boost::asio::io_service io_service;
        boost::thread thread;

        boost::shared_ptr<tcp::acceptor> acceptor;
        set<Connection::pointer> connections;
        tcp::socket upstream;
        MessageHeader message_header;
        vector<unsigned char> message_data;

        Delegate* delegate;
    };
}

Boardcaster* Boardcaster::CreateTCP(const char* hostname, const char* config_file) {
    return new Boardcaster_Impl_TCP(hostname, config_file);
}
