#include <iostream>
#include <iterator>
#include <exception>
#include <vector>

#include "boost/asio.hpp"
#include <boost/program_options.hpp>

#include "router.h"

int main(int argc, char *argv[]) {
    boost::asio::io_service io_service;

    Router *router = nullptr;

    try {
        boost::program_options::options_description desc("Options");

        desc.add_options()("help", "produce help message")
                          ("tcp-port", boost::program_options::value<int>()->required(), "tcp server port")
                          ("serial-baud,b", boost::program_options::value<int>()->default_value(115200), "serial port baud")
                          ("serial-port", boost::program_options::value<std::string>()->default_value(""), "serial port to use");

        boost::program_options::variables_map vm;
        boost::program_options::store(boost::program_options::parse_command_line(argc, argv, desc), vm);
        boost::program_options::notify(vm);

        if (vm.count("help")) {
            std::cerr << desc << std::endl;
            exit(0);
        }

        if (vm.count("tcp-port") == 0) {
            std::cerr << "No tcp port provided" << std::endl;
            exit(1);
        }

        int tcp_port = vm["tcp-port"].as<int>();

        std::string serial_port;
        int baud = 115200;

        if (vm.count("serial-port") == 1) {
            serial_port = vm["serial-port"].as<std::string>();
            baud = vm["serial-baud"].as<int>();
        }

        router = new Router(io_service, tcp_port, baud, serial_port);
        router->setup();
        io_service.run();
    } catch (std::exception &ex) {
        std::cerr << "Error: " << ex.what() << std::endl;
        exit(1);
    } catch (...) {
        std::cerr << "Unknown exception occurred" << std::endl;
        exit(1);
    }

    return 0;
}
