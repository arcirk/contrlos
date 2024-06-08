#ifndef SERVER_CONF_HPP
#define SERVER_CONF_HPP
#include "global.hpp"
#include <boost/filesystem.hpp>


#define ARCIRK_VERSION "1.2.0"
#define CLIENT_VERSION 4
//#define ARCIRK_SERVER_CONF "server_conf.json"

using namespace arcirk::strings;

BOOST_FUSION_DEFINE_STRUCT(
    (arcirk), version_application,
    (int, major)
    (int, minor)
    (int, path)
)
BOOST_FUSION_DEFINE_STRUCT(
    (arcirk::client), client_conf,
    (std::string, app_name)
    (bool, is_auto_connect)
    (std::string, server_host)
    (std::string, user_name)
    (std::string, hash)
    (arcirk::BJson, device_id)
    (arcirk::BJson, servers)
    (std::string, price_checker_repo)
    (std::string, server_repo)
    (std::string, system_user)
    (std::string, firefox)
    (bool, use_sid)
);

BOOST_FUSION_DEFINE_STRUCT(
    (arcirk::client), session_info,
    (arcirk::BJson, session_uuid)
    (std::string, user_name)
    (arcirk::BJson, user_uuid)
    (std::string, start_date)
    (std::string, app_name)
    (std::string, role)
    (arcirk::BJson, device_id)
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
    (arcirk::BJson, user_uuid)
    (std::string, user_name)
    (std::string, hash)
    (std::string, host_name)
    (std::string, password)
    (arcirk::BJson, session_uuid)
    (std::string, system_user)
    (arcirk::BJson, device_id)
    (std::string, info_base)
    (std::string, product)
    (std::string, sid)
    (int, version)
)

BOOST_FUSION_DEFINE_STRUCT(
    (arcirk::server), server_response,
    (std::string, command)
    (std::string, message)
    (std::string, param)
    (std::string, result)
    (std::string, sender)
    (std::string, receiver)
    (std::string, uuid_form)
    (std::string, app_name)
    (arcirk::BJson, sender_uuid)
    (arcirk::BJson, receiver_uuid)
    (std::string, version)
    (arcirk::BJson, data)
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
)

BOOST_FUSION_DEFINE_STRUCT(
    (arcirk::server), server_command_result,
    (std::string, command)
    (arcirk::BJson, uuid_form)
    (std::string, result)
    (std::string, message)
    (std::string, error_description)
    (arcirk::BJson, param)
    (arcirk::BJson, data)
)
namespace arcirk{

#ifndef IS_USE_QT_LIB
    static inline void fail(const std::string& what, const std::string& error, bool conv = true, const std::string& log_folder = "logs"){
        const std::tm tm = arcirk::date::current_date();
        char cur_date[100];
        std::strftime(cur_date, sizeof(cur_date), "%c", &tm);

        std::string res = std::string(cur_date);
        res.append(" " + what + ": ");
        if(conv)
            res.append(from_utf(error));
        else
            res.append(error);

        std::cerr << res << std::endl;

        if(log_folder.empty())
            return;

        namespace fs = boost::filesystem;

        fs::path log_dir(log_folder);
        log_dir /= "errors";
        if(!fs::exists(log_dir)){
            try {
                fs::create_directories(log_dir);
            } catch (const std::exception &e) {
                std::cerr << e.what() << std::endl;
                return;
            }
        }
        char date_string[100];
        strftime(date_string, sizeof(date_string), "%u_%m_%Y", &tm);

        fs::path file = log_dir / (std::string(date_string) + ".log");

        std::ofstream out;			// поток для записи
        out.open(file.string(), std::ios::app); 		// открываем файл для записи
        if (out.is_open())
        {
            out << res << '\n';
        }
        out.close();
    };

    static inline void log(const std::string& what, const std::string& message, bool conv = true, const std::string& log_folder = "logs"){
        const std::tm tm = arcirk::date::current_date();
        char cur_date[100];
        std::strftime(cur_date, sizeof(cur_date), "%c", &tm);

        std::string res = std::string(cur_date);
        res.append(" " + what + ": ");
        if(conv)
            res.append(from_utf(message));
        else
            res.append(message);

        std::cout << res << std::endl;

        if(log_folder.empty())
            return;

        namespace fs = boost::filesystem;

        fs::path log_dir(log_folder);
        log_dir /= "days";
        if(!fs::exists(log_dir)){
            try {
                fs::create_directories(log_dir);
            } catch (const std::exception &e) {
                std::cerr << e.what() << std::endl;
                return;
            }
        }
        char date_string[100];
        strftime(date_string, sizeof(date_string), "%u_%m_%Y", &tm);

        fs::path file = log_dir / (std::string(date_string) + ".log");
        std::ofstream out;			// поток для записи
        out.open(file.string(), std::ios::app); 		// открываем файл для записи
        if (out.is_open())
        {
            out << res  << '\n';
        }
        out.close();

    };

#endif

    static inline void* _crypt(void* data, unsigned data_size, void* key, unsigned key_size)
    {
        assert(data && data_size);
        if (!key || !key_size) return data;

        auto* kptr = (uint8_t*)key; // начало ключа
        uint8_t* eptr = kptr + key_size; // конец ключа

        for (auto* dptr = (uint8_t*)data; data_size--; dptr++)
        {
            *dptr ^= *kptr++;
            if (kptr == eptr) kptr = (uint8_t*)key; // переход на начало ключа
        }
        return data;
    }

    static inline std::string crypt(const std::string &source, const std::string& key) {
        if(source.empty())
            return {};
        try {
#ifdef _WINDOWS
            std::string s  = from_utf(source);
            std::string p  = from_utf(key);
            std::vector<char> source_(s.c_str(), s.c_str() + s.size() + 1);
            std::vector<char> key_(p.c_str(), p.c_str() + p.size() + 1);
            void* text = std::data(source_);
            void* pass = std::data(key_);
            //_crypt(text, ARR_SIZE(source_.c_str()), pass, ARR_SIZE(key.c_str()));
            _crypt(text, source_.size(), pass, key_.size());
            std::string result(to_utf((char*)text));
            return result;
#else
            std::vector<char> source_(source.c_str(), source.c_str() + source.size() + 1);
            std::vector<char> key_(key.c_str(), key.c_str() + key.size() + 1);
            void* text = std::data(source_);
            void* pass = std::data(key_);
            _crypt(text, ARR_SIZE(source.c_str()), pass, ARR_SIZE(key.c_str()));
            std::string result((char*)text);
            return result;
#endif
        } catch (const std::exception &e) {
            //fail(__FUNCTION__, e.what());
            std::cerr << __FUNCTION__ << "  " << e.what() << std::endl;
        }

        return {};

    }

}

namespace arcirk::server {

    static inline version_application get_version() {
        T_vec vec = arcirk::strings::split(ARCIRK_VERSION, ".");
        auto ver = version_application();
        ver.major = std::stoi(vec[0]);
        ver.minor = std::stoi(vec[1]);
        ver.path = std::stoi(vec[2]);
        return ver;
    }

    enum server_objects{
        OnlineUsers,
        Root,
        Services,
        Database,
        DatabaseTables,
        Devices,
        DatabaseUsers,
        ProfileDirectory,
        CertManager,
        Containers,
        Certificates,
        CertUsers,
        LocalhostUser,
        LocalhostUserCertificates,
        LocalhostUserContainers,
        LocalhostUserContainersRegistry,
        LocalhostUserContainersVolume,
        AvailableCertificates,
        QueryBuilderStructure,
        QueryBuilderTables,
        QueryBuilderJnners,
        OBJ_INVALID=-1,
    };

    NLOHMANN_JSON_SERIALIZE_ENUM(server_objects, {
        {OBJ_INVALID, nullptr} ,
        {OnlineUsers, "OnlineUsers"} ,
        {Root, "Root"} ,
        {Services, "Services"} ,
        {Database, "Database"} ,
        {DatabaseTables, "DatabaseTables"},
        {Devices, "Devices"},
        {DatabaseUsers, "DatabaseUsers"},
        {ProfileDirectory, "ProfileDirectory"},
        {CertManager, "CertManager"},
        {Containers, "Containers"},
        {Certificates, "Certificates"},
        {CertUsers, "CertUsers"},
        {LocalhostUser, "LocalhostUser"},
        {LocalhostUserCertificates, "LocalhostUserCertificates"},
        {LocalhostUserContainers, "LocalhostUserContainers"},
        {LocalhostUserContainersRegistry, "LocalhostUserContainersRegistry"},
        {LocalhostUserContainersVolume, "LocalhostUserContainersVolume"},
        {AvailableCertificates, "AvailableCertificates"},
    });
}
#endif
