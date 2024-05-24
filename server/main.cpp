
#ifdef _WINDOWS
#include <SDKDDKVer.h>
#endif

#include <iostream>
#include <vector>
#include <string>
#include <fs/standard_paths.hpp>
#include <cl/command_line.hpp>
#include <server_conf.hpp>
#include <crypt/cryptography.hpp>
#include <fs/network.hpp>

using namespace arcirk::strings;
using namespace boost::filesystem;
using namespace arcirk;

const std::string index_html_text = "<!DOCTYPE html>\n"
                                    "<html>\n"
                                    "<head>\n"
                                    "    <meta charset=\"UTF-8\" name=\"viewport\" content=\"width=device-width, initial-scale=1\">\n"
                                    "    <title>arcirk websockets</title>\n"
                                    "</head>\n"
                                    "<body>\n"
                                    "\n"
                                    "\n"
                                    "    WebSocket Server Is Run\n"
                                    "    <p>Файлы для загрузки:</p>\n"
                                    "    <div id='files_list'>\n"
                                    "\n"
                                    "\n"
                                    "    </div>\n"
                                    "\n"
                                    "    <script type=\"text/javascript\">\n"
                                    "        let div = document.getElementById('files_list');\n"
                                    "\n"
                                    "\n"
                                    "        const Http = new XMLHttpRequest();\n"
                                    "        const url = 'http://192.168.10.80:8080/api/files';\n"
                                    "        Http.open(\"GET\", url);\n"
                                    "        Http.send();\n"
                                    "\n"
                                    "        function isArray(ob) {\n"
                                    "            return ob.constructor === Array;\n"
                                    "        }\n"
                                    "\n"
                                    "        Http.onreadystatechange = (e) => {\n"
                                    "            //console.log(Http.responseText);  // => получим массив данных в формате JSON\n"
                                    "            let resp = Http.responseText\n"
                                    "            if (resp.length === 0)\n"
                                    "                return\n"
                                    "            let arr = JSON.parse(resp)\n"
                                    "\n"
                                    "            //console.log(arr.length)\n"
                                    "\n"
                                    "            if (isArray(arr)) {\n"
                                    "                div.innerHTML = '';\n"
                                    "                arr.forEach(function (item, i, arr) {\n"
                                    "                    let mA = item.split('\\\\')\n"
                                    "                    mA.splice(0, 2)\n"
                                    "                    let val = mA.join('\\\\')\n"
                                    "                    const p = document.createElement('p')\n"
                                    "                    const a = document.createElement('a')\n"
                                    "                    a.href = val                    \n"
                                    "                    a.title = mA[mA.length - 1]\n"
                                    "                    a.text = mA[mA.length - 1]\n"
                                    "                    p.appendChild(a)\n"
                                    "                    files_list.appendChild(p)\n"
                                    "                })\n"
                                    "            } else\n"
                                    "                console.log(\"Ошибка получения списка файлов\")\n"
                                    "\n"
                                    "\n"
                                    "        }\n"
                                    "    </script>\n"
                                    "\n"
                                    "\n"
                                    "\n"
                                    "</body>\n"
                                    "</html>";

boost::filesystem::path m_root_conf;

static inline boost::filesystem::path app_directory() {

    return boost::filesystem::path(standard_paths::program_data()) /+ ARCIRK_VERSION;

}

void verify_directories(const std::string& working_directory_dir = ""){

    using namespace boost::filesystem;

    if(!working_directory_dir.empty()){
        m_root_conf = path(working_directory_dir);
        m_root_conf /= ARCIRK_VERSION;
    }else{
        m_root_conf = standard_paths::program_data();
        m_root_conf /= ARCIRK_VERSION;
    }

    bool is_conf = arcirk::standard_paths::verify_directory(m_root_conf);

    if(is_conf){
        path html = m_root_conf;
        html /= "html";
        if(arcirk::standard_paths::verify_directory(html)){
            path index_html = html;
            index_html /= "index.html";
            if(!boost::filesystem::exists(index_html)){
                std::ofstream out;
                out.open(index_html.string(), std::ofstream::out | std::ofstream::app);
                if(out.is_open()){
                    out << index_html_text << std::endl;
                    out.close();
                }
            }
        }

        path data = m_root_conf;
        data /= "data";
        arcirk::standard_paths::verify_directory(data);

        path ssl_dir = m_root_conf;
        ssl_dir /= "ssl";
        arcirk::standard_paths::verify_directory(ssl_dir);

        path api_dir = html;
        api_dir /= "api/info";
        arcirk::standard_paths::verify_directory(api_dir);

        path api_files = html;
        api_files /= "api/files";
        arcirk::standard_paths::verify_directory(api_files);

        path _1c_dir = m_root_conf;
        _1c_dir /= "1c/bsl";
        arcirk::standard_paths::verify_directory(_1c_dir);

        //каталог с обновлениями
        path bin = m_root_conf;
        bin /= "bin";
        arcirk::standard_paths::verify_directory(bin);
    }
}

void write_conf(server::server_config & conf, const boost::filesystem::path& root_conf, const std::string& file_name) {
    using namespace boost::filesystem;

    if (!exists(from_utf(root_conf.string())))
        return;
    try {
        std::string result = to_string(pre::json::to_json(conf));
        std::ofstream out;
        path conf_file = root_conf /+ file_name.c_str();
        out.open(from_utf(conf_file.string()));
        if (out.is_open()) {
            out << result;
            out.close();
        }
    } catch (std::exception &e) {
        std::cerr << e.what() << std::endl;
    }

}

void save_conf(server::server_config& conf){
    write_conf(conf, m_root_conf, ARCIRK_SERVER_CONF);
}

void read_conf(server::server_config & result, const boost::filesystem::path& root_conf, const std::string& file_name){

    using namespace boost::filesystem;

    try {
        path conf = root_conf /+ file_name.c_str();

        if(exists(conf)){
            std::ifstream file(conf.string(), std::ios_base::in);
            std::string str{std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>()};
            if(!str.empty()){
                result = arcirk::secure_serialization<server::server_config>(str);
            }
        }
    } catch (std::exception &e) {
        std::cerr << e.what() << std::endl;
    }

}

void copy_ssl_file(const std::string& file_patch, server::server_config& conf){

    using namespace boost::filesystem;
    if(!exists(m_root_conf))
        return;

    path ssl_dir = m_root_conf;
    ssl_dir /= "ssl";

    path current(file_patch);

    if(!exists(current)){
        copy_file(
                current,
                ssl_dir / current.filename()
        );
        if(current.extension() == "key")
            conf.SSL_key_file = current.filename().string();
        else
            conf.SSL_crt_file = current.filename().string();
    }

}

void read_command_line(const command_line_parser::line_parser& parser, server::server_config& conf){

    if(parser.option_exists("-h")){
        conf.ServerHost = parser.option("-h");
    }
    if(parser.option_exists("-p")){
        const std::string &_port = parser.option("-p");
        if(!_port.empty())
            conf.ServerPort = static_cast<unsigned short>(std::atoi(_port.c_str()));
    }
    if(parser.option_exists("-wd")){
        conf.ServerWorkingDirectory= parser.option("-wd");
    }
    if(parser.option_exists("-t")){
        const std::string &_threads = parser.option("-t");
        if(!_threads.empty())
            conf.ThreadsCount =  std::max<int>(1, static_cast<unsigned short>(std::atoi(_threads.c_str())));
    }
    if(parser.option_exists("-usr")){
        conf.ServerUser = parser.option("-usr");
    }
    if(parser.option_exists("-pwd")){
        conf.ServerUserHash = get_hash(conf.ServerUser, parser.option("-pwd"));
    }
    if(parser.option_exists("-crt_file")){
        copy_ssl_file(parser.option("-crt_file"), conf);
    }
    if(parser.option_exists("-key_file")){
        copy_ssl_file(parser.option("-key_file"), conf);
    }
    if(parser.option_exists("-use_auth"))
        conf.UseAuthorization = true; //требуется авторизация на сервере
    if(parser.option_exists("-ada"))
        conf.AllowDelayedAuthorization = true;// разрешить отложенную авторизацию
    if(parser.option_exists("-ahm"))
        conf.AllowHistoryMessages = true; //разрешить хранение истории сообщений
}

void verify_database_structure(arcirk::DatabaseType type, const arcirk::server::server_config& sett){


    using namespace boost::filesystem;
    using namespace soci;
    using namespace arcirk::database;
    //using namespace arcirk::cryptography;

    auto version = arcirk::server::get_version();
//    std::string db_name = arcirk::str_sample("arcirk_v%1%%2%%3%", std::to_string(version.major), std::to_string(version.minor), std::to_string(version.path));

    session sql;
    if(type == arcirk::DatabaseType::dbTypeSQLite){
        path data = m_root_conf /+ "data" /+ "arcirk.sqlite";
        std::string connection_string = arcirk::str_sample("db=%1% timeout=2 shared_cache=true", data.string());
        sql.open(soci::sqlite3, connection_string);
    }else{
        if(sett.SQLHost.empty())
            throw native_exception("Не указан адрес SQL сервера!");

        const std::string pwd = sett.SQLPassword;
        std::string connection_string = arcirk::str_sample("DRIVER={SQL Server};"
                                                           "SERVER=%1%;Persist Security Info=true;"
                                                           "uid=%2%;pwd=%3%", sett.SQLHost, sett.SQLUser, arcirk::crypt(pwd, CRYPT_KEY));
//        std::string connection_string = arcirk::str_sample("DRIVER={SQL Server};"
//                                                           "SERVER=%1%;Persist Security Info=true;"
//                                                           "uid=%2%;pwd=%3%", sett.SQLHost, sett.SQLUser, crypt_utils().decrypt_string(pwd));
        try {
            sql.open(soci::odbc, connection_string);
        } catch (const std::exception &e) {
            std::cerr << e.what() << std::endl;
        }

        if(sql.is_connected())
            is_odbc_database(sql);
        else
            throw native_exception("Ошибка подключения к серверу баз данных!");
    }

    if(!sql.is_connected()){
        fail(__FUNCTION__, "Error connection database!");
        return;
    }

    try {
        verify_database(sql, type, pre::json::to_json(version));
        sql.close();
    }catch (std::exception &e) {
        fail(__FUNCTION__, e.what());
    }

}

int
main(int argc, char* argv[])
{
    setlocale(LC_ALL, "Russian");

    command_line_parser::line_parser input(argc, argv);

    if(input.option_exists("-v") || input.option_exists("-version")){
        std::cout << from_utf("arcirk.websocket.server v.") << ARCIRK_VERSION << std::endl;
        return EXIT_FAILURE;
    }

    //Проверяем доступность структуры каталогов
    verify_directories();

    auto conf = server::server_config();
    //инициализируем настройки
    read_conf(conf, m_root_conf, ARCIRK_SERVER_CONF);
    //если рабочий каталог не задан используем каталог по умолчанию
    if(conf.ServerWorkingDirectory.empty())
        conf.ServerWorkingDirectory = standard_paths::program_data().string();

    //читаем командную строку
    read_command_line(input, conf);

    if(conf.ServerHost.empty()){
        //выбираем первый локальный хост по шаблону "192.168.xxx.xxx"
        conf.ServerHost = bIp::get_default_host("0.0.0.0", "192.168");
    }
    if(conf.ServerHttpRoot.empty()){
        path html = m_root_conf;
        html /= "html";
        conf.ServerHttpRoot = html.string();
    }

    conf.Version = ARCIRK_VERSION;

    save_conf(conf);

}