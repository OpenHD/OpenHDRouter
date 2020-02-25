#ifndef ENDPOINT_H
#define ENDPOINT_H

#include <array>
#include <vector>

#include <boost/asio.hpp>

#include <openhd/mavlink.h>

class Router;

class Endpoint: public std::enable_shared_from_this<Endpoint> {
public:
    typedef std::shared_ptr<Endpoint> pointer;

    static pointer create(Router *router, boost::asio::io_service& io_service) {
        return pointer(new Endpoint(router, io_service));
    }

    void start();

    void add_known_sys_id(uint8_t sys_id) {
        bool found = false;
        for(auto const& known_sys_id: m_known_sys_ids) {
            if (known_sys_id == sys_id) {
                found = true;
            }
        }
        if (!found) {
            std::cout << "Adding known sys ID for TCP endpoint: " << static_cast<int16_t>(sys_id) << std::endl;
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

    void send_message(uint8_t *buf, int size);

    boost::asio::ip::tcp::socket& get_socket();

protected:
    void handle_write(const boost::system::error_code& error,
                      size_t bytes_transferred);

    void handle_read(std::shared_ptr<Endpoint>& s,
                     const boost::system::error_code& err,
                     size_t bytes_transferred);

    Router *m_router = nullptr;

    std::vector<uint8_t> m_known_sys_ids;


    enum { max_length = 1024 };
    char data[max_length];

    boost::asio::ip::tcp::socket m_socket;

    mavlink_status_t m_mavlink_status;
private:
  Endpoint(Router *router, boost::asio::io_service& io_service): m_router(router), m_socket(io_service) {}
};

#endif // ENDPOINT_H
