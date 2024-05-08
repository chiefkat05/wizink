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
    enum class owner
    {
        server,
        client
    };

    connection(owner parent, asio::io_context &asioContext, asio::ip::tcp::socket socket, tsqueue<owned_message<T>> &qIn)
        : m_asioContext(asioContext), m_socket(std::move(socket)), m_qMessagesIn(qIn)
    {
        m_nOwnerType = parent;
    }
    virtual ~connection() {}

    uint32_t GetID() const
    {
        return id;
    }

    void ConnectToClient(uint32_t uid = 0)
    {
        if (m_nOwnerType != owner::server)
            return;

        if (m_socket.is_open())
            id = uid;
    }

    bool ConnectToServer();
    bool Disconnect();
    bool IsConnected() const
    {
        return m_socket.is_open();
    }

    bool Send(const message<T> &msg);

protected:
    tcp::socket m_socket;

    asio::io_context &m_asioContext;

    tsqueue<message<T>> m_qMessagesOut;
    tsqueue<owned_message<T>> &m_qMessagesIn;

    owner m_nOwnerType = owner::server;
    uint32_t id = 0;
};

template <typename T>
class client_interface
{
public:
    client_interface() : m_socket(m_context)
    {
    }

    virtual ~client_interface()
    {
        Disconnect();
    }

    bool Connect(const std::string &host, const uint16_t port)
    {
        try
        {
            m_connection = std::make_unique<connection<T>>();

            tcp::resolver resolver(m_context);
            tcp::resolver::results_type m_endpoints = resolver.resolve(host, std::to_string(port));

            m_connection->ConnectToServer(m_endpoints);

            thrContext = std::thread([this]()
                                     { m_context.run(); });
        }
        catch (const std::exception &e)
        {
            std::cerr << e.what() << '\n';
        }

        return true;
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
    server_interface(uint16_t port)
        : m_asioAcceptor(m_asioContext, tcp::endpoint(tcp::v4(), port)), port(port)
    {
    }

    virtual ~server_interface()
    {
        Stop();
    }

    bool Start()
    {
        try
        {
            std::cout << "[SERVER WAITING FOR CONNECTION]\n";
            WaitForClientConnection();

            m_threadContext = std::thread([this]()
                                          { m_asioContext.run(); });
        }
        catch (std::exception &e)
        {
            std::cerr << e.what() << std::endl;
            return false;
        }

        std::cout << "[SERVER ONLINE] port = " << port << "\n";
        return true;
    }
    void Stop()
    {
        m_asioContext.stop();

        if (m_threadContext.joinable())
            m_threadContext.join();

        std::cout << "[SERVER OFFLINE]\n";
    }

    void WaitForClientConnection()
    {
        m_asioAcceptor.async_accept([this](std::error_code ec, tcp::socket socket)
                                    {
            if (!ec)
            {
                std::cout << "[SERVER CONNECTION SUCCESS]: " << socket.remote_endpoint() << std::endl;

                std::shared_ptr<connection<T>> newconnection =
                    std::make_shared<connection<T>>(connection<T>::owner::server,
                    m_asioContext, std::move(socket), m_qMessagesIn);

                if (OnClientConnect(newconnection))
                {
                    m_deqConnections.push_back(std::move(newconnection));

                    m_deqConnections.back()->ConnectToClient(nIDCounter++);

                    std::cout << "[SERVER CONNECTION APPROVED]: " << m_deqConnections.back()->GetID() << std::endl;
                }
                else
                {
                    std::cout << "[SERVER CONNECTION DENIED]\n";
                }
            }
            else
            {
                std::cout << "[SERVER CONNECTION FAILED]: " << ec.message() << std::endl;
            }
            WaitForClientConnection(); });
        std::cout << "huh\n";
    }
    void MessageClient(std::shared_ptr<connection<T>> client, const message<T> &msg)
    {
        if (!client || !client->IsConnected())
        {
            std::cout << "[SERVER MESSAGE FAILED]\n";
            OnClientDisconnect(client);
            client.reset();
            m_deqConnections.erase(
                std::remove(m_deqConnections.begin(), m_deqConnections.end, client),
                m_deqConnections.end());
            return;
        }

        client->Send(msg);
    }

    void MessageAllClients(const message<T> &msg, std::shared_ptr<connection<T>> pIgnoreClient = nullptr)
    {
        bool invalidClientExists = false;
        for (auto &client : m_deqConnections)
        {
            if (!client || !client->IsConnected())
            {
                std::cout << "[SERVER MESSAGE FAILED]\n";
                OnClientDisconnect(client);
                client.reset();
                invalidClientExists = true;
                continue;
            }

            if (client == pIgnoreClient)
                continue;

            client->Send(msg);
        }

        if (!invalidClientExists)
            return;

        m_deqConnections.erase(
            std::remove(m_deqConnections.begin(), m_deqConnections.end, nullptr),
            m_deqConnections.end());
    }

    void Update(size_t maxMessages = -1)
    {
        size_t messageCount = 0;
        while (messageCount < maxMessages && m_qMessagesIn.empty())
        {
            auto msg = m_qMessagesIn.pop_front();

            OnMessage(msg.remote, msg.msg);

            ++messageCount;
        }
    }

protected:
    virtual bool OnClientConnect(std::shared_ptr<connection<T>> client)
    {
        return false;
    }

    virtual void OnClientDisconnect(std::shared_ptr<connection<T>> connection)
    {
    }

    virtual void OnMessage(std::shared_ptr<connection<T>> client, message<T> &msg)
    {
    }

protected:
    tsqueue<owned_message<T>> m_qMessagesIn;

    std::deque<std::shared_ptr<connection<T>>> m_deqConnections;

    asio::io_context m_asioContext;
    std::thread m_threadContext;

    tcp::acceptor m_asioAcceptor;

    uint32_t nIDCounter = 10000;

    uint16_t port;
};

#endif