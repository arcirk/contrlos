#ifndef QUERY_BUILDER_METADATA_HPP
#define QUERY_BUILDER_METADATA_HPP

#include "../global/global.hpp"
#include "../global/variant/item_data.h"
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlRecord>
#include "query_builder.hpp"
#include <itree.hpp>
#include <itable.hpp>

BOOST_FUSION_DEFINE_STRUCT(
    (arcirk::database), ibase_object_structure,
    (std::string, name)
    (std::string, alias)
    (std::string, full_name)
    (std::string, object_type)
    (std::string, data_type)
    (std::string, query)
    (bool, user_query)
    (arcirk::BJson, ref)
    (arcirk::BJson, parent)
    (arcirk::BJson, base_ref)
    (arcirk::BJson, base_parent)
    (std::string, parent_name)
    (std::string, parent_alias)
    (std::string, groupe_as)
    (std::string, package_ref)
    (std::string, def_value)
    (int, size)
    (bool, is_group)
)

BOOST_FUSION_DEFINE_STRUCT(
    (arcirk::database), table_info_sqlite,
    (int, cid)
    (std::string, name)
    (std::string, type)
    (int, notnull)
    (std::string, dflt_value)
    (int, bk)
)

BOOST_FUSION_DEFINE_STRUCT(
    (arcirk::query_builder_ui), query_builder_packet,
    (int, line_num)
    (std::string, name)
    (BJson, ref)
    (BJson, parent)
    (int, type)
    (bool, use_limit)
    (int, row_limit)
    (bool, use_distinct)
    (int, is_group)
)

using namespace arcirk::database::builder;

inline QStringList sqlite_types_qt = {
    "INTEGER",
    "BIGINT",
    "DATE",
    "DATETIME",
    "DECIMAL",
    "DOUBLE",
    "INT",
    "NONE",
    "NUMERIC",
    "REAL",
    "STRING",
    "TEXT",
    "TIME",
    "VARCHAR",
    "CHAR"
};

namespace arcirk::database::metadata{

    static inline QList<QString> get_database_tables(QSqlDatabase& sql){

        QSqlQuery rs("SELECT name FROM sqlite_master WHERE type='table';", sql);
        rs.exec();
        QList<QString> result;
        while (rs.next())
        {
            result.append(rs.value(0).toString());
        }

        return result;
    }

    static inline QList<QString> get_database_views(QSqlDatabase& sql){

        QSqlQuery rs("SELECT name FROM sqlite_master WHERE type='view';", sql);
        rs.exec();
        QList<QString> result;
        while (rs.next())
        {
            result.append(rs.value(0).toString());
        }

        return result;
    }

    static inline std::map<std::string, arcirk::database::table_info_sqlite>  table_info(QSqlDatabase& sql, const std::string& table, sql_database_type type, const std::string& database_name = "") {

        std::map<std::string, table_info_sqlite> result{};
        std::string  query = strings::str_sample("PRAGMA table_info(\"%1%\");", table);
        std::string c_name = "name";
        std::string c_type = "type";
        if(type == sql_database_type::type_ODBC){
            QSqlQuery rc(strings::str_sample("USE %1%;", database_name).c_str(), sql);
            rc.exec();
            query = strings::str_sample("SELECT * FROM INFORMATION_SCHEMA.COLUMNS WHERE table_name = '%1%'", table);
            c_name = "COLUMN_NAME";
            c_type = "DATA_TYPE";
        }

        QSqlQuery rs(query.c_str(), sql);

        while (rs.next()) {
            auto info = table_info_sqlite();
            info.name = rs.value(c_name.c_str()).toString().toStdString();
            info.type = rs.value(c_type.c_str()).toString().toStdString();
            result.emplace(info.name, info);
        }
        return result;
    }

    inline json sql_metadata(QSqlDatabase& sql,
                      std::map<std::string, std::map<std::string, ibase_object_structure>>& vec,
                      const BJson& parent = {}){

        typedef std::map<std::string, ibase_object_structure> details_t;
        typedef std::pair<std::string, ibase_object_structure> details_pair;
        typedef std::pair<std::string, details_t> object_pair;

        if(!sql.isOpen())
            return {};

        auto database_tables = get_database_tables(sql);
        auto database_views = get_database_views(sql);

        std::vector<json> m_groups;
        std::vector<json> m_children;

        BJson m_parent = to_nil_uuid();

        if(!parent.empty())
            m_parent = parent;

        auto res = json::object();
        auto m_tables= ibase_object_structure();
        m_tables.name = "Таблицы";
        m_tables.alias = "Таблицы";
        m_tables.full_name = "Таблицы";
        m_tables.object_type = "tablesRoot";
        std::string ref = boost::to_string(arcirk::md5_to_uuid(arcirk::to_md5(m_tables.name + m_tables.object_type)));
        m_tables.ref = to_byte(to_binary(QUuid::fromString(ref.c_str())));
        m_tables.parent = m_parent;
        m_tables.is_group = 1;
        m_groups.push_back(pre::json::to_json(m_tables));
        auto m_views= ibase_object_structure();
        m_views.name = "Представления";
        m_views.alias = "Представления";
        m_views.alias = "Представления";
        m_views.object_type = "viewsRoot";
        ref = boost::to_string(arcirk::md5_to_uuid(arcirk::to_md5(m_views.name + m_views.object_type)));
        m_views.ref = to_byte(to_binary(QUuid::fromString(ref.c_str())));
        m_views.parent = m_parent;
        m_views.is_group = 1;
        m_groups.push_back(pre::json::to_json(m_views));

        for (auto const& table: database_tables) {
            //int count = 0;
            if(table == "sqlite_sequence")
                continue;

            auto m_struct = ibase_object_structure();
            m_struct.name = table.toStdString();
            m_struct.alias = table.toStdString();
            m_struct.full_name = table.toStdString();
            m_struct.data_type = "";
            ref = boost::to_string(arcirk::md5_to_uuid(arcirk::to_md5(m_struct.name + m_struct.object_type)));
            m_struct.ref =  to_byte(to_binary(QUuid::fromString(ref.c_str())));
            m_struct.parent = m_tables.ref;
            m_struct.is_group = 1;
            m_struct.object_type = "table";
            m_struct.base_ref = m_struct.ref; //arcirk::uuids::random_uuid());
            m_struct.base_parent = m_tables.ref;

            m_groups.push_back(pre::json::to_json(m_struct));


            auto details = table_info(sql, table.toStdString(), sql_database_type::type_Sqlite3);

            auto details_vec = details_t();

            for (auto const& itr : details) {
                auto m_details = ibase_object_structure();
                m_details.data_type = itr.second.type;
                m_details.name = itr.second.name;
                m_details.alias = itr.second.name;
                m_details.full_name = table.toStdString() + "." + itr.second.name;
                m_details.query = table.toStdString() + "." + itr.second.name;
                ref = boost::to_string(arcirk::md5_to_uuid(arcirk::to_md5(m_details.name + m_details.object_type)));
                m_details.ref = to_byte(to_binary(QUuid::fromString(ref.c_str())));
                m_details.parent = m_struct.ref;
                m_details.base_ref = m_details.ref; //arcirk::uuids::random_uuid());
                m_details.base_parent = m_struct.ref;
                m_details.is_group = 0;
                m_details.object_type = "field";
                m_details.parent_alias = m_struct.name;
                m_details.parent_name = m_struct.name;
                m_children.push_back(pre::json::to_json(m_details));
                details_vec.insert(details_pair(m_details.name, m_details));
            }

            vec.insert(object_pair(table.toStdString(), details_vec));
        }

        for (auto const& view: database_views) {
            auto m_struct = ibase_object_structure();
            m_struct.name = view.toStdString();
            m_struct.alias = view.toStdString();
            m_struct.full_name = view.toStdString();
            m_struct.object_type = "view";
            ref = boost::to_string(arcirk::md5_to_uuid(arcirk::to_md5(m_struct.name + m_struct.object_type)));
            m_struct.ref =  to_byte(to_binary(QUuid::fromString(ref.c_str())));
            m_struct.parent = m_views.ref;
            m_struct.is_group = 0;
            m_struct.base_ref = m_struct.ref; //arcirk::uuids::random_uuid());
            m_struct.base_parent = m_views.ref;
            m_children.push_back(pre::json::to_json(m_struct));
            vec.insert(object_pair(view.toStdString(), {}));
        }

        auto m_empty = pre::json::to_json(ibase_object_structure());

        auto columns = json::array();

        for (auto itr = m_empty.items().begin(); itr != m_empty.items().end(); ++itr) {
            columns += itr.key();
        }
        res["columns"] = columns;

        auto rows = json::array();
        for (auto gr : m_groups) {
            rows += gr;
        }
        for (auto gr : m_children) {
            rows += gr;
        }
        res["rows"] = rows;

        return res;
    }

}

typedef ITable<arcirk::database::ibase_object_structure> ITableIBaseModel;

#endif //QUERY_BUILDER_METADATA_HPP