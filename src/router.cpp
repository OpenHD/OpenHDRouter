#include <algorithm>
#include <iostream>

#include <boost/asio.hpp>
#include <boost/bind.hpp>

#include <openhd/mavlink.h>
#include <mavlink_types.h>


#include "router.h"
#include "endpoint.h"


Router::Router(boost::asio::io_service &io_service, int tcp_port, std::string serial_port):
    m_io_service(io_service),
    m_tcp_acceptor(io_service, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), tcp_port)),
    m_serial(io_service) {

    try {
        m_serial.open(serial_port);
    } catch (boost::system::system_error::exception e) {
        std::cerr << "Router::Router(): failed to open serial port: " << serial_port << std::endl;
        exit(1);
    }

    try {
        m_serial.set_option(boost::asio::serial_port_base::baud_rate(115200));
        m_serial.set_option(boost::asio::serial_port_base::character_size(8));
        m_serial.set_option(boost::asio::serial_port_base::flow_control(boost::asio::serial_port_base::flow_control::none));
        m_serial.set_option(boost::asio::serial_port_base::parity(boost::asio::serial_port_base::parity::none));
        m_serial.set_option(boost::asio::serial_port_base::stop_bits(boost::asio::serial_port_base::stop_bits::one));
    } catch (boost::system::system_error::exception e) {
        std::cerr << "Router::Router(): failed to set serial parameters on: " << serial_port << std::endl;
        exit(1);
    }
    start_accept();
    start_serial_read();
}


void Router::setup() {
    std::cerr << "Router::setup()" << std::endl;

}


void Router::start_accept() {
    std::cerr << "Router::start_accept()" << std::endl;

    Endpoint::pointer new_connection = Endpoint::create(this, m_io_service);

    m_tcp_acceptor.async_accept(new_connection->get_socket(),
                                boost::bind(&Router::handle_accept,
                                            this,
                                            new_connection,
                                            boost::asio::placeholders::error));
}

void Router::start_serial_read() {
    std::cerr << "Router::start_serial_read()" << std::endl;

    m_serial.async_read_some(boost::asio::buffer(data, max_length),
                             boost::bind(&Router::handle_serial_read,
                                         this,
                                         boost::asio::placeholders::error,
                                         boost::asio::placeholders::bytes_transferred));
}


void Router::handle_accept(Endpoint::pointer new_connection, const boost::system::error_code& error) {
    std::cerr << "Router::handle_accept()" << std::endl;

    if (!error) {
        m_endpoints.push_back(new_connection);
        new_connection->start();
    }

    start_accept();
}


void Router::process_mavlink_message(bool source_is_tcp, Endpoint::pointer source_endpoint, mavlink_message_t msg) {
    std::cerr << "Router::process_mavlink_message()" << std::endl;

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
        if (source_is_tcp) {
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

    if (source_is_tcp && send_serial) {
        boost::asio::async_write(m_serial,
                                 boost::asio::buffer(buf, size),
                                 boost::bind(&Router::handle_serial_write,
                                             this,
                                             boost::asio::placeholders::error,
                                             boost::asio::placeholders::bytes_transferred));
    }
}


void Router::close_endpoint(std::shared_ptr<Endpoint> endpoint) {
    std::cerr << "Router::close_endpoint()" << std::endl;

    m_endpoints.erase(std::remove(m_endpoints.begin(), m_endpoints.end(), endpoint), m_endpoints.end());
    std::cerr << "Router::close_endpoint(): now have " << m_endpoints.size() << " endpoints " << std::endl;
}


void Router::handle_serial_write(const boost::system::error_code& error,
                                 size_t bytes_transferred) {}


void Router::handle_serial_read(const boost::system::error_code& error,
                                size_t bytes_transferred) {
    std::cerr << "Router::handle_serial_read()" << std::endl;

    if (!error) {
        mavlink_message_t msg;
        for (int i = 0; i < bytes_transferred; i++) {
            uint8_t res = mavlink_parse_char(MAVLINK_COMM_0, (uint8_t)data[i], &msg, &m_mavlink_status);
            if (res) {
                add_known_sys_id(msg.sysid);
                process_mavlink_message(false, nullptr, msg);
            }
        }
    }
    start_serial_read();
}
