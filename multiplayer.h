#ifndef MULTIPLAYER_H
#define MULTIPLAYER_H

#define ASIO_STANDALONE
#include "class.h"
#include <asio.hpp>
#include <string>
#include <ctime>

using asio::ip::tcp;
class tcp_connection
    : public std::enable_shared_from_this<tcp_connection>
{
public:
    typedef std::shared_ptr<tcp_connection> pointer;

    static pointer create(asio::io_context &io_context)
    {
        return pointer(new tcp_connection(io_context));
    }

    tcp::socket &socket()
    {
        return socket_;
    }

    void start()
    {
        message_ = "connection message pending\n";

        asio::async_write(socket_, asio::buffer(message_),
                          std::bind(&tcp_connection::handle_write, shared_from_this(),
                                    asio::placeholders::error, asio::placeholders::bytes_transferred));
    }
    void message(std::string msg)
    {
        asio::async_write(socket_, asio::buffer(msg),
                          std::bind(&tcp_connection::handle_write, shared_from_this(),
                                    asio::placeholders::error, asio::placeholders::bytes_transferred));
    }

private:
    tcp_connection(asio::io_context &io_context)
        : socket_(io_context) {}

    void handle_write(const std::error_code & /*error*/, size_t /*bytes_transferred*/) {}

    tcp::socket socket_;
    std::string message_;
};

class tcp_server
{
public:
    tcp_server(asio::io_context &io_context)
        : io_context_(io_context),
          acceptor_(io_context, tcp::endpoint(tcp::v4(), 4444))
    {
        start_accept();
    }

private:
    void start_accept()
    {
        tcp_connection::pointer new_connection = tcp_connection::create(io_context_);

        acceptor_.async_accept(new_connection->socket(),
                               std::bind(&tcp_server::handle_accept,
                                         this, new_connection, asio::placeholders::error));
    }
    void handle_accept(tcp_connection::pointer new_connection,
                       const std::error_code &error)
    {
        if (!error)
        {
            new_connection->start();
        }

        start_accept();
    }

    asio::io_context &io_context_;
    tcp::acceptor acceptor_;
};

void chatToServer(tcp_connection &connection, std::string message)
{
    connection.message(message);
}

void server()
{
    try
    {
        asio::io_context io_context;

        tcp_server server(io_context);

        io_context.run();
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << '\n';
    }
}

tcp_connection & chatConnection;

void client(std::string ip, std::string port)
{
    try
    {
        asio::io_context io_context;

        tcp::resolver resolver(io_context);

        tcp::resolver::results_type endpoints = resolver.resolve(ip, port);

        tcp::socket socket(io_context);
        asio::connect(socket, endpoints);

        for (;;)
        {
            std::array<char, 128> buf;
            std::error_code error;

            size_t len = socket.read_some(asio::buffer(buf), error);

            if (error == asio::error::eof)
                break;
            else if (error)
                throw std::system_error(error);

            std::cout.write(buf.data(), len);
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << '\n';
    }
}

#endif