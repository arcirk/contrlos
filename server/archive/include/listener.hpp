//
// Created by arcady on 05.07.2021.
//

#ifndef SERVER_LISTENER_HPP
#define SERVER_LISTENER_HPP

#include "beast.hpp"
#include "net.hpp"
#include <boost/smart_ptr.hpp>
#include <memory>
#include <string>

// Forward declaration
class shared_state;

//class channel;

// Accepts incoming connections and launches the sessions
class listener : public boost::enable_shared_from_this<listener>
{
    net::io_context& ioc_;
    tcp::acceptor acceptor_;
    boost::shared_ptr<shared_state> state_;
    //boost::shared_ptr<channel> channel_; //Общий канал

    void fail(beast::error_code ec, char const* what);
    void on_accept(beast::error_code ec, tcp::socket socket);

public:
    listener(
            net::io_context& ioc,
            tcp::endpoint endpoint,
            boost::shared_ptr<shared_state> const& state);//,
            //boost::shared_ptr<channel> const& ch);

    // Start accepting incoming connections
    void run();
};

#endif //UNTITLED1_LISTENER_HPP
