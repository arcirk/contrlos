
#include "../include/websocket_session.hpp"
#include <iostream>
#include <utility>

websocket_session::
websocket_session(tcp::socket&& socket, boost::shared_ptr<shared_state>  state)
: ws_(std::move(socket))
, state_(std::move(state))
, deadline_(socket.get_executor())
{
    boost::beast::error_code ec;
    auto remote = ws_.next_layer().socket().remote_endpoint(ec);
    _ip_address = remote.address().to_string();
    port = std::to_string(remote.port());
    uuid_ = boost::uuids::random_generator()();
    join_channel(this);

    last_error = 0;

}

boost::uuids::uuid & websocket_session::get_uuid() {
    return uuid_;
}

void websocket_session::deliver(const boost::shared_ptr<const std::string> &msg) {
    //
    send(msg);
};

websocket_session::
~websocket_session()
{
    // Remove this session from the list of active sessions
    state_->leave(this);

}

void
websocket_session::
fail(beast::error_code ec, char const* what)
{
    // Don't report these
    if( ec == net::error::operation_aborted ||
        ec == websocket::error::closed)
        return;

    last_error = ec.value();

    if(ec.value() == 10054){
        deadline_cancel();
    }

    std::cerr << what << " code: " << ec.value() << " " << ec.message() << std::endl;
}

void
websocket_session::
on_accept(beast::error_code ec)
{
    // Handle the error, if any
    if(ec)
        return fail(ec, "accept");

    // Add this session to the list of active sessions
    state_->join(this);

    std::cout << "connect client" << std::endl;

    //Установка крайнего срока для авторизации
    deadline_.expires_after(std::chrono::seconds(60));
    deadline_.async_wait(boost::bind(&websocket_session::check_deadline, this));

    // Read a message
    ws_.async_read(
            buffer_,
            beast::bind_front_handler(
                    &websocket_session::on_read,
                    shared_from_this()));
}

void
websocket_session::
on_read(beast::error_code ec, std::size_t)
{
    if (ec == boost::asio::error::eof){
        std::cerr << "websocket_session::on_read: " << "boost::asio::error::eof" << std::endl;
        return;
    }
//    if (ec.value() == 2){
//        std::cerr << "websocket_session::on_read: " << "End of file" << std::endl;
//        return;
//    }

//    if( ec == net::error::operation_aborted ||
//        ec == websocket::error::closed)
//            return;

    // Handle the error, if any
    if(ec){
        return fail(ec, "read");
    }

//    // Send to all connections
//    state_->send(beast::buffers_to_string(buffer_.data()), uuid_);

    std::string msg = beast::buffers_to_string(buffer_.data());

    //отправляем только подписчикам на канал
    state_->deliver(msg, this);

    // Clear the buffer
    buffer_.consume(buffer_.size());

    // Read another message
    ws_.async_read(
            buffer_,
            beast::bind_front_handler(
                    &websocket_session::on_read,
                    shared_from_this()));
}

void
websocket_session::
send(boost::shared_ptr<std::string const> const& ss)
{
    // Post our work to the strand, this ensures
    // that the members of `this` will not be
    // accessed concurrently.

    // Размещаем нашу работу в цепочке, это гарантирует
    // что члены `this` не будут
    // доступ одновременно.


    net::post(
            ws_.get_executor(),
            beast::bind_front_handler(
                    &websocket_session::on_send,
                    shared_from_this(),
                    ss));
}

//void websocket_session::connect_channel(int channel_id)
//{
//
//}

void
websocket_session::
on_send(boost::shared_ptr<std::string const> const& ss)
{
    // Always add to queue
    queue_.push_back(ss);

    // Are we already writing?
    if(queue_.size() > 1)
        return;

    // We are not currently writing, so deliver this immediately
    ws_.async_write(
            net::buffer(*queue_.front()),
            beast::bind_front_handler(
                    &websocket_session::on_write,
                    shared_from_this()));
}

void
websocket_session::
on_write(beast::error_code ec, std::size_t)
{
    // Handle the error, if any
    if(ec)
        return fail(ec, "write");

    // Remove the string from the queue
    queue_.erase(queue_.begin());

    // Send the next message if any
    if(! queue_.empty())
        ws_.async_write(
                net::buffer(*queue_.front()),
                beast::bind_front_handler(
                        &websocket_session::on_write,
                        shared_from_this()));
}

std::string websocket_session::ip_address() const{

    return _ip_address;
}

std::string websocket_session::host_name() const
{
    return _host_name;
}

void websocket_session::set_host_name(const std::string &value)
{
    _host_name = value;
}

void
websocket_session::
join_channel(websocket_session *subscriberPtr) {
    boost::uuids::uuid uuid = subscriberPtr->get_uuid();
    auto iter = subscribers_.find(uuid);
    if (iter == subscribers_.end() ){
        subscribers_.insert(std::pair<boost::uuids::uuid, websocket_session*>(uuid, subscriberPtr));
    }
}
void
websocket_session::
close_channel(boost::uuids::uuid& uuid){
    auto iter = subscribers_.find(uuid);
    if (iter != subscribers_.end() )
        subscribers_.erase(iter);
}

std::map<boost::uuids::uuid, websocket_session *>*
websocket_session::
get_subscribers() {
    return &subscribers_;
}

void
websocket_session::throw_authorized() {
    std::string msg = QString("Отказано в доступе!").toLocal8Bit().toStdString();
    if (!this->authorized)
        boost::throw_exception( std::out_of_range( msg ), BOOST_CURRENT_LOCATION );
}

void
websocket_session::close() {

    deadline_cancel();

    ws_.async_close(websocket::close_code::normal,
                beast::bind_front_handler(
                        &websocket_session::on_close,
                        shared_from_this()));
}
void
websocket_session::on_close(beast::error_code ec)
{
    if(ec)
        return fail(ec, "close");

    // If we get here then the connection is closed gracefully

    // The make_printable() function helps print a ConstBufferSequence
    std::cout << beast::make_printable(buffer_.data()) << std::endl;
}
bool websocket_session::stopped() const
{

    bool result = false;
    try {
        result  = !ws_.next_layer().socket().is_open();
    }  catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
    }

    return result;

}

void
websocket_session::deadline_cancel(){
    deadline_.cancel();
}

void
websocket_session::check_deadline()
{
    if(this->last_error < 0)
        return;

    if (this->authorized){
        deadline_.cancel();
    }else{
        if (deadline_.expiry() <= steady_timer::clock_type::now())
        {
            std::cerr << QString("Превышено время на авторизацию! Отключение клиента...").toLocal8Bit().toStdString() << std::endl;
            ws_.async_close(websocket::close_code::normal,
                            beast::bind_front_handler(
                                    &websocket_session::on_close,
                                    shared_from_this()));

            deadline_.expires_at((steady_timer::time_point::max)());
            deadline_.cancel();
            return;
        }
        // Put the actor back to sleep.
        deadline_.async_wait(std::bind(&websocket_session::check_deadline, this));
    }

}
