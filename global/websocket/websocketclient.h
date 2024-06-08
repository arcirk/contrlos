#ifndef WEBSOCKETCLIENT_H
#define WEBSOCKETCLIENT_H

#include "../global.hpp"
#include <QObject>
#include <QtWebSockets/QWebSocket>
#include <QUuid>
#include <QQueue>
#include <QUrl>
#include <functional>
#include <QTimer>
#include "../server_conf.hpp"

using namespace arcirk;

#define CONF_FILENAME "client_conf.json"

class WebSocketClient : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QUrl url READ url WRITE setUrl NOTIFY urlChanged)

public:
    explicit WebSocketClient(QObject *parent = nullptr);
    explicit WebSocketClient(const QUrl& url, QObject *parent = nullptr);
    ~WebSocketClient();

    Q_INVOKABLE static QString generateHash(const QString& usr, const QString& pwd);
    static QString get_sha1(const QByteArray& p_arg);

    Q_INVOKABLE bool isStarted();

    client::client_conf& conf();
    server::server_config& server_conf();
    client::client_param &client_server_param();
    void set_client_conf(const json& value);
    void set_server_conf(const json& value);
    void set_client_param(const json& value);

    bool isConnected();

    QUrl url() const;
    Q_INVOKABLE void setUrl(const QUrl& url);
    QUrl http_url() const;

    void open(const std::string& sid = "");

    Q_INVOKABLE void close();
    void write_conf();
    void set_conf(const client::client_conf& value);

    void send_command(arcirk::server::server_commands cmd, const json& param = {});
    void command_to_client(const std::string &receiver, const std::string &command,
                                       const json &param = {});
    void send_message(const std::string &receiver, const std::string &message,
                      const json &param = {});

    json exec_http_query(const std::string& command, const json& param, const ByteArray& data = {}, bool returnAllMessage = false);
    static json http_query(const QUrl& ws, const QString& token, const std::string& command, const json& param, const ByteArray& data = {});

    QByteArray exec_http_query_get(const std::string& command, const json& param);

    static std::string crypt(const QString &source, const QString &key);

    static arcirk::client::version_application get_version(){
         QStringList vec = QString(ARCIRK_VERSION).split(".");
         auto ver = arcirk::client::version_application();
         ver.major = vec[0].toInt();
         ver.minor = vec[1].toInt();
         ver.path = vec[2].toInt();
        return ver;
    }

    void register_device(const arcirk::client::session_info& sess_info);

    void set_system_user(const QString& value);

    QUuid currentSession() const;
    QUuid currentUserUuid() const;

protected:
    QWebSocket* m_client;
    client::client_conf conf_;
    client::client_param client_param_;

private:

    server::server_config server_conf_;

    QString system_user_;
    std::string m_sid;

    QUuid m_currentSession;
    QUuid m_currentUserUuid;

    bool m_isConnected;

    void read_conf(const QString& app_name = "ServerManager");

    void initialize();

    void parse_response(const QString& resp);

    void doDisplayError(const QString& what, const QString& err);
    void doConnectionSuccess(); //при успешной авторизации
    void doConnectionChanged(bool state);

    static QString get_hash(const QString& first, const QString& second);
    void get_server_configuration_sync();
    static arcirk::client::client_param parse_client_param(const std::string& response);

signals:
    void displayError(const QString& what, const QString& err);
    void error(const QString& what, const QString& command, const QString& err);
    void connectionSuccess(); //при успешной авторизации
    void connectionChanged(bool state);
    void serverResponse(const arcirk::server::server_response& message);
    void notify(const QString& message);
    void commandToClientResponse(const arcirk::server::server_response& message);
    void userMessage(const arcirk::server::server_response& message);
    void userInfo(const json& info);
    void urlChanged();

    void syncGetDiscrepancyInData(const arcirk::server::server_response& resp);
    void updateBarcodeInformation(const arcirk::server::server_response& resp);

private slots:

    void onConnected();
    void onDisconnected();
    void onTextMessageReceived(const QString &message);
    void onError(QAbstractSocket::SocketError error, const QString& errorString);
    void onReconnect();

};

#define PATH_TO_RELEASE "arcirk_server/update/"

typedef std::function<void()> async_await_;

class IWClient : public WebSocketClient{

    Q_OBJECT

public:
    explicit IWClient(QObject *parent = nullptr): WebSocketClient{parent}{
        m_reconnect = new QTimer(this);
        connect(m_reconnect,SIGNAL(timeout()),this,SLOT(onReconnect()));
    };

    ~IWClient(){m_reconnect->stop();};

    Q_INVOKABLE void checkConnection(){
        if(!m_reconnect->isActive())
            startReconnect();
    }
    Q_INVOKABLE void openConnection(){
        this->open();
    }
    Q_INVOKABLE void startReconnect(){
        m_reconnect->start(1000 * 60);
    };

    QString token() const{
        return conf_.hash.c_str();
    }

private:
    QTimer * m_reconnect;
    QTimer * m_tmr_synchronize;
    QQueue<async_await_> m_async_await;

    void asyncAwait(){
        if(m_async_await.size() > 0){
            auto f = m_async_await.dequeue();
            f();
        }
    };

    void reconnect(){
        open();
    }

private slots:
    void onReconnect(){
        //qDebug() << QDateTime::currentDateTime().toString("hh:mm:ss") << __FUNCTION__;
        if(isStarted()){
            if(m_reconnect->isActive())
                m_reconnect->stop();
        }else{
            m_async_await.append(std::bind(&IWClient::reconnect, this));
            asyncAwait();
        }
    }

};

#endif // WEBSOCKETCLIENT_H
