#include "broadcaster.h"

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
using boost::asio::ip::udp;

namespace {

    double get_precise_time() {
        static boost::posix_time::ptime epoch(boost::gregorian::date(1970, 1, 1));
        double t = (boost::posix_time::microsec_clock::local_time() - epoch).total_microseconds() / 1000000.0;
        return t;
    }

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

    struct RunningAverage {
        int count;
        double sum;
        deque<double> values;

        RunningAverage(int count_) {
            count = count_;
            sum = 0;
        }

        double get() const {
            if(values.empty()) return 0;
            return sum / values.size();
        }

        void feed(double val) {
            values.push_back(val);
            sum += val;
            if(values.size() > count) {
                sum -= values.front();
                values.pop_front();
            }
        }
    };

    struct PROTOCOL_UDPTimeSync {
        static const unsigned int MAGIC = 0xFC307D2E;

        unsigned int magic;
        double time_sent;
    };

    struct Config {
        string hostname;
        map<string, Host> hosts;
        string udp_broadcast;
        int udp_broadcast_port;
        string udp_listen;
        int udp_listen_port;

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
                } else if(args[0] == "udp-broadcast") {
                    udp_broadcast = args[1];
                    udp_broadcast_port = atoi(args[2].c_str());
                } else if(args[0] == "udp-listen") {
                    udp_listen = args[1];
                    udp_listen_port = atoi(args[2].c_str());
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

    class Broadcaster_Impl_TCP : public Broadcaster {
    public:

        class Connection : public boost::enable_shared_from_this<Connection> {
        public:

            Connection(boost::asio::io_service& io_service, Broadcaster_Impl_TCP* broadcaster_)
            : socket(io_service) {
                broadcaster = broadcaster_;
                is_writing = false;
            }

            typedef boost::shared_ptr<Connection> pointer;

            static pointer create(boost::asio::io_service& io_service, Broadcaster_Impl_TCP* broadcaster) {
                return pointer(new Connection(io_service, broadcaster));
            }

            void start() {
                broadcaster->addConnection(shared_from_this());
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
                    broadcaster->removeConnection(shared_from_this());
                    return;
                }
                is_writing = false;
                check_write();
            }

            ~Connection() { }

            tcp::socket socket;
            Broadcaster_Impl_TCP* broadcaster;
        };

        Broadcaster_Impl_TCP(const char* hostname_, const char* config_file_) : time_difference(10) {
            hostname = hostname_;
            config_file = config_file_;

            thread = boost::thread(boost::bind(&Broadcaster_Impl_TCP::ioThread, this));
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
                boost::bind(&Broadcaster_Impl_TCP::handleUpstreamConnect, this,
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
                boost::bind(&Broadcaster_Impl_TCP::handleReadHeader, this,
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
                boost::bind(&Broadcaster_Impl_TCP::handleReadData, this,
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
        virtual void sendBroadcast(const void* data, size_t length) {
            if(udp_broadcast_socket) {
                udp::endpoint senderEndpoint(boost::asio::ip::address::from_string(config.udp_broadcast), config.udp_broadcast_port);
                udp_broadcast_socket->send_to(boost::asio::buffer(data, length), senderEndpoint);
            }
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
                boost::bind(&Broadcaster_Impl_TCP::handleAccept, this, new_connection,
                  boost::asio::placeholders::error));
        }

        void handleAccept(Connection::pointer new_connection, const boost::system::error_code& error) {
            if(!error) {
                new_connection->start();
                startAccept();
            } else {
            }
        }

        virtual ~Broadcaster_Impl_TCP() {
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

            if(!config.udp_broadcast.empty()) {
                udp_broadcast_socket.reset(new udp::socket(*io_service));
                udp_broadcast_socket->open(udp::v4());
                udp_broadcast_socket->set_option(udp::socket::reuse_address(true));
                udp_broadcast_socket->set_option(boost::asio::socket_base::broadcast(true));
            }
            if(!config.udp_listen.empty()) {
                udp_listen_socket.reset(new udp::socket(*io_service));
                udp_listen_socket->open(udp::v4());
                udp_listen_socket->bind(udp::endpoint(boost::asio::ip::address::from_string(config.udp_listen), config.udp_listen_port));
                udp_listen_buffer.resize(65536);
                startUDPReceive();
            }

            io_service->run();
        }

        void startUDPReceive() {
            udp_listen_socket->async_receive(
                boost::asio::buffer(udp_listen_buffer),
                boost::bind(&Broadcaster_Impl_TCP::handleUDPReceive, this,
                    boost::asio::placeholders::error,
                    boost::asio::placeholders::bytes_transferred)
            );
        }

        void handleUDPReceive(const boost::system::error_code& error, std::size_t size) {
            if(delegate) {
                delegate->onBroadcast(&udp_listen_buffer[0], size);
                if(size == sizeof(PROTOCOL_UDPTimeSync)) {
                    PROTOCOL_UDPTimeSync* packet = (PROTOCOL_UDPTimeSync*)&udp_listen_buffer[0];
                    if(packet->magic == PROTOCOL_UDPTimeSync::MAGIC) {
                        time_difference.feed(packet->time_sent - get_precise_time());
                    }
                }
            }
            startUDPReceive();
        }

        void performTimeSync() {
            PROTOCOL_UDPTimeSync packet;
            packet.magic = PROTOCOL_UDPTimeSync::MAGIC;
            packet.time_sent = get_precise_time();
            sendBroadcast(&packet, sizeof(PROTOCOL_UDPTimeSync));
        }

        virtual double getTime() {
            return get_precise_time() + time_difference.get();
        }

        RunningAverage time_difference;

        Config config;

        boost::shared_ptr<boost::asio::io_service> io_service;
        boost::thread thread;

        boost::shared_ptr<tcp::acceptor> acceptor;
        set<Connection::pointer> connections;
        boost::shared_ptr<tcp::socket> upstream;
        boost::shared_ptr<udp::socket> udp_broadcast_socket;
        boost::shared_ptr<udp::socket> udp_listen_socket;
        vector<unsigned char> udp_listen_buffer;
        udp::endpoint udp_listen_remote_endpoint;

        //boost::asio::deadline_timer performTimeSync_timer;

        MessageHeader message_header;
        vector<unsigned char> message_data;

        Delegate* delegate;

        string hostname;
        string config_file;
    };
}

Broadcaster* Broadcaster::CreateTCP(const char* hostname, const char* config_file) {
    return new Broadcaster_Impl_TCP(hostname, config_file);
}
