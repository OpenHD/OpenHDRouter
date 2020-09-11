#ifndef TCPENDPOINT_H
#define TCPENDPOINT_H

#include <array>
#include <vector>

#include <boost/asio.hpp>

#include <openhd/mavlink.h>

#include "endpoint.h"

class Router;

class TCPEndpoint: public Endpoint {
public:
    static pointer create(Router *router, boost::asio::io_service& io_service) {
        return pointer(new TCPEndpoint(router, io_service));
    }

    void send_message(uint8_t *buf, int size);

    void start();
    
protected:
    void handle_write(const boost::system::error_code& error,
                      size_t bytes_transferred);

    void handle_read(const boost::system::error_code& err,
                     size_t bytes_transferred);

    mavlink_status_t m_mavlink_status;

private:
  TCPEndpoint(Router *router, boost::asio::io_service& io_service): Endpoint(router, io_service) {}
};

#endif // TCPENDPOINT_H
