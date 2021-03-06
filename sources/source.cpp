// Copyright 2018 Your Name <your_email>

#include <header.hpp>

void server::acceptThread() {
    while (true) {
        Socket socket = _acceptor->accept();
        auto clt = std::make_shared<client>(_io_context, std::move(socket));
        mutexLock();
        _clients.push_back(clt);
        setClientsStatus(true);
        mutexUnlock();
        BOOST_LOG_TRIVIAL(info)
                << "Client connected: "
                << socket.remote_endpoint().address().to_string()
                << " Port: " << socket.remote_endpoint().port()
                << std::endl;
    }
}

void server::handleClientsThread() {
    std::this_thread::sleep_for(std::chrono_literals::operator ""ms(1));
    while (true) {
        mutexLock();
        if (!_clients.empty()) {
            for (auto &client : _clients) {
                if (client->isClose()) {
                    continue;
                }
                if (_clients_changed)
                    continue;
                try {
                    boost::asio::streambuf buffer{};
                    read_until(client->getSocket(), buffer, "\n");

                    std::string message
                            {std::istreambuf_iterator < char > {&buffer},
                             std::istreambuf_iterator < char > {}};

                    if (message.find("login", 0) != (unsigned int) (-1))
                        login(client);
                    else if (message.find("clients", 0) != (unsigned int) (-1))
                        getClients(client);
                    else
                        std::cout << "invalid msg: " << message << std::endl;
                    buffer.consume(buffer.size());

                    std::this_thread::sleep_for(std::chrono_literals
                                                ::operator ""ms(1));
                    client->setTime(std::move(time(NULL)));
                    std::this_thread::sleep_for(std::chrono_literals
                                                ::operator ""ms(1));
                    ping(client);
                    if (client->timed_out()) client->close();
                } catch (std::runtime_error &exception) {
                    client->close();
                    BOOST_LOG_TRIVIAL(debug)
                            << "Client dissconected: "
                            << client->getSocket().
                                    remote_endpoint().address().to_string()
                            << " Port: " << client->getSocket()
                                    .remote_endpoint().port()
                            << std::endl;
                }
            }
            setClientsStatus(false);
            for (auto it = _clients.begin(); it != _clients.end();) {
                if ((*it)->isClose()) {
                    _clients.erase(it);
                    setClientsStatus(true);
                    BOOST_LOG_TRIVIAL(info) << "Client changed: "
                                            << std::endl;
                } else {
                    it++;
                }
            }
        }
        mutexUnlock();
        std::this_thread::sleep_for(std::chrono_literals::operator ""ms(1));
    }
}

void server::mutexUnlock() {
    _mutex.lock();
}

void server::mutexLock() {
    _mutex.lock();
}

void server::setClientsStatus(bool status) {
    _clients_changed = status;
}

void server::startServer() {
    boost::thread th1(&server::acceptThread, this);
    boost::thread th2(&server::handleClientsThread, this);
    th1.join();
    th2.join();
}

void server::login(std::shared_ptr <client> &client) {
    boost::asio::streambuf buffer{};
    read_until(client->getSocket(), buffer, "\n");
    std::string message{std::istreambuf_iterator < char > {&buffer},
                        std::istreambuf_iterator < char > {}};
    client->setName(message);
    buffer.consume(buffer.size());
    std::ostream out(&buffer);
    out << "login ok\n";
    write(client->getSocket(), buffer);
    buffer.consume(buffer.size());
}

void server::ping(std::shared_ptr <client> &client) {
    boost::asio::streambuf buffer{};
    read_until(client->getSocket(), buffer, "\n");
    std::string message{std::istreambuf_iterator < char > {&buffer},
                        std::istreambuf_iterator < char > {}};
    std::cout << message << std::endl;
    buffer.consume(buffer.size());
    std::ostream out(&buffer);
    out << (_clients_changed ? "ping client_list_changed\n" : "ping ok\n");
    write(client->getSocket(), buffer);
    buffer.consume(buffer.size());
}

void server::getClients(std::shared_ptr <client> &current_client) {
    boost::asio::streambuf buffer{};
    std::ostream out(&buffer);
    for (auto &client : _clients) {
        out << "Adress: "
            << client->getSocket().remote_endpoint().address().to_string()
            << " Port: " << client->getSocket().remote_endpoint().port()
            << " Name: " << client->getName() << std::endl;
    }
    write(current_client->getSocket(), buffer);
    buffer.consume(buffer.size());
    _clients_changed = false;
}

namespace logging = boost::log;
namespace src = boost::log::sources;
namespace expr = boost::log::expressions;
namespace keywords = boost::log::keywords;
namespace sinks = boost::log::sinks;

void server::initLog() {
    logging::add_file_log
            (
                    keywords::file_name = "/logs/info.log",
                    keywords::rotation_size = 256 * 1024 * 1024,
                    keywords::time_based_rotation =
                            sinks::file::rotation_at_time_point(0, 0, 0),
                    keywords::filter =
                            logging::trivial::severity
                            >= logging::trivial::info,
                    keywords::format =
                            (
                                    expr::stream
                                            << boost::posix_time
                                            ::second_clock::local_time()
                                            << " : <" << logging
                                            ::trivial::severity
                                            << "> " << expr::smessage));

    logging::add_file_log
            (
                    keywords::file_name = "/logs/trace.log",
                    keywords::rotation_size = 256 * 1024 * 1024,
                    keywords::time_based_rotation = sinks::file
                    ::rotation_at_time_point(0, 0, 0),
                    keywords::filter = logging::trivial::severity
                                       >= logging::trivial::trace,
                    keywords::format =
                            (
                                    expr::stream
                                            << boost::posix_time
                                            ::second_clock::local_time()
                                            << " : <" << logging::
                                            trivial::severity
                                            << "> " << expr::smessage));
    logging::add_file_log
            (
                    keywords::file_name = "/logs/debug.log",
                    keywords::rotation_size = 256 * 1024 * 1024,
                    keywords::time_based_rotation = sinks::file
                    ::rotation_at_time_point(0, 0, 0),
                    keywords::filter = logging::trivial::severity
                                       >= logging::trivial::debug,
                    keywords::format =
                            (
                                    expr::stream
                                            << boost::posix_time
                                            ::second_clock::local_time()
                                            << " : <" << logging::
                                            trivial::severity
                                            << "> " << expr::smessage));
}
