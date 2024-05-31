//
// Created by admin on 27.05.2024.
//
#include "../include/sql_query.hpp"

sql_query::sql_query(std::shared_ptr<session>& sess)
: sql(sess),
  m_recordset(*sql)
{
    m_values = {};
}

sql_query::~sql_query() {

}

void sql_query::set_text(const std::string &query_text) {
    m_text = query_text;
}

void sql_query::set_value(const std::string &field, const variant_type& value) {
    m_values.emplace(field, value);
}

bool sql_query::execute() {

    m_recordset.clean_up();
    //m_recordset  = (sql->prepare << m_text, use(m_values));

//    soci::session sql_ = soci::session();
//
//    for (auto itr : m_values) {
//        if (itr.second.type() == typeid(std::string)){
//            std::string value = boost::get<std::string>(itr.second);
//            sql_ << "" , (soci::use(value, std::string(itr.first))), soci::use(m_values);
//        }
//
//    }

    return m_recordset.execute();
}

bool sql_query::next() {
    return m_recordset.fetch();
}

void sql_query::clear() {
    m_recordset.clean_up();
    m_text = "";
    m_values.clear();
}

