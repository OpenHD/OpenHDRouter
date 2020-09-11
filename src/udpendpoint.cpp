#include <iostream>

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>

#include <openhd/mavlink.h>

#include "router.h"
#include "udpendpoint.h"


void UDPEndpoint::start() {
    std::cerr << "UDPEndpoint::start()" << std::endl;
}


void UDPEndpoint::setup(std::string endpoint_s) {
    std::cerr << "UDPEndpoint::setup(" << endpoint_s << ")" << std::endl;

    boost::smatch result;

    boost::regex r{ "([\\d]+)\\:([\\w\\d\\.]+)\\:([\\d]+)"};
    if (!boost::regex_match(endpoint_s, result, r)) {
        std::cerr << "Failed to match regex" << std::endl;
        return;
    }

    if (result.size() != 4) {
        std::cerr << "Failed size" << std::endl;

        return;
    }

    std::string local_port_s = result[1];
    uint16_t local_port = atoi(local_port_s.c_str());

    std::string address = result[2];
    std::string port_s = result[3];
    uint16_t port = atoi(port_s.c_str());


    m_endpoint = boost::asio::ip::udp::endpoint(boost::asio::ip::address::from_string(address), port);
    m_udp_socket.open(boost::asio::ip::udp::v4());
    m_udp_socket.bind(boost::asio::ip::udp::endpoint(boost::asio::ip::address::from_string("0.0.0.0"), local_port));

    start_receive();
}


void UDPEndpoint::start_receive() {
    std::cerr << "UDPEndpoint::start_receive()" << std::endl;

    m_udp_socket.async_receive_from(boost::asio::buffer(data, max_length),
                                    m_endpoint,
                                    boost::bind(&UDPEndpoint::handle_read, 
                                                this,
                                                boost::asio::placeholders::error, 
                                                boost::asio::placeholders::bytes_transferred));
}


void UDPEndpoint::handle_read(const boost::system::error_code& error,
                              size_t bytes_transferred) {
    std::cerr << "UDPEndpoint::handle_read()" << std::endl;

    if (!error) {
        mavlink_message_t msg;
        for (int i = 0; i < bytes_transferred; i++) {
            uint8_t res = mavlink_parse_char(MAVLINK_COMM_0, (uint8_t)data[i], &msg, &m_mavlink_status);
            if (res) {
                if (msg.sysid != 0) {
                    add_known_sys_id(msg.sysid);
                }
                m_router->process_mavlink_message(true, shared_from_this(), msg);
            }
        }
        start_receive();
    }
}


void UDPEndpoint::send_message(uint8_t *buffer, int size) {
    std::cerr << "UDPEndpoint::send_message()" << std::endl;

    auto sent = m_udp_socket.send_to(boost::asio::buffer(buffer, size), m_endpoint);
}