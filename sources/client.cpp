// Copyright 2018 Your Name <your_email>

#include <client.hpp>

#define KICK_FROM_SERVER  -100

void client::connect(const ip::tcp::endpoint &ep) {
    _socket.connect(ep);
}

void client::close() {
    _socket.close();
}

Socket &client::getSocket() {
    return _socket;
}

void client::answerFromServer() {
    boost::asio::streambuf buffer{};
    std::ostream out(&buffer);
    while (true) {
        try {
            time_t seconds = time(NULL);
            std::cout << time(&seconds);
            std::cout << "Enter msg: ";
            std::string word;
            std::cin >> word;
            word += "\r\n\r\n";
            out << word;
            write(this->getSocket(), buffer);
            if (word.find("login", 0) != -1)
                login();
            else if (word.find("clients", 0) != -1)
                getClients();
            else 
                std::cout << "invalid msg: " << word << std::endl;
            buffer.consume(buffer.size());
			static unsigned int rand = time(nullptr);
            boost::this_thread::sleep(boost::posix_time::millisec(rand_r(&rand)%7000));
            ping();
        } catch (std::runtime_error &exception) {
            this->stop();
            exit(KICK_FROM_SERVER);
        }
    }
}


void client::login() {
    boost::asio::streambuf buffer{};
    std::ostream out(&buffer);
    std::string name;
    std::cin >> name;
    name += "\r\n\r\n";
    out << name;
    this->setName(name);
    write(this->getSocket(), buffer);
    buffer.consume(buffer.size());
    read_until(this->getSocket(), buffer, "\n");
    std::string message{std::istreambuf_iterator<char>{&buffer},
                        std::istreambuf_iterator<char>{}};
    std::cout << message << std::endl;
    buffer.consume(buffer.size());
}

void client::ping() {
    boost::asio::streambuf buffer{};
    std::ostream out(&buffer);
    out << "Ping Ok!\n";
    write(this->getSocket(), buffer);
    buffer.consume(buffer.size());
    read_until(this->getSocket(), buffer, "\n");
    std::string message{std::istreambuf_iterator<char>{&buffer},
                        std::istreambuf_iterator<char>{}};
    std::cout << message << std::endl;
    buffer.consume(buffer.size());
}

void client::getClients() {
    boost::asio::streambuf buffer{};
    read_until(this->getSocket(), buffer, "\n");
    std::string message{std::istreambuf_iterator<char>{&buffer},
                        std::istreambuf_iterator<char>{}};
    std::cout << message << std::endl;
    buffer.consume(buffer.size());
}

[[nodiscard]] bool client::isClose() {
    return !this->getSocket().is_open();
}

void client::stop() {
    boost::system::error_code err;
    _socket.close(err);
}

bool client::timed_out() const {
    time_t now = time(NULL);
    return (now - _lastTime) > 5000;
}

std::string client::getName() {
    return _username;
}
