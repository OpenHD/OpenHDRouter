#ifndef ROUTER_H
#define ROUTER_H

#include <array>
#include <vector>

#include <boost/asio.hpp>

#include <openhd/mavlink.h>

#include "endpoint.h"

class Router {
public:
    Router(boost::asio::io_context &io_context, int tcp_port, std::string serial_port);
    void setup();

    void start_accept();    
    void handle_accept(Endpoint::pointer new_connection, const boost::system::error_code& error);
    void close_endpoint(std::shared_ptr<Endpoint> endpoint);

    void handle_serial_read(const boost::system::error_code& error, 
                            size_t bytes_transferred);

    void process_mavlink_message(bool source_is_tcp, Endpoint::pointer source_endpoint, mavlink_message_t msg);

    void handle_serial_write(const boost::system::error_code& error, 
                             size_t bytes_transferred);

    void add_known_sys_id(int8_t sys_id) {
        bool found = false;
        for(auto const& known_sys_id: m_known_sys_ids) {
            if (known_sys_id == sys_id) {
                found = true;
            }
        }
        if (!found) {
            m_known_sys_ids.push_back(sys_id);
        }
    }

    bool seen_sys_id(int8_t sys_id) {
        bool found = false;
        for(auto const& known_sys_id: m_known_sys_ids) {
            if (known_sys_id == sys_id) {
                found = true;
            }
        }
        return found;
    }

protected:
    mavlink_status_t m_mavlink_status;

    enum { max_length = 1024 };
    char data[max_length];

    std::vector<int8_t> m_known_sys_ids;

    boost::asio::io_context &m_io_context;
    std::vector<std::shared_ptr<Endpoint> > m_endpoints;
    boost::asio::ip::tcp::acceptor m_tcp_acceptor;
    boost::asio::serial_port m_serial;
};

#endif
