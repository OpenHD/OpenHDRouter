#ifndef ROUTER_H
#define ROUTER_H

#include <array>
#include <vector>

#include <boost/asio.hpp>

#include <openhd/mavlink.h>

#include "endpoint.h"
#include "tcpendpoint.h"
#include "udpendpoint.h"

#include "serial.h"

class Router {
public:
    Router(boost::asio::io_service &io_service, int tcp_port, int baud, std::string serial_port);
    void setup(std::vector<std::string> new_udp_endpoints);

    void start_accept();
    void handle_accept(TCPEndpoint::pointer new_connection, const boost::system::error_code& error);
    void close_endpoint(std::shared_ptr<Endpoint> endpoint);

    void handle_serial_read(char* buffer,
                            size_t size);

    void process_mavlink_message(bool source_is_net, Endpoint::pointer source_endpoint, mavlink_message_t msg);

    void add_known_sys_id(uint8_t sys_id) {
        bool found = false;
        for(auto const& known_sys_id: m_known_sys_ids) {
            if (known_sys_id == sys_id) {
                found = true;
            }
        }
        if (!found) {
            std::cout << "Adding known sys ID for serial link: " << static_cast<int16_t>(sys_id) << std::endl;
            m_known_sys_ids.push_back(sys_id);
        }
    }

    bool seen_sys_id(uint8_t sys_id) {
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

    std::vector<uint8_t> m_known_sys_ids;

    boost::asio::io_service &m_io_service;
    std::vector<std::shared_ptr<Endpoint> > m_endpoints;
    boost::asio::ip::tcp::acceptor m_tcp_acceptor;

    Serial m_serial;
};

#endif
