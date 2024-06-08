//
// Created by admin on 29.01.2024.
//
#include "../../include/sql/datautils.h"
#include <QStandardPaths>
#include <sql/verify_database.hpp>
#include <sql/query_builder.hpp>
#include <variant/item_data.h>

using namespace arcirk::database;


DataUtils::DataUtils(QSqlDatabase& database, arcirk::server::server_config& conf) :
    m_data(database),
    m_conf(conf)
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
        std::make_tuple(tbMstscConnections, pre::json::to_json(mstsc_item()), 0),
        std::make_tuple(tbServerConfig, pre::json::to_json(arcirk::server::server_config()), 0),
    };
    std::vector<std::tuple<views, std::string>> m_initial_data_views{};

    return verify_database<tables, views>().verify(m_data, m_initial_data_table, m_initial_data_views);

}

bool DataUtils::verify_default_data() {

    auto builder = builder::query_builder();
    QSqlQuery rs(builder.row_count().from(enum_synonym(tbServerConfig)).prepare().c_str(), m_data);
    rs.exec();
    rs.next();
    int count = rs.value(0).toInt();

    builder.use(pre::json::to_json(m_conf));

    if(count == 0){
        exec_query(builder, builder::sql_query_type::Insert, enum_synonym(tbServerConfig));
    }else{
        exec_query(builder, builder::sql_query_type::Update, enum_synonym(tbServerConfig));
    }

    return true;
}

bool DataUtils::exec_query(builder::query_builder &builder, builder::sql_query_type type, const std::string& table_name, const BJson& ref) {

    QSqlQuery query(m_data);
    m_data.transaction();
    std::string q_string;
    std::vector<std::tuple<std::string, arcirk::BJson>> blobs;
    if(type == builder::sql_query_type::Insert){       
        q_string = builder.insert(table_name, blobs).prepare();
    }else if(type == builder::sql_query_type::Update){
        q_string = builder.update(table_name, blobs).where(json::object({{"ref", ref}}), blobs).prepare();
    }
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
    auto result = m_data.commit();
    if(!result)
        qCritical() << Q_FUNC_INFO << m_data.lastError().text();
    return result;
}
