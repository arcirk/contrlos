//
// Created by arcady on 05.07.2021.
//

#ifndef SERVER_WEBSOCKET_SESSION_HPP
#define SERVER_WEBSOCKET_SESSION_HPP
#include "net.hpp"
#include "beast.hpp"
#include "shared_state.hpp"
#include "subscriber.h"

#include <cstdlib>
#include <memory>
#include <string>
#include <vector>
#include <boost/asio/steady_timer.hpp>


using boost::asio::steady_timer;
// Forward declaration
class shared_state;

/** Represents an active WebSocket connection to the server
*/
class websocket_session : public subscriber, public boost::enable_shared_from_this<websocket_session>
{
    beast::flat_buffer buffer_;
    websocket::stream<beast::tcp_stream> ws_;
    boost::shared_ptr<shared_state> state_;
    std::vector<boost::shared_ptr<std::string const>> queue_;

    std::map<boost::uuids::uuid, websocket_session*> subscribers_;

    void fail(beast::error_code ec, char const* what);
    void on_accept(beast::error_code ec);
    void on_read(beast::error_code ec, std::size_t bytes_transferred);
    void on_write(beast::error_code ec, std::size_t bytes_transferred);

    int last_error;
public:
//    websocket_session(
//            tcp::socket&& socket,
//            boost::shared_ptr<shared_state>  state, boost::shared_ptr<channel> ch);
//
    websocket_session(
            tcp::socket&& socket,
            boost::shared_ptr<shared_state> state);

    ~websocket_session();

    template<class Body, class Allocator>
    void
    run(http::request<Body, http::basic_fields<Allocator>> req);

    // Send a message
    void
    send(boost::shared_ptr<std::string const> const& ss);

    //boost::shared_ptr<channel> & get_channel();

    //void set_channel_param(const std::string& base64_);

    //std::string get_channel_param();

    boost::uuids::uuid& get_uuid() override;

    void deliver(const boost::shared_ptr<const std::string> &msg) override;

    void join_channel(websocket_session* subscriberPtr);

    std::map<boost::uuids::uuid, websocket_session*>* get_subscribers();

    void throw_authorized();

    void close();

    void close_channel(boost::uuids::uuid& uuid);

    void set_agent_current_client(websocket_session* sess);

    void set_agent(websocket_session* sess);

    void deadline_cancel();

    std::string ip_address() const;
    std::string host_name() const;
    void set_host_name(const std::string& value);

private:

    std::string _ip_address = "0.0.0.0";
    std::string port = "0";
    steady_timer deadline_;
    std::string _host_name;

    void
    on_send(boost::shared_ptr<std::string const> const& ss);
    void
    on_close(beast::error_code ec);

    void check_deadline();

    bool stopped() const;

    websocket_session* agent_current_client;
    websocket_session* agent_session;
};

template<class Body, class Allocator>
void
websocket_session::
run(http::request<Body, http::basic_fields<Allocator>> req)
{
    // Set suggested timeout settings for the websocket
    ws_.set_option(
            websocket::stream_base::timeout::suggested(
                    beast::role_type::server));

    // Set a decorator to change the Server of the handshake
    ws_.set_option(websocket::stream_base::decorator(
            [](websocket::response_type& res)
            {
                res.set(http::field::server,
                        std::string(BOOST_BEAST_VERSION_STRING) +
                        " websocket-chat-multi");
            }));

    // Accept the websocket handshake
    ws_.async_accept(
            req,
            beast::bind_front_handler(
                    &websocket_session::on_accept,
                    shared_from_this()));
}
#endif //UNTITLED1_WEBSOCKET_SESSION_HPP
