//
// Created by admin on 29.01.2024.
//
#include "../include/sql/datautils.h"
#include <QStandardPaths>
#include "../global/sql/verify_database.hpp"
#include "../global/sql/query_builder.hpp"

using namespace arcirk::database;

DataUtils::DataUtils(QSqlDatabase& database) :
m_data(database)
{

}

DataUtils::~DataUtils() {

}

bool DataUtils::verify() {

    if (!m_data.open())
        return false;

    std::vector<std::tuple<tables, json, int>> m_initial_data_table{
        std::make_tuple(tbDatabaseConfig, pre::json::to_json(database_config()), 0),
        std::make_tuple(tbHttpAddresses, pre::json::to_json(profile_item()), 0),
        std::make_tuple(tbApplicationConfig, pre::json::to_json(application_config()), 0),
        std::make_tuple(tbCertificates, pre::json::to_json(certificates()), 0),
        std::make_tuple(tbContainers, pre::json::to_json(containers()), 0),
        std::make_tuple(tbAvailableCertificates, pre::json::to_json(available_certificates()), 0),
    };
    std::vector<std::tuple<views, std::string>> m_initial_data_views{};

    return verify_database<tables, views>().verify(m_data, m_initial_data_table, m_initial_data_views);

}

bool DataUtils::verify_default_data() {

    auto builder = builder::query_builder();
    QSqlQuery rs(builder.row_count().from(enum_synonym(tbApplicationConfig)).prepare().c_str(), m_data);
    rs.exec();
    rs.next();
    int count = rs.value(0).toInt();

    if(count == 0){
        builder.clear();
        auto ref = QUuid::createUuid();
        auto app_conf = application_config();
#ifdef _WINDOWS
        app_conf.firefox  = "C:/Program Files/Mozilla Firefox/firefox.exe";
#endif
        app_conf.name = "profile_manager";
        app_conf.server = "localhost";
        app_conf.autoreconnect = false;
        app_conf.def_password = false;
        app_conf.log_arhive = false;
        app_conf.use_sid = false;
        app_conf.ref = to_byte(to_binary(ref));
        builder.use(pre::json::to_json(app_conf));
        exec_query(builder, builder::sql_query_type::Insert, enum_synonym(tbApplicationConfig));
    }

    return true;
}

bool DataUtils::exec_query(builder::query_builder &builder, builder::sql_query_type type, const std::string& table_name) {

    QSqlQuery query(m_data);
    m_data.transaction();
    if(type == builder::sql_query_type::Insert){
        std::vector<std::tuple<std::string, arcirk::BJson>> blobs;
        auto q_string = builder.insert(table_name, blobs).prepare();
        if(!blobs.empty()){
            query.prepare(q_string.c_str());
            for(auto t : blobs){
                auto f_name = std::get<0>(t);
                arcirk::BJson ba = std::get<1>(t);
                query.bindValue(f_name.c_str(), QByteArray(reinterpret_cast<const char*>(ba.data()), qsizetype(ba.size())));
            }
            query.exec();
        }else
            query.exec(q_string.c_str());
    }
    auto result = m_data.commit();
    if(!result)
        qCritical() << Q_FUNC_INFO << m_data.lastError().text();
    return result;
}
//
//template<class T>
//std::vector<T> DataUtils::get_database_rows(const json &where, const std::string& table_name) {
//
//    //std::vector<arcirk::database::application_config> m_result;
//    auto query = builder::query_builder();
//    query.select().from(table_name).where(where);
//    return query.rows_to_array<T>(m_data);
//}
