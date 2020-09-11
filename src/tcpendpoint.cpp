#include <iostream>

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>

#include <openhd/mavlink.h>

#include "router.h"
#include "tcpendpoint.h"



void TCPEndpoint::start() {
    std::cerr << "TCPEndpoint::start()" << std::endl;

    m_tcp_socket.async_read_some(boost::asio::buffer(data, max_length),
                                 boost::bind(&TCPEndpoint::handle_read,
                                             this,
                                             boost::asio::placeholders::error,
                                             boost::asio::placeholders::bytes_transferred));
}


void TCPEndpoint::handle_write(const boost::system::error_code& error,
                               size_t bytes_transferred) {
    if (error) {
        m_router->close_endpoint(shared_from_this());
    }
}


void TCPEndpoint::handle_read(const boost::system::error_code& error,
                              size_t bytes_transferred) {
    std::cerr << "TCPEndpoint::handle_read()" << std::endl;

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
        m_tcp_socket.async_read_some(boost::asio::buffer(data, max_length),
                                 boost::bind(&TCPEndpoint::handle_read,
                                             this,
                                             boost::asio::placeholders::error,
                                             boost::asio::placeholders::bytes_transferred));
    } else {
        m_router->close_endpoint(shared_from_this());
    }
}


void TCPEndpoint::send_message(uint8_t *buf, int size) {
    std::cerr << "TCPEndpoint::send_message()" << std::endl;

    boost::asio::async_write(this->get_tcp_socket(), boost::asio::buffer(buf, size),
                             boost::bind(&TCPEndpoint::handle_write, 
                                         this,
                                         boost::asio::placeholders::error,
                                         boost::asio::placeholders::bytes_transferred));
}
