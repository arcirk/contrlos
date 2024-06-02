#ifndef SERVER_CONF_HPP
#define SERVER_CONF_HPP

#include <QtCore>
#include <QDateTime>
#include <QUuid>
#include <global.hpp>
#include "directory_structure_check.hpp"


#define ARCIRK_VERSION "1.2.0"
#define ARCIRK_SERVER_CONF "server_conf.json"
#define ARCIRK_SERVER_NAME "wssrv"
#define CLIENT_VERSION 4

using namespace arcirk::strings;

BOOST_FUSION_DEFINE_STRUCT(
    (arcirk), version_application,
    (int, major)
    (int, minor)
    (int, path)
    )

BOOST_FUSION_DEFINE_STRUCT(
    (arcirk::server), server_config,
    (std::string, ServerHost)
    (int, ServerPort)
    (std::string, ServerUser)
    (std::string, ServerUserHash)
    (std::string, ServerName)
    (std::string, ServerHttpRoot)
    (std::string, ServerWorkingDirectory)
    (bool, AutoConnect)
    (bool, UseLocalWebDavDirectory)
    (std::string, LocalWebDavDirectory)
    (std::string, WebDavHost)
    (std::string, WebDavUser)
    (std::string, WebDavPwd)
    (std::string, WebDavRoot)
    (bool, WebDavSSL)
    (int, SQLFormat)
    (std::string, SQLHost)
    (std::string, SQLUser)
    (std::string, SQLPassword)
    (std::string, HSHost)
    (std::string, HSUser)
    (std::string, HSPassword)
    (bool, ServerSSL)
    (std::string, SSL_crt_file)
    (std::string, SSL_key_file)
    (bool, UseAuthorization)
    (std::string, ApplicationProfile)
    (int, ThreadsCount)
    (std::string, Version)
    (bool, ResponseTransferToBase64)
    (bool, AllowDelayedAuthorization)
    (bool, AllowHistoryMessages)
    (std::string, ExchangePlan)
    (std::string, ServerProtocol)
    (bool, WriteJournal)
    (bool, AllowIdentificationByWINSID)
    (arcirk::BJson, ref)
    );

BOOST_FUSION_DEFINE_STRUCT(
    (arcirk::server), server_command_result,
    (std::string, command)
    (std::string, uuid_form)
    (std::string, result)
    (std::string, message)
    (std::string, error_description)
    (std::string, param)
    (arcirk::BJson, data)
)

BOOST_FUSION_DEFINE_STRUCT(
    (arcirk::server), server_response,
    (std::string, command)
    (std::string, message)
    (std::string, param)
    (std::string, result)
    (std::string, sender)
    (std::string, receiver)
    (arcirk::BJson, uuid_form)
    (std::string, app_name)
    (arcirk::BJson, sender_uuid)
    (arcirk::BJson, receiver_uuid)
    (std::string, version)
    (arcirk::BJson, data)
)

BOOST_FUSION_DEFINE_STRUCT(
    (arcirk::client), session_info,
    (std::string, session_uuid)
    (std::string, user_name)
    (std::string, user_uuid)
    (std::string, start_date)
    (std::string, app_name)
    (std::string, role)
    (std::string, device_id)
    (std::string, address)
    (std::string, info_base)
    (std::string, host_name)
    (std::string, product)
    (std::string, system_user)
    (std::string, sid)
)

BOOST_FUSION_DEFINE_STRUCT(
    (arcirk::client), client_param,
    (std::string, app_name)
    (std::string, user_uuid)
    (std::string, user_name)
    (std::string, hash)
    (std::string, host_name)
    (std::string, password)
    (std::string, session_uuid)
    (std::string, system_user)
    (std::string, device_id)
    (std::string, info_base)
    (std::string, product)
    (std::string, sid)
    (int, version)
    )

inline QUuid uuid_to_quuid(const boost::uuids::uuid& value){
    return QUuid::fromString(boost::to_string(value).c_str());
}

namespace arcirk {

    namespace server {
        static inline version_application get_version() {
            T_vec vec = arcirk::strings::split(ARCIRK_VERSION, ".");
            auto ver = version_application();
            ver.major = std::stoi(vec[0]);
            ver.minor = std::stoi(vec[1]);
            ver.path = std::stoi(vec[2]);
            return ver;
        }
        enum server_commands{
            ServerVersion, //Версия сервера
            ServerOnlineClientsList, //Список активных пользователей
            SetClientParam, //Параметры клиента
            ServerConfiguration, //Конфигурация сервера
            UserInfo, //Информация о пользователе (база данных)
            InsertOrUpdateUser, //Обновить или добавить пользователя (база данных)
            CommandToClient, //Команда клиенту (подписчику)
            ServerUsersList, //Список пользователей (база данных)
            ExecuteSqlQuery, //выполнить запрос к базе данных
            GetMessages, //Список сообщений
            UpdateServerConfiguration, //Обновить конфигурацию сервера
            HttpServiceConfiguration, //Получить конфигурацию http сервиса 1С
            InsertToDatabaseFromArray, //Добавить массив записей в базу //устарела удалить
            SetNewDeviceId, //Явная установка идентификатора на устройствах где не возможно его получить
            ObjectSetToDatabase, //Синхронизация объекта 1С с базой
            ObjectGetFromDatabase, //Получить объект типа 1С из базы данных для десериализации
            SyncGetDiscrepancyInData, //Получить расхождения в данных между базами на клиенте и на Сервере
            SyncUpdateDataOnTheServer, //Обновляет данные на сервере по запросу клиента
            WebDavServiceConfiguration, //Получить настройки WebDav
            SyncUpdateBarcode, //синхронизирует на сервере штрихкод и номенклатуру по запросу клиента с сервером 1с
            DownloadFile, //Загружает файл на сервер
            GetInformationAboutFile, //получить информацию о файле
            CheckForUpdates, //поиск фалов обрновления
            UploadFile, //скачать файл
            GetDatabaseTables,
            FileToDatabase, //Загрузка таблицы базы данных из файла
            ProfileDirFileList,
            ProfileDeleteFile,
            DeviceGetFullInfo,
            GetTasks,
            UpdateTaskOptions,
            TasksRestart,
            RunTask,
            StopTask,
            SendNotify,
            GetCertUser,
            VerifyAdministrator,
            UserMessage,
            GetChannelToken,
            IsChannel,
            GetDatabaseStructure,
            Run1CScript,
            CreateDirectories,
            DeleteDirectory,
            bDeleteFile,
            CMD_INVALID=-1,
        };

        NLOHMANN_JSON_SERIALIZE_ENUM(server_commands, {
           {CMD_INVALID, nullptr}    ,
           {ServerVersion, "ServerVersion"}  ,
           {ServerOnlineClientsList, "ServerOnlineClientsList"}    ,
           {SetClientParam, "SetClientParam"}    ,
           {ServerConfiguration, "ServerConfiguration"}    ,
           {UserInfo, "UserInfo"}    ,
           {InsertOrUpdateUser, "InsertOrUpdateUser"}    ,
           {CommandToClient, "CommandToClient"}    ,
           {ServerUsersList, "ServerUsersList"}    ,
           {ExecuteSqlQuery, "ExecuteSqlQuery"}    ,
           {GetMessages, "GetMessages"}    ,
           {UpdateServerConfiguration, "UpdateServerConfiguration"}    ,
           {HttpServiceConfiguration, "HttpServiceConfiguration"}    ,
           {InsertToDatabaseFromArray, "InsertToDatabaseFromArray"}    ,
           {SetNewDeviceId, "SetNewDeviceId"}    ,
           {ObjectSetToDatabase, "ObjectSetToDatabase"}    ,
           {ObjectGetFromDatabase, "ObjectGetFromDatabase"}    ,
           {SyncGetDiscrepancyInData, "SyncGetDiscrepancyInData"}    ,
           {SyncUpdateDataOnTheServer, "SyncUpdateDataOnTheServer"}    ,
           {WebDavServiceConfiguration, "WebDavServiceConfiguration"}    ,
           {SyncUpdateBarcode, "SyncUpdateBarcode"}    ,
           {DownloadFile, "DownloadFile"}    ,
           {GetInformationAboutFile, "GetInformationAboutFile"}    ,
           {CheckForUpdates, "CheckForUpdates"}    ,
           {UploadFile, "UploadFile"}    ,
           {GetDatabaseTables, "GetDatabaseTables"}    ,
           {FileToDatabase, "FileToDatabase"}    ,
           {ProfileDirFileList, "ProfileDirFileList"}    ,
           {ProfileDeleteFile, "ProfileDeleteFile"}    ,
           {DeviceGetFullInfo, "DeviceGetFullInfo"}    ,
           {GetTasks, "GetTasks"}    ,
           {UpdateTaskOptions, "UpdateTaskOptions"}    ,
           {TasksRestart, "TasksRestart"}    ,
           {RunTask, "RunTask"}    ,
           {StopTask, "StopTask"}    ,
           {SendNotify, "SendNotify"}    ,
           {GetCertUser, "GetCertUser"}    ,
           {VerifyAdministrator, "VerifyAdministrator"}    ,
           {UserMessage, "UserMessage"}    ,
           {GetChannelToken, "GetChannelToken"}    ,
           {IsChannel, "IsChannel"}    ,
           {GetDatabaseStructure, "GetDatabaseStructure"}    ,
           {Run1CScript, "Run1CScript"}    ,
           {CreateDirectories, "CreateDirectories"}    ,
           {DeleteDirectory, "DeleteDirectory"}    ,
           {bDeleteFile, "DeleteFile"}    ,
           })
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

#endif // SERVER_CONF_HPP

