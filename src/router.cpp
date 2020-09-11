#include <algorithm>
#include <iostream>
#include <vector>

#include <boost/asio.hpp>
#include <boost/bind.hpp>

#include <openhd/mavlink.h>
#include <mavlink_types.h>


#include "router.h"
#include "endpoint.h"
#include "tcpendpoint.h"
#include "udpendpoint.h"


Router::Router(boost::asio::io_service &io_service, int tcp_port, int baud, std::string serial_port):
    m_io_service(io_service),
    m_tcp_acceptor(io_service, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), tcp_port)),
    m_serial(io_service, this, baud, serial_port) {

    start_accept();
}


void Router::setup(std::vector<std::string> new_udp_endpoints) {
    std::cerr << "Router::setup()" << std::endl;

    for (auto &_udp_endpoint : new_udp_endpoints) {
        std::cerr << "Creating UDP endpoint: " << _udp_endpoint << std::endl;

        UDPEndpoint::pointer new_connection = UDPEndpoint::create(this, m_io_service);
        std::static_pointer_cast<UDPEndpoint>(new_connection)->setup(_udp_endpoint);
        m_endpoints.push_back(new_connection);
    }
}


void Router::start_accept() {
    std::cerr << "Router::start_accept()" << std::endl;

    TCPEndpoint::pointer new_connection = TCPEndpoint::create(this, m_io_service);

    m_tcp_acceptor.async_accept(new_connection->get_tcp_socket(),
                                boost::bind(&Router::handle_accept,
                                            this,
                                            new_connection,
                                            boost::asio::placeholders::error));
}


void Router::handle_accept(TCPEndpoint::pointer new_connection, const boost::system::error_code& error) {
    std::cerr << "Router::handle_accept()" << std::endl;

    if (!error) {
        m_endpoints.push_back(new_connection);
        new_connection->start();
    }

    start_accept();
}


void Router::process_mavlink_message(bool source_is_net, Endpoint::pointer source_endpoint, mavlink_message_t msg) {
    uint8_t buf[MAVLINK_MAX_PACKET_LEN];

    auto size = mavlink_msg_to_send_buffer(buf, &msg);

    auto entry = mavlink_get_msg_entry(msg.msgid);


    int target_sys_id = -1;
    int target_comp_id = -1;

    if (entry) {
        if (entry->flags & MAV_MSG_ENTRY_FLAG_HAVE_TARGET_SYSTEM) {
            target_sys_id = (_MAV_PAYLOAD(&msg))[entry->target_system_ofs];
        }
        if (entry->flags & MAV_MSG_ENTRY_FLAG_HAVE_TARGET_COMPONENT) {
            target_comp_id = (_MAV_PAYLOAD(&msg))[entry->target_component_ofs];
        }
    }

    std::cerr << "Processing message " << msg.msgid << " from " << static_cast<int16_t>(msg.sysid) << ":" << static_cast<int16_t>(msg.compid) << " to " << static_cast<int16_t>(target_sys_id) << ":" << static_cast<int16_t>(target_comp_id) << std::endl;

    /*
     * This implements the routing logic described in https://ardupilot.org/dev/docs/mavlink-routing-in-ardupilot.html,
     * however we do not need to care about component IDs for routing purposes, only system IDs
     *
     */
    for (auto const& endpoint: m_endpoints) {
        auto send = false;
        if (target_sys_id == -1) {
            send = true;
        } else if (target_sys_id == 0) {
            send = true;
        } else {
            send = endpoint->seen_sys_id(target_sys_id);
        }

        // don't send the packet right back out the interface it came from
        if (source_is_net) {
            if (source_endpoint != nullptr) {
                if (source_endpoint == endpoint) {
                    send = false;
                }
            }
        }

        if (send) {
            endpoint->send_message(buf, size);
        }
    }

    auto send_serial = false;
    if (target_sys_id == -1) {
        send_serial = true;
    } else if (target_sys_id == 0) {
        send_serial = true;
    } else {
        send_serial = seen_sys_id(target_sys_id);
    }

    if (source_is_net && send_serial) {
        m_serial.write(buf, size);
    }

    std::cerr << "----------------------------------------------------------------------------------------------------" << std::endl;
}


void Router::close_endpoint(std::shared_ptr<Endpoint> endpoint) {
    std::cerr << "Router::close_endpoint()" << std::endl;

    m_endpoints.erase(std::remove(m_endpoints.begin(), m_endpoints.end(), endpoint), m_endpoints.end());
    std::cerr << "Router::close_endpoint(): now have " << m_endpoints.size() << " endpoints " << std::endl;
}


void Router::handle_serial_read(char* buffer,
                                size_t size) {
    std::cerr << "Router::handle_serial_read()" << std::endl;

    mavlink_message_t msg;
    for (int i = 0; i < size; i++) {
        uint8_t res = mavlink_parse_char(MAVLINK_COMM_0, (uint8_t)buffer[i], &msg, &m_mavlink_status);
        if (res) {
            if (msg.sysid != 0) {
                add_known_sys_id(msg.sysid);
            }
            process_mavlink_message(false, nullptr, msg);
        }
    }
}
