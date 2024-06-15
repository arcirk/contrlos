//
// Created by admin on 13.01.2024.
//
#include "../include/tableconf.h"

using namespace arcirk::widgets;

TableConf::TableConf()
: IViewsConf()
{
    Q_INIT_RESOURCE(controls_resource);
    m_size                  = QSize(22, 22);
    m_columns               = HeaderItems {std::make_shared<header_item>("ref", "Ссылка")};
    m_enable_rows_icons     = false;
    m_read_only             = true;
    m_row_icon              = QIcon("://img/item.png");
}

QSize TableConf::size() const {
    return m_size;
}

HeaderItems& TableConf::columns() {
    return m_columns;
}

void TableConf::reset_columns(const json& arr){
    Q_ASSERT(arr.is_array());
    m_columns.clear();

    for (auto itr = arr.begin(); itr != arr.end(); ++itr) {
        const auto& item = *itr;
        if(item.is_string())
            m_columns.push_back(std::make_shared<header_item>(item.get<std::string>(), ""));
        else if(item.is_object()){
            m_columns.push_back(std::make_shared<header_item>(item));
        }
//        else if(item.is_array()){
//            auto ba = item.get<ByteArray>();
//            std::error_code ec;
//            auto m_raw = alpaca::deserialize<header_item_wrapper>(ba, ec);
//            if (!ec) {
//                m_columns.push_back(to_header_item(m_raw));
//            }else
//                throw arcirk::NativeException(ec.message().c_str());
//        }
        else
            throw arcirk::NativeException("Не верный формат данных!");
    }
    if(index_of_for_name("ref", m_columns) == -1)
        m_columns.push_back(std::make_shared<header_item>("ref", "Ссылка"));
}

QString TableConf::column_name(int index, bool alias) const {
    Q_ASSERT(m_columns.size() > index);
    Q_ASSERT(index>=0);
    if(!alias)
        return m_columns[index]->name.c_str();
    else{
        if(!m_columns[index]->alias.empty())
            return m_columns[index]->alias.c_str();
        else
            return m_columns[index]->name.c_str();
    }
}

int TableConf::column_index(const QString &name) {
    return (int)index_of_for_name(name.toStdString(), m_columns);
}

void TableConf::set_columns_aliases(const QMap<QString, QString> &aliases) {
    m_column_aliases = aliases;
    for(auto itr = aliases.begin(); itr != aliases.end(); ++itr){
        auto index = index_of_for_name(itr.key().toStdString(), m_columns);
        if(index != -1){
            m_columns[index]->alias = itr.value().toStdString();
        }
    }
}

QMap<QString, QString> TableConf::columns_aliases() const  {
    return m_column_aliases;
}

void TableConf::set_columns_order(const QList<QString> &names) {
    size_t i = 0;
    foreach(auto name, names){
        auto current_index = index_of_for_name(name.toStdString(), m_columns);
        if(current_index != -1){
            move_element(m_columns, current_index, i);
        }
        i++;
    }
    m_order_columns.clear();
    m_order_columns.resize(names.size());
    std::copy(names.begin(), names.end(), m_order_columns.begin());
}

QList<QString> TableConf::columns_order() const {
    return m_order_columns;
}

bool TableConf::read_only() const {
    return m_read_only;
}

void TableConf::set_read_only(bool value) {
    m_read_only = value;
}

void TableConf::reorder_columns(const QList<QString>& param) {
    set_columns_order(param);
}

void TableConf::display_icons(bool value) {
    m_enable_rows_icons = value;
}

bool TableConf::is_display_icons() const {
    return m_enable_rows_icons;
}

QIcon TableConf::default_icon() const {
    return m_row_icon;
}

void TableConf::set_default_icon(const QIcon &ico) {
    m_row_icon = ico;
}

void TableConf::set_column_role(const QString &column, editor_inner_role role) {

    auto index = index_of_for_name(column.toStdString(), m_columns);
    if(index != -1){
        m_columns[index]->default_type = role;
    }

}

void TableConf::set_column_not_public(const QString &column, bool value) {
    auto index = index_of_for_name(column.toStdString(), m_columns);
    if(index != -1){
        m_columns[index]->not_public = value;
    }
}

void TableConf::set_column_select_type(const QString &column, bool value) {
    auto index = index_of_for_name(column.toStdString(), m_columns);
    if(index != -1){
        m_columns[index]->select_type = value;
    }
}

void TableConf::set_column_not_public(const QList<QString> &columns, bool value) {
    foreach(auto column, columns){
        set_column_not_public(column, value);
    }
}

QList<QString> TableConf::predefined_list() const {
    return {"ref", "row_state", "predefined", "version"};
}

header_item * TableConf::header_column(const QString &name) {
    auto index = index_of_for_name(name.toStdString(), m_columns);
    Q_ASSERT(index != -1);
    return m_columns[index].get();
}
