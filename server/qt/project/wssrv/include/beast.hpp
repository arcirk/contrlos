//
// Created by arcady on 05.07.2021.
//

#ifndef SERVER_BEAST_HPP
#define SERVER_BEAST_HPP

#ifdef _WINDOWS
#pragma warning (disable: 4001)
#pragma warning (disable: 4101)
#pragma warning (disable: 4061)
#endif // _WINDOWS

#include <boost/beast.hpp>
#include <boost/beast/ssl.hpp>

namespace beast = boost::beast;                 // from <boost/beast.hpp>
namespace http = beast::http;                   // from <boost/beast/http.hpp>
namespace websocket = beast::websocket;         // from <boost/beast/websocket.hpp>
namespace ssl = boost::asio::ssl;

#endif //UNTITLED1_BEAST_HPP
