#ifndef MULTIPLAYER_H
#define MULTIPLAYER_H

#define ASIO_STANDALONE
#include <iostream>
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

    size_t size() const;

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

    void push_back(const T &input);
    void push_front(const T &input);

    bool empty();

    size_t length();

    void clear();

    T pop_front();
    T pop_back();

    void wait();

protected:
    std::mutex mux_;
    std::deque<T> deq_;

    std::condition_variable cvB_;
    std::mutex muxB_;
};

class server;

class connection : public std::enable_shared_from_this<connection>
{
public:
    enum class owner
    {
        server,
        client
    };

    connection(owner parent, asio::io_context &context, tcp::socket socket, queue<owned_message> &qin);
    virtual ~connection();

    void ConnectToServer(const tcp::resolver::results_type &endP);
    bool ConnectToClient(server *server, uint32_t uid = 0);
    bool Disconnect();
    bool IsConnected() const;

    void Send(const message &msg);

    uint32_t GetID() const;

private:
    void ReadHeader();
    void ReadBody();
    void WriteHeader();
    void WriteBody();

    void AddToIncomingMessageQueue();

    uint64_t scramble(uint64_t input);

    void WriteValidation();
    void ReadValidation(server *server = nullptr);

protected:
    tcp::socket socket_;
    asio::io_context &io_;

    queue<message> queueOut_;
    queue<owned_message> &queueIn_;
    message msgTmp_;

    owner ownerType_ = owner::server;
    uint32_t id;

    uint64_t handshakeIn = 0;
    uint64_t handshakeOut = 0;
    uint64_t handshakeCheck = 0;
};

class client
{
public:
    client();
    virtual ~client();

    bool Connect(const std::string &host, const std::string port);

    void SendMessage(std::array<char, 128> &i);
    void MessageAll(std::array<char, 128> &i);

    void Disconnect();

    bool IsConnected();

    queue<owned_message> &Incoming();

protected:
    asio::io_context io_;
    std::thread thr_;

    std::unique_ptr<connection> connection_;

private:
    queue<owned_message> msgIn_;
};

class server
{
public:
    server(uint16_t port);
    virtual ~server();
    bool Start();
    void Stop();

    void WaitForClientConnection();

    void MessageClient(std::shared_ptr<connection> client, const message &msg);
    void MessageAllClients(const message &msg, std::shared_ptr<connection> cIgnore = nullptr);

    bool OnClientConnect(std::shared_ptr<connection> client);
    void OnClientDisconnect(std::shared_ptr<connection> client);
    void OnMessage(std::shared_ptr<connection> client, message &msg);

    void Update(size_t messageLimit = -1);

    void OnClientValidated(std::shared_ptr<connection> client);

protected:
    queue<owned_message> msgIn_;

    std::deque<std::shared_ptr<connection>> connections_;

    asio::io_context io_;
    std::thread thr_;

    tcp::acceptor acc_;
    uint32_t idCounter_ = 400;
};

#endif