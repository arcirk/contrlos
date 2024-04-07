//
// Created by admin on 14.02.2024.
//
#include "../include/itabledatabasemodel.h"
#include "../../global/sql/query_builder.hpp"
#include <QSqlQuery>
#include <QSqlRecord>
#include <QSqlField>
#include <QSqlError>

using namespace arcirk::widgets;
using namespace arcirk::database::builder;

template<class T>
ITableDatabase<T>::ITableDatabase(const T &rootData, QSqlDatabase &data, const std::string& table, QObject *parent)
        : TableModel(pre::json::to_json(T()), parent),
        m_db(data)
{
    Q_ASSERT(!table.empty());
    m_table_name = table;
    is_use_database = true;
}

template<class T>
void ITableDatabase<T>::load() {

    if(!m_db.isOpen())
        return;

    this->clear();

    auto query = query_builder();
    query.select().from(m_table_name);
    if(!m_column_order.empty())
        query.order_by(json{m_column_order});

    auto rs = QSqlQuery(query.prepare().c_str(), m_db);
    rs.exec();

    while (rs.next()){
        auto item = T();
        auto item_json = pre::json::to_json(item);
        QSqlRecord row = rs.record();

        for (int i = 0; i < row.count(); ++i) {
            std::string column_name = row.fieldName(i).toStdString();
            if(item_json.find(column_name) == item_json.end())
                continue;
            QVariant val = row.field(i).value();
            if(val.userType() == QMetaType::QString && item_json[column_name].is_string())
                item_json[column_name] = val.toString().toStdString();
            else if(val.userType() == QMetaType::Double && item_json[column_name].is_number())
                item_json[column_name] = val.toDouble();
            else if(val.userType() == QMetaType::Int && item_json[column_name].is_number())
                item_json[column_name] = val.toInt();
            else if(val.userType() == QMetaType::Int && item_json[column_name].is_boolean())
                item_json[column_name] = val.toInt() > 0 ? true : false;
            else if(val.userType() == QMetaType::LongLong && item_json[column_name].is_number())
                item_json[column_name] = val.toLongLong();
            else if(val.userType() == QMetaType::ULongLong && item_json[column_name].is_number())
                item_json[column_name] = val.toULongLong();
            else if(val.userType() == QMetaType::QByteArray && item_json[column_name].is_array())
                item_json[column_name] = qbyte_to_byte(val.toByteArray());
        }
        item = pre::json::from_json<T>(item_json);
        auto index = add_object(item, false);
        if(index.isValid()){
            auto item_d = arcirk::widgets::item_data(item.icon);
            if(item_d.data()->subtype == subtypeByte){
                auto qba = QByteArray(reinterpret_cast<const char*>(item_d.data()->data.data()), item_d.data()->data.size());
                QPixmap p;
                p.loadFromData(qba);
                auto icon = QIcon(p);
                if(!icon.isNull())
                    this->setData(index, QIcon(p), Qt::DecorationRole);
            }
        }
    }

}

template<class T>
void ITableDatabase<T>::set_column_order(const std::string &column) {
    m_column_order = column;
}

template<class T>
void ITableDatabase<T>::set_object(int row, const T &object) {
    set_object(row, true);
}

template<class T>
QModelIndex ITableDatabase<T>::add_object(const T &object) const {
    return add_object(object, true);
}

template<class T>
void ITableDatabase<T>::set_object(int row, const T &object, bool update) {
    auto index = this->index(row, 0);
    if(index.isValid()){
        set_object(index, pre::json::to_json(object));
        if(update)
            update_database(object);
    }
}

template<class T>
QModelIndex ITableDatabase<T>::add_object(const T &object, bool update) const {
    auto index = add(pre::json::to_json(object));
    if(update)
        update_database(object);
    return index;
}

template<class T>
void ITableDatabase<T>::update_database(const T &object) {

    if(!m_db.isOpen())
        return;

    auto query = query_builder();
    auto rs = QSqlQuery(m_db);
    std::vector<std::tuple<std::string, arcirk::ByteArray>> blobs;
    rs.prepare(query.row_count().from(m_table_name).where(json{{"ref", object.ref}}, blobs).prepare().c_str());
    auto ba = QByteArray(reinterpret_cast<const char*>(object.ref.data()), (qsizetype)object.ref.size());
    rs.bindValue(0, ba);
    rs.exec();
    int count = 0;
    if(rs.lastError().type() == QSqlError::NoError){
        while (rs.next()){
            count = rs.value(0).toInt();
        }
    }
    query.clear();
    query.use(pre::json::to_json(object));
    rs.clear();
    blobs.clear();

    m_db.transaction();
    std::string query_string;
    if(count > 0){
        query_string = query.update(m_table_name, blobs).where(json{{"ref", object.ref}}, blobs).prepare();
    }else{
        query_string = query.insert(m_table_name, blobs).prepare();
    }
    rs.prepare(query_string.c_str());
    for(auto itr : blobs){
        std::string key = std::get<0>(itr);
        ByteArray b = std::get<1>(itr);
        auto qba = QByteArray(reinterpret_cast<const char*>(b.data()), (qsizetype)b.size());
        rs.bindValue(key.c_str(), qba);
    }
    auto result = rs.exec();
    m_db.commit();
    if(!result){
        std::cerr << query_string << std::endl;
        std::cerr << rs.lastError().text().toStdString() << std::endl;
    }
}

template<class T>
void ITableDatabase<T>::onRowChanged(const QModelIndex &index) {
    TableModel::onRowChanged(index);
}

template<class T>
void ITableDatabase<T>::updateRowsPositions() {
    TableModel::updateRowsPositions();
}

template<class T>
void ITableDatabase<T>::removeRow(const json &object) {
    TableModel::removeRow(object);
}

