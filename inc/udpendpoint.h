#ifndef UDPENDPOINT_H
#define UDPENDPOINT_H

#include <array>
#include <vector>

#include <boost/asio.hpp>
#include <boost/regex.hpp>

#include <openhd/mavlink.h>

#include "endpoint.h"

class Router;

class UDPEndpoint: public Endpoint {
public:
    static pointer create(Router *router, boost::asio::io_service& io_service) {
        return pointer(new UDPEndpoint(router, io_service));
    }

    void setup(std::string endpoint_s);

    void send_message(uint8_t *buf, int size);

    void start();

    void start_receive();

protected:
    void handle_read(const boost::system::error_code& err,
                     size_t bytes_transferred);

    boost::asio::ip::udp::endpoint m_endpoint;

    mavlink_status_t m_mavlink_status;

private:
    UDPEndpoint(Router *router, boost::asio::io_service& io_service): Endpoint(router, io_service) {}
};

#endif // UDPENDPOINT_H
