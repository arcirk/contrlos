//
// Created by admin on 25.05.2024.
//

#ifndef CONTROLSPROG_DATAUTILS_H
#define CONTROLSPROG_DATAUTILS_H

#include <global.hpp>
#include <soci/soci.h>
#include <soci/sqlite3/soci-sqlite3.h>
#include <soci/boost-fusion.h>
#include <soci/odbc/soci-odbc.h>

#include <sql/query_builder.hpp>

#include <memory>

namespace arcirk::database{

    class DataUtils{

    public:
        explicit DataUtils(const std::string& connection_string);
        ~DataUtils();

        bool verify();
        bool verify_default_data();
        bool exec_query(arcirk::database::builder::query_builder& builder, arcirk::database::builder::sql_query_type type, const std::string& table_name);

    private:
        std::string m_connection_string;
    };

}
#endif //CONTROLSPROG_DATAUTILS_H
