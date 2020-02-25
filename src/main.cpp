#include <iostream>
#include <iterator>
#include <exception>

#include "boost/asio.hpp"
#include <boost/program_options.hpp>

#include "router.h"

int main(int argc, char *argv[]) {
    boost::asio::io_context io_context;

    Router *router = nullptr;

    try {
        boost::program_options::options_description desc("Options");

        desc.add_options()("help", "produce help message")
                          ("tcp-port", boost::program_options::value<int>()->required(), "tcp server port")
                          ("serial-port", boost::program_options::value<std::string>()->required(), "serial port to use");

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

       if (vm.count("serial-port") == 0) {
            std::cerr << "No serial port provided" << std::endl;
            exit(1);
        }
        std::string serial_port = vm["serial-port"].as<std::string>();

        router = new Router(io_context, tcp_port, serial_port);
        router->setup();
        io_context.run();
    } catch (std::exception &ex) {
        std::cerr << "Error: " << ex.what() << std::endl;
        exit(1);
    } catch (...) {
        std::cerr << "Unknown exception occurred" << std::endl;
        exit(1);
    }

    return 0;
}
