#include <algorithm>
#include <iostream>

#include <boost/asio.hpp>
#include <boost/bind.hpp>

#include "router.h"
#include "serial.h"


Serial::Serial(boost::asio::io_service &io_service, Router* router, int baud, std::string serial_port):
    m_io_service(io_service),
    m_router(router),
    m_serial(io_service) {

    try {
        m_serial.open(serial_port);
    } catch (boost::system::system_error::exception e) {
        return;
    }

    try {
        m_serial.set_option(boost::asio::serial_port_base::baud_rate(baud));
        m_serial.set_option(boost::asio::serial_port_base::character_size(8));
        m_serial.set_option(boost::asio::serial_port_base::flow_control(boost::asio::serial_port_base::flow_control::none));
        m_serial.set_option(boost::asio::serial_port_base::parity(boost::asio::serial_port_base::parity::none));
        m_serial.set_option(boost::asio::serial_port_base::stop_bits(boost::asio::serial_port_base::stop_bits::one));
    } catch (boost::system::system_error::exception e) {
        std::cerr << "Router::Router(): failed to set serial parameters on: " << serial_port << std::endl;
        exit(1);
    }

    start_serial_read();
}


void Serial::setup() {
    std::cerr << "Serial::setup()" << std::endl;

}


void Serial::write(uint8_t* buffer, size_t size) {
    if (!m_serial.is_open()) {
        return;
    }

    boost::asio::async_write(m_serial,
                             boost::asio::buffer(buffer, size),
                             boost::bind(&Serial::handle_serial_write,
                                         this,
                                         boost::asio::placeholders::error,
                                         boost::asio::placeholders::bytes_transferred));
}



void Serial::start_serial_read() {
    std::cerr << "Serial::start_serial_read()" << std::endl;

    m_serial.async_read_some(boost::asio::buffer(data, max_length),
                             boost::bind(&Serial::handle_serial_read,
                                         this,
                                         boost::asio::placeholders::error,
                                         boost::asio::placeholders::bytes_transferred));
}


void Serial::handle_serial_write(const boost::system::error_code& error,
                                 size_t bytes_transferred) {
    std::cerr << "Serial::handle_serial_write()" << std::endl;
}


void Serial::handle_serial_read(const boost::system::error_code& error,
                                size_t bytes_transferred) {
    std::cerr << "Serial::handle_serial_read()" << std::endl;

    if (!error) {
        m_router->handle_serial_read(data, bytes_transferred);
    }
}
