//
// Created by arcady on 05.07.2021.
//

#ifndef SERVER_SHARED_STATE_HPP
#define SERVER_SHARED_STATE_HPP

#include <boost/smart_ptr.hpp>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_set>
#include <map>
#include <set>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp> // generators
#include <boost/uuid/uuid_io.hpp>         // streaming operators e

#include "subscriber.h"
//#include "../include/base.h"
//
//#include <arcirk.h>

using namespace arcirk;
#ifdef USE_QT_CREATOR
class SqlInterface;
#else
class SqlWrapper;
#endif
// Forward declaration
class websocket_session;

typedef std::function<bool(boost::uuids::uuid &uuid, bJson*, ws_message*, std::string&, std::string&)> cmd_func;

// Represents the shared server state
class shared_state
{
    std::string const doc_root_;

    // This mutex synchronizes all access to sessions_
    std::mutex mutex_;

    // Keep a list of all the connected clients
    //std::unordered_set<websocket_session*> sessions_;
    std::map<boost::uuids::uuid, websocket_session*> sessions_; //сессии
    std::map<boost::uuids::uuid, std::vector<websocket_session*>> user_sessions; //все сессии пользователя
    std::map<std::string, std::vector<boost::uuids::uuid>> channels; //каналы

public:
    explicit
    shared_state(std::string doc_root);
    ~shared_state(){
        if(sqlite3Db) {
            sqlite3Db->close();
            delete sqlite3Db;
        }

        if(sqlWrapper) {
            delete sqlWrapper;
        }
    };

    [[nodiscard]] std::string const&
    doc_root() const noexcept
    {
        return doc_root_;
    }

    void join  (websocket_session* session);
    void leave (websocket_session* session);
    void send(const std::string& message, std::string command = "message", bool to_base64 = true, const std::string& filterApps = "", const std::string sender = "", const std::string param = "");
    void deliver(const std::string& message, websocket_session* session);
    void on_start();

private:
    std::string currentHost;
    int currentPort;
    arc_sqlite::sqlite3_db* sqlite3Db;
#ifdef USE_QT_CREATOR
    SqlInterface * sqlWrapper;
#else
    SqlWrapper * sqlWrapper;
#endif
    std::string sql_format;

    _ws_message createMessage(websocket_session *session);
    static bool is_cmd(const std::string& msg);
    static bool is_msg(const std::string& msg);
    std::string run_cmd(const std::string &cmd, boost::uuids::uuid &uuid, std::string &command, std::string &uuid_form,
                        std::string &res, boost::uuids::uuid &recipient, bool &disable_return);
    bool send_message(const std::string &message, boost::uuids::uuid &recipient, websocket_session *current_sess);
    static bool is_valid_message(const std::string &message, boost::uuids::uuid &uuid_recipient, std::string &base64,
                          std::string &err);
    bool get_active_users(boost::uuids::uuid &uuid, bJson* params, ws_message* msg, std::string& err, std::string& custom_result);
    bool get_db_users(boost::uuids::uuid &uuid, bJson* params, ws_message* msg, std::string& err, std::string& custom_result);
    bool notify(boost::uuids::uuid &uuid, bJson* params, ws_message* msg, std::string& err, std::string& custom_result);
    bool set_client_param(boost::uuids::uuid &uuid, bJson* params, ws_message* msg, std::string& err, std::string& custom_result);
    bool add_new_user(boost::uuids::uuid &uuid, arcirk::bJson* params, ws_message* msg, std::string& err, std::string& custom_result);
    bool update_user(boost::uuids::uuid &uuid, arcirk::bJson* params, ws_message* msg, std::string& err, std::string& custom_result);
    bool get_messages(boost::uuids::uuid &uuid, arcirk::bJson* params, ws_message* msg, std::string& err, std::string& custom_result);
    bool get_user_info(boost::uuids::uuid &uuid, arcirk::bJson* params, ws_message* msg, std::string& err, std::string& custom_result);
    bool get_group_list(boost::uuids::uuid &uuid, arcirk::bJson* params, ws_message* msg, std::string& err, std::string& custom_result);
    bool add_group(boost::uuids::uuid &uuid, arcirk::bJson* params, ws_message* msg, std::string& err, std::string& custom_result);
    bool edit_group(boost::uuids::uuid &uuid, arcirk::bJson* params, ws_message* msg, std::string& err, std::string& custom_result);
    bool remove_group(boost::uuids::uuid &uuid, arcirk::bJson* params, ws_message* msg, std::string& err, std::string& custom_result);
    bool set_parent(boost::uuids::uuid &uuid, arcirk::bJson* params, ws_message* msg, std::string& err, std::string& custom_result);
    bool remove_user(boost::uuids::uuid &uuid, arcirk::bJson* params, ws_message* msg, std::string& err, std::string& custom_result);
    bool kill_session(boost::uuids::uuid &uuid, arcirk::bJson* params, ws_message* msg, std::string& err, std::string& custom_result);
    bool set_app_name(boost::uuids::uuid &uuid, arcirk::bJson* params, ws_message* msg, std::string& err, std::string& custom_result);
    bool exec_query(boost::uuids::uuid &uuid, arcirk::bJson* params, ws_message* msg, std::string& err, std::string& custom_result);
    bool exec_query_qt(boost::uuids::uuid &uuid, arcirk::bJson* params, ws_message* msg, std::string& err, std::string& custom_result);
    bool get_users_catalog(boost::uuids::uuid &uuid, arcirk::bJson* params, ws_message* msg, std::string& err, std::string& custom_result);
    bool set_user_cache(boost::uuids::uuid &uuid, arcirk::bJson* params, ws_message* msg, std::string& err, std::string& custom_result);
    bool get_user_cache(boost::uuids::uuid &uuid, arcirk::bJson* params, ws_message* msg, std::string& err, std::string& custom_result);
    bool set_content_type(boost::uuids::uuid &uuid, arcirk::bJson* params, ws_message* msg, std::string& err, std::string& custom_result);
    bool set_message_struct_type(boost::uuids::uuid &uuid, arcirk::bJson* params, ws_message* msg, std::string& err, std::string& custom_result);
    bool get_user_status(boost::uuids::uuid &uuid, arcirk::bJson* params, ws_message* msg, std::string& err, std::string& custom_result);
    bool get_user_status_(boost::uuids::uuid &uuid, const std::string& filter_app_name = "");
    bool get_user_data(boost::uuids::uuid &uuid, arcirk::bJson* params, ws_message* msg, std::string& err, std::string& custom_result);
    bool reset_unread_messages(boost::uuids::uuid &uuid, arcirk::bJson* params, ws_message* msg, std::string& err, std::string& custom_result);
    bool get_unread_messages(boost::uuids::uuid &uuid, arcirk::bJson* params, ws_message* msg, std::string& err, std::string& custom_result);
    bool command_to_qt_client(boost::uuids::uuid &uuid, arcirk::bJson* params, ws_message* msg, std::string& err, std::string& custom_result);
    bool command_to_qt_agent(boost::uuids::uuid &uuid, arcirk::bJson* params, ws_message* msg, std::string& err, std::string& custom_result);
    bool get_webdav_settings(boost::uuids::uuid &uuid, arcirk::bJson* params, ws_message* msg, std::string& err, std::string& custom_result);
    bool set_webdav_settings(boost::uuids::uuid &uuid, arcirk::bJson* params, ws_message* msg, std::string& err, std::string& custom_result);
    bool get_channel_token(boost::uuids::uuid &uuid, arcirk::bJson* params, ws_message* msg, std::string& err, std::string& custom_result);
    bool export_tables_to_ext(boost::uuids::uuid &uuid, arcirk::bJson* params, ws_message* msg, std::string& err, std::string& custom_result);
    bool sync_users(boost::uuids::uuid &uuid, arcirk::bJson* params, ws_message* msg, std::string& err, std::string& custom_result);
    bool send_technical_information(boost::uuids::uuid &uuid, arcirk::bJson* params, ws_message* msg, std::string& err, std::string& custom_result);
    bool set_sql_settings(boost::uuids::uuid &uuid, arcirk::bJson* params, ws_message* msg, std::string& err, std::string& custom_result);
    bool insert_file_to_data(boost::uuids::uuid &uuid, arcirk::bJson* params, ws_message* msg, std::string& err, std::string& custom_result);
    bool mpl_form_loaded(boost::uuids::uuid &uuid, arcirk::bJson* params, ws_message* msg, std::string& err, std::string& custom_result);
    bool command_to_client(boost::uuids::uuid &uuid, arcirk::bJson* params, ws_message* msg, std::string& err, std::string& custom_result);
    bool create_pay_html_page(boost::uuids::uuid &uuid, arcirk::bJson* params, ws_message* msg, std::string& err, std::string& custom_result);
    bool set_document_name(boost::uuids::uuid &uuid, arcirk::bJson* params, ws_message* msg, std::string& err, std::string& custom_result);
    int get_unread_messages_from_data(boost::uuids::uuid &uuid, boost::uuids::uuid &uuid_recipient);


    websocket_session* get_session(boost::uuids::uuid &uuid);
    std::vector<websocket_session *> get_sessions(boost::uuids::uuid &user_uuid);

    void get_user_subdivision(const std::string &user_ref, arcirk::bJson *jsonObj);
    void get_user_department(const std::string &user_ref, arcirk::bJson *jsonObj);

    void _add_new_user(const std::string &usr, const std::string &pwd, const std::string &role,
                       const std::string &uuid, const std::string &perf, const std::string &parent, std::string& error, bool pwd_is_hash = false);

    bool is_valid_param_count(const std::string &command, unsigned int params);
    static bool is_valid_command_name(const std::string &command);
    cmd_func get_cmd_func(const std::string& command);

    void get_group_hierarchy_keys(const std::string &current_uuid, std::set<int> &vec_uuid, std::string &err);

    void remove_group_hierarchy(const std::string &current_uuid, std::string &err);

    void send_error(const std::string& err, websocket_session *session, const std::string& command, const std::string& uuid_form);

    [[nodiscard]] std::string  _get_webdav_settings() const;

    void connect_sqlite_database();
    void connect_sqlserver_database();

};


#endif //UNTITLED1_SHARED_STATE_HPP
