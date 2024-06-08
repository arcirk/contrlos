//
// Created by admin on 25.05.2024.
//

#ifndef SERVER_DATAUTILS_H
#define SERVER_DATAUTILS_H

#include <global.hpp>
#include <QSqlDatabase>

#include <sql/query_builder.hpp>


namespace arcirk::database{

    class ServerDataUtils{

    public:
        explicit ServerDataUtils(QSqlDatabase& database);
        ~ServerDataUtils();

        bool verify();
        bool verify_default_data();
        bool exec_query(arcirk::database::builder::query_builder& builder, arcirk::database::builder::sql_query_type type, const std::string& table_name);

    private:
        QSqlDatabase& m_data;
    };

}
#endif //CONTROLSPROG_DATAUTILS_H
