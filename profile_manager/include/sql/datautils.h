//
// Created by admin on 29.01.2024.
//

#ifndef CONTROLSPROG_DATAUTILS_H
#define CONTROLSPROG_DATAUTILS_H

#include "../../global/global.hpp"
#include "../../global/sql/query_builder.hpp"
#include <QSqlDatabase>
#include <QUuid>

namespace arcirk::database{

    class DataUtils{

        public:
            explicit DataUtils(QSqlDatabase& database);
            ~DataUtils();

            bool verify();
            bool verify_default_data();
            bool exec_query(arcirk::database::builder::query_builder& builder, arcirk::database::builder::sql_query_type type, const std::string& table_name);
//        template<class T>
//            std::vector<T> get_database_rows(const json& where, const std::string& table_name);
//

    private:
        QSqlDatabase& m_data;
    };

}

#endif //CONTROLSPROG_DATAUTILS_H
