// Copyright 2018 Your Name <your_email>

#ifndef INCLUDE_HEADER_HPP_
#define INCLUDE_HEADER_HPP_

#include <client.hpp>
#include <vector>
#include <iostream>
#include <boost/asio.hpp>
#include <boost/thread/pthread/recursive_mutex.hpp>
#include <boost/thread/thread.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/support/date_time.hpp>
#include <boost/log/sources/severity_channel_logger.hpp>
#include <boost/log/sources/record_ostream.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/sinks.hpp>

class server {
public:
    using Endpoint = boost::asio::ip::tcp::endpoint;
    using Acceptor = boost::asio::ip::tcp::acceptor;
    using Context = boost::asio::io_context;
    using Socket = boost::asio::ip::tcp::socket;

    explicit server(const Endpoint &endpoint)
            : _endpoint(endpoint),
              _acceptor(std::make_unique<Acceptor>(*_io_context, endpoint)) {}

    void acceptThread();

    void handleClientsThread();

    void startServer();

    void login(std::shared_ptr <client> &);

    void ping(std::shared_ptr <client> &);

    void getClients(std::shared_ptr <client> &);

    static void initLog();

    void mutexLock();

    void mutexUnlock();

    void setClientsStatus(bool &);

private:
    std::shared_ptr <Context> _io_context = std::make_shared<Context>();
    Endpoint _endpoint;
    std::unique_ptr <Acceptor> _acceptor;
    std::vector <std::shared_ptr<client>> _clients;
    std::mutex _mutex;
    bool _clients_changed = false;
};

#endif // INCLUDE_HEADER_HPP_
