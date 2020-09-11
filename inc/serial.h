#ifndef SERIAL_H
#define SERIAL_H

#include <array>
#include <vector>

#include <boost/asio.hpp>

#include <openhd/mavlink.h>

#include "endpoint.h"

class Router;


class Serial {
public:
    Serial(boost::asio::io_service &io_service, Router* router, int baud, std::string serial_port);
    void setup();

    void start_serial_read();

    void handle_serial_read(const boost::system::error_code& error,
                            size_t bytes_transferred);

    void handle_serial_write(const boost::system::error_code& error,
                             size_t bytes_transferred);

    void write(uint8_t* buffer, size_t size);

protected:
    mavlink_status_t m_mavlink_status;
    
    Router *m_router = nullptr;

    enum { max_length = 1024 };
    char data[max_length];

    boost::asio::io_service &m_io_service;
    boost::asio::serial_port m_serial;
};

#endif
