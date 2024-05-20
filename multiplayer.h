#ifndef MULTIPLAYER_H
#define MULTIPLAYER_H

#define ASIO_STANDALONE
#include "class.h"
#include <asio.hpp>
#include <string>
#include <thread>
#include <deque>

using asio::ip::tcp;

class connection;

struct message_header
{
    int id;
    uint32_t size = 0;
};

struct message
{
    message_header header;
    std::vector<uint8_t> body;

    size_t size() const
    {
        return sizeof(message_header) + body.size();
    }

    friend std::ostream &operator<<(std::ostream &os, const message &msg)
    {
        os << "ID: " << msg.header.id << " Size: " << msg.header.size << "\n";
        return os;
    }

    template <typename DataType>
    friend message &operator<<(message &msg, const DataType data)
    {
        static_assert(std::is_standard_layout<DataType>::value, "Data is too complex!\n");

        size_t i = msg.body.size();

        msg.body.resize(msg.body.size() + sizeof(DataType));

        std::memcpy(msg.body.data() + i, &data, sizeof(DataType));

        msg.header.size = msg.size();

        return msg;
    }

    template <typename DataType>
    friend message &operator>>(message &msg, DataType &data)
    {
        static_assert(std::is_standard_layout<DataType>::value, "Data is too complex!\n");

        size_t i = msg.body.size() - sizeof(DataType);

        std::memcpy(&data, msg.body.data() + i, sizeof(DataType));

        msg.body.resize(i);

        msg.header.size = msg.size();

        return msg;
    }
};
struct owned_message
{
    std::shared_ptr<connection> remote = nullptr;
    message msg;

    friend std::ostream &operator<<(std::ostream &os, const owned_message &msg)
    {
        os << msg.msg;
        return os;
    }
};

template <typename T>
class queue
{
public:
    queue() = default;
    queue(const queue<T> &) = delete;
    virtual ~queue() { clear(); }

    const T &front()
    {
        std::scoped_lock lock(mux_);
        return deq_.front();
    }
    const T &back()
    {
        std::scoped_lock lock(mux_);
        return deq_.back();
    }

    void push_back(const T &input)
    {
        std::scoped_lock lock(mux_);
        deq_.emplace_back(std::move(input));
    }
    void push_front(const T &input)
    {
        std::scoped_lock lock(mux_);
        deq_.emplace_front(std::move(input));
    }

    bool empty()
    {
        std::scoped_lock lock(mux_);
        return deq_.empty();
    }

    size_t length()
    {
        std::scoped_lock lock(mux_);
        return deq_.size();
    }

    void clear()
    {
        std::scoped_lock lock(mux_);
        deq_.clear();
    }

    T pop_front()
    {
        std::scoped_lock lock(mux_);
        auto t = std::move(deq_.front());
        deq_.pop_front();
        return t;
    }
    T pop_back()
    {
        std::scoped_lock lock(mux_);
        auto t = std::move(deq_.back());
        deq_.pop_back();
        return t;
    }

protected:
    std::mutex mux_;
    std::deque<T> deq_;
};

class connection : public std::enable_shared_from_this<connection>
{
public:
    enum class owner
    {
        server,
        client
    };

    connection(owner parent, asio::io_context &context, tcp::socket socket, queue<owned_message> &qin)
        : io_(context), socket_(std::move(socket)), queueIn_(qin)
    {
        ownerType_ = parent;
    }
    virtual ~connection() {}

    bool ConnectToServer(const tcp::resolver::results_type &endP)
    {
        return true;
    }
    bool ConnectToClient(uint32_t uid = 0)
    {
        if (ownerType_ != owner::server)
            return false;

        if (socket_.is_open())
        {
            id = uid;
        }
        return true;
    }
    bool Disconnect();
    bool IsConnected() const
    {
        return socket_.is_open();
    }

    bool Send(const message &msg);

    uint32_t GetID() const
    {
        return id;
    }

private:
    void ReadHeader()
    {
        asio::async_read(socket_, asio::buffer(&msgTmp_.header, sizeof(message_header)),
                         [this](std::error_code ec, std::size_t length)
                         {
                             if (ec)
                             {
                                 std::cout << "Read Header Failed: " << ec.message() << "\n";
                                 socket_.close();
                                 return;
                             }

                             if (msgTmp_.header.size > 0)
                             {
                                 msgTmp_.body.resize(msgTmp_.header.size);
                                 ReadBody();
                             }

                             AddToIncomingMessageQueue();
                         });
    }
    void ReadBody()
    {
    }
    void WriteHeader()
    {
    }
    void WriteBody()
    {
    }

protected:
    tcp::socket socket_;
    asio::io_context &io_;

    queue<message> queueOut_;
    queue<owned_message> &queueIn_;
    message msgTmp_;

    owner ownerType_ = owner::server;
    uint32_t id;
};

class client
{
    client()
    {
    }
    virtual ~client()
    {
        Disconnect();
    }

public:
    bool Connect(const std::string &host, const std::string port)
    {
        try
        {
            connection_ = std::make_unique<connection>(connection::owner::client, io_, tcp::socket(io_), msgIn_);

            tcp::resolver resolver(io_);
            tcp::resolver::results_type endpoints = resolver.resolve(host, port);

            connection_->ConnectToServer(endpoints);

            thr_ = std::thread([this]()
                               { io_.run(); });
        }
        catch (const std::exception &e)
        {
            std::cerr << "\n\tClient Connection Failed: " << e.what() << '\n';
        }

        return true;
    }

    void Disconnect()
    {
        if (IsConnected())
            connection_->Disconnect();

        io_.stop();

        if (thr_.joinable())
            thr_.join();

        connection_.release();
    }

    bool IsConnected()
    {
        if (connection_)
            return connection_->IsConnected();

        return false;
    }

    queue<owned_message> &Incoming()
    {
        return msgIn_;
    }

protected:
    asio::io_context io_;
    std::thread thr_;
    // tcp::socket sock_;

    std::unique_ptr<connection> connection_;

private:
    queue<owned_message> msgIn_;
};

class server
{
public:
    server(uint16_t port) : acc_(io_, tcp::endpoint(tcp::v4(), port))
    {
    }
    virtual ~server()
    {
        Stop();
    }
    bool Start()
    {
        try
        {
            WaitForClientConnection();

            thr_ = std::thread([this]()
                               { io_.run(); });
        }
        catch (const std::exception &e)
        {
            std::cerr << "\n\tServer Startup Fail: " << e.what() << '\n';
        }

        std::cout << "\n\tServer Online\n";
        return true;
    }
    void Stop()
    {
        io_.stop();

        if (thr_.joinable())
            thr_.join();

        std::cout << "\n\tServer Offline\n";
    }

    void WaitForClientConnection()
    {
        acc_.async_accept([this](std::error_code ec, tcp::socket socket)
                          {
                            if (ec)
                            {
                                std::cout << "\n\tServer Connection Failed: " << ec.message() << "\n";
                                socket.close();
                                WaitForClientConnection();
                            }
                            std::cout << "\n\tClient Connected: " << socket.remote_endpoint() << "\n";
                            std::shared_ptr<connection> newConnection = std::make_shared<connection>(connection::owner::server,
                                                                                                    io_, std::move(socket), msgIn_);

                            if (OnClientConnect(newConnection))
                            {
                                connections_.push_back(std::move(newConnection));
                                connections_.back()->ConnectToClient(++idCounter_);

                                std::cout << "\n\tServer Connection Succeeded: " << connections_.back()->GetID() << "\n";
                            }
                            else
                            {
                                std::cout << "\n\tServer Connection Failed: Connection Denied\n";
                            }
                            WaitForClientConnection(); });
    }

    void MessageClient(std::shared_ptr<connection> client, const message &msg)
    {
        if (!client || !client->IsConnected())
        {
            OnClientDisconnect(client);
            client.reset();
            connections_.erase(
                std::remove(connections_.begin(), connections_.end(), client), connections_.end());
            return;
        }

        client->Send(msg);
    }
    void MessageAllClients(const message &msg, std::shared_ptr<connection> cIgnore = nullptr)
    {
        bool disconnectedClient = false;
        for (auto &client : connections_)
        {
            if (!client || !client->IsConnected())
            {
                OnClientDisconnect(client);
                client.reset();
                disconnectedClient = true;
                continue;
            }

            if (client == cIgnore)
                continue;

            client->Send(msg);
        }

        connections_.erase(
            std::remove(connections_.begin(), connections_.end(), nullptr), connections_.end());
    }

    bool OnClientConnect(std::shared_ptr<connection> client)
    {
        std::cout << "\n\tClient Connection Succeeded: " << client->GetID();
        return true;
    }
    void OnClientDisconnect(std::shared_ptr<connection> client)
    {
    }
    void OnMessage(std::shared_ptr<connection> client, message &msg)
    {
    }

    void Update(size_t messageLimit = -1)
    {
        size_t messageCount = 0;

        while (messageCount < messageLimit && !msgIn_.empty())
        {
            auto msg = msgIn_.pop_front();

            OnMessage(msg.remote, msg.msg);

            ++messageCount;
        }
    }

protected:
    queue<owned_message> msgIn_;

    std::deque<std::shared_ptr<connection>> connections_;

    asio::io_context io_;
    std::thread thr_;

    tcp::acceptor acc_;
    uint32_t idCounter_ = 400;
};

#endif