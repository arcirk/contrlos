//
// Created by admin on 29.01.2024.
//

#ifndef CONTROLSPROG_DATAUTILS_H
#define CONTROLSPROG_DATAUTILS_H

#include <global.hpp>
#include <sql/query_builder.hpp>
#include <QSqlDatabase>
#include <QUuid>
#include "../../include/server_conf_n.hpp"

namespace arcirk::database{

    class DataUtils{

        public:
            explicit DataUtils(QSqlDatabase& database, arcirk::server::server_config& conf);
            ~DataUtils();

            bool verify();
            bool verify_default_data();
            bool exec_query(arcirk::database::builder::query_builder& builder, arcirk::database::builder::sql_query_type type, const std::string& table_name, const BJson& ref = {});

    private:
        QSqlDatabase& m_data;
        arcirk::server::server_config& m_conf;
    };

}

#endif //CONTROLSPROG_DATAUTILS_H
