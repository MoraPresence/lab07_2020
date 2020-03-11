// Copyright 2018 Your Name <your_email>

#ifndef INCLUDE_CLIENT_HPP_
#define INCLUDE_CLIENT_HPP_

#include <iostream>
#include <boost/asio.hpp>
#include <utility>
#include <boost/thread/pthread/recursive_mutex.hpp>
#include <algorithm>
#include <boost/log/support/date_time.hpp>
#include <thread>
#include <boost/thread/thread.hpp>
#include <string>

namespace ip = boost::asio::ip;
using Context = boost::asio::io_context;
using Socket = boost::asio::ip::tcp::socket;
using Resolver = boost::asio::ip::tcp::resolver;
using StreamBuffer = boost::asio::streambuf;
using Buffer = boost::asio::mutable_buffer;

class client {
public:
    client(std::shared_ptr<Context> &io_context, Socket socket)
    : _socket(std::move(socket)),
    _starrted(true), _resolver(*io_context) {}


    void connect(const ip::tcp::endpoint &);

    void loop();

    void close();

    Socket &getSocket();

    void answerFromServer();

    [[nodiscard]]  bool isClose();

    void login();

    void ping();

    void getClients();

    std::string getName();

    void setName(const std::string &username) {
        _username = username;
    }

    void setTime(time_t timeNow) {
        _lastTime = timeNow;
    }

    void setStart(bool health) {
        _starrted = health;
    }

    [[nodiscard]] bool timed_out() const;

    void stop();

private:
    std::string _username = "user";
    Socket _socket;
    bool _starrted;
    std::shared_ptr<Context> _io_context = std::make_shared<Context>();
    Resolver _resolver;
    time_t _lastTime;
};
#endif // INCLUDE_CLIENT_HPP_
