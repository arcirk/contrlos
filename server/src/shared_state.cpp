#include "../include/shared_state.hpp"

#include <utility>
#include "../include/websocket_session.hpp"


#ifdef _WINDOWS
#pragma warning(disable:4100)
#endif

shared_state::
shared_state(std::string doc_root)
        : doc_root_(std::move(doc_root))
{
    arcirk::bConf conf("conf.json", true);
    sql_format = conf[SQLFormat].get_string();
    std::cout << "shared_state::shared_state sql_format:" << sql_format << std::endl;
    if(sql_format.empty()){
        sql_format = "SQLITE";
    }

    currentHost = conf[ServerHost].get_string();
    currentPort = conf[ServerPort].get_int();

    sqlite3Db = new arc_sqlite::sqlite3_db();

    if(sql_format == "SQLITE"){
        connect_sqlite_database();
    }else if(sql_format == "SQLSERVER"){
        connect_sqlserver_database();
    }else{
        std::cerr << "Not support sql format " + sql_format << std::endl;
    }

}

void shared_state::_add_new_user(const std::string &usr, const std::string &pwd, const std::string &role,
                                 const std::string &uuid, const std::string &perf, const std::string &parent, std::string& error, bool pwd_is_hash) {

    std::string  hash;
    if (!pwd_is_hash)
        hash = arcirk::get_hash(usr, pwd);
    else
        hash = pwd;

    boost::uuids::uuid _uuid{};
    _uuid = arcirk::string_to_uuid(uuid, true);

    std::vector<arcirk::content_value> fields;

    std::string _parent = parent;
    if (_parent.empty())
        _parent = arcirk::nil_string_uuid();

    fields.emplace_back(arcirk::content_value("FirstField", usr));
    fields.emplace_back(arcirk::content_value("SecondField", perf));
    fields.emplace_back(arcirk::content_value("Ref", arcirk::uuid_to_string(_uuid)));
    fields.emplace_back(arcirk::content_value("hash", hash));
    fields.emplace_back(arcirk::content_value("role", role));
    fields.emplace_back(arcirk::content_value("channel", _parent));

    sqlite3Db->insert(arc_sqlite::tables::eUsers, fields, error, "");
    if (error.empty())
        std::cout << QString("Регистрация пользователя - ").toLocal8Bit().toStdString() << QString::fromStdString(usr).toLocal8Bit().toStdString() << ":" << hash << std::endl;

}

void
shared_state::
join(websocket_session* session)
{
    std::lock_guard<std::mutex> lock(mutex_);
    sessions_.insert(std::pair<boost::uuids::uuid, websocket_session*>(session->get_uuid(), session));
}

void
shared_state::
leave(websocket_session* session)
{
    std::cout << "client leave: " << QString::fromStdString(session->get_name()).toLocal8Bit().toStdString() << std::endl;

    if (session->authorized && !session->disable_notify()){
        // Оповещаем всех об отключении клиента
        send(std::string ("{\"name\": \"") + session->get_name() +
             "\", \"uuid_user\": \"" + arcirk::uuid_to_string(session->get_user_uuid()) + "\", \"uuid\": \"" +
             arcirk::uuid_to_string(session->get_uuid()) + "\"}", "client_leave", true, session->notify_apps());
    }

    std::lock_guard<std::mutex> lock(mutex_);

    auto iter_u = user_sessions.find(session->get_user_uuid());
    if (iter_u != user_sessions.end() ){
        if (iter_u->second.size() <= 1){
            user_sessions.erase(session->get_user_uuid());
            std::cout << "erase user sessions" << std::endl;
        }
        else{
            iter_u->second.erase(find(iter_u->second.begin(), iter_u->second.end(),session));
            std::cout << "close user session" << std::endl;
        }
    }

    auto iter = sessions_.find(session->get_uuid());
    if (iter != sessions_.end() ){
        sessions_.erase(session->get_uuid());
    }

}

_ws_message shared_state::createMessage(websocket_session *session) {
    _ws_message _message = _ws_message();
    _message.uuid = session->get_uuid();
    _message.name = session->get_name();
    _message.app_name = session->get_app_name();
    _message.uuid_user = session->get_user_uuid();
    _message.role = session->get_role();
    _message.ip_address = session->ip_address();
    _message.host_name = session->host_name();
    _message.user_name = session->get_user_name();
    _message.device_id = session->get_device_id();
    //std::cout << _message.uuid << std::endl;
    return _message;
}

void
shared_state::deliver(const std::string& message, websocket_session *session)
{
    std::string result;
    std::string command("message");
    std::string uuid_form;
    bool is_disable_return = false;

    //это команда: cmd base64
    if (is_cmd(message))
    {
        //std::cout << "deliver: Передана команда серверу" << std::endl;
        std::string res;
        boost::uuids::uuid recipient;

        std::string msg_ = run_cmd(message, session->get_uuid(), command, uuid_form, res, recipient, is_disable_return);

        //если установка параметров сессии, требуется обновить параметры на клиенте
        if (command == "set_client_param")
            result.append("result ");

        _ws_message _message = createMessage(session);
        _message.message = msg_;
        _message.command = command;
        _message.result = res;
        _message.uuid_form = arcirk::string_to_uuid(uuid_form, false);
        _message.uuid_channel = recipient;

        auto * msg = new ws_message(_message);
        std::cout <<"shared_state::deliver:" <<  command << ": " << msg->get_json(false) << std::endl;
        result.append(msg->get_json(true));
        delete msg;
    }else
    {
        result = message;
    }

    if (result == "\n")
        return;

    if (result == "ping"){
        result = "pong";
    } else{
        //отправка сообщения доступна только авторизованным пользователям
        try {
            session->throw_authorized();
        }catch (boost::exception const &e){
            std::cerr << boost::diagnostic_information( e ) << std::endl;
            if (command != "set_client_param"){
                _ws_message _message = createMessage(session);
                _message.message = "Отказано в доступе!";
                _message.result = "error";
                _message.uuid_form = arcirk::string_to_uuid(uuid_form, false);

                auto * msg = new ws_message(_message);
                result = msg->get_json(true);

                delete msg;
            }
            auto const ss = boost::make_shared<std::string const>(std::move(result));
            session->send(ss);
            return;
        }
    }

    //на всякий случай
    if (result.empty())
        return;

    //это сообщение: msg uuid_получателя base64
    if (is_msg(message)){

        boost::uuids::uuid uuid_recipient{}; //uuid пользователя (не сессии)
        std::string err;
        std::string base64;
        if (is_valid_message(message, uuid_recipient, base64, err)){
            if (!send_message(base64, uuid_recipient, session)){
                send_error("Ошибка отправки сообщения!", session, command, uuid_form);
            }
        }else{
            send_error(err, session, command, uuid_form);
        }

    }else{
        if(is_disable_return)
            return;

        auto const ss = boost::make_shared<std::string const>(std::move(result));

        for(auto const& wp : *session->get_subscribers())
            wp.second->deliver(ss);
    }



}

void shared_state::send_error(const std::string &err, websocket_session *session, const std::string& command, const std::string& uuid_form) {

    _ws_message _message = createMessage(session);
    _message.message = err;
    _message.result = "error";
    _message.uuid_form = arcirk::string_to_uuid(uuid_form, false);
    _message.command = command;

    auto * msg = new ws_message(_message);
    std::string result = msg->get_json(true);

    auto const ss = boost::make_shared<std::string const>(std::move(result));
    session->send(ss);
    delete msg;
}

// Рассылка сообщения на все клиентские сеансы websocket
// uuid - отправитель сообщения
void
shared_state::
send(const std::string& message, std::string command, bool to_base64, const std::string& filterApps, const std::string sender, const std::string param)
{

    arcirk::T_vec apps;
    if(!filterApps.empty()){
        apps = arcirk::split(std::move(filterApps), "|");
    }

    std::vector<boost::weak_ptr<websocket_session>> v;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        v.reserve(sessions_.size());
        for(auto p : sessions_)
            v.emplace_back(p.second->weak_from_this());
    }

    for(auto const& wp : v)
        if(auto sp = wp.lock()){
            if(sp->disable_notify())
                continue;
            if(apps.size() > 0){
                if(std::find(apps.begin(), apps.end(), sp->get_app_name()) == apps.end())
                    continue;
            }
            if (to_base64){
                _ws_message _message = createMessage(sp.get());
                _message.message = message;
                _message.command = command;
                auto * new_msg = new ws_message(_message);
                new_msg->add_custom_value("sender", sender);
                new_msg->add_custom_value("param", param);
                std::string sz_new_msg = new_msg->get_json(true);
                auto const ss = boost::make_shared<std::string const>(std::move(sz_new_msg));
                sp->send(ss);
                delete new_msg;
            } else
            {
                auto const ss = boost::make_shared<std::string const>(std::move(message));
                sp->send(ss);
            }

        }

}

bool shared_state::is_cmd(const std::string& msg)
{
    std::string cmd = msg.substr(0, 3);

    if (cmd == "cmd")
    {
        return true;
    }

    return false;
}

std::string
shared_state::run_cmd(const std::string &cmd, boost::uuids::uuid &uuid, std::string &command, std::string &uuid_form,
                      std::string &res, boost::uuids::uuid  &recipient, bool &disable_return)
{

    //формат сообщения
    //cmd base64

    //делим строку, разделитель пробел

    std::string message = cmd;

    if (cmd.substr(cmd.length() - 1, cmd.length()) == "\n"){
        message = cmd.substr(0, cmd.length()  - 1) ;
    }

    arcirk::T_vec v = arcirk::split(std::move(message), " ");

    std::string result = "unknown command!";

    command = "unknown command!";

    if (v.size() != 2){
        res = "error";
        return "request format error!";
    }


    std::string base64 = v[1];
    std::string json;

    //декодируем 2 элемент массива
    try {
        json = arcirk::base64_decode(base64);
    }catch (std::exception& e){
        res = "error";
        return std::string ("error: ") + e.what() ;
    }

    // парсим
    auto * msg = new ws_message(json);

    if (msg->is_parse()) {

//        //возможно команда имеет формат
//        //command uuid "message"
//        //поэтому с начала удаляем строки заключенные в кавычки
//        std::string sz_command = arc_json::remove_text_enclosed_in_quotation_marks(msg->get_message());
//        //Получаем строку заключенную в кавычки в массив т.к. таких строк может быть несколько
//        arc_json::T_vec vec_message = arc_json::get_text_in_quotation_marks(msg->get_message());
//        //получаем массив значений команды в формате command arg1 arg2 ...argn
//        arc_json::T_vec vec_command = arc_json::split(sz_command, " ");
//
//        command = vec_command[0];

        command = msg->message().command;

        std::cout << "run command: " << command << std::endl;

        auto __uuid_form = msg->message().uuid_form;
        if (!__uuid_form.is_nil())
            uuid_form = arcirk::uuid_to_string(msg->message().uuid_form);
        else
            uuid_form = arcirk::nil_string_uuid();

        if (!is_valid_command_name(command)){
            res = "error";
            return "unknown command!";
        }

        auto * params = new arcirk::bJson();
        std::string tmp_msg = msg->message().message;
        params->parse(tmp_msg);

        if (!params->is_parse()){
            res = "error";
            return "error parse parameters command!";
        }

        if (!is_valid_param_count(command, params->get_member_count())){
            res = "error";
            return "wrong number of parameters!";
        }

        std::string err = "error";
        std::string custom_result = "";
        disable_return = params->get_member("disable_return").get_bool();

        bool _res = get_cmd_func(command)(uuid, params, msg, err, custom_result);
        if (_res){
            if (custom_result.empty())
                result = "success";
            else
                result = custom_result;
            res = "success";
        } else{
            result = err;
            res = "error";
        }
        recipient = msg->message().uuid_channel;
        //std::cout << "end command: " << command << std::endl;

        delete params;
    }

    delete msg;

    return result;
}

//получение непосредственно сессии
websocket_session*
shared_state::get_session(boost::uuids::uuid &uuid) {
    auto iter = sessions_.find(uuid);
    if (iter != sessions_.end() ){
        return iter->second;
    }
    return nullptr;
}

//получение всех сессий пользователя
std::vector<websocket_session *>
shared_state::get_sessions(boost::uuids::uuid &user_uuid) {
    auto iter = user_sessions.find(user_uuid);
    if (iter != user_sessions.end() ){
        return iter->second;
    }
    return {};
}

void shared_state::on_start() {

    //get shared channels
    //run_session_shared_channels();
}


bool shared_state::is_valid_param_count(const std::string &command, unsigned int params) {
    if (command == "set_client_param")
        return true; //params == 7;
    else if (command == "get_active_users")
        return true;//params == 1; //динамически
    else if (command == "notify")
        return true; // params == 2;
    else if (command == "add_user")
        return params == 6;
    else if (command == "get_users")
        return params == 1;
    else if (command == "update_user")
        return params == 2;
    else if (command == "get_messages")
        return params == 4;
    else if (command == "get_user_info")
        return params == 1;
    else if (command == "get_user_status")
        return true; //params == 1; //динамически, обязательный 1 user_uuid
    else if (command == "get_group_list")
        return params == 1;
    else if (command == "add_group")
        return params == 3;
    else if (command == "edit_group")
        return params == 4;
    else if (command == "remove_group")
        return params == 1;
    else if (command == "set_parent")
        return params == 2;
    else if (command == "remove_user")
        return params == 1;
    else if (command == "kill_session")
        return params == 1;
    else if (command == "get_user_data")
        return true; //params == 1; //динамически, обязательный 1 user_uuid
    else if (command == "set_app_name")
        return params == 2;
    else if (command == "exec_query")
        return true;// params == 1;
    else if (command == "get_users_catalog")
        return params == 1;
    else if (command == "get_user_cache")
        return params == 1;
    else if (command == "set_user_cache")
        return params == 2;
    else if (command == "set_message_struct_type")
        return params == 1;
    else if (command == "set_content_type")
        return params == 1;
    else if (command == "reset_unread_messages")
        return params == 1;
    else if (command == "get_unread_messages")
        return params == 1;
    else if (command == "command_to_qt_client")
        return true; // params == 3;
    else if (command == "command_to_qt_agent")
        return true; // params == 4;
    else if (command == "get_webdav_settings")
        return params == 1;
    else if (command == "set_webdav_settings")
        return params == 5;
    else if (command == "get_channel_token")
        return params == 3;
    else if (command == "set_sql_settings")
        return params == 5;
    else if (command == "export_tables_to_ext")
        return params == 1;
    else if (command == "sync_users")
        return params == 2;
    else if (command == "exec_query_qt")
        return true; //return params == 1;
    else if (command == "mpl_form_loaded")
        return true;
    else if (command == "command_to_client")
        return true;
    else if (command == "create_pay_html_page")
        return true;
    else if (command == "set_document_name")
        return true;
    else
        return false;
}

bool shared_state::is_valid_command_name(const std::string &command) {
    std::vector<std::string> commands;

    commands.emplace_back("set_client_param");
    commands.emplace_back("get_active_users");
    commands.emplace_back("notify");
    commands.emplace_back("add_user");
    commands.emplace_back("reset_unread_messages");
    commands.emplace_back("message");
    commands.emplace_back("get_users");
    commands.emplace_back("update_user");
    commands.emplace_back("get_messages");
    commands.emplace_back("get_user_info");
    commands.emplace_back("get_user_status");
    commands.emplace_back("get_group_list");
    commands.emplace_back("add_group");
    commands.emplace_back("edit_group");
    commands.emplace_back("remove_group");
    commands.emplace_back("set_parent");
    commands.emplace_back("remove_user");
    commands.emplace_back("kill_session");
    commands.emplace_back("get_user_data");
    commands.emplace_back("set_app_name");
    commands.emplace_back("exec_query");
    commands.emplace_back("get_users_catalog");
    commands.emplace_back("set_message_struct_type");
    commands.emplace_back("set_content_type");
    commands.emplace_back("get_user_cache");
    commands.emplace_back("set_user_cache");
    commands.emplace_back("get_unread_messages");
    commands.emplace_back("command_to_qt_client");
    commands.emplace_back("command_to_qt_agent");
    commands.emplace_back("get_webdav_settings");
    commands.emplace_back("set_webdav_settings");
    commands.emplace_back("get_channel_token");
    commands.emplace_back("set_sql_settings");
    commands.emplace_back("export_tables_to_ext");
    commands.emplace_back("sync_users");
    commands.emplace_back("exec_query_qt");
    commands.emplace_back("mpl_form_loaded");
    commands.emplace_back("command_to_client");
    commands.emplace_back("create_pay_html_page");
    commands.emplace_back("set_document_name");

    return std::find(commands.begin(), commands.end(), command) != commands.end();
}

cmd_func shared_state::get_cmd_func(const std::string& command) {
    if (command == "set_client_param")
        return  std::bind(&shared_state::set_client_param, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5);
    else if (command == "get_active_users")
        return  std::bind(&shared_state::get_active_users, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5);
    else if (command == "notify")
        return  std::bind(&shared_state::notify, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5);
    else if (command == "add_user")
        return  std::bind(&shared_state::add_new_user, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5);
    else if (command == "get_users")
        return  std::bind(&shared_state::get_db_users, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5);
    else if (command == "update_user")
        return  std::bind(&shared_state::update_user, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5);
    else if (command == "get_messages")
        return  std::bind(&shared_state::get_messages, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5);
    else if (command == "get_user_info")
        return  std::bind(&shared_state::get_user_info, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5);
    else if (command == "get_group_list")
        return  std::bind(&shared_state::get_group_list, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5);
    else if (command == "add_group")
        return  std::bind(&shared_state::add_group, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5);
    else if (command == "edit_group")
        return  std::bind(&shared_state::edit_group, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5);
    else if (command == "remove_group")
        return  std::bind(&shared_state::remove_group, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5);
    else if (command == "set_parent")
        return  std::bind(&shared_state::set_parent, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5);
    else if (command == "remove_user")
        return  std::bind(&shared_state::remove_user, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5);
    else if (command == "kill_session")
        return  std::bind(&shared_state::kill_session, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5);
    else if (command == "get_user_status")
        return  std::bind(&shared_state::get_user_status, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5);
    else if (command == "set_app_name")
        return  std::bind(&shared_state::set_app_name, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5);
    else if (command == "exec_query")
        return  std::bind(&shared_state::exec_query, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5);
   else if (command == "get_users_catalog")
        return  std::bind(&shared_state::get_users_catalog, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5);
    else if (command == "get_user_cache")
        return  std::bind(&shared_state::get_user_cache, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5);
    else if (command == "set_user_cache")
        return  std::bind(&shared_state::set_user_cache, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5);
    else if (command == "set_content_type")
        return  std::bind(&shared_state::set_content_type, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5);
    else if (command == "set_message_struct_type")
        return  std::bind(&shared_state::set_message_struct_type, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5);
    else if (command == "get_user_data")
        return  std::bind(&shared_state::get_user_data, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5);
    else if (command == "reset_unread_messages")
        return  std::bind(&shared_state::reset_unread_messages, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5);
   else if (command == "get_unread_messages")
        return  std::bind(&shared_state::get_unread_messages, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5);
   else if (command == "command_to_qt_client")
       return  std::bind(&shared_state::command_to_qt_client, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5);
   else if (command == "command_to_qt_agent")
       return  std::bind(&shared_state::command_to_qt_agent, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5);
   else if (command == "get_webdav_settings")
       return  std::bind(&shared_state::get_webdav_settings, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5);
   else if (command == "set_webdav_settings")
       return  std::bind(&shared_state::set_webdav_settings, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5);
   else if (command == "get_channel_token")
       return  std::bind(&shared_state::get_channel_token, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5);
    else if (command == "set_sql_settings")
        return  std::bind(&shared_state::set_sql_settings, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5);
    else if (command == "export_tables_to_ext")
        return  std::bind(&shared_state::export_tables_to_ext, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5);
    else if (command == "sync_users")
        return  std::bind(&shared_state::sync_users, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5);
    else if (command == "exec_query_qt")
        return  std::bind(&shared_state::exec_query_qt, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5);
    else if (command == "mpl_form_loaded")
        return  std::bind(&shared_state::mpl_form_loaded, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5);
    else if (command == "command_to_client")
        return  std::bind(&shared_state::command_to_client, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5);
    else if (command == "create_pay_html_page")
        return  std::bind(&shared_state::create_pay_html_page, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5);
    else if (command == "set_document_name")
        return  std::bind(&shared_state::set_document_name, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5);
    else
       return nullptr;
}

bool
shared_state::set_client_param(boost::uuids::uuid &uuid, arcirk::bJson* params, ws_message* msg, std::string& err, std::string& custom_result){

    Q_UNUSED(custom_result);

    //Установка основных параметров сессии + авторизация
    try {

        //boost::uuids::uuid new_uuid = params->get_member("uuid").get_uuid();
        std::string name = params->get_member("name").get_string();
        std::string user_name = params->get_member("user_name").get_string();
        std::string host_name = params->get_member("host_name").get_string();
        std::string pwd = params->get_member("pwd").get_string();
        std::string hash = params->get_member("hash").get_string(); //если указан хеш пароль игронится
        std::string device_id = params->get_member("device_id").get_string();
        std::string product = params->get_member("product").get_string();
        bool disable_notify = params->get_member("no_notify").get_bool();
        std::string notify_apps = params->get_member("notify_apps").get_string();
        std::string document_name = params->get_member("document_name").get_string();
        std::string _job = params->get_member("job").get_string();
        boost::uuids::uuid job;
        arcirk::is_valid_uuid(_job, job);

        std::string job_description = params->get_member("job_description").get_string();

        if (hash.empty())
            hash = arcirk::get_hash(name, pwd);

        std::string app_name = params->get_member("app_name").get_string();
        boost::uuids::uuid user_uuid = params->get_member("user_uuid").get_uuid();

        websocket_session* session = get_session(uuid);

        if (!session){
            err = "Требуемая сессия не найдена!";
            return false;
        }

        session->set_name(name);

        std::string role = "user";

        if (!session->authorized){

            if(!sqlite3Db->isOpen()){
                err = "База данных не открыта!";
                return false;
            }

            if (!hash.empty()){

                std::vector<std::map<std::string, std::string>> table;
                err = "";
                std::string dbo = sqlite3Db->is_use_wrapper() ? "dbo." : "";
                std::string query = arcirk::str_sample("select [_id], [Ref], [role], [FirstField] from %1%Users where [hash] = '%2%'", dbo, hash);

                int result = 0;

                result = sqlite3Db->execute(query,
                                            "Users", table, err);

                bool is_valid_name = false;

                if (table.size() > 0){
                    std::string db_name = table[0].at("FirstField");
                    if (arcirk::get_hash(name, "") == arcirk::get_hash(db_name, "") )
                        is_valid_name = true;
                }

                if (result > 0 && is_valid_name){
                    session->authorized = true;
                    role = table[0].at("role");
                    std::string ref = table[0].at("Ref");
                    if (!ref.empty())
                        user_uuid = arcirk::string_to_uuid(ref, true);
                    session->deadline_cancel();
                    session->set_host_name(msg->message().host_name);
                    session->set_user_name(msg->message().user_name);
                }
            }

        }

        if (session->authorized){

            sessions_.erase(uuid);

            //session->set_uuid(new_uuid);
            session->set_user_uuid(user_uuid);
            session->set_app_name(app_name);
            session->set_role(role);
            session->set_user_name(user_name);
            session->set_host_name(host_name);
            session->set_device_id(device_id);
            session->set_product(product);
            session->set_disable_notify(disable_notify);
            session->set_notify_apps(notify_apps);
            session->set_document_name(document_name);
            session->set_job_description(job_description);
            session->set_job(job);

            sessions_.insert(std::pair<boost::uuids::uuid, websocket_session*>(session->get_uuid(), session));

            auto _message = createMessage(session);
            _message.message = "success";
            _message.command = "client_join";

            std::string message = arcirk::ws_message(_message).get_json(true);

            // Оповещаем всех клиентов о регистрации нового клиент
//            std::string m_ = arcirk::str_sample("{\"name\": \"%1%\", \"uuid\": \"%2%\", \"uuid_user\": \"%3%\", \"active\": %4%}",
//                                               session->get_name(),
//                                               boost::to_string(session->get_uuid()),
//                                               boost::to_string(session->get_user_uuid()),
//                                               "true");

//            //используется синхронное подключение, не оповещаем
//            if(app_name != "client_sync")

            if(!disable_notify)
                send(message, "client_join", true, session->notify_apps());

            std::cout << QString("Успешная аутентификация пользователя: ").toLocal8Bit().toStdString() + QString::fromStdString(session->get_name()).toLocal8Bit().toStdString() << std::endl;

            //Добавляем в список собственных сессий пользователя
            auto iter = user_sessions.find(session->get_user_uuid());
            if (iter == user_sessions.end() ){
                std::vector<websocket_session*> vec;
                vec.push_back(session);
                user_sessions.insert(std::pair<boost::uuids::uuid, std::vector<websocket_session*>>(session->get_user_uuid(), vec));
                std::cout << QString("Добавляем в список собственных сессий").toLocal8Bit().toStdString() << std::endl;
            }else{
                iter->second.push_back(session);
                std::cout << QString("Обновляем в список собственных сессий").toLocal8Bit().toStdString() << std::endl;
            }

            return true;
        }else
            err = "Отказано в доступе";

        return false;

    } catch(std::exception& e){
        err = QString::fromLocal8Bit(e.what()).toStdString();
        return false;
    }
}

bool shared_state::get_active_users(boost::uuids::uuid &uuid, arcirk::bJson* params, ws_message* msg, std::string& err, std::string& custom_result) {

    std::string filter_app_name;
    bool unique = false;
    bool table = false;

    Q_UNUSED(uuid);
    Q_UNUSED(msg);
    Q_UNUSED(err);

    if(params->get_member_count() > 0){
        bVariant val;
        if (params->getMember("app_name", val)){
            filter_app_name = val.get_string();
        }
        if (params->getMember("unique", val)){
            unique = val.get_bool();
        }
        if (params->getMember("table", val)){
            table = val.get_bool();
        }
    }

    arcirk::T_vec apps;
    if(!filter_app_name.empty()){
       apps = arcirk::split(std::move(filter_app_name), "|");
    }

    arcirk::bJson json_arr{};
    json_arr.set_array();

    if(!unique){
        for (auto p : sessions_){

            if(apps.size() > 0){
                if(std::find(apps.begin(), apps.end(), p.second->get_app_name()) == apps.end())
                    continue;
            }

            _Value doc(rapidjson::Type::kObjectType);
            json_arr.addMember(&doc, arcirk::content_value("Empty", ""));
            json_arr.addMember(&doc, arcirk::content_value("uuid", to_string(p.second->get_uuid())));
            json_arr.addMember(&doc, arcirk::content_value("name", p.second->get_name()));
            json_arr.addMember(&doc, arcirk::content_value("user_uuid", to_string(p.second->get_user_uuid())));
            json_arr.addMember(&doc, arcirk::content_value("app_name", p.second->get_app_name()));
            json_arr.addMember(&doc, arcirk::content_value("user_name", p.second->get_user_name()));
            json_arr.addMember(&doc, arcirk::content_value("ip_address", p.second->ip_address()));
            json_arr.addMember(&doc, arcirk::content_value("host_name", p.second->host_name()));
            json_arr.addMember(&doc, arcirk::content_value("device_id", p.second->get_device_id()));
            json_arr.addMember(&doc, arcirk::content_value("product", p.second->get_product()));
            json_arr.addMember(&doc, arcirk::content_value("document_name", p.second->document_name()));
            json_arr.addMember(&doc, arcirk::content_value("job", to_string(p.second->job())));
            json_arr.addMember(&doc, arcirk::content_value("job_description", p.second->job_description()));
            json_arr.push_back(doc) ;
        }
    }else{
        std::map<boost::uuids::uuid, std::vector<content_value>> users;
        for (auto p : sessions_){
            boost::uuids::uuid uuid_ = p.second->get_user_uuid();
            if (users.find(uuid_) == users.end())
                continue;
            if(!filter_app_name.empty()){
                if (filter_app_name != p.second->get_app_name())
                    continue;
            }
            std::vector<content_value> object;
            object.emplace_back("Empty", "");
            object.emplace_back("uuid", to_string(p.second->get_uuid()));
            object.emplace_back(arcirk::content_value("name", p.second->get_name()));
            object.emplace_back(arcirk::content_value("user_uuid", to_string(p.second->get_user_uuid())));
            object.emplace_back(arcirk::content_value("app_name", p.second->get_app_name()));
            object.emplace_back(arcirk::content_value("user_name", p.second->get_user_name()));
            object.emplace_back(arcirk::content_value("ip_address", p.second->ip_address()));
            object.emplace_back(arcirk::content_value("host_name", p.second->host_name()));
            object.emplace_back(arcirk::content_value("device_id", p.second->get_device_id()));
            object.emplace_back(arcirk::content_value("product", p.second->get_product()));
            object.emplace_back(arcirk::content_value("document_name", p.second->document_name()));
            object.emplace_back(arcirk::content_value("job", to_string(p.second->job())));
            object.emplace_back(arcirk::content_value("job_description", p.second->job_description()));
            users.insert(std::pair<boost::uuids::uuid, std::vector<content_value>>(uuid_, object));
        }

        for(const auto& itr : users){
            _Value doc(rapidjson::Type::kObjectType);
            for(const auto& p : itr.second){
                json_arr.addMember(&doc, arcirk::content_value(p.key, p.value));
            }
            json_arr.push_back(doc) ;
        }

    }

    if(!table)
        custom_result = json_arr.to_string();
    else{
        auto mainDoc = arcirk::bJson();
        mainDoc.set_object();
        //_Value rows = json_arr.to_array(); //json_arr.GetArray();
        //mainDoc.addMember("rows", rows);
        mainDoc.AddMember("rows", json_arr.GetArray(), mainDoc.GetAllocator());
        _Value cols(rapidjson::Type::kArrayType);
        cols.PushBack("Empty", mainDoc.GetAllocator());
        cols.PushBack("uuid", mainDoc.GetAllocator());
        cols.PushBack("name", mainDoc.GetAllocator());
        cols.PushBack("user_uuid", mainDoc.GetAllocator());
        cols.PushBack("app_name", mainDoc.GetAllocator());
        cols.PushBack("user_name", mainDoc.GetAllocator());
        cols.PushBack("ip_address", mainDoc.GetAllocator());
        cols.PushBack("host_name", mainDoc.GetAllocator());
        cols.PushBack("device_id", mainDoc.GetAllocator());
        cols.PushBack("product", mainDoc.GetAllocator());
        cols.PushBack("document_name", mainDoc.GetAllocator());
        cols.PushBack("job", mainDoc.GetAllocator());
        cols.PushBack("job_description", mainDoc.GetAllocator());
        mainDoc.addMember("columns", cols);
        custom_result = mainDoc.to_string();
    }

    return true;

}

bool shared_state::notify(boost::uuids::uuid &uuid, arcirk::bJson* params, ws_message* msg, std::string& err, std::string& custom_result) {

    auto  current_sess = get_session(uuid);

    try {
        current_sess->throw_authorized();
    }catch (boost::exception const &e) {
        err = boost::diagnostic_information(e);
        std::cerr << err << std::endl;
        return false;

    }

    std::string message = params->get_member("message").get_string();    
    std::string apps = params->get_member("app_name").get_string();
    std::string param = params->get_member("param").get_string();

    if (message.empty()){
        err = "Передано пустое оповещение!";
        return false;
    }
    Q_UNUSED(custom_result);
    Q_UNUSED(msg);

    send(message, "notify", true, apps, arcirk::uuid_to_string(uuid), param);

    return true;

}

bool shared_state::mpl_form_loaded(boost::uuids::uuid &uuid, arcirk::bJson* params, ws_message* msg, std::string& err, std::string& custom_result) {

    auto  current_sess = get_session(uuid);

    try {
        current_sess->throw_authorized();
    }catch (boost::exception const &e) {
        err = boost::diagnostic_information(e);
        std::cerr << err << std::endl;
        return false;

    }

    _ws_message _message = createMessage(current_sess);
    _message.message = "sucsess";
    _message.command = "mpl_form_loaded";

    std::string message = arcirk::ws_message(_message).get_json(true);

    std::vector<boost::weak_ptr<websocket_session>> v;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        v.reserve(get_sessions(current_sess->get_user_uuid()).size());
        for(auto p : get_sessions(current_sess->get_user_uuid()))
            v.emplace_back(p->weak_from_this());
    }

    auto const ss = boost::make_shared<std::string const>(std::move(message));

    for(auto const& wp : v)
        if(auto sp = wp.lock()){
            std::cout << "shared_state::mpl_form_loaded: " << sp->get_app_name() << std::endl;
            if(sp->get_app_name() == "qt_cert_manager")
                sp->send(ss);
        }

    Q_UNUSED(custom_result);
    Q_UNUSED(params);
    Q_UNUSED(msg);

    return true;

}
bool shared_state::is_msg(const std::string& msg)
{
    std::string cmd = msg.substr(0, 3);

    if (cmd == "msg")
    {
        return true;
    }

    return false;
}

bool
shared_state::send_message(const std::string &message, boost::uuids::uuid &recipient, websocket_session *current_sess) {

    if (recipient == boost::uuids::nil_uuid()){
        //отправить всем (общий чат)
        send(message, "message", false);
    } else
    {

       std::string ref;

       std::string filter_app_name;
        if (current_sess->get_app_name() == "qt_client")
            filter_app_name = "qt_client";

       bool active = get_user_status_(recipient, filter_app_name);

       bool result = false;

       result = sqlite3Db->save_message(message, current_sess->get_user_uuid(), recipient, ref, active,
                                             current_sess->get_name());

        if (!result)
            return false;

        //////////
        std::string struct_type = current_sess->get_message_struct_type();
        std::string tmp_message = message;
        std::string dbo = sqlite3Db->is_use_wrapper() ? "dbo." : "";
        std::string query = arcirk::str_sample("select * from %1%Messages where [Ref] = '%2%'", dbo, ref);

        std::string err;
        result = (bool)sqlite3Db->execute(query, "Messages", tmp_message, err, true);
        if (!result)
            tmp_message = message;
        else{
            tmp_message = arcirk::base64_encode(tmp_message);
        }
        _ws_message _message = createMessage(current_sess);
        _message.message = tmp_message;
        _message.command = "message";
        _message.uuid_channel = recipient;

        std::string cur_session_db_msg = arcirk::ws_message(_message).get_json(true);

        std::string rec_session_db_msg = "";
        if(get_sessions(recipient).size() > 0){
            //заполняем по первой доступной сессии
            websocket_session * session_recipient = get_sessions(recipient)[0];
            _message = createMessage(session_recipient);
            _message.message = tmp_message;
            _message.command = "message";
            _message.uuid_channel = current_sess->get_user_uuid();
            _message.channel_name = current_sess->get_name();
            rec_session_db_msg = arcirk::ws_message(_message).get_json(true);
        } else
            rec_session_db_msg = cur_session_db_msg;
        //////////////////



        //отправляем сообщение во все сессии получателя
        //recipient - идентификатор пользователя
        //найдем все сессии с этим идентификатором
        //также дублируем сообщение во все текущие сессии

        std::vector<boost::weak_ptr<websocket_session>> v;
        {
            std::lock_guard<std::mutex> lock(mutex_);
            v.reserve(get_sessions(recipient).size() + get_sessions(current_sess->get_user_uuid()).size());
            for(auto p : get_sessions(recipient))
                v.emplace_back(p->weak_from_this());
            for(auto p : get_sessions(current_sess->get_user_uuid()))
                v.emplace_back(p->weak_from_this());
        }

        auto const ss = boost::make_shared<std::string const>(std::move(message));
        auto const ss_db_cur = boost::make_shared<std::string const>(std::move(cur_session_db_msg));
        auto const ss_db_rec = boost::make_shared<std::string const>(std::move(rec_session_db_msg));

        for(auto const& wp : v)
            if(auto sp = wp.lock()){
                if(sp->get_message_struct_type() == "DB"){
                    if(recipient == sp->get_user_uuid())
                        sp->send(ss_db_rec);
                    else
                        sp->send(ss_db_cur);
                }else
                    sp->send(ss);
            }
    }

    return true;
}

bool shared_state::is_valid_message(const std::string &message, boost::uuids::uuid &uuid_recipient, std::string &base64,
                                    std::string &err) {

    std::string _message = message;

    if (message.substr(message.length() - 1, message.length()) == "\n"){
        _message = message.substr(0, message.length()  - 1) ;
    }

    arcirk::T_vec v = arcirk::split(std::move(_message), " ");


    if (v.size() != 3){
        err = "request format error!";
        return false;
    }

    std::string _uuid_recipient = v[1];
    base64 = v[2];

    if (_uuid_recipient != "00000000-0000-0000-0000-000000000000"){
        if (!arcirk::is_valid_uuid(_uuid_recipient, uuid_recipient)){
            err = "error uuid recipient!";
            return false;
        }
    } else
        uuid_recipient = boost::uuids::nil_uuid();

    return true;
}

bool shared_state::add_new_user(boost::uuids::uuid &uuid, arcirk::bJson* params, ws_message *msg,
                                std::string &err, std::string &custom_result) {

    err = "";

    _add_new_user(
            params->get_member("user").get_string(), //user
            params->get_member("pwd").get_string(), //pwd
            params->get_member("role").get_string(), //role
            params->get_member("uuid").to_string(), //uuid
            params->get_member("presentation").get_string(), //presentation
            params->get_member("parent").get_string(), //
            err,
            false);
    if (!err.empty() && err != "no error")
        return false;
    else
        return true;
}

bool shared_state::get_db_users(boost::uuids::uuid &uuid, arcirk::bJson* params, ws_message *msg,
                                std::string &err, std::string &custom_result) {

    std::string channel = params->get_member("channel").get_string();
    std::string dbo = sqlite3Db->is_use_wrapper() ? "dbo." : "";
    std::string query = arcirk::str_sample("SELECT * FROM %1%Users", dbo);

    if (!channel.empty()){
        query += " WHERE channel = '" + channel + "'";
    }

    err = "";
    sqlite3Db->execute(query, "Users", custom_result, err);

    if (!err.empty() && err != "no error"){
        return false;
    }

    return true;
}

bool shared_state::update_user(boost::uuids::uuid &uuid, arcirk::bJson* params, ws_message *msg,
                               std::string &err, std::string &custom_result) {

    std::string dbo = sqlite3Db->is_use_wrapper() ? "dbo." : "";
    std::string query = arcirk::str_sample("UPDATE %1%Users SET ", dbo);

    try {
        std::vector<arcirk::content_value> m_sets;
        if (!params->getMembers("set", m_sets)) {
            err = "error parse parameters command";
            return false;
        }
        std::vector<arcirk::content_value> m_where;
        if (!params->getMembers("where", m_where)) {
            err = "error parse parameters command";
            return false;
        }

        for (auto itr = m_sets.begin(); itr < m_sets.end(); itr++){
            std::string tmp = itr->key;
            tmp.append(" = '");
            tmp.append(itr->value.get_string());
            tmp.append("'");
            query.append(" " + tmp);
            if (itr != --m_sets.end())
                query.append(",");

        }

        query.append(" WHERE ");

        for (auto itr = m_where.begin(); itr < m_where.end(); itr++){
            std::string tmp = itr->key;
            tmp.append(" = '");
            tmp.append(itr->value.get_string());
            tmp.append("'");
            query.append(" " + tmp);
            if (itr != --m_where.end())
                query.append(" AND ");
        }

    }catch (std::exception& e){
        err = e.what();
        return false;
    }

    err = "";
    sqlite3Db->exec(query, err);

    if (err.empty() || err != "no error")
        return true;
    else
        return false;
}

bool shared_state::get_messages(boost::uuids::uuid &uuid, arcirk::bJson* params, ws_message *msg,
                                std::string &err, std::string &custom_result) {

    auto  current_sess = get_session(uuid);

    try {
        current_sess->throw_authorized();
    }catch (boost::exception const &e) {
        err = boost::diagnostic_information(e);
        std::cerr << err << std::endl;
        return false;

    }

    boost::uuids::uuid recipient{};
    std::string _recipient = params->get_member("recipient").to_string();
    if (!arcirk::is_valid_uuid(_recipient, recipient)){
        err = "Не действительный идентификатор подписчика!";
        return false;
    }

    std::string token = sqlite3Db->get_channel_token(get_session(uuid)->get_user_uuid(), recipient);

    if (token.empty() || token == "error"){
        err = "Ошибка получения токена сообщений!";
        return false;
    }

    int limit = params->get_member("limit").get_int();
    int start_date = params->get_member("start_date").get_int();
    int end_date = params->get_member("end_date").get_int();


    std::string json;

    err = "";

    std::map<std::string, arcirk::bVariant> fields;
    fields.insert(std::pair<std::string, arcirk::bVariant>("uuid", arcirk::uuid_to_string(uuid)));
    fields.insert(std::pair<std::string, arcirk::bVariant>("uuid_recipient", _recipient));
    fields.insert(std::pair<std::string, arcirk::bVariant>("token", token));

    int result = sqlite3Db->get_save_messages(json, token, err, limit, start_date, end_date, fields);

    if (result > 0){
        custom_result = json;
    } else{
        std::vector<std::string> arr;
        sqlite3Db->get_columns_arr("Messages", arr);
        auto obj_json = arcirk::bJson();
        obj_json.set_object();
        _Value _header(rapidjson::Type::kArrayType);
        _header.SetArray();
        for(auto column : arr){
            arcirk::bVariant column_name = column;
            obj_json.push_back(_header, column_name);
        }
        _Value _rows(rapidjson::Type::kArrayType);
        _rows.SetArray();
        arcirk::bVariant uuid_var = arcirk::uuid_to_string(uuid);
        obj_json.addMember("uuid", uuid_var);
        arcirk::bVariant uuid_recipient_var = _recipient;
        obj_json.addMember("uuid_recipient", uuid_recipient_var);
        obj_json.addMember("token", token);
        obj_json.addMember("columns", _header);
        obj_json.addMember("rows", _rows);
        custom_result = obj_json.to_string();
    }

    if (!err.empty() && err != "no error")
        return false;

    msg->message().uuid_channel = recipient;

    return true;

}

bool shared_state::get_user_info(boost::uuids::uuid &uuid, arcirk::bJson* params, ws_message *msg,
                                 std::string &err, std::string &custom_result) {


    boost::uuids::uuid user_uuid = params->get_member("uuid_user").get_uuid();

    if (user_uuid != arcirk::nil_uuid()){

        std::string dbo = sqlite3Db->is_use_wrapper() ? "dbo." : "";
        std::string query = arcirk::str_sample("select * from %1%Users where [Ref] = '", dbo) + to_string(user_uuid) + "';";
        std::vector<std::map<std::string , std::string>> row;
        std::string err = "";

        auto json = new arcirk::bJson();
        json->set_object();

        int result = sqlite3Db->execute(query, "Users", row, err);

        if (result > 0){
            json->addMember(arcirk::content_value("name", row[0].at("FirstField")));
            json->addMember(arcirk::content_value("performance", row[0].at("SecondField")));
            json->addMember(arcirk::content_value("uuid", row[0].at("Ref")));
            get_user_subdivision(to_string(user_uuid), json);
            get_user_department(to_string(user_uuid), json);
        } else
            json->addMember(arcirk::content_value("name", "не зарегистрирован"));

        //websocket_session* session = nullptr;

        auto sessions = get_sessions(user_uuid);

        if (sessions.size() > 0){
            json->addMember(arcirk::content_value("active", true));
        } else
            json->addMember(arcirk::content_value("active", false));

        custom_result = json->to_string();
        delete json;
    } else{
        err = "Ошибка получения данных пользователя!";
        return false;
    }

    return true;
}

bool shared_state::get_group_list(boost::uuids::uuid &uuid, arcirk::bJson* params, ws_message *msg,
                                  std::string &err, std::string &custom_result){
    std::string dbo = sqlite3Db->is_use_wrapper() ? "dbo." : "";
    std::string query = arcirk::str_sample("SELECT [SecondField], [FirstField], [Ref], [Parent] FROM %1%Channels;", dbo);
    err = "";
    sqlite3Db->execute(query, "Channels", custom_result, err);

    if (!err.empty() && err != "no error"){
        return false;
    }

    return true;
}

bool shared_state::add_group(boost::uuids::uuid &uuid, arcirk::bJson* params, ws_message *msg,
                             std::string &err, std::string &custom_result) {

    auto  current_sess = get_session(uuid);

    try {
        current_sess->throw_authorized();
    }catch (boost::exception const &e) {
        err = boost::diagnostic_information(e);
        std::cerr << err << std::endl;
        return false;

    }

    if (current_sess->get_role() != "admin"){
        err = "не достаточно прав доступа для команды!";
        return false;
    }

    std::string group_name = params->get_member("name").get_string();
    std::string group_presentation = params->get_member("presentation").get_string();
    std::string group_parent = params->get_member("parent").get_string();
    boost::uuids::uuid _uuid = boost::uuids::random_generator()();

    std::string _group_parent;
    if (group_parent.empty()){
        _group_parent = arcirk::nil_string_uuid();
    } else
        _group_parent = group_parent;

    std::vector<arcirk::content_value> fields;

    fields.emplace_back(arcirk::content_value("FirstField", group_name));
    fields.emplace_back(arcirk::content_value("SecondField", group_presentation));
    fields.emplace_back(arcirk::content_value("Ref", arcirk::uuid_to_string(_uuid)));
    fields.emplace_back(arcirk::content_value("Parent", _group_parent));

    err = "";
    sqlite3Db->insert(arc_sqlite::tables::eChannels, fields, err, "");

    if (!err.empty() && err != "no error")
        return false;
    else{
        auto * obj = new arcirk::bJson();
        obj->set_object();
        obj->addObject(fields);
        custom_result = obj->to_string();
        delete obj;
        return true;
    }

}
bool shared_state::edit_group(boost::uuids::uuid &uuid, arcirk::bJson* params, ws_message *msg,
                              std::string &err, std::string &custom_result) {
    auto  current_sess = get_session(uuid);

    try {
        current_sess->throw_authorized();
    }catch (boost::exception const &e) {
        err = boost::diagnostic_information(e);
        std::cerr << err << std::endl;
        return false;

    }

    if (current_sess->get_role() != "admin"){
        err = "не достаточно прав доступа для команды!";
        return false;
    }

    std::string group_name = params->get_member("name").get_string();
    std::string group_presentation = params->get_member("presentation").get_string();
    std::string group_parent = params->get_member("parent").get_string();
    std::string _uuid = params->get_member("ref").get_string();

    if (_uuid.empty()){
        err = "не указан идентификатор группы!";
        return false;
    }

    std::vector<arcirk::content_value> _sets;

    _sets.emplace_back(arcirk::content_value("FirstField", group_name));
    _sets.emplace_back(arcirk::content_value("SecondField", group_presentation));
    _sets.emplace_back(arcirk::content_value("Parent", group_parent));

    std::vector<arcirk::content_value> _where;
    _where.emplace_back(arcirk::content_value("Ref", _uuid));

    err = "";
    sqlite3Db->update(arc_sqlite::tables::eChannels, _sets, _where, err);

    if (!err.empty()  && err != "no error")
        return false;
    else{
        _sets.emplace_back(arcirk::content_value("Ref", _uuid));
        auto * obj = new arcirk::bJson();
        obj->set_object();
        obj->addObject(_sets);
        custom_result = obj->to_string();
        delete obj;
        return true;
    }
}

bool shared_state::remove_group(boost::uuids::uuid &uuid, arcirk::bJson* params, ws_message *msg,
                                std::string &err, std::string &custom_result)  {
    auto  current_sess = get_session(uuid);

    try {
        current_sess->throw_authorized();
    }catch (boost::exception const &e) {
        err = boost::diagnostic_information(e);
        std::cerr << err << std::endl;
        return false;

    }

    if (current_sess->get_role() != "admin"){
        err = "не достаточно прав доступа для команды!";
        return false;
    }

    std::string _uuid = params->get_member("ref").get_string();
    if (_uuid.empty()){
        err = "не указан идентификатор группы!";
        return false;
    }

    //удаляем группу вместе с подчиненными элементами
    //пользователей перемещаем в root группу
    remove_group_hierarchy(_uuid, err);
    if(!err.empty() && err != "no error")
        return false;
    custom_result = params->to_string();

    return true;
}

void shared_state::remove_group_hierarchy(const std::string &current_uuid, std::string &err) {

    std::set<int> m_idList;
    std::string dbo = sqlite3Db->is_use_wrapper() ? "dbo." : "";
    std::string query = arcirk::str_sample("SELECT _id FROM %1%Channels WHERE [Ref] = '", dbo) + current_uuid + "';";
    std::vector<std::map<std::string, arcirk::bVariant>> table;

    //Получаем _id верхнего узла иерархии по uuid и добавляем его в общий список set
    err = "";
    int result = sqlite3Db->execute(query, "Channels", table, err);

    if (!err.empty() && err != "no error"){
        return;
    }
    if (result > 0){
        std::map<std::string, arcirk::bVariant> row = table[0];
        if(row["_id"].is_int())
            m_idList.insert(row["_id"].get_int());
    }

    if (m_idList.empty()){
        err = "Группа в списке не найдена!";
        return;
    }

    //собираем идентификаторы всей иерархии рекурсивно
    get_group_hierarchy_keys(current_uuid, m_idList, err);

    //подготавливаем список ключей для перемещения пользователей в группу root
    query = arcirk::str_sample("SELECT %1%Users._id FROM %1%Users INNER JOIN %1%Channels ON %1%Users.channel = %1%Channels.Ref WHERE %1%Channels._id IN (", dbo);

    for (auto itr = m_idList.begin(); itr != m_idList.end() ; ++itr) {
        query.append(std::to_string(*itr));
        if (itr !=  --m_idList.end())
            query.append(",");
    }
    query.append(");");
    std::vector<std::map<std::string, arcirk::bVariant>> tableUsers;
    err = "";
    sqlite3Db->execute(query, "Users", tableUsers, err);
    if (!err.empty() && err != "no error"){
        return;
    }

    //Перемещаем
    query = arcirk::str_sample("UPDATE %1%Users SET channel = '" + arcirk::nil_string_uuid() + "' WHERE _id in (", dbo);

    for (auto itr = tableUsers.begin(); itr != tableUsers.end() ; ++itr) {
        std::map<std::string, arcirk::bVariant> row = *itr;
        if(row["_id"].is_int())
            query.append(std::to_string(row["_id"].get_int()));
        if (itr !=  --tableUsers.end())
            query.append(",");
    }
    query.append(");");
    err = "";
    sqlite3Db->exec(query, err);

   // std::cout << query << std::endl;

    if (!err.empty() && err != "no error"){
        return;
    }

    //удаляем группы
    query = arcirk::str_sample("DELETE FROM %1%Channels WHERE _id in (", dbo);
    for (auto itr = m_idList.begin(); itr != m_idList.end() ; ++itr) {
        query.append(std::to_string(*itr));
        if (itr !=  --m_idList.end())
            query.append(",");
    }
    query.append(");");
    err = "";
    sqlite3Db->exec(query, err);

}

void shared_state::get_group_hierarchy_keys(const std::string &current_uuid, std::set<int> &vec_uuid, std::string &err) {

    std::string dbo = sqlite3Db->is_use_wrapper() ? "dbo." : "";
    std::string query = arcirk::str_sample("select [_id], [Ref] from %1%Channels where [Parent] = '", dbo) + current_uuid + "';";

    std::vector<std::map<std::string, arcirk::bVariant>> table;
    err = "";
    int result = sqlite3Db->execute(query, "Channels", table, err);

    if (result > 0){
        std::map<std::string, arcirk::bVariant> row = table[0];
        if(row["_id"].is_int())
            vec_uuid.insert(row["_id"].get_int());

        std::string parent = row["Ref"].get_string();
        get_group_hierarchy_keys(parent, vec_uuid, err);
    }

}

bool shared_state::set_parent(boost::uuids::uuid &uuid, arcirk::bJson *params, ws_message *msg,
                              std::string &err, std::string &custom_result) {

    auto  current_sess = get_session(uuid);

    try {
        current_sess->throw_authorized();
    }catch (boost::exception const &e) {
        err = boost::diagnostic_information(e);
        std::cerr << err << std::endl;
        return false;

    }

    if (current_sess->get_role() != "admin"){
        err = "не достаточно прав доступа для команды!";
        return false;
    }
    std::string _uuid = params->get_member("user").get_string();
    if (_uuid.empty()){
        err = "не указан идентификатор пользователя!";
        return false;
    }

    std::string _parent = params->get_member("parent").get_string();
    if (_uuid.empty()){
        _parent = arcirk::nil_string_uuid();
    }
    std::string dbo = sqlite3Db->is_use_wrapper() ? "dbo." : "";
    std::string query = arcirk::str_sample("UPDATE %1%Users\n"
                        " SET [channel] = '" + _parent + "'\n"
                        " WHERE [Ref] = '" + _uuid + "';", dbo);

    err = "";
    sqlite3Db->exec(query, err);
    if(!err.empty() && err != "no error")
        return false;

    custom_result = params->to_string();

    return true;
}

bool shared_state::remove_user(boost::uuids::uuid &uuid, arcirk::bJson *params, ws_message *msg,
                               std::string &err, std::string &custom_result) {

    auto  current_sess = get_session(uuid);

    try {
        current_sess->throw_authorized();
    }catch (boost::exception const &e) {
        err = boost::diagnostic_information(e);
        std::cerr << err << std::endl;
        return false;

    }

    if (current_sess->get_role() != "admin"){
        err = "не достаточно прав доступа для команды!";
        return false;
    }

    std::string _uuid = params->get_member("ref").get_string();
    if (_uuid.empty()){
        err = "не указан идентификатор пользователя!!";
        return false;
    }

    std::string dbo = sqlite3Db->is_use_wrapper() ? "dbo." : "";
    std::string query = arcirk::str_sample("DELETE FROM %1%Users\n"
                        "WHERE [Ref] = '" + _uuid + "';", dbo);

    err = "";
    sqlite3Db->exec(query, err);
    if(!err.empty() && err != "no error")
        return false;

    custom_result = params->to_string();

    return true;
}

bool shared_state::kill_session(boost::uuids::uuid &uuid, arcirk::bJson *params, ws_message *msg,
                                std::string &err, std::string &custom_result){

    auto  current_sess = get_session(uuid);

    try {
        current_sess->throw_authorized();
    }catch (boost::exception const &e) {
        err = boost::diagnostic_information(e);
        std::cerr << err << std::endl;
        return false;

    }

    if (current_sess->get_role() != "admin"){
        err = "не достаточно прав доступа для команды!";
        return false;
    }

    std::string _uuid = params->get_member("ref").get_string();
    if (_uuid.empty()){
        err = "не указан идентификатор пользователя!!";
        return false;
    }

    boost::uuids::uuid kill_uuid = arcirk::string_to_uuid(_uuid, false);
    auto  kill_sess = get_session(kill_uuid);
    if (kill_sess)
        kill_sess->close();
    else
        return false;

    return true;

}

bool shared_state::set_app_name(boost::uuids::uuid &uuid, arcirk::bJson *params, ws_message *msg,
                                std::string &err, std::string &custom_result) {
    auto  current_sess = get_session(uuid);

    try {
        current_sess->throw_authorized();
    }catch (boost::exception const &e) {
        err = boost::diagnostic_information(e);
        std::cerr << err << std::endl;
        return false;

    }
    if (current_sess->get_role() != "admin"){
        err = "не достаточно прав доступа для команды!";
        return false;
    }

    std::string _uuid = params->get_member("uuid_set").get_string();
    if (_uuid.empty()){
        err = "не указан идентификатор сессии!!";
        return false;
    }

    boost::uuids::uuid uuid_set = arcirk::string_to_uuid(_uuid, false);

    std::string new_app_name = params->get_member("new_app_name").get_string();
    if (new_app_name.empty())
        return false;

    if (uuid != uuid_set){
        auto  session_set = get_session(uuid_set);
        if (session_set)
            session_set->set_app_name(new_app_name);
        else
            return false;
    } else
        current_sess->set_app_name(new_app_name);

    return true;

}
bool shared_state::exec_query_qt(boost::uuids::uuid &uuid, arcirk::bJson *params, ws_message *msg,
                              std::string &err, std::string &custom_result) {

    auto  current_sess = get_session(uuid);

    try {
        current_sess->throw_authorized();
    }catch (boost::exception const &e) {
        err = boost::diagnostic_information(e);
        std::cerr << err << std::endl;
        return false;

    }

    std::string query = params->get_member("query").get_string();
    std::string id_command = params->get_member("id_command").get_string();
    std::string run_on_return = params->get_member("run_on_return").get_string();
    bool isTable = params->get_member("table").get_bool();
    err = "";
    if (query.empty())
        return false;   //std::string szResult;


    if(!sqlite3Db->isOpen()){
        err = "База данных не открыта!";
        return false;
    }

    std::string table;
    bool result;
    if(isTable)
        result = sqlite3Db->exec_qt(query, table, err);
    else
        result = sqlite3Db->exec_qt(query, err);

    if(result){
        custom_result = "Запрос успешно выполнен!";
    }

    if (err.empty() || err == "no error"){
        //std::string base64 = arcirk::base64_encode(szResult);
        auto json = arcirk::bJson();
        json.set_object();
        json.addMember("id_command", id_command);
        json.addMember("run_on_return", run_on_return);
        json.addMember("table", table);

        custom_result = json.to_string();
    }

    return result;

}

bool shared_state::exec_query(boost::uuids::uuid &uuid, arcirk::bJson *params, ws_message *msg,
                              std::string &err, std::string &custom_result) {

    auto  current_sess = get_session(uuid);

    try {
        current_sess->throw_authorized();
    }catch (boost::exception const &e) {
        err = boost::diagnostic_information(e);
        std::cerr << err << std::endl;
        return false;

    }

    std::string query = params->get_member("query").get_string();
    bool header = params->get_member("header").get_bool();
    std::string id_command = params->get_member("id_command").get_string();
    std::string run_on_return = params->get_member("run_on_return").get_string();

    if (query.empty())
        return false;

    if(!sqlite3Db->isOpen()){
        err = "База данных не открыта!";
        return false;
    }

    err = "";
    std::string szResult;
    sqlite3Db->execute(query, "", szResult, err, header);

    if(id_command.length() == 0){
        if (err.empty() || err == "no error"){
            custom_result =  arcirk::base64_encode(szResult);
        }
        else
            return false;
    }else
    {
        if (err.empty() || err == "no error"){
            std::string base64 = arcirk::base64_encode(szResult);
            auto json = arcirk::bJson();
            json.set_object();
            json.addMember("id_command", id_command);
            json.addMember("table", base64);
            json.addMember("run_on_return", run_on_return);
            custom_result = json.to_string();
        }
        else{
            return false;
        }
    }

    return true;

}

bool shared_state::get_users_catalog(boost::uuids::uuid &uuid, arcirk::bJson *params, ws_message *msg,
                                     std::string &err, std::string &custom_result) {

    auto  current_sess = get_session(uuid);

    try {
        current_sess->throw_authorized();
    }catch (boost::exception const &e) {
        err = boost::diagnostic_information(e);
        std::cerr << err << std::endl;
        return false;

    }

    std::string dbo = sqlite3Db->is_use_wrapper() ? "dbo." : "";
    std::string query = arcirk::str_sample("select * from %1%UsersCatalog;", dbo);
    err = "";
    std::string szResult;
    sqlite3Db->execute(query, "UsersCatalog", szResult, err, true);
    if (err.empty()  || err == "no error")
        custom_result = szResult;
    else
        return false;

    return true;
}

bool shared_state::get_user_cache(boost::uuids::uuid &uuid, arcirk::bJson *params, ws_message *msg, std::string &err,
                                   std::string &custom_result) {

    auto  current_sess = get_session(uuid);

    try {
        current_sess->throw_authorized();
    }catch (boost::exception const &e) {
        err = boost::diagnostic_information(e);
        std::cerr << err << std::endl;
        return false;

    }

    std::string _uuid = arcirk::uuid_to_string(current_sess->get_user_uuid());
    if (_uuid.empty() || _uuid == arcirk::nil_string_uuid()){
        err = "не указан идентификатор пользователя!!";
        return false;
    }

    std::string dbo = sqlite3Db->is_use_wrapper() ? "dbo." : "";
    std::string query = arcirk::str_sample("SELECT [cache] FROM %1%Users WHERE [Ref] = '%2%'", dbo, _uuid);

    std::vector<std::map<std::string, arcirk::bVariant>> table;
    err = "";

    int result = sqlite3Db->execute(query, "Users", table, err);

    if (!err.empty() && err != "no error"){
        custom_result = arcirk::base64_encode("{\"currentRow\":\"\",\"chats\":{ \"columns\":[\"uuid\",\"name\",\"draft\"], \"rows\":[]}}");
    }
    if (result > 0){
        std::string _cache = table[0]["cache"].get_string();
        if(_cache.empty())
            custom_result = arcirk::base64_encode("{\"currentRow\":\"\",\"chats\":{ \"columns\":[\"uuid\",\"name\",\"draft\"], \"rows\":[]}}");
        else
            custom_result = _cache;
    } else
        custom_result = arcirk::base64_encode("{\"currentRow\":\"\",\"chats\":{ \"columns\":[\"uuid\",\"name\",\"draft\"], \"rows\":[]}}");

    return true;
}

bool shared_state::set_user_cache(boost::uuids::uuid &uuid, arcirk::bJson *params, ws_message *msg, std::string &err,
                                  std::string &custom_result) {

    auto  current_sess = get_session(uuid);

    try {
        current_sess->throw_authorized();
    }catch (boost::exception const &e) {
        err = boost::diagnostic_information(e);
        std::cerr << err << std::endl;
        return false;

    }

    std::string _uuid = arcirk::uuid_to_string(current_sess->get_user_uuid());
    if (_uuid.empty()){
        err = "не указан идентификатор пользователя!!";
        return false;
    }

    std::string cache = params->get_member("cache").get_string();
    std::string dbo = sqlite3Db->is_use_wrapper() ? "dbo." : "";
    std::string query = arcirk::str_sample("UPDATE %1%Users SET [cache] = '%2%' WHERE [Ref] = '%3%'", dbo, cache, _uuid);

    err = "";
    sqlite3Db->exec(query, err);
    if(!err.empty() && err != "no error")
        return false;

    custom_result = params->to_string();

    return true;
}

void shared_state::get_user_subdivision(const std::string &user_ref, arcirk::bJson *jsonObj) {

    std::string dbo = sqlite3Db->is_use_wrapper() ? "dbo." : "";
    std::string sample = "SELECT [channel] FROM %1%Users WHERE %2% = '%3%'";
    std::string query = arcirk::str_sample(sample, dbo, "Ref", user_ref);
    std::string err = "";
    std::vector<std::map<std::string, arcirk::bVariant>> table;
    err = "";

    int result = sqlite3Db->execute(query, "Users", table, err);

    if (!err.empty() && err != "no error"){
        return;
    }
    if (result > 0){
        std::string parent = table[0]["channel"].get_string();

        sample = "SELECT SecondField, Ref, parent FROM %1%Channels WHERE %2% = '%3%'";
        table.clear();

        while (result > 0){
            if(parent.empty()){
                break;
            }
            query = arcirk::str_sample(sample, dbo, "Ref", parent);
            std::vector<std::map<std::string, arcirk::bVariant>> _table;
            result = sqlite3Db->execute(query, "Channels", _table, err);
            if (result > 0){
                parent = _table[0]["Parent"].get_string();
                table = _table;
            }
        }

        if (table.size() > 0){
            _Value object(rapidjson::Type::kObjectType);
            object.SetObject();
            jsonObj->addMember(&object, content_value("uuid", table[0]["Ref"].get_string()));
            jsonObj->addMember(&object, content_value("name", table[0]["SecondField"].get_string()));
            jsonObj->addMember("subdivision", object);
        }
    }

}

void shared_state::get_user_department(const std::string &user_ref, arcirk::bJson *jsonObj) {

    std::string dbo = sqlite3Db->is_use_wrapper() ? "dbo." : "";
    std::string sample = "SELECT [channel] FROM %1%Users WHERE %2% = '%3%'";
    std::string query = arcirk::str_sample(sample, dbo, "[Ref]", user_ref);
    std::string err = "";
    std::vector<std::map<std::string, arcirk::bVariant>> table;
    err = "";

    int result = sqlite3Db->execute(query, "Users", table, err);

    if (!err.empty() && err != "no error"){
        return;
    }
    if (result > 0) {
        std::string parent = table[0]["channel"].get_string();
        table.clear();
        sample = "SELECT SecondField, Ref FROM %1%Channels WHERE %2% = '%3%'";
        query = arcirk::str_sample(sample, dbo, "Ref", parent);
        result = sqlite3Db->execute(query, "Channels", table, err);

        if (result > 0) {
            _Value object(rapidjson::Type::kObjectType);
            object.SetObject();
            jsonObj->addMember(&object, content_value("uuid", table[0]["Ref"].get_string()));
            jsonObj->addMember(&object, content_value("name", table[0]["SecondField"].get_string()));
            jsonObj->addMember("department", object);
        }
    }
}

bool shared_state::set_content_type(boost::uuids::uuid &uuid, arcirk::bJson *params, ws_message *msg,
                                           std::string &err, std::string &custom_result) {

    auto  current_sess = get_session(uuid);

    try {
        current_sess->throw_authorized();
    }catch (boost::exception const &e) {
        err = boost::diagnostic_information(e);
        std::cerr << err << std::endl;
        return false;

    }

    std::string content_type = params->get_member("content_type").get_string();

    current_sess->set_content_type(content_type);

    return true;

}

bool shared_state::set_message_struct_type(boost::uuids::uuid &uuid, arcirk::bJson *params, ws_message *msg,
                                                  std::string &err, std::string &custom_result) {
    auto  current_sess = get_session(uuid);

    try {
        current_sess->throw_authorized();
    }catch (boost::exception const &e) {
        err = boost::diagnostic_information(e);
        std::cerr << err << std::endl;
        return false;

    }

    std::string struct_type = params->get_member("struct_type").get_string();

    current_sess->set_message_struct_type(struct_type);

    return true;

}

bool shared_state::get_user_status(boost::uuids::uuid &uuid, arcirk::bJson *params, ws_message *msg, std::string &err,
                                   std::string &custom_result) {

    auto  current_sess = get_session(uuid);

    try {
        current_sess->throw_authorized();
    }catch (boost::exception const &e) {
        err = boost::diagnostic_information(e);
        std::cerr << err << std::endl;
        return false;

    }

    boost::uuids::uuid user_uuid = params->get_member("uuid_user").get_uuid();

    if (user_uuid != arcirk::nil_uuid()){

        arcirk::bJson json{};
        json.set_object();

        std::string filter_app_name;

        bVariant val;
        if (params->getMember("app_name", val)){
            filter_app_name = val.get_string();
        }

        bool status_ = false;
        for (auto p : get_sessions(user_uuid)){
            if(!filter_app_name.empty()){
                if (filter_app_name != p->get_app_name())
                    continue;
            }
            status_ = true;
            break;
        }

        json.addMember("uuid_user", params->get_member("uuid_user"));
        json.addMember("active", status_);

        custom_result = json.to_string();

        return true;
    } else{
        err = "Ошибка получения данных пользователя!";
        return false;
    }

    return true;

}

bool shared_state::get_user_data(boost::uuids::uuid &uuid, arcirk::bJson *params, ws_message *msg, std::string &err,
                                 std::string &custom_result) {

    auto  current_sess = get_session(uuid);

    try {
        current_sess->throw_authorized();
    }catch (boost::exception const &e) {
        err = boost::diagnostic_information(e);
        std::cerr << err << std::endl;
        return false;

    }

    boost::uuids::uuid user_uuid = params->get_member("uuid_user").get_uuid();
    if (user_uuid == arcirk::nil_uuid()){
        err = "Не указан пользователь!";
        return false;
    }

    bVariant value;
    arcirk::bJson result{};
    result.SetObject();
    if(params->getMember("status", value)){
        if(current_sess->get_app_name() == "qt_client"){
            bVariant status = get_user_status_(user_uuid, "qt_client");
            result.addMember("status", status);
        }else{
            bool status = get_user_status_(user_uuid);
            result.addMember("status", status);
        }
    }

    if(params->getMember("unreadMessages", value)){
        int count = get_unread_messages_from_data(current_sess->get_user_uuid(), user_uuid);
        result.addMember("unreadMessages", count);
    }

    result.addMember("uuid_user", params->get_member("uuid_user").to_string());
    custom_result = result.to_string();

    return true;

}

bool shared_state::get_user_status_(boost::uuids::uuid &uuid, const std::string& filter_app_name){

    bool status_ = false;
    for (auto p : get_sessions(uuid)){
        if(!filter_app_name.empty()){
            if (filter_app_name != p->get_app_name())
                continue;
        }
        status_ = true;
        break;
    }

    return status_;
}

int shared_state::get_unread_messages_from_data(boost::uuids::uuid &uuid, boost::uuids::uuid &uuid_recipient) {

    std::string token = sqlite3Db->get_channel_token(uuid, uuid_recipient);

    if (token.empty() || token == "error"){
        return 0;
    }

    std::string dbo = sqlite3Db->is_use_wrapper() ? "dbo." : "";
    std::string query = arcirk::str_sample("select sum([unreadMessages]) as unreadMessages from %1%Messages where [token] = '%2%' and [SecondField] = '%3%'", dbo, token, boost::to_string(uuid));
    std::vector<std::map<std::string, arcirk::bVariant>> table;
    std::string err;

    int result = sqlite3Db->execute(query, "Messages", table, err);

    if(result == 0){
        return 0;
    }

    return table[0].at("unreadMessages").get_int();
}

bool
shared_state::reset_unread_messages(boost::uuids::uuid &uuid, arcirk::bJson *params, ws_message *msg, std::string &err,
                                    std::string &custom_result) {

    auto  current_sess = get_session(uuid);

    try {
        current_sess->throw_authorized();
    }catch (boost::exception const &e) {
        err = boost::diagnostic_information(e);
        std::cerr << err << std::endl;
        return false;

    }

    boost::uuids::uuid uuid_sender = params->get_member("sender").get_uuid();

    std::string token = sqlite3Db->get_channel_token(current_sess->get_user_uuid(), uuid_sender);

    if (token.empty() || token == "error"){
        err = "Не верный токен!";
        return false;
    }
    std::string dbo = sqlite3Db->is_use_wrapper() ? "dbo." : "";
    std::string query = arcirk::str_sample("UPDATE %1%Messages SET [unreadMessages] = '0' WHERE [SecondField] = '%2%' AND [token] = '%3%' AND [unreadMessages] > '0';", dbo, boost::to_string(current_sess->get_user_uuid()), token);
    err = "";
    sqlite3Db->exec(query, err);
    if(!err.empty() && err != "no error")
        return false;

    return true;
}

bool
shared_state::get_unread_messages(boost::uuids::uuid &uuid, arcirk::bJson *params, ws_message *msg, std::string &err,
                                  std::string &custom_result) {

    auto  current_sess = get_session(uuid);

    try {
        current_sess->throw_authorized();
    }catch (boost::exception const &e) {
        err = boost::diagnostic_information(e);
        std::cerr << err << std::endl;
        return false;
    }

    std::string json;
    err = "";

    sqlite3Db->get_unread_messages(boost::to_string(current_sess->get_user_uuid()), json, err);

    if(!err.empty() && err != "no error"){
        return false;
    }

    custom_result = json;
    return true;

}

bool shared_state::command_to_qt_client(boost::uuids::uuid &uuid, arcirk::bJson *params, ws_message *msg, std::string &err,
                            std::string &custom_result) {

    auto  current_sess = get_session(uuid);

    try {
        current_sess->throw_authorized();
    }catch (boost::exception const &e) {
        err = boost::diagnostic_information(e);
        std::cerr << err << std::endl;
        return false;
    }

    bVariant value;
    bVariant command;

    if (params->getMember("uuid_client", value)){
        boost::uuids::uuid uuid_sess{};
        arcirk::is_valid_uuid(value.get_string(), uuid_sess);
        auto sess = get_session(uuid_sess);
        if(!sess)
            return false;

        if(!params->getMember("command", command)){
            return false;
        }

        _ws_message _message = createMessage(current_sess);
        _message.message = command.get_string();
        _message.command = "command_to_qt_client";
        _message.uuid_channel = sess->get_user_uuid();

        std::string msg = arcirk::ws_message(_message).get_json(true);

        auto const ss = boost::make_shared<std::string const>(std::move(msg));
        sess->send(ss);

        std::cout << "shared_state::command_to_qt_client " << uuid << " to " << uuid_sess << std::endl;

    }else
        return false;

    return true;
}

bool shared_state::create_pay_html_page(boost::uuids::uuid &uuid, arcirk::bJson *params, ws_message *msg, std::string &err,
                                        std::string &custom_result) {

    auto  current_sess = get_session(uuid);

    try {
        current_sess->throw_authorized();
    }catch (boost::exception const &e) {
        err = boost::diagnostic_information(e);
        std::cerr << err << std::endl;
        return false;
    }

    bVariant param;
    arcirk::bConf conf("conf.json", true);
    QString otputDir = QString::fromStdString(conf[ServerWorkingDirectory].get_string());

    QString fileName = otputDir + "html";
    fileName.append(QDir::separator());
    fileName.append("pay_template.html");
    QFile file(QDir::toNativeSeparators(fileName));

    if(!file.open(QIODevice::ReadOnly)){
        std::cerr << "shared_state::create_pay_html_page::error " << QString("Файл шаблона html страницы не найден!").toLocal8Bit().toStdString() << std::endl;
        return false;
    }

    QString htmlTemplate = file.readAll();
    file.close();

    if(htmlTemplate.isEmpty()){
        std::cerr << "shared_state::create_pay_html_page::error " << QString("Пустой файл html шаблона!").toLocal8Bit().toStdString() << std::endl;
        return false;
    }

    QString title = QString::fromStdString(params->get_member("Title").get_string());

    if(title.isEmpty()){
        std::cerr << "shared_state::create_pay_html_page::error " << QString("Ошибка в данных!").toLocal8Bit().toStdString() << std::endl;
        return false;
    }

    QString sha = QString::fromStdString(arcirk::get_sha1(title.toStdString()));


    QString titleDesc = QString::fromStdString(params->get_member("DescriptionTitle").get_string());
    QString desc = QString::fromStdString(params->get_member("Description").get_string());
    QString img = QString::fromStdString(params->get_member("ImageQr").get_string());

    std::vector<std::map<std::string, std::string>> bnkDetails;
    params->getMembers("BankDetails", bnkDetails);
    if(bnkDetails.size() != 6)
        return false;

    QString org = QString::fromStdString(bnkDetails[0]["OrganisationTitle"]);
    QString innkpp = QString::fromStdString(bnkDetails[1]["INNKPP"]);
    QString bank = QString::fromStdString(bnkDetails[2]["Bank"]);
    QString bnkAcc = QString::fromStdString(bnkDetails[3]["BankAccount"]);
    QString corAcc = QString::fromStdString(bnkDetails[4]["CorrAccount"]);
    QString bik = QString::fromStdString(bnkDetails[5]["BIK"]);


    htmlTemplate.replace("____TITLE____", title);
    htmlTemplate.replace("____TITLE_DESCRIPTION____", titleDesc);
    htmlTemplate.replace("____DESCRIPTION____", desc);
    htmlTemplate.replace("____ORGANISATION____", org);
    htmlTemplate.replace("____INNKPP____", innkpp);
    htmlTemplate.replace("____BANK____", bank);
    htmlTemplate.replace("____BANK_ACCOUNT____", bnkAcc);
    htmlTemplate.replace("____CORR_ACCOUNT____", corAcc);
    htmlTemplate.replace("____BIK____", bik);
    htmlTemplate.replace("____QR_CODE_BASE64____", img);

    QString dirPath = otputDir + "html";
    dirPath.append(QDir::separator());
    dirPath.append(sha);

    QDir dir(QDir::toNativeSeparators(dirPath));
    if(!dir.exists())
        dir.mkdir(dirPath);

    QFile f(dir.path() + QDir::separator() + "index.html");

    if(!f.open(QIODevice::WriteOnly)){
        std::cerr << "shared_state::create_pay_html_page::error " << QString("Ошибка доступа!").toLocal8Bit().toStdString() << std::endl;
        return false;
    }

    f.write(htmlTemplate.toUtf8());
    f.close();

    auto r = arcirk::bJson();
    r.set_object();
    r.addMember("result", "success");
    r.addMember("link", f.fileName().toStdString());
    r.addMember("sha", sha.toStdString());

    custom_result = r.to_string();

    return true;
}

bool shared_state::set_document_name(boost::uuids::uuid &uuid, arcirk::bJson *params, ws_message *msg, std::string &err,
                                        std::string &custom_result) {

    auto  current_sess = get_session(uuid);

    try {
        current_sess->throw_authorized();
    }catch (boost::exception const &e) {
        err = boost::diagnostic_information(e);
        std::cerr << err << std::endl;
        return false;
    }

    bVariant value;
    if (params->getMember("document_name", value)){
        std::string doc_name = value.get_string();
        current_sess->set_document_name(doc_name);
        send(std::string ("{\"document_name\": \"") + doc_name +
             "\", \"uuid_user\": \"" + arcirk::uuid_to_string(current_sess->get_user_uuid()) + "\", \"uuid\": \"" +
             arcirk::uuid_to_string(uuid) + "\"}", "document_name", true, current_sess->notify_apps());
    }else
        return false;

    return true;
}

bool shared_state::command_to_client(boost::uuids::uuid &uuid, arcirk::bJson *params, ws_message *msg, std::string &err,
                                        std::string &custom_result) {

    auto  current_sess = get_session(uuid);

    try {
        current_sess->throw_authorized();
    }catch (boost::exception const &e) {
        err = boost::diagnostic_information(e);
        std::cerr << err << std::endl;
        return false;
    }

    bVariant value;
    bVariant command;

    if (params->getMember("uuid_agent", value)){

        std::string sz_sessions = value.get_string();
        arcirk::T_vec m_sess = arcirk::split(sz_sessions, "|");
        if(m_sess.size() == 0){
            err = "Не указана сессия получателя!";
            return false;
        }

        std::vector<websocket_session*> sessions;
        arcirk::T_vec apps;
        bVariant filterApps;

        if(params->getMember("app_name", filterApps)){
            if(!filterApps.is_string()){
                apps = arcirk::split(filterApps.get_string(), "|");
            }
        }


        foreach(const std::string u, m_sess){
            boost::uuids::uuid uuid_sess{};
            arcirk::is_valid_uuid(u, uuid_sess);
            if(uuid_sess.is_nil())
                continue;
            auto sess = get_session(uuid_sess);
            if(!sess)
                continue;
            if(apps.size() > 0){
                if(std::find(apps.begin(), apps.end(), sess->get_app_name()) == apps.end())
                    continue;
            }
            sessions.push_back(sess);

        }

        if(sessions.size() == 0){
            err = "Не найдена севссия получателя!";
            return false;
        }

        _Value param;
        if(!params->getMember("param", param)){
            err = "Не указаны обязательные реквизиты (param)!";
            return false;
        }

        _ws_message _message = createMessage(current_sess);
        _message.message = params->get_member("message").to_string();
        _message.command = "command_to_client";
        _message.uuid_channel = current_sess->get_user_uuid();
        _message.uuid_form = msg->message().uuid_form;

        std::string msg = arcirk::ws_message(_message).get_json(false);

        auto json = bJson();
        json.parse(msg);
        json.addMember("param", param);
        std::string r = json.to_string(true);

        auto const ss = boost::make_shared<std::string const>(std::move(r));

        foreach(websocket_session* sess, sessions){
            sess->send(ss);
        }

        return true;

    }else
        err = "Не указаны обязательные реквизиты (uuid_agent)!";

    return false;

}

bool shared_state::command_to_qt_agent(boost::uuids::uuid &uuid, arcirk::bJson *params, ws_message *msg, std::string &err,
                                        std::string &custom_result) {

    auto  current_sess = get_session(uuid);

    try {
        current_sess->throw_authorized();
    }catch (boost::exception const &e) {
        err = boost::diagnostic_information(e);
        std::cerr << err << std::endl;
        return false;
    }

    bVariant value;
    bVariant command;

    if (params->getMember("uuid_agent", value)){
        boost::uuids::uuid uuid_sess{};

        arcirk::is_valid_uuid(value.get_string(), uuid_sess);
        if(uuid_sess.is_nil()){
            for (auto iter : sessions_) {
                if(iter.second->get_app_name() == "qt_cert_manager"){
                    uuid_sess = iter.second->get_uuid();
                }
            }
        }
        auto sess = get_session(uuid_sess);
        if(!sess)
            return false;

        if(!params->getMember("command", command)){
            return false;
        }

        _ws_message _message = createMessage(current_sess);
        _message.message = params->get_member("message").to_string();
        _message.command = "command_to_qt_agent";
        _message.uuid_channel = current_sess->get_user_uuid();
        //_message.app_name = current_sess->get_app_name();

        std::string msg = arcirk::ws_message(_message).get_json(true);

        auto const ss = boost::make_shared<std::string const>(std::move(msg));

        sess->send(ss);

        std::cout << "shared_state::command_to_qt_agent " << uuid << " to " << uuid_sess << std::endl;

    }else
        return false;

    return true;
}

std::string shared_state::_get_webdav_settings() const {

    using arcirk::bConfFields;
    arcirk::bConf conf("conf.json", true);

    arcirk::bJson wd_json{};
    wd_json.set_object();
    wd_json.addMember(arcirk::content_value(bConf::get_field_alias(WebDavHost), conf[WebDavHost].get_string()));
    wd_json.addMember(arcirk::content_value(bConf::get_field_alias(WebDavUser), conf[WebDavUser].get_string()));
    wd_json.addMember(arcirk::content_value(bConf::get_field_alias(WebDavPwd), conf[WebDavPwd].get_string()));
    wd_json.addMember(arcirk::content_value(bConf::get_field_alias(WebDavSSL), conf[WebDavSSL].get_bool()));

    return wd_json.to_string();

}

bool shared_state::get_webdav_settings(boost::uuids::uuid &uuid, arcirk::bJson *params, ws_message *msg,
                                       std::string &err, std::string &custom_result) {

    auto  current_sess = get_session(uuid);

    try {
        current_sess->throw_authorized();
    }catch (boost::exception const &e) {
        err = boost::diagnostic_information(e);
        std::cerr << err << std::endl;
        return false;
    }

    custom_result = _get_webdav_settings();

    return true;

}

bool shared_state::set_sql_settings(boost::uuids::uuid &uuid, arcirk::bJson *params, ws_message *msg,
                                       std::string &err, std::string &custom_result){
    auto  current_sess = get_session(uuid);

    try {
        current_sess->throw_authorized();
    }catch (boost::exception const &e) {
        err = boost::diagnostic_information(e);
        std::cerr << err << std::endl;
        return false;
    }

    if (current_sess->get_role() != "admin"){
        err = "не достаточно прав доступа для команды!";
        return false;
    }

    std::string sql_format = params->get_member(bConf::get_field_alias(arcirk::SQLFormat)).get_string();
    std::string sql_host = params->get_member(bConf::get_field_alias(arcirk::SQLHost)).get_string();
    std::string sql_user = params->get_member(bConf::get_field_alias(arcirk::SQLUser)).get_string();
    std::string sql_password = params->get_member(bConf::get_field_alias(arcirk::SQLPassword)).get_string();
    arcirk::bConf conf("conf.json", true);
    conf[SQLFormat] = sql_format;
    conf[SQLHost] = sql_host;
    conf[SQLUser] = sql_user;
    conf[SQLPassword] = sql_password;
    conf.save();

    custom_result = QString("Изменился формат базы данных. Требуется перезапустить сервер.").toLocal8Bit().toStdString();

    std::cout << custom_result << std::endl;

    return true;

}

bool shared_state::insert_file_to_data(boost::uuids::uuid &uuid, bJson *params, ws_message *msg, std::string &err, std::string &custom_result)
{

    auto  current_sess = get_session(uuid);

    try {
        current_sess->throw_authorized();
    }catch (boost::exception const &e) {
        err = boost::diagnostic_information(e);
        std::cerr << err << std::endl;
        return false;
    }

    err = "команда не доступна!";

    return false;

}

bool shared_state::set_webdav_settings(boost::uuids::uuid &uuid, arcirk::bJson *params, ws_message *msg,
                                        std::string &err, std::string &custom_result){

    auto  current_sess = get_session(uuid);

    try {
        current_sess->throw_authorized();
    }catch (boost::exception const &e) {
        err = boost::diagnostic_information(e);
        std::cerr << err << std::endl;
        return false;
    }

    if (current_sess->get_role() != "admin"){
        err = "не достаточно прав доступа для команды!";
        return false;
    }

    std::string _webdav_host = params->get_member(bConf::get_field_alias(WebDavHost)).get_string();
    std::string _webdav_user = params->get_member(bConf::get_field_alias(WebDavUser)).get_string();
    std::string _webdav_pwd = params->get_member(bConf::get_field_alias(WebDavPwd)).get_string();
    bool _webdav_ssl = params->get_member(bConf::get_field_alias(WebDavSSL)).get_bool();

    using arcirk::bConfFields;

    arcirk::bConf conf("conf.json", true);
    conf[WebDavHost] = _webdav_host;
    conf[WebDavUser] = _webdav_user;
    conf[WebDavPwd] = _webdav_pwd; //уже шифрованный
    conf[WebDavSSL] = _webdav_ssl;
    conf.save();

    //оповестим всех клиентов с изменениями настроек webdav
    std::string resp = _get_webdav_settings();
    std::vector<boost::weak_ptr<websocket_session>> v;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        v.reserve(sessions_.size());
        for(auto p : sessions_){
            if (p.second->get_app_name() == "qt_client" || p.second->get_app_name() == "qt_agent")
                v.emplace_back(p.second->weak_from_this());
        }

    }

    for(auto const& wp : v)
        if(auto sp = wp.lock()){
                _ws_message _message = createMessage(sp.get());
                _message.message = resp;
                _message.command = "get_webdav_settings";
                auto * new_msg = new ws_message(_message);
                std::string sz_new_msg = new_msg->get_json(true);
                auto const ss = boost::make_shared<std::string const>(std::move(sz_new_msg));
                sp->send(ss);
                delete new_msg;
        }

    return true;
}

bool shared_state::get_channel_token(boost::uuids::uuid &uuid, arcirk::bJson *params, ws_message *msg, std::string &err,
                                     std::string &custom_result) {
    auto  current_sess = get_session(uuid);

    try {
        current_sess->throw_authorized();
    }catch (boost::exception const &e) {
        err = boost::diagnostic_information(e);
        std::cerr << err << std::endl;
        return false;
    }

    std::string user_uuid = params->get_member("user_uuid").get_string();
    std::string recipient_uuid = params->get_member("recipient_uuid").get_string();

    std::string result = sqlite3Db->get_channel_token(arcirk::string_to_uuid(user_uuid), arcirk::string_to_uuid(recipient_uuid));

    if(result == "error")
        return false;

    custom_result = result;

    return true;
}

bool shared_state::send_technical_information(boost::uuids::uuid &uuid, arcirk::bJson *params, ws_message *msg,
                                              std::string &err, std::string &custom_result) {
    return false;
}

bool shared_state::export_tables_to_ext(boost::uuids::uuid &uuid, arcirk::bJson *params, ws_message *msg,
                                              std::string &err, std::string &custom_result) {

    auto  current_sess = get_session(uuid);

    try {
        current_sess->throw_authorized();
    }catch (boost::exception const &e) {
        err = boost::diagnostic_information(e);
        std::cerr << err << std::endl;
        return false;
    }

    if (current_sess->get_role() != "admin"){
        err = "не достаточно прав доступа для команды!";
        return false;
    }

    if(!sqlite3Db->is_use_wrapper()){
        err = "Нет подключения к в sql серверу!";
        return false;
    }

    err = "";
    bool _result;

    _result = sqlite3Db->export_tables();

    return _result;
}

bool shared_state::sync_users(boost::uuids::uuid &uuid, arcirk::bJson *params, ws_message *msg,
                                              std::string &err, std::string &custom_result) {

    auto  current_sess = get_session(uuid);

    try {
        current_sess->throw_authorized();
    }catch (boost::exception const &e) {
        err = boost::diagnostic_information(e);
        std::cerr << err << std::endl;
        return false;
    }

    if (current_sess->get_role() != "admin"){
        err = "не достаточно прав доступа для команды!";
        return false;
    }

    sqlite3Db->sync_users(params->to_string());

    return true;
}

void shared_state::connect_sqlite_database() {
#ifndef USE_QT_CREATOR
    std::string parent;
    std::string err;

    sqlite3Db = new arc_sqlite::sqlite3_db();
    bool res = sqlite3Db->open("base/db.sqlite");
    if(!res)
        std::cerr << "Файл базы данных не найден!";

    sqlite3Db->check_database_table(arc_sqlite::eUsers);
    sqlite3Db->check_database_table(arc_sqlite::eMessages);
    sqlite3Db->check_database_table(arc_sqlite::eChannels);
    sqlite3Db->check_database_table(arc_sqlite::eSubscribers);


    //ищем пользователя с правами администратора
    std::string query = "SELECT * FROM Users WHERE role = 'admin'";

    int result = sqlite3Db->exec(query);

    if (result == 0){
        _add_new_user("admin", "admin", "admin", "", "admin", parent, err);
    }
    //справочник пользователей
    query = "CREATE VIEW IF NOT EXISTS UsersCatalog AS "
            "    SELECT FirstField, "
            "           SecondField, "
            "           Ref, "
            "           Parent, "
            "           1 AS IsGroup "
            "      FROM Channels "
            "    UNION "
            "    SELECT FirstField, "
            "           SecondField, "
            "           Ref, "
            "           channel AS Parent, "
            "           0 AS IsGroup "
            "      FROM Users;";

    result = sqlite3Db->exec(query);
#else

    std::string parent;
    std::string err;

    arcirk::bConf conf("conf.json", true);

    sqlWrapper = new SqlInterface();
    sqlite3Db->set_qt_wrapper(sqlWrapper);

    std::string sql_host = conf[SQLHost].get_string();
    std::string sql_user = conf[SQLUser].get_string();
    std::string _passEn = conf[SQLPassword].get_string();
    std::string sql_pass;
    if(!_passEn.empty()){
        sql_pass = arcirk::_crypt(_passEn, "my_key");
    }
    sqlWrapper->setHost(sql_host.c_str());
    sqlWrapper->setSqlUser(sql_user.c_str());
    sqlWrapper->setSqlPwd(sql_pass.c_str());

    sqlWrapper->setDriver("QSQLITE");
    sqlWrapper->setDatabaseName("base/db.sqlite");
    bool sqlResult = sqlWrapper->connect("QSQLITE");
    std::cout << "shared_state::shared_state: " << sqlWrapper->databaseName().toStdString() << std::endl;
    if(!sqlResult)
        std::cerr << "shared_state::shared_state error connect to sqlite!" << std::endl;
    else{
        std::cout << "shared_state::shared_state connect to sqlite!" << std::endl;
        sqlWrapper->verifyTables();
        sqlWrapper->verifyViews();
    }

    QString err_;
    int result = sqlWrapper->exec("SELECT * FROM Users WHERE [role] = 'admin'", err_);
    err = err_.toStdString();
    if(result == 0){
       _add_new_user("admin", "admin", "admin", "", "admin", parent, err);
    }
#endif

}

void shared_state::connect_sqlserver_database() {

    std::string parent;
    std::string err;

    arcirk::bConf conf("conf.json", true);

#ifdef USE_QT_CREATOR
    sqlWrapper = new SqlInterface();
    sqlite3Db->set_qt_wrapper(sqlWrapper);
#else
    sqlWrapper = new SqlWrapper();
    sqlite3Db->set_qt_wrapper(sqlWrapper);
#endif

    std::string sql_host = conf[SQLHost].get_string();
    std::string sql_user = conf[SQLUser].get_string();
    std::string _passEn = conf[SQLPassword].get_string();
    std::string sql_pass;
    if(!_passEn.empty()){
        sql_pass = arcirk::_crypt(_passEn, "my_key");
    }
    sqlWrapper->setHost(sql_host.c_str());
    sqlWrapper->setSqlUser(sql_user.c_str());
    sqlWrapper->setSqlPwd(sql_pass.c_str());

#ifdef USE_QT_CREATOR
    sqlWrapper->setDriver("QODBC");
    sqlWrapper->setDatabaseName("arcirk");
    bool sqlResult = sqlWrapper->connect("QODBC");
#else
    bool sqlResult = sqlWrapper->connect("arcirk");
#endif
    if(!sqlResult){
        std::cerr << "shared_state::shared_state error connect to sql server!" << std::endl;
        delete sqlWrapper;
        std::cout << "try connect to local database!" << std::endl;
        connect_sqlite_database();
        return;
    }
    else{
        sqlWrapper->verifyTables();
        sqlWrapper->verifyViews();
#ifdef USE_QT_CREATOR
        std::cout << "connect to sql server! " << sqlWrapper->host().toStdString() << " " << sqlWrapper->driver().toStdString() << " " << sqlWrapper->databaseName().toStdString() << std::endl;
//#else
//        std::cout << "connect to sql server! " << sqlWrapper->host()  << " " << sqlWrapper->databaseName() << std::endl;
#endif
    }
    if(sqlResult){
#ifndef USE_QT_CREATOR
        char * err_ = nullptr;
        int result = sqlWrapper->exec("SELECT * FROM dbo.Users WHERE [role] = 'admin'", &err_);
        err = std::string(err_);
#else
        QString err_;
        int result = sqlWrapper->exec("SELECT * FROM dbo.Users WHERE [role] = 'admin'", err_);
        err = err_.toStdString();

#endif
        if(result == 0){
           _add_new_user("admin", "admin", "admin", "", "admin", parent, err);
        }
#ifdef USE_QT_CREATOR
        err_ = "";
        sqlWrapper->exec("TRUNCATE TABLE dbo.WSConf", err_);
        std::string uuid = arcirk::random_uuid();
        sqlWrapper->exec(QString("INSERT INTO [dbo].[WSConf]\n"
                                 "(\n"
                                 "[Ref]\n"
                                 ",[host]\n"
                                 ",[port])\n"
                                 "VALUES\n"
                                 "(\n"
                                 "'%1'\n"
                                 ",'%2'\n"
                                 ",'%3');").arg(QString::fromStdString(uuid),
                                                QString::fromStdString(currentHost),
                                                QString::number(currentPort)), err_);

        std::cout << "update currentHost:" + currentHost << std::endl;
        conf.save();
#endif
    }
}
