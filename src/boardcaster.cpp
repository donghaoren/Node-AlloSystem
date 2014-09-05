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
#include <deque>

using namespace std;
using boost::asio::ip::tcp;

namespace {

    struct Host {
        string ip;
        int port;
        bool isRoot;
        string upstream;

        bool isConnect;
        string connectIP;
        int connectPort;


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
                    host.isRoot = false;
                    host.isConnect = false;
                    string name = args[1];
                    if(args[2] == "connect") {
                        host.isConnect = true;
                        host.connectIP = args[3];
                        host.connectPort = atoi(args[4].c_str());
                    } else {
                        host.ip = args[2];
                        host.port = atoi(args[3].c_str());
                        if(args[4] == "root") host.isRoot = true;
                        if(args[4] == "upstream") host.upstream = args[5];
                    }
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

    typedef shared_ptr<vector<unsigned char> > data_ptr;

    class Boardcaster_Impl_TCP : public Boardcaster {
    public:

        class Connection : public boost::enable_shared_from_this<Connection> {
        public:

            Connection(boost::asio::io_service& io_service, Boardcaster_Impl_TCP* boardcaster_)
            : socket(io_service) {
                boardcaster = boardcaster_;
                is_writing = false;
            }

            typedef boost::shared_ptr<Connection> pointer;

            static pointer create(boost::asio::io_service& io_service, Boardcaster_Impl_TCP* boardcaster) {
                return pointer(new Connection(io_service, boardcaster));
            }

            void start() {
                boardcaster->addConnection(shared_from_this());
            }

            deque<data_ptr> write_queue;

            struct WriteBuffer {
                data_ptr total_data;
                pointer self;
                void operator()(const boost::system::error_code& error, size_t bytes) {
                    self->handleWrite(error);
                }
            };

            bool is_writing;
            void check_write() {
                if(write_queue.empty() || is_writing) return;
                data_ptr data = write_queue.front();
                write_queue.pop_front();

                WriteBuffer buf;
                buf.total_data = data;
                buf.self = shared_from_this();
                boost::asio::async_write(socket, boost::asio::buffer(*data), buf);
                is_writing = true;
            }

            void sendMessage(const data_ptr& data) {
                write_queue.push_back(data);
                check_write();
            }

            void handleWrite(const boost::system::error_code& error) {
                if(error) {
                    boardcaster->removeConnection(shared_from_this());
                    return;
                }
                is_writing = false;
                check_write();
            }

            ~Connection() { }

            tcp::socket socket;
            Boardcaster_Impl_TCP* boardcaster;
        };

        Boardcaster_Impl_TCP(const char* hostname_, const char* config_file_) {
            hostname = hostname_;
            config_file = config_file_;

            thread = boost::thread(boost::bind(&Boardcaster_Impl_TCP::ioThread, this));
        }

        void connectUpstream() {
            if(config.hosts[config.hostname].isRoot) return;
            string ip;
            int port;
            if(config.hosts[config.hostname].isConnect) {
                ip = config.hosts[config.hostname].connectIP;
                port = config.hosts[config.hostname].connectPort;
            } else {
                Host upstream_info = config.hosts[config.hosts[config.hostname].upstream];
                ip = upstream_info.ip;
                port = upstream_info.port;
            }
            tcp::resolver resolver(*io_service);
            tcp::resolver::query query(ip, boost::to_string(port));
            tcp::resolver::iterator iterator = resolver.resolve(query);
            boost::asio::async_connect(*upstream, iterator,
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
            boost::asio::async_read(*upstream,
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
            message_data.resize(sizeof(MessageHeader) + message_header.length);
            memcpy(&message_data[0], &message_header, sizeof(MessageHeader));
            boost::asio::async_read(*upstream,
                boost::asio::buffer(&message_data[sizeof(MessageHeader)], message_header.length),
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
            relayMessage(data_ptr(new vector<unsigned char>(message_data)));
            if(delegate) {
                delegate->onMessage(&message_data[sizeof(message_header)], message_header.length);
            }
            readUpstreamMessage();
        }

        virtual void setDelegate(Delegate* delegate_) {
            delegate = delegate_;
        }

        // Send message to all clients.
        // Only "ROOT" can do that.
        virtual void sendMessage(const void* data, size_t length) {
            // for(set<Connection::pointer>::iterator it = connections.begin(); it != connections.end(); it++) {
            //     MessageHeader header;
            //     header.length = length;
            //     (*it)->sendMessage(header, data, length);
            // }
        }

        virtual void relayMessage(const data_ptr& data) {
            for(set<Connection::pointer>::iterator it = connections.begin(); it != connections.end(); it++) {
                (*it)->sendMessage(data);
            }
        }

        void addConnection(Connection::pointer pointer) {
            connections.insert(pointer);
        }

        void removeConnection(Connection::pointer pointer) {
            connections.erase(pointer);
        }

        void startAccept() {
            Connection::pointer new_connection = Connection::create(*io_service, this);
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
            io_service->stop();
            thread.join();
            acceptor.reset();
            upstream.reset();
            connections.clear();
            delegate = NULL;
        }

        virtual void ioThread() {
            io_service.reset(new boost::asio::io_service());
            upstream.reset(new tcp::socket(*io_service));
            if(config_file.empty()) {
                config.test(hostname.c_str());
            } else {
                config.read(config_file.c_str());
                config.hostname = hostname;
            }
            if(!config.hosts[config.hostname].isConnect) {
                acceptor.reset(new tcp::acceptor(*io_service, tcp::endpoint(tcp::v4(), config.hosts[config.hostname].port)));
                startAccept();
            }
            connectUpstream();

            io_service->run();
        }

        Config config;

        boost::shared_ptr<boost::asio::io_service> io_service;
        boost::thread thread;

        boost::shared_ptr<tcp::acceptor> acceptor;
        set<Connection::pointer> connections;
        boost::shared_ptr<tcp::socket> upstream;
        MessageHeader message_header;
        vector<unsigned char> message_data;

        Delegate* delegate;

        string hostname;
        string config_file;
    };
}

Boardcaster* Boardcaster::CreateTCP(const char* hostname, const char* config_file) {
    return new Boardcaster_Impl_TCP(hostname, config_file);
}
