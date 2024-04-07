//
// Created by admin on 29.01.2024.
//

#ifndef CONTROLSPROG_VERIFY_DATABASE_HPP
#define CONTROLSPROG_VERIFY_DATABASE_HPP

#include "../global.hpp"
//#include "../variant_p.hpp"
#include "../variant/item_data.h"
#include "tuple"
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QUuid>
#include "query_builder.hpp"

BOOST_FUSION_DEFINE_STRUCT(
    (arcirk::database), table_info_sqlite,
    (int, cid)
    (std::string, name)
    (std::string, type)
    (int, notnull)
    (std::string, dflt_value)
    (int, bk)
);

using namespace arcirk::widgets;

namespace arcirk::database{



    template<typename T, typename A>
    class verify_database{

    public:
        explicit verify_database(){

        }
        ~verify_database(){};

        std::map<T, int> get_release_tables_versions(){
            std::map<T, int> result;
            foreach (auto itr , m_initial_data_table) {
                result.insert(std::make_pair(std::get<0>(itr), std::get<2>(itr)));
            }
            return result;
        }

        static std::vector<std::string> get_database_tables(QSqlDatabase& sql){

            QSqlQuery rs("SELECT name FROM sqlite_master WHERE type='table';", sql);
            std::vector<std::string> result;
            while (rs.next())
            {
                result.push_back(rs.value(0).toString().toStdString());
            }

            return result;
        }
        static std::vector<std::string> get_database_views(QSqlDatabase& sql){

            QSqlQuery rs("SELECT name FROM sqlite_master WHERE type='view';", sql);
            std::vector<std::string> result;
            while (rs.next())
            {
                result.push_back(rs.value(0).toString().toStdString());
            }

            return result;
        }

        static void verify_database_views(QSqlDatabase& sql, const std::vector<std::string>& db_views, const std::string& view_name,  const std::string& t_ddl, bool is_rebase = false){

            if(std::find(db_views.begin(), db_views.end(), view_name) == db_views.end()) {
                QSqlQuery rs(t_ddl.c_str(), sql);
                rs.exec();
            }else{
                if(is_rebase){
                    sql.transaction();
                    QSqlQuery rs(QString("drop view %1").arg(view_name.c_str()), sql);
                    rs.exec();
                    rs.exec(t_ddl.c_str());
                    sql.commit();
                }
            }
        }

        static QString get_ddl(T table, const json& object){
            auto fields = builder::from_object_structure(object);
            return builder::query_builder().create_table(enum_synonym(table), fields).prepare().c_str();
        }

        static QMap<std::string, table_info_sqlite>  table_info(QSqlDatabase& sql, T table) {
            QMap<std::string, table_info_sqlite> result{};
            QString  query = QString("PRAGMA table_info(\"%1\");").arg(QString::fromStdString(arcirk::enum_synonym(table)));
            QSqlQuery rs(query, sql);
            while (rs.next())
            {
                auto info = table_info_sqlite();
                info.name = rs.value("name").toString().toStdString();
                info.type = rs.value("type").toString().toStdString();
                result.insert(info.name, info);
            }
            return result;
        }

        static QMap<std::string, table_info_sqlite>  table_info(QSqlDatabase& sql, A table) {
            QMap<std::string, table_info_sqlite> result{};
            QString  query = QString("PRAGMA table_info(\"%1\");").arg(QString::fromStdString(arcirk::enum_synonym(table)));
            QSqlQuery rs(query, sql);
            while (rs.next())
            {
                auto info = table_info_sqlite();
                info.name = rs.value("name").toString().toStdString();
                info.type = rs.value("type").toString().toStdString();
                result.insert(info.name, info);
            }
            return result;
        }

        static std::string query_insert(const std::string& table_name, nlohmann::json values, std::vector<std::tuple<std::string, ByteArray>>& blobs){
            std::string result = QString("insert into %1 (").arg(QString::fromStdString(table_name)).toStdString();
            std::string string_values;
            std::vector<std::pair<std::string, nlohmann::json>> m_list;
            auto items_ = values.items();
            for (auto itr = items_.begin(); itr != items_.end(); ++itr) {
                m_list.emplace_back(itr.key(), itr.value());
            }

            for (auto itr = m_list.cbegin(); itr != m_list.cend() ; ++itr) {
                result.append("[" + itr->first + "]");
                std::string value;
                if(itr->second.is_string())
                    value = itr->second.get<std::string>();
                else if(itr->second.is_number_float()){
                    value = std::to_string(itr->second.get<double>());
                    auto index = value.find(",");
                    if(index > 0)
                        value.replace(index, 1, ".");
                }else if(itr->second.is_number_integer())
                    value = std::to_string(itr->second.get<int>());
                else if(itr->second.is_array()){
                    std::string f_name = ":";
                    f_name.append(itr->first);
                    try{
                        auto ba = itr->second.get<ByteArray>();
                        blobs.push_back(std::make_tuple(f_name, ba));
                    } catch (const std::exception &) {
                        blobs.push_back(std::make_tuple(f_name, ByteArray()));
                    }
                    value = f_name;
                }else if(itr->second.is_boolean()){
                    value = itr->second.get<bool>() ? 1 : 0;
                }

                boost::erase_all(value, "'");

                if(value.empty())
                    string_values.append("''");
                else{
                    if(left(value, 1) != ":")
                        string_values.append(QString("'%1'").arg(QString::fromStdString(value)).toStdString());
                    else
                        string_values.append(value);
                }

                if(itr != (--m_list.cend())){
                    result.append(",\n");
                    string_values.append(",\n");
                }
            }
            result.append(")\n");
            result.append("values(");
            result.append(string_values);
            result.append(")");

            return result;
        }

        static void rebase(QSqlDatabase& sql, T table, json object){

            QString table_name = QString::fromStdString(arcirk::enum_synonym(table));
            QString temp_query = QString("create temp table %1_temp as select * from %1;").arg(table_name);

            sql.transaction();
            QSqlQuery query(sql);
            query.exec(temp_query);
            query.exec(QString("drop table %1;").arg(table_name));
            query.exec(get_ddl(object, table));
            sql.commit();

            sql.transaction();
            QSqlQuery rs(QString("select * from %1_temp;").arg(table_name), sql);
            //std::vector<std::string> columns{};
            auto t_info = table_info(sql, table);

            while (rs.next())
            {
                QSqlRecord row = rs.record();
                nlohmann::json values{};

                for(int i = 0; i < row.count(); ++i)
                {
                    //const column_properties & props = row.get_properties(i);
                    const std::string column_name = row.fieldName(i).toStdString();

                    if(t_info.find(column_name) == t_info.end())
                        continue;

                    QVariant val = row.field(i).value();

                    if(val.userType() == QMetaType::QString)
                        values[column_name] = val.toString().toStdString();
                    else if(val.userType() == QMetaType::Double)
                        values[column_name] = val.toDouble();
                    else if(val.userType() == QMetaType::Int)
                        values[column_name] = val.toInt();
                    else if(val.userType() == QMetaType::LongLong)
                        values[column_name] = val.toLongLong();
                    else if(val.userType() == QMetaType::ULongLong)
                        values[column_name] = val.toULongLong();
                    else if(val.userType() == QMetaType::QByteArray){
                        auto q_ba = val.toByteArray();
                        auto ba = ByteArray(q_ba.size());
                        std::copy(q_ba.begin(), q_ba.end(), ba.begin());
                        values[column_name] = ba;
                    }else if(val.userType() == QMetaType::Bool){
                        values[column_name] = val.toBool() ? 1 : 0;
                    }
                }
                std::vector<std::tuple<std::string, ByteArray>> blobs{};
                auto q_string = query_insert(table_name.toStdString(), values, blobs);
                if(!blobs.empty()){
                    query.prepare(q_string.c_str());
                    foreach(auto t, blobs){
                        auto f_name = std::get<0>(t);
                        ByteArray ba = std::get<1>(t);
                        query.bindValue(f_name.c_str(), QByteArray(reinterpret_cast<const char*>(ba.data()), qsizetype(ba.size())));
                    }
                    query.exec();
                }else
                    query.exec(q_string.c_str());
            }
            query.exec(QString("drop table if exists %1_temp;").arg(table_name));
            sql.commit();
        }

        bool verify_structure(QSqlDatabase& sql){

            qDebug() << __FUNCTION__;
            if(m_initial_data_table.empty())
                return false;
            auto release_table_versions = get_release_tables_versions(); //Текущие версии релиза
            auto database_tables = get_database_tables(sql); //Массив существующих таблиц
            auto database_views = get_database_views(sql); //Массив существующих представлений
            //auto tables_arr = tables_name_array(); //Массив имен таблиц

            QSqlQuery rs(sql);
            //Сначала проверим, существует ли таблица версий
            if(std::find(database_tables.begin(), database_tables.end(), "DatabaseConfig") == database_tables.end()) {
                auto tp = m_initial_data_table[0];
                auto table = std::get<0>(tp);
                json data = std::get<1>(tp);
                auto ddl = get_ddl(table, data);
                bool result = rs.exec(ddl);
                if(!result){
                    std::cerr << rs.lastError().text().toStdString() << std::endl;
                    return false;
                }
            }
            //Заполним массив версий для сравнения
            std::map<T, int> current_table_versions;
            rs.exec(QString("select * from %1;").arg("DatabaseConfig"));
            while (rs.next())
            {
                nlohmann::json t_name = rs.value("name").toString().toStdString();
                auto t_ver = rs.value("version").toInt();
                current_table_versions.emplace(t_name.get<T>(), t_ver);
            }

            bool is_rebase = false;
            //Выполним реструктуризацию
            for (const auto& t_name : m_initial_data_table) {
                if(enum_synonym(std::get<0>(t_name))  == "DatabaseConfig")
                    continue;
                //версия таблицы
                int current_ver = 0;
                auto itr_ver = current_table_versions.find(std::get<0>(t_name));
                if(itr_ver != current_table_versions.end())
                    current_ver = itr_ver->second;

                if(std::find(database_tables.begin(), database_tables.end(), arcirk::enum_synonym(std::get<0>(t_name))) == database_tables.end()){
                    //Таблицы не существует, просто создаем новую
                    sql.transaction();
                    rs.exec(get_ddl(std::get<0>(t_name) , std::get<1>(t_name)));
                    std::vector<std::tuple<std::string, ByteArray>> blobs{};
                    auto ref = QUuid::createUuid();
                    auto q_string = query_insert("DatabaseConfig", nlohmann::json{
                            {"name", arcirk::enum_synonym(std::get<0>(t_name))},
                            {"version", release_table_versions[std::get<0>(t_name)]},
                            {"ref",  to_byte(to_binary(ref))} //to_raw(to_variant_p(ref, "DatabaseConfig")}
                    }, blobs);
                    bool result = rs.prepare(q_string.c_str());
                    if(!result)
                        std::cerr << rs.lastError().text().toStdString() << std::endl;
                    foreach(auto t, blobs){
                        auto f_name = std::get<0>(t);
                        ByteArray ba = std::get<1>(t);
                        rs.bindValue(f_name.c_str(), QByteArray(reinterpret_cast<const char*>(ba.data()), qsizetype(ba.size())));
                    }
                    rs.exec();
                    result = sql.commit();
                    if(!result)
                        std::cerr << sql.lastError().text().toStdString() << std::endl;

                    is_rebase = true;
                }else{
                    //Если существует, проверяем версию таблицы если не совпадает запускаем реструктуризацию
                    if(release_table_versions[std::get<0>(t_name)] != current_ver) {
                        rebase(sql, std::get<0>(t_name), std::get<1>(t_name));
                        sql.transaction();

                        if (current_ver == 0){
                            std::vector<std::tuple<std::string, ByteArray>> blobs{};
                            auto ref = QUuid::createUuid();
                            auto q_string = query_insert("DatabaseConfig", nlohmann::json{
                                    {"name", arcirk::enum_synonym(std::get<0>(t_name))},
                                    {"version", release_table_versions[std::get<0>(t_name)]},
                                    {"ref",  to_byte(to_binary(ref))} //ref_to_byte(QUuid::createUuid().toString(QUuid::WithoutBraces).toStdString(), "DatabaseConfig")}
                            }, blobs);
                            rs.prepare(q_string.c_str());
                            foreach(auto t, blobs){
                                auto f_name = std::get<0>(t);
                                ByteArray ba = std::get<1>(t);
                                rs.bindValue(f_name.c_str(), QByteArray(reinterpret_cast<const char*>(ba.data()), qsizetype(ba.size())));
                            }
                            rs.exec();
                        }else
                            rs.exec(QString("update %1 set version='%2' where [name]='%3'").arg(
                                    "DatabaseConfig",
                                    QString::number(release_table_versions[std::get<0>(t_name)]),
                                    QString::fromStdString(arcirk::enum_synonym(std::get<0>(t_name)))));
                        sql.commit();
                    }
                    is_rebase = true;
                }
            }
            //Проверка представлений
            for (auto view : m_initial_data_views) {
                verify_database_views(sql, database_views,  arcirk::enum_synonym(std::get<0>(view)), std::get<1>(view), is_rebase);
            }

            return true;
        }

        bool verify(QSqlDatabase& sql, std::vector<std::tuple<T, json, int>>& initial_data_table, std::vector<std::tuple<A, std::string>>& initial_data_views){

            m_initial_data_table = initial_data_table;
            m_initial_data_views = initial_data_views;

            return verify_structure(sql);
        }
    private:
        std::vector<std::tuple<T, json, int>> m_initial_data_table;
        std::vector<std::tuple<A, std::string>> m_initial_data_views;

    };

}

#endif //CONTROLSPROG_VERIFY_DATABASE_HPP
