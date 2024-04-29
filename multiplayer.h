#ifndef MULTIPLAYER_H
#define MULTIPLAYER_H

#define ASIO_STANDALONE
#include "class.h"
#include <asio.hpp>
#include <string>
#include <ctime>

using asio::ip::tcp;
void server()
{
    try
    {
        asio::io_context io_context;

        tcp::acceptor acceptor(io_context, tcp::endpoint(tcp::v4(), 4444));

        for (;;)
        {
            tcp::socket socket(io_context);
            acceptor.accept(socket);

            std::error_code ignored_error;
            asio::write(socket, asio::buffer("server hears you!"), ignored_error);
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << '\n';
    }
}

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