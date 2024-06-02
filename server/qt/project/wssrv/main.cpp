#include <QCoreApplication>
#include <global.hpp>
#include <crypt/cryptography.hpp>
#include <cl/command_line.hpp>
#include "include/server_conf.hpp"
#include <variant/item_data.h>
#include <QDebug>
#include <QUuid>

#include "include/net.hpp"
#include "include/beast.hpp"
#include "include/common/server_certificate.hpp"
#include "include/shared_state.hpp"
#include "include/listener.hpp"

using namespace arcirk::strings;
using namespace arcirk;

void read_command_line(const command_line_parser::line_parser& parser, server::server_config& conf){

    using namespace arcirk::cryptography;

    if(parser.option_exists("-h")){
        conf.ServerHost = parser.option("-h");
    }
    if(parser.option_exists("-p")){
        const std::string &_port = parser.option("-p");
        if(!_port.empty())
            conf.ServerPort = static_cast<unsigned short>(strtol(_port.c_str(), NULL, 0));
    }
    if(parser.option_exists("-t")){
        const std::string &_threads = parser.option("-t");
        if(!_threads.empty())
            conf.ThreadsCount =  std::max<int>(1, static_cast<unsigned short>(strtol(_threads.c_str(), NULL, 0)));
    }
    if(parser.option_exists("-usr")){
        conf.ServerUser = parser.option("-usr");
    }
    if(parser.option_exists("-pwd")){
        conf.ServerUserHash = get_hash(conf.ServerUser, parser.option("-pwd"));
    }

}

void load_certs(boost::asio::ssl::context& ctx, const QString& cert, const QString& key){

    using namespace arcirk::filesystem;
    using namespace arcirk::verify_application;

    FSPath ssl_dir(working_directory());
    ssl_dir /= ARCIRK_VERSION;
    ssl_dir /= "ssl";
    ssl_dir.mkpath();

    auto cert_file = ssl_dir.to_file(cert);
    auto key_file = ssl_dir.to_file(key);

    if(!cert_file.exists() || !key_file.exists())
        return;

    std::string _cert;
    std::ifstream c_in(cert_file.fileName().toStdString());
    std::ostringstream c_oss;
    c_oss << c_in.rdbuf();
    _cert = c_oss.str();

    std::string _key;
    std::ifstream k_in(key_file.fileName().toStdString());
    std::ostringstream k_oss;
    k_oss << k_in.rdbuf();
    _key = k_oss.str();

    if(_cert.empty() || _key.empty()){
        fail("error", "error read certificate files", __FUNCTION__, true);
        return;
    }

    load_server_certificate(ctx, _cert, _key);
}

int main(int argc, char *argv[])
{

    using namespace arcirk::cryptography;

    QCoreApplication a(argc, argv);

    log("start server", "Проверка структуры каталогов", __FUNCTION__, true);

    auto conf = arcirk::server::server_config();
    conf.ServerUser = "admin";
    conf.ServerUserHash = get_hash("admin", "admin");
    conf.ServerHost = "127.0.0.1";
    conf.ServerPort = 8080;

    auto result = verify_application::verify_directorias<arcirk::server::server_config>(conf, ARCIRK_SERVER_CONF, ARCIRK_VERSION);
    if(!result){
        fail("Ошибка", "Ошибка проверки структуры каталогов", __FUNCTION__, true);
        return a.exec();
    }

    command_line_parser::line_parser parser(argc, argv);
    read_command_line(parser, conf);

    using namespace arcirk::verify_application;
    using namespace arcirk::filesystem;

    FSPath dir(working_directory());
    dir /= ARCIRK_VERSION;
    auto file = dir.to_file(ARCIRK_SERVER_CONF);
    if(!file.exists()){
        conf.ref = to_byte(to_binary(QUuid::createUuid()));
        conf.ThreadsCount = 4;
        auto dump = pre::json::to_json(conf).dump(4);
        if(file.open(QIODevice::WriteOnly)){
            file.write(dump.c_str());
            file.close();
        }
    }

    auto const address = net::ip::make_address(conf.ServerHost);
    auto const port = static_cast<unsigned short>(conf.ServerPort);
    auto const doc_root = std::make_shared<std::string>(conf.ServerHttpRoot);
    auto const threads = std::max<int>(1, conf.ThreadsCount);

    // The io_context is required for all I/O
    net::io_context ioc{threads};

    // The SSL context is required, and holds certificates
    ssl::context ctx{ssl::context::tlsv12};

    // This holds the self-signed certificate used by the server
    if(!conf.SSL_crt_file.empty() && !conf.SSL_key_file.empty() && !parser.option_exists("-ssl_def")){
        load_certs(ctx, conf.SSL_crt_file.c_str(), conf.SSL_key_file.c_str());
    }else
        load_server_default_certificate(ctx);

    // Create and launch a listening port
    std::make_shared<listener>(
        ioc,
        ctx,
        tcp::endpoint{address, port},
        doc_root,
        boost::make_shared<arcirk::shared_state>())->run();

    // Capture SIGINT and SIGTERM to perform a clean shutdown
    net::signal_set boost_signals(ioc, SIGINT, SIGTERM);
    boost_signals.async_wait(
        [&](beast::error_code const&, int)
        {
            // Stop the `io_context`. This will cause `run()`
            // to return immediately, eventually destroying the
            // `io_context` and all of the sockets in it.
            ioc.stop();
        });

    // Run the I/O service on the requested number of threads
    std::vector<std::thread> v;
    v.reserve(threads - 1);
    for(auto i = threads - 1; i > 0; --i)
        v.emplace_back(
            [&ioc]
            {
                ioc.run();
            });
    ioc.run();

    // (If we get here, it means we got a SIGINT or SIGTERM)

    // Block until all the threads exit
    for(auto& t : v)
        t.join();

    return a.exec();
}
