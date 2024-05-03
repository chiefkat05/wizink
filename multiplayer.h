#ifndef MULTIPLAYER_H
#define MULTIPLAYER_H

#define ASIO_STANDALONE
#include "class.h"
#include <asio.hpp>
#include <string>
#include <thread>
#include <deque>

using asio::ip::tcp;

template <typename T>
struct message_header
{
    T id{};
    uint32_t size = 0;
};

template <typename T>
struct message
{
    message_header<T> header{};
    std::vector<uint8_t> body;

    size_t size() const
    {
        return sizeof(message_header<T>) + body.size();
    }

    friend std::ostream &operator<<(std::ostream &os, const message<T> &msg)
    {
        os << "ID:" << static_cast<int>(msg.header.id) << " Size:" << msg.header.size;
        return os;
    }

    template <typename DataType>
    friend message<T> &operator<<(message<T> &msg, const DataType &data)
    {
        static_assert(std::is_standard_layout<DataType>::value, "Data is too complex!\n");

        size_t i = msg.body.size();

        msg.body.resize(msg.body.size() + sizeof(DataType));

        std::memcpy(msg.body.data() + i, &data, sizeof(DataType));

        msg.header.size = msg.size();

        return msg;
    }

    template <typename DataType>
    friend message<T> &operator>>(message<T> &msg, DataType &data)
    {
        static_assert(std::is_standard_layout<DataType>::value, "Data is too complex!\n");

        size_t i = msg.body.size() - sizeof(DataType);

        std::memcpy(&data, msg.body.data() + i, sizeof(DataType));

        msg.body.resize(i);

        msg.header.size = msg.size();

        return msg;
    }
};

template <typename T>
class tsqueue
{
public:
    tsqueue() = default;
    tsqueue(const tsqueue<T> &) = delete;
    virtual ~tsqueue() { clear(); }

    const T &front()
    {
        std::scoped_lock lock(muxQueue);
        return deqQueue.front();
    }
    const T &back()
    {
        std::scoped_lock lock(muxQueue);
        return deqQueue.back();
    }

    void push_back(const T &item)
    {
        std::scoped_lock lock(muxQueue);
        deqQueue.emplace_back(std::move(item));
    }
    void push_front(const T &item)
    {
        std::scoped_lock lock(muxQueue);
        deqQueue.emplace_front(std::move(item));
    }

    bool empty()
    {
        std::scoped_lock lock(muxQueue);
        return deqQueue.empty();
    }

    size_t count()
    {
        std::scoped_lock lock(muxQueue);
        return deqQueue.size();
    }

    void clear()
    {
        std::scoped_lock lock(muxQueue);
        deqQueue.clear();
    }

    T pop_front()
    {
        std::scoped_lock lock(muxQueue);
        auto t = std::move(deqQueue.front());
        deqQueue.pop_front();
        return t;
    }
    T pop_back()
    {
        std::scoped_lock lock(muxQueue);
        auto t = std::move(deqQueue.back());
        deqQueue.pop_back();
        return t;
    }

protected:
    std::mutex muxQueue;
    std::deque<T> deqQueue;
};

template <typename T>
class connection;

template <typename T>
struct owned_message
{
    std::shared_ptr<connection<T>> remote = nullptr;
    message<T> msg;

    friend std::ostream &operator<<(std::ostream &os, const owned_message<T> &msg)
    {
        os << msg.msg;
        return os;
    }
};

template <typename T>
class connection : public std::enable_shared_from_this<connection<T>>
{
public:
    connection() {}
    virtual ~connection() {}

    bool ConnectToServer();
    bool Disconnect();
    bool IsConnected() const;

    bool Send(const message<T> &msg);

protected:
    tcp::socket m_socket;

    asio::io_context &m_asioContext;

    tsqueue<owned_message<T>> m_qMessagesIn;
};

template <typename T>
class client_interface
{
    client_interface() : m_socket(m_context)
    {
    }

    virtual ~client_interface()
    {
        Disconnect();
    }

public:
    bool Connect(const std::string &host, const uint16_t port)
    {
        try
        {
            m_connection = std::make_unique<connection<T>>();

            tcp::resolver resolver(m_context);
            m_endpoints = resolver.resolve(host, std::to_string(port));

            m_connection->ConnectToServer(m_endpoints);

            thrContext = std::thread([this]()
                                     { m_context.run(); });
        }
        catch (const std::exception &e)
        {
            std::cerr << e.what() << '\n';
        }
    }

    void Disconnect()
    {
        if (IsConnected())
        {
            m_connection->Disconnect();
        }

        m_context.stop();

        if (thrContext.joinable())
            thrContext.join();

        m_connection.release();
    }

    bool IsConnected()
    {
        if (m_connection)
            return m_connection->IsConnected();
        else
            return false;
    }

    tsqueue<owned_message<T>> &Incoming()
    {
        return m_qMessagesIn;
    }

protected:
    asio::io_context m_context;
    std::thread thrContext;

    tcp::socket m_socket;

    std::unique_ptr<connection<T>> m_connection;

private:
    tsqueue<owned_message<T>> m_qMessagesIn;
};

template <typename T>
class server_interface
{
public:
    server_interface(uint16_t port) {}

    virtual ~server_interface() {}

    bool Start()
    {
    }
    void Stop()
    {
    }
};

#endif