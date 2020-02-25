#include <iostream>

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>

#include <openhd/mavlink.h>

#include "router.h"
#include "endpoint.h"


boost::asio::ip::tcp::socket& Endpoint::get_socket() {
    return m_socket;
}


void Endpoint::start() {
    std::cerr << "Endpoint::start()" << std::endl;

    m_socket.async_read_some(boost::asio::buffer(data, max_length),
                             boost::bind(&Endpoint::handle_read,
                                         this,
                                         shared_from_this(),
                                         boost::asio::placeholders::error,
                                         boost::asio::placeholders::bytes_transferred));
}


void Endpoint::handle_write(const boost::system::error_code& error, 
                            size_t bytes_transferred) {
    if (error) {
        m_router->close_endpoint(shared_from_this());
    }
}


void Endpoint::handle_read(std::shared_ptr<Endpoint>& s,
                           const boost::system::error_code& error,
                           size_t bytes_transferred) {
    std::cerr << "Endpoint::handle_read()" << std::endl;

    if (!error) {
        mavlink_message_t msg;
        for (int i = 0; i < bytes_transferred; i++) {
            uint8_t res = mavlink_parse_char(MAVLINK_COMM_0, (uint8_t)data[i], &msg, &m_mavlink_status);
            if (res) {
                add_known_sys_id(msg.sysid);
                m_router->process_mavlink_message(false, shared_from_this(), msg);
            }
        }
        m_socket.async_read_some(boost::asio::buffer(data, max_length),
                                 boost::bind(&Endpoint::handle_read, 
                                             this,
                                             shared_from_this(),
                                             boost::asio::placeholders::error,
                                             boost::asio::placeholders::bytes_transferred));
    } else {
        m_router->close_endpoint(shared_from_this());
    }
}


void Endpoint::send_message(uint8_t *buf, int size) {
    std::cerr << "Endpoint::send_message()" << std::endl;

    boost::asio::async_write(this->get_socket(), boost::asio::buffer(buf, size),
                             boost::bind(&Endpoint::handle_write, shared_from_this(),
                                         boost::asio::placeholders::error,
                                         boost::asio::placeholders::bytes_transferred));
}