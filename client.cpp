// client.cpp
#include <boost/asio.hpp>
#include <iostream>
#include <thread>

using boost::asio::ip::tcp;

void read_thread(tcp::socket& socket) {
    while (true) {
        char data[1024];
        boost::system::error_code ec;
        size_t len = socket.read_some(boost::asio::buffer(data), ec);

        if (ec) break;

        std::cout << std::string(data, len);
    }
}

int main() {
    boost::asio::io_context io;
    tcp::socket socket(io);

    socket.connect(tcp::endpoint(
        boost::asio::ip::make_address("127.0.0.1"), 9000));

    std::thread t(read_thread, std::ref(socket));

    while (true) {
        std::string msg;
        std::getline(std::cin, msg);
        msg += "\n";
        boost::asio::write(socket, boost::asio::buffer(msg));
    }

    t.join();
}