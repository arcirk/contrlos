//
// Created by admin on 25.05.2024.
//
#include "../include/datautils.h"
#include <sql/verify_database.hpp>

using namespace arcirk::database;

DataUtils::DataUtils(const std::string& connection_string)
{
    m_connection_string = connection_string;
}

DataUtils::~DataUtils() {

}

bool DataUtils::verify() {

//    if (!m_data->is_connected())
//        return false;

    std::vector<std::tuple<tables, json, int>> m_initial_data_table{
            std::make_tuple(tbDatabaseConfig, pre::json::to_json(database_config()), 0),
            std::make_tuple(tbHttpAddresses, pre::json::to_json(profile_item()), 0),
            std::make_tuple(tbApplicationConfig, pre::json::to_json(application_config()), 0),
            std::make_tuple(tbCertificates, pre::json::to_json(certificates()), 0),
            std::make_tuple(tbContainers, pre::json::to_json(containers()), 0),
            std::make_tuple(tbAvailableCertificates, pre::json::to_json(available_certificates()), 0),
            std::make_tuple(tbMstscConnections, pre::json::to_json(mstsc_item()), 0),
    };
    std::vector<std::tuple<views, std::string>> m_initial_data_views{};

    auto util =  verify_database();

    auto result = util.verify(m_connection_string, m_initial_data_table, m_initial_data_views);

    return true;

}

bool DataUtils::verify_default_data() {

//    auto builder = builder::query_builder();
//    soci::rowset<soci::row> rs = m_data->prepare << builder.row_count().from(enum_synonym(tbApplicationConfig)).prepare();
//    int count = 0;
//    for (rowset<row>::const_iterator it = rs.begin(); it != rs.end(); ++it)
//    {
//        count = it->get<int>(0);
//    }
//
//    if(count == 0){
//        builder.clear();
//        auto ref = generate_uuid();
//        auto app_conf = application_config();
//#ifdef _WINDOWS
//        app_conf.firefox  = "C:/Program Files/Mozilla Firefox/firefox.exe";
//#endif
//        app_conf.name = "profile_manager";
//        app_conf.server = "localhost";
//        app_conf.autoreconnect = false;
//        app_conf.def_password = false;
//        app_conf.log_arhive = false;
//        app_conf.use_sid = false;
//        app_conf.ref = to_byte(to_binary(ref));
//        builder.use(pre::json::to_json(app_conf));
//        exec_query(builder, builder::sql_query_type::Insert, enum_synonym(tbApplicationConfig));
//    }

    return true;
}

bool DataUtils::exec_query(builder::query_builder &builder, builder::sql_query_type type, const std::string& table_name) {

//    auto tr = soci::transaction(*m_data);
//    if(type == builder::sql_query_type::Insert){
//        std::vector<std::tuple<std::string, arcirk::BJson>> blobs;
//        auto q_string = builder.insert(table_name, blobs).prepare();
//        if(!blobs.empty()){
//            soci::statement rs = m_data->prepare << q_string;
//            for(auto t : blobs){
//                auto f_name = std::get<0>(t);
//                arcirk::BJson ba = std::get<1>(t);
//                soci::blob b(*m_data);
//                b.write(0, reinterpret_cast<const char*>( ba.data() ), ba.size());
//                //m_data << soci::use(b, f_name);
//            }
//            rs.execute();
//        }else
//            *m_data << q_string;
//    }
//    tr.commit();

    return true;
}