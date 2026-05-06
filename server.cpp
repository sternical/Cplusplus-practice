//g++ server.cpp -o server -lboost_system -lpthread
// server.cpp
#include <boost/asio.hpp>
#include <iostream>
#include <set>
#include <memory>

using boost::asio::ip::tcp;

std::set<std::shared_ptr<tcp::socket>> clients;

void read_loop(std::shared_ptr<tcp::socket> socket) {
    auto buf = std::make_shared<std::vector<char>>(1024);

    socket->async_read_some(boost::asio::buffer(*buf),
        [socket, buf](boost::system::error_code ec, std::size_t length) {
            if (!ec) {
                std::string msg(buf->data(), length);

                // Broadcast to all clients
                for (auto& client : clients) {
                    if (client != socket) {
                        boost::asio::async_write(*client,
                            boost::asio::buffer(msg),
                            [](auto, auto) {});
                    }
                }

                read_loop(socket);
            } else {
                clients.erase(socket);
            }
        });
}

int main() {
    boost::asio::io_context io;
    tcp::acceptor acceptor(io, tcp::endpoint(tcp::v4(), 9000));

    std::function<void()> accept_loop;
    accept_loop = [&]() {
        auto socket = std::make_shared<tcp::socket>(io);
        acceptor.async_accept(*socket, [&, socket](auto ec) {
            if (!ec) {
                clients.insert(socket);
                read_loop(socket);
            }
            accept_loop();
        });
    };

    accept_loop();
    io.run();
}