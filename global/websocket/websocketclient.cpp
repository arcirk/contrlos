#include "websocketclient.h"
#include <QStandardPaths>
#include <QDir>
#include <QFile>
#include <QUrl>
#include <QEventLoop>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QCryptographicHash>
#include <QIODevice>
#include "../variant/item_data.h"

using namespace arcirk;

static bool field_is_exists(const nlohmann::json &object, const std::string &name) {
    auto itr = object.find(name);
    return itr != object.end();
}

WebSocketClient::WebSocketClient(QObject *parent)
    : QObject{parent}
{
    m_isConnected = false;

    read_conf();
    initialize();

}

WebSocketClient::WebSocketClient(const QUrl &url, QObject *parent)
: QObject{parent}
{
    Q_UNUSED(url);
    m_isConnected = false;
    read_conf();
    initialize();
}

WebSocketClient::~WebSocketClient()
{

}

QString WebSocketClient::generateHash(const QString &usr, const QString &pwd)
{
    return get_hash(usr, pwd);
}

bool WebSocketClient::isStarted()
{
    qDebug() << __FUNCTION__ << m_client->state();
    return m_client->state() == QAbstractSocket::SocketState::ConnectedState;
}

QString WebSocketClient::get_sha1(const QByteArray& p_arg){
    auto sha = QCryptographicHash::hash(p_arg, QCryptographicHash::Sha1);
    return sha.toHex();
}

void WebSocketClient::get_server_configuration_sync()
{
    try {
        auto result_http = exec_http_query(arcirk::enum_synonym(arcirk::server::server_commands::ServerConfiguration), nlohmann::json{});
        server_conf_ = pre::json::from_json<arcirk::server::server_config>(result_http);
    } catch (const std::exception& e) {
        qCritical() << e.what();
    }

}

arcirk::client::client_param WebSocketClient::parse_client_param(const std::string &response)
{
    arcirk::client::client_param result;

    try {
        result = pre::json::from_json<arcirk::client::client_param>(json::parse(response));
    } catch (...) {
        auto tmp_struct = json::parse(response);
        auto tmp = pre::json::to_json(arcirk::client::client_param());
        auto items = tmp_struct.items();
        for (auto it = items.begin(); it != items.end() ; ++it) {
            if(tmp.find(it.key()) != tmp.end())
                tmp[it.key()] = it.value();
        }
        result = pre::json::from_json<arcirk::client::client_param>(tmp);
        result.version = CLIENT_VERSION;

    }

    return result;
}

std::string WebSocketClient::crypt(const QString &source, const QString &key)
{
    if(source.isEmpty())
        return "";
    return arcirk::crypt(source.toStdString(), key.toStdString());
}

QString WebSocketClient::get_hash(const QString& first, const QString& second){
    QString _usr = first.trimmed();
    _usr = _usr.toUpper();
    return get_sha1(QString(_usr + second).toUtf8());
}

client::client_conf &WebSocketClient::conf()
{
    return conf_;
}

server::server_config &WebSocketClient::server_conf()
{
    return server_conf_;
}

client::client_param &WebSocketClient::client_server_param()
{
    return client_param_;
}

bool WebSocketClient::isConnected()
{
    return m_isConnected;
}

QUrl WebSocketClient::url() const
{
    QString scheme = server_conf_.ServerSSL ? "wss" : "ws";
    QUrl url_{};
    url_.setHost(server_conf_.ServerHost.c_str());
    url_.setPort(server_conf_.ServerPort);
    url_.setScheme(scheme);

    return url_;
}

void WebSocketClient::setUrl(const QUrl &url)
{
    server_conf_.ServerSSL = url.scheme() == "wss";
    server_conf_.ServerHost = url.host().toStdString();
    server_conf_.ServerPort = url.port();

}


QUrl WebSocketClient::http_url() const
{
    QString scheme = server_conf_.ServerSSL ? "https" : "http";
    QUrl url_{};
    url_.setHost(server_conf_.ServerHost.c_str());
    url_.setPort(server_conf_.ServerPort);
    url_.setScheme(scheme);

    return url_;
}

void WebSocketClient::open(const std::string& sid)
{
    m_sid = sid;

    if(conf_.server_host.empty())
        return;

    if(isConnected())
        return;

    QUrl _url(conf_.server_host.data());
    server_conf_.ServerHost = _url.host().toStdString();
    server_conf_.ServerPort = _url.port();
    server_conf_.ServerSSL = _url.scheme() == "wss" ? true : false;
    m_client->open(_url);

}

void WebSocketClient::close()
{
    if(isConnected())
        m_client->close();
}

void WebSocketClient::read_conf(const QString& app_name)
{
    auto path = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir dir(path);

    if(!dir.exists())
        dir.mkpath(path);

    auto fileName= path + "/" + CONF_FILENAME;
    if(QFile::exists(fileName)){

        QFile f(fileName);
        f.open(QIODevice::ReadOnly);

        std::string m_text = f.readAll().toStdString();
        f.close();

        if(json::accept(m_text)){
            conf_ = secure_serialization<client::client_conf>(m_text);
        }else
            conf_ = client::client_conf();
    }else{
        conf_ = client::client_conf();
    }

#ifndef QML_APPLICATION
    conf_.app_name = app_name.toStdString(); //"ServerManager";
#else
    conf_.app_name = "PriceChecker";
#endif
#ifndef Q_OS_ANDROID
    auto m_uuid = QSysInfo::machineUniqueId();
    if(!m_uuid.isEmpty()){
        conf_.device_id = to_byte(to_binary(qbyte_to_byte(m_uuid), variant_subtype::subtypeByte));
    }
#endif
    write_conf();

}

void WebSocketClient::write_conf()
{
    try {
        std::string result = pre::json::to_json(conf_).dump() ;
        auto path = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
        auto fileName= path + "/" + CONF_FILENAME;
        QFile f(fileName);
        qDebug() << fileName;
        f.open(QIODevice::WriteOnly);
        f.write(QByteArray::fromStdString(result));
        f.close();
    } catch (std::exception& e) {
        qCritical() << __FUNCTION__ << e.what();
    }
}

void WebSocketClient::initialize()
{
    m_client = new QWebSocket();
    connect(m_client, &QWebSocket::connected, this, &WebSocketClient::onConnected);
    connect(m_client, &QWebSocket::disconnected, this, &WebSocketClient::onDisconnected);
    connect(m_client, &QWebSocket::textMessageReceived, this, &WebSocketClient::onTextMessageReceived);

    connect(m_client, QOverload<QAbstractSocket::SocketError>::of(&QWebSocket::error),
            [=](QAbstractSocket::SocketError error){
        onError(error, m_client->errorString());
    });
}

void WebSocketClient::parse_response(const QString &resp)
{
    //qDebug() << __FUNCTION__;
    try {
        //std::string resp_ = QByteArray::fromBase64(resp.toUtf8()).toStdString();
        auto msg = pre::json::from_json<arcirk::server::server_response>(resp.toStdString());
        qDebug() << QDateTime::currentDateTime().toString("hh:mm:ss") << __FUNCTION__ << QString::fromStdString(msg.command);
        if(msg.command == arcirk::enum_synonym(arcirk::server::server_commands::SetClientParam)){
            if(msg.message == "OK"){
                get_server_configuration_sync();
                QString result = QByteArray::fromBase64(msg.param.data());
                QString usr_info = QByteArray::fromBase64(msg.result.data());
                client_param_ = parse_client_param(result.toStdString());
                auto user_info_ = json::parse(usr_info.toStdString());
                server_conf_.ServerUserHash = user_info_["hash"].get<std::string>();
                if(!client_param_.session_uuid.empty()){
                    m_currentSession = to_qt_uuid(client_param_.session_uuid);
                }
                if(!client_param_.user_uuid.empty()){
                    m_currentUserUuid = to_qt_uuid(client_param_.user_uuid);
                }
                doConnectionSuccess();
                doConnectionChanged(true);

                emit userInfo(user_info_);
            }else{
                doDisplayError("SetClientParam", "Ошибка авторизации");
            }
        }else if(msg.command == arcirk::enum_synonym(arcirk::server::server_commands::HttpServiceConfiguration)){
            //if(msg.result != "error")
                //update_server_configuration("httpService", msg.result);
        }else if(msg.command == arcirk::enum_synonym(arcirk::server::server_commands::WebDavServiceConfiguration)){
            //if(msg.result != "error")
                //update_server_configuration("davService", msg.result);
        }else if(msg.command == arcirk::enum_synonym(arcirk::server::server_commands::ExecuteSqlQuery)){
            emit serverResponse(msg);
        }else if(msg.command == arcirk::enum_synonym(arcirk::server::server_commands::SyncGetDiscrepancyInData)){
            emit syncGetDiscrepancyInData(msg);
        }
        else if(msg.command == arcirk::enum_synonym(arcirk::server::server_commands::SyncUpdateBarcode)){
            emit updateBarcodeInformation(msg);
        }
        else if(msg.command == arcirk::enum_synonym(arcirk::server::server_commands::ServerOnlineClientsList)){
            emit serverResponse(msg);
        }
        else if(msg.command == arcirk::enum_synonym(arcirk::server::server_commands::ServerConfiguration)){
            emit serverResponse(msg);
        }
        else if(msg.command == arcirk::enum_synonym(arcirk::server::server_commands::GetDatabaseTables)){
            emit serverResponse(msg);
        }else if(msg.command == "notify"){
            emit notify(msg.message.c_str());
        }else if(msg.command == arcirk::enum_synonym(arcirk::server::server_commands::CommandToClient)){
            emit commandToClientResponse(msg);
        }else if(msg.command == arcirk::enum_synonym(arcirk::server::server_commands::UserMessage)){
            emit userMessage(msg);
        }else
            emit serverResponse(msg);
    } catch (std::exception& e) {
        qCritical() << QDateTime::currentDateTime().toString("hh:mm:ss") << __FUNCTION__ << e.what() << '\n' << qPrintable(resp);
        doDisplayError(__FUNCTION__, e.what());
    }
}

void WebSocketClient::doDisplayError(const QString &what, const QString &err)
{
    qDebug() << QDateTime::currentDateTime().toString("hh:mm:ss") << __FUNCTION__;
    emit displayError(what, err);
}

void WebSocketClient::doConnectionSuccess()
{
    qDebug() << QDateTime::currentDateTime().toString("hh:mm:ss") << __FUNCTION__;
    emit connectionSuccess();
}

void WebSocketClient::doConnectionChanged(bool state)
{
    qDebug() << QDateTime::currentDateTime().toString("hh:mm:ss") << __FUNCTION__ << state;
    m_isConnected = state;
    emit connectionChanged(state);
}

nlohmann::json WebSocketClient::exec_http_query(const std::string &command, const nlohmann::json &param, const ByteArray& data, bool returnAllMessage)
{

    QEventLoop loop;
    int httpStatus = 200;
    QByteArray httpData;
    QNetworkAccessManager httpService;

    auto finished = [&loop, &httpStatus, &httpData](QNetworkReply* reply) -> void
    {
       QVariant status_code = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
       if(status_code.isValid()){
           httpStatus = status_code.toInt();
           if(httpStatus != 200){
                qCritical() << QDateTime::currentDateTime().toString("hh:mm:ss") << __FUNCTION__ << "Error: " << httpStatus << " " + reply->errorString() ;
           }else
           {
               httpData = reply->readAll();
           }
       }
       loop.quit();

    };

    loop.connect(&httpService, &QNetworkAccessManager::finished, finished);

    QUrl ws(conf_.server_host.data());
    QString protocol = ws.scheme() == "wss" ? "https://" : "http://";
    QString http_query = "/api/info";
    QUrl url(protocol + ws.host() + ":" + QString::number(ws.port()) + http_query);
    QNetworkRequest request(url);
    //request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QString headerData = "Token " + QByteArray(conf_.hash.data()).toBase64();;
    request.setRawHeader("Authorization", headerData.toLocal8Bit());

    if(data.size() > 0){
        //ByteArray bt = param["data"];
        QStringList contentDisposition{"form-data"};
        auto items = param.items();
        for (auto itr = items.begin(); itr != items.end(); ++itr) {
            auto val = *itr;
            if(val.key() != "data"){
                contentDisposition.append(QString("%1=\"%2\"").arg(val.key().c_str(), val.value().get<std::string>().c_str()));
            }
        }
        request.setRawHeader("Content-Disposition", contentDisposition.join(";").toLocal8Bit());
        auto* q_data = new QByteArray(reinterpret_cast<const char*>(data.data()), data.size());
        request.setHeader(QNetworkRequest::ContentTypeHeader, "multipart/form-data");
        httpService.post(request, *q_data);
    }else{
        auto http_param = arcirk::https::http_param();
        http_param.command = command;
        http_param.param = QByteArray(param.dump().data()).toBase64().toStdString();
        request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
        httpService.post(request, QByteArray::fromStdString(pre::json::to_json(http_param).dump()));
    }
    loop.exec();

    if(httpStatus != 200){
        return WS_RESULT_ERROR;
    }

    if(httpData.isEmpty())
        return WS_RESULT_ERROR;

    if(httpData == WS_RESULT_ERROR){
         return WS_RESULT_ERROR;
    }

    if(returnAllMessage){
        if(json::accept(httpData.toStdString())){
            return json::parse(httpData.toStdString());
        }else
            return WS_RESULT_ERROR;
    }

    auto msg = pre::json::from_json<arcirk::server::server_response>(httpData.toStdString());

    if(msg.result.empty())
        return {};
    try {
        if(msg.result != WS_RESULT_ERROR){
            if(msg.result != WS_RESULT_SUCCESS){
                std::string text = QByteArray::fromBase64(msg.result.data()).toStdString();
                if(json::accept(text)){
                    auto http_result = nlohmann::json::parse(QByteArray::fromBase64(msg.result.data()).toStdString());
                    return http_result;
                }else
                    return text;
            }else
                return WS_RESULT_SUCCESS;
        }else{
            emit error(__FUNCTION__, msg.command.c_str(), msg.message.c_str());
            return WS_RESULT_ERROR;
        }
    } catch (const std::exception& e) {
        emit error(__FUNCTION__, msg.command.c_str(), e.what());
        return WS_RESULT_ERROR;
    }

}

nlohmann::json WebSocketClient::http_query(const QUrl &ws, const QString &token, const std::string &command, const nlohmann::json &param, const ByteArray &data)
{
    QEventLoop loop;
    int httpStatus = 200;
    QByteArray httpData;
    QNetworkAccessManager httpService;

    auto finished = [&loop, &httpStatus, &httpData](QNetworkReply* reply) -> void
    {
       QVariant status_code = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
       if(status_code.isValid()){
           httpStatus = status_code.toInt();
           if(httpStatus != 200){
                qCritical() << QDateTime::currentDateTime().toString("hh:mm:ss") << __FUNCTION__ << "Error: " << httpStatus << " " + reply->errorString() ;
           }else
           {
               httpData = reply->readAll();
           }
       }
       loop.quit();

    };

    loop.connect(&httpService, &QNetworkAccessManager::finished, finished);

    QString protocol = ws.scheme() == "wss" ? "https://" : "http://";
    QString http_query = "/api/info";
    QUrl url(protocol + ws.host() + ":" + QString::number(ws.port()) + http_query);
    QNetworkRequest request(url);

    QString headerData = "Token " + token.toUtf8().toBase64();
    request.setRawHeader("Authorization", headerData.toLocal8Bit());

    if(data.size() > 0){
        //ByteArray bt = param["data"];
        QStringList contentDisposition{"form-data"};
        auto items = param.items();
        for (auto itr = items.begin(); itr != items.end(); ++itr) {
            auto val = *itr;
            if(val.key() != "data"){
                contentDisposition.append(QString("%1=\"%2\"").arg(val.key().c_str(), val.value().get<std::string>().c_str()));
            }
        }
        request.setRawHeader("Content-Disposition", contentDisposition.join(";").toLocal8Bit());
        QByteArray* q_data = new QByteArray(reinterpret_cast<const char*>(data.data()), data.size());
        request.setHeader(QNetworkRequest::ContentTypeHeader, "multipart/form-data");
        httpService.post(request, *q_data);
    }else{
        auto http_param = arcirk::https::http_param();
        http_param.command = command;
        http_param.param = QByteArray(param.dump().data()).toBase64().toStdString();
        request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
        httpService.post(request, QByteArray::fromStdString(pre::json::to_json(http_param).dump()));
    }
    loop.exec();

    if(httpStatus != 200){
        return false;
    }

    if(httpData.isEmpty())
        return false;

    if(httpData == WS_RESULT_ERROR){
         return false;
    }

    auto msg = pre::json::from_json<arcirk::server::server_response>(httpData.toStdString());

    if(msg.result.empty())
        return {};

    try {
        auto http_result = nlohmann::json::parse(QByteArray::fromBase64(msg.result.data()).toStdString());
        return http_result;
    } catch (...) {
        return msg.result;
    }

}

QByteArray WebSocketClient::exec_http_query_get(const std::string &command, const nlohmann::json &param)
{
    QEventLoop loop;
    int httpStatus = 200;
    QByteArray httpData;
    QNetworkAccessManager httpService;

    auto finished = [&loop, &httpStatus, &httpData](QNetworkReply* reply) -> void
    {
       QVariant status_code = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
       if(status_code.isValid()){
           httpStatus = status_code.toInt();
           if(httpStatus != 200){
                qCritical() << QDateTime::currentDateTime().toString("hh:mm:ss") << __FUNCTION__ << "Error: " << httpStatus << " " + reply->errorString() ;
           }else
           {
               httpData = reply->readAll();
           }
       }
       loop.quit();

    };
    loop.connect(&httpService, &QNetworkAccessManager::finished, finished);

    QUrl ws(conf_.server_host.data());
    QString protocol = ws.scheme() == "wss" ? "https://" : "http://";
    QString http_query = "/api/info";
    QUrl url(protocol + ws.host() + ":" + QString::number(ws.port()) + http_query);
    QNetworkRequest request(url);

    QString headerData = "Token " + QByteArray(conf_.hash.data()).toBase64();;
    request.setRawHeader("Authorization", headerData.toLocal8Bit());
    if(command == "GetBlob"){
        QStringList contentDisposition{"form-data"};
        auto items = param.items();
        for (auto itr = items.begin(); itr != items.end(); ++itr) {
            auto val = *itr;
            contentDisposition.append(QString("%1=\"%2\"").arg(val.key().c_str(), val.value().get<std::string>().c_str()));
        }
        request.setRawHeader("Content-Disposition", contentDisposition.join(";").toLocal8Bit());
        request.setHeader(QNetworkRequest::ContentTypeHeader, "multipart/form-data");
    }else{
        request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    }

    httpService.get(request);
    loop.exec();

    if(httpStatus != 200){
        return {};
    }

    if(httpData.isEmpty())
        return {};

    if(httpData == WS_RESULT_ERROR){
        return {};
    }

    return httpData;

}

void WebSocketClient::send_command(server::server_commands cmd, const nlohmann::json &param)
{
    qDebug() << QDateTime::currentDateTime().toString("hh:mm:ss") << __FUNCTION__ << QString::fromStdString(arcirk::enum_synonym(cmd));
    std::string p = QByteArray(param.dump().data()).toBase64().toStdString();

    nlohmann::json _param = {
        {arcirk::enum_synonym(cmd), p}
    };

    QString cmd_text = "cmd " + QString::fromStdString(arcirk::enum_synonym(cmd)) + " " + QString::fromStdString(_param.dump()).toUtf8().toBase64();

    m_client->sendTextMessage(cmd_text);
}

void WebSocketClient::command_to_client(const std::string &receiver, const std::string &command, const nlohmann::json &param)
{
    if(command.empty() || receiver.empty())
        return;

    std::string cmd = "cmd " + enum_synonym(arcirk::server::server_commands::CommandToClient) + " " + receiver;

    using json = nlohmann::json;


    std::string private_param = QString::fromStdString(param.dump()).toUtf8().toBase64().toStdString();
    json param_ = {
            {"parameters", private_param}        ,
            {"recipient", receiver},
            {"command", command}
    };

    json p = {
        {enum_synonym(arcirk::server::server_commands::CommandToClient), QString::fromStdString(param_.dump()).toUtf8().toBase64().toStdString()}
    };

    cmd.append(" ");
    cmd.append(QString::fromStdString(p.dump()).toUtf8().toBase64().toStdString());

    QString cmd_text = QString::fromStdString(cmd);

    m_client->sendTextMessage(cmd_text);
}

void WebSocketClient::send_message(const std::string &receiver, const std::string &message, const nlohmann::json &param)
{
    if(receiver.empty() || message.empty()){
        qCritical() << __FUNCTION__ << "Не верные параметры сообщения!";
        return;
    }

    std::string _param = QString::fromStdString(param.dump()).toUtf8().toBase64().toStdString();
    std::string _message = QString::fromStdString(message).toUtf8().toBase64().toStdString();
    std::string msg = "msg ";
    msg.append(receiver + " ");
    msg.append(_message);

    if(!param.empty()){
        msg.append(" ");
        msg.append(_param);
    }

    m_client->sendTextMessage(msg.c_str());
}

void WebSocketClient::set_conf(const client::client_conf& value){
    conf_ = value;
    write_conf();
}

void WebSocketClient::onConnected()
{
    qDebug() << QDateTime::currentDateTime().toString("hh:mm:ss") << __FUNCTION__;

    auto param = arcirk::client::client_param();
    param.app_name = conf_.app_name;
    param.host_name = QSysInfo::machineHostName().toStdString();
    param.system_user = "root";
    param.version = CLIENT_VERSION;
    param.user_name = conf_.user_name;
    param.hash = conf_.hash;
    param.device_id = conf_.device_id;
    param.product = QSysInfo::prettyProductName().toStdString();
    param.system_user = system_user_.toStdString();
    param.sid = m_sid;
    std::string p = pre::json::to_json(param).dump();
    QByteArray ba(p.c_str());
    nlohmann::json _param = {
        {"SetClientParam", QString(ba.toBase64()).toStdString()}
    };

    QString cmd = "cmd SetClientParam " + QString::fromStdString(_param.dump()).toUtf8().toBase64();

    m_client->sendTextMessage(cmd);

    m_isConnected = true;
}

void WebSocketClient::onDisconnected()
{
    m_isConnected = false;
    qDebug() << QDateTime::currentDateTime().toString("hh:mm:ss") << __FUNCTION__;
    emit connectionChanged(false);
}

void WebSocketClient::onTextMessageReceived(const QString &message)
{
    qDebug() << QDateTime::currentDateTime().toString("hh:mm:ss") << __FUNCTION__;

    if (message == "\n" || message.isEmpty() || message == "pong")
        return;

    //qDebug() << message;
    parse_response(message);
}

void WebSocketClient::onError(QAbstractSocket::SocketError error, const QString& errorString)
{
    //qDebug() << QDateTime::currentDateTime().toString("hh:mm:ss") << __FUNCTION__ << errorString;
    emit displayError("WebSocket", errorString);
}

void WebSocketClient::onReconnect()
{

}

void WebSocketClient::register_device(const arcirk::client::session_info& sess_info)
{
//    qDebug() << QDateTime::currentDateTime().toString("hh:mm:ss") << __FUNCTION__;

//        QUuid devId = QUuid::fromString(cli_conf.device_id.c_str());
//        if(devId.isNull()){
//            qCritical() << QDateTime::currentDateTime().toString("hh:mm:ss") << __FUNCTION__ << "Не верный идентификатор устройства!";
//            return;
//        }

//        QString product = cli_conf->;

//        auto record = arcirk::database::devices();
//        record.ref = devId.toString(QUuid::StringFormat::WithoutBraces).toStdString();
//        record.deviceType = arcirk::enum_synonym(arcirk::database::devices_type::devTablet);
//        record.first = product.toStdString();
//        record.second = product.toStdString();

//        auto j = pre::json::to_json<arcirk::database::devices>(record);

//        nlohmann::json struct_query_param = {
//            {"table_name", arcirk::enum_synonym(arcirk::database::tables::tbDevices)},
//            {"query_type", "update_or_insert"},
//            {"values", j}
//        };

//        std::string query_param = QByteArray::fromStdString(struct_query_param.dump()).toBase64().toStdString();

//        send_command(arcirk::server::server_commands::ExecuteSqlQuery, {
//                         {"query_param", query_param}
    //                     });
}

void WebSocketClient::set_system_user(const QString &value)
{
    system_user_ = value;
}

QUuid WebSocketClient::currentSession() const
{
    return m_currentSession;
}

QUuid WebSocketClient::currentUserUuid() const
{
    return m_currentUserUuid;
}

void WebSocketClient::set_client_conf(const json& value)
{
    conf_ = arcirk::secure_serialization<client::client_conf>(value, __FUNCTION__);
}

void WebSocketClient::set_client_param(const json& value){
    qDebug() << value.dump().c_str();
    client_param_ = arcirk::secure_serialization<client::client_param>(value, __FUNCTION__);
}

void WebSocketClient::set_server_conf(const json& value){
    auto name = server_conf_.ServerName;
    auto addr = server_conf_.ServerHost;
    auto hash = server_conf_.ServerUserHash;

    server_conf_ = arcirk::secure_serialization<server::server_config>(value, __FUNCTION__);
    if(server_conf_.ServerName.empty() && server_conf_.ServerHost == addr){
        server_conf_.ServerName = name;
    }

    if(server_conf_.ServerUserHash.empty())
        server_conf_.ServerUserHash = hash;
}
