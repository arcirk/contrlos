#ifndef SERVER_CONF_H_HPP
#define SERVER_CONF_H_HPP

#include <server_conf.hpp>
#include <QtCore>
#include <QDateTime>
#include <QUuid>
#include "directory_structure_check.hpp"
#include <QString>

#define ARCIRK_SERVER_CONF "server_conf.json"
#define ARCIRK_SERVER_NAME "wssrv"

#define ARCIRK_SERVER_UUID "539bdf4d-5663-48c7-bf8b-af6002d410ea"
#define ARCIRK_DATABASE_FILE  "arcirk.sqlite"

using namespace arcirk::strings;

namespace arcirk {

    inline QUuid uuid_to_quuid(const boost::uuids::uuid& value){
        return QUuid::fromString(QString::fromStdString(boost::to_string(value)));
    }

    static inline void fail(const QString& what, const QString& error, const QString& function = {}, bool save_log = false, const QString& error_log_dir = {}){

        QString result = QDateTime::currentDateTime().toString() + " ";
        result.append("error: ");
        if(!function.isEmpty()){
            result.append(function);
            result.append(":");
        }
        result.append(what);
        result.append(": ");
        result.append(error);

        qCritical() << result;

        if(!save_log)
            return;

        QString l_dir = arcirk::verify_application::working_directory() + "/" + ARCIRK_VERSION;
        if(error_log_dir.isEmpty()){
            l_dir.append("/logs/error");
        }else
            l_dir.append(error_log_dir);

        auto dir = QDir(l_dir);
        if(!dir.exists()){
            if(!dir.mkpath(l_dir))
                return;
        }

        auto file = QFile(dir.filePath("errors.log"));
        if(file.open(QIODevice::Append | QIODevice::Text)){
            QTextStream writeStream(&file);
            writeStream << result;
            file.close();
        }
    }

    static inline void log(const std::string& what, const std::string& message, const std::string& function = {}, bool save_log = false, const QString& error_log_dir = {}){

        std::string time = QDateTime::currentDateTime().toString("dd.MM.yyyy hh:mm:ss").toStdString();

        std::string _function;
        if(!function.empty()){
            _function.append(function);
            _function.append(" : ");
        }

        std::string result;
        result.append(what);
        result.append(": ");
        result.append(message);

        qDebug() << time.c_str() << _function.c_str() << result.c_str();

        if(!save_log)
            return;

        QString l_dir = arcirk::verify_application::working_directory() + "/" + ARCIRK_VERSION;
        if(error_log_dir.isEmpty()){
            l_dir.append("/logs/access");
        }else
            l_dir.append(error_log_dir);

        auto dir = QDir(l_dir);
        if(!dir.exists()){
            if(!dir.mkpath(l_dir))
                return;
        }

        auto file = QFile(dir.filePath(QString(ARCIRK_SERVER_NAME) + ".log"));
        if(file.open(QIODevice::Append | QIODevice::Text)){
            QTextStream writeStream(&file);
            writeStream << '\n' << time.c_str() << '\t' << _function.c_str() << '\t' << result.c_str();
            file.close();
        }
    }
}

    class server_commands_exception : public std::exception
    {
    private:
        std::string m_error{}; // handle our own string
        std::string uuid_form_{};
        std::string command_{};
    public:
        server_commands_exception(std::string_view error, std::string_view command, std::string_view uuid_form)
            : m_error{error},
            uuid_form_{uuid_form},
            command_{command}
        {
        }

        [[nodiscard]] const char* what() const noexcept override { return m_error.c_str(); }
        [[nodiscard]] const char* uuid_form() const noexcept { return uuid_form_.c_str(); }
        [[nodiscard]] const char* command() const noexcept { return command_.c_str(); }
    };

    inline  std::vector<std::tuple<std::string, std::string, json>> server_to_tuple(const arcirk::server::server_config& conf){

        const std::map<std::string, std::string> m_aliases = {
            std::make_pair("ServerHost", "IP адрес хоста"),
            std::make_pair("ServerPort", "Порт сервера"),
            std::make_pair("ServerUser", "Пользователь"),
            std::make_pair("ServerUserHash", "Хеш пользователя"),
            std::make_pair("ServerName", "Имя сервера"),
            std::make_pair("ServerHttpRoot", "Путь к HTTP каталогу"),
            std::make_pair("ServerWorkingDirectory", "Рабочая директория"),
            std::make_pair("AutoConnect", "Автоматическое подключение"),
            std::make_pair("UseLocalWebDavDirectory", "Использовать локальный WebDav каталог"),
            std::make_pair("LocalWebDavDirectory", "Локальный WebDav каталог"),
            std::make_pair("WebDavHost", "Адрес WebDav сервера"),
            std::make_pair("WebDavUser", "Пользователь WebDav сервера"),
            std::make_pair("WebDavPwd", "Пароль пользователя WebDav сервера"),
            std::make_pair("WebDavRoot", "Коренвая директория WebDav сервера"),
            std::make_pair("WebDavSSL", "Использование SSL на WebDav сервере"),
            std::make_pair("SQLFormat", "Тип SQL сервера"),
            std::make_pair("SQLHost", "Адрес SQL сервера"),
            std::make_pair("SQLUser", "Пользователь SQL сервера"),
            std::make_pair("SQLPassword", "Пароль пользователя SQL сервера"),
            std::make_pair("SQLPassword", "Пароль пользователя SQL сервера"),
            std::make_pair("HSHost", "Адрес веб сервиса"),
            std::make_pair("HSUser", "Пользователь веб сервиса"),
            std::make_pair("HSPassword", "Пароль пользователь веб сервиса"),
            std::make_pair("ServerSSL", "Использование SSL на веб сервисе"),
            std::make_pair("SSL_crt_file", "Имя файла сертификата"),
            std::make_pair("SSL_key_file", "Имя файла ключа сертификата"),
            std::make_pair("UseAuthorization", "Использовать авторизацию на сервере"),
            std::make_pair("ApplicationProfile", "Профиль приложения"),
            std::make_pair("ThreadsCount", "Количество потоков"),
            std::make_pair("Version", "Версия"),
            std::make_pair("ResponseTransferToBase64", "Конвертировать ответ свервера в base64"),
            std::make_pair("AllowDelayedAuthorization", "Разрешить отложенную авторизацию"),
            std::make_pair("AllowHistoryMessages", "Хранить историю сообщений"),
            std::make_pair("ExchangePlan", "Идентификатор плана обмена"),
            std::make_pair("ServerProtocol", "Протокол сервера"),
            std::make_pair("WriteJournal", "Записывать журнал"),
            std::make_pair("AllowIdentificationByWINSID", "Разрешить авторизацию по Win SID"),
            std::make_pair("ref", "Идентификатор сервера"),
        };

        auto object = pre::json::to_json(conf);

        std::vector<std::tuple<std::string, std::string, json>> result{};

        for(auto itr = object.items().begin(); itr != object.items().end(); ++itr){
            result.push_back(std::make_tuple(itr.key(), m_aliases.find(itr.key())->second, itr.value()));
        }

        return result;

    }

#endif // SERVER_CONF_HPP

