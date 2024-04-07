//
// Created by admin on 13.01.2024.
//
#include "../include/tableitem.h"

using namespace arcirk::widgets;

TableItem::TableItem(const json &data, std::shared_ptr<TableConf>& conf, TableItem *parentItem)
    : m_conf(conf)
{
    m_parentItem = parentItem;
    m_childItems = {};
    init(data);
}

TableItem::~TableItem() {
    qDeleteAll(m_childItems);
}

void TableItem::appendChild(TableItem *child) {
    m_childItems.append(child);
}

TableItem *TableItem::child(int row) {
    return m_childItems.value(row);
}

int TableItem::childCount() const {
    return (int)m_childItems.count();
}

QVariant TableItem::data(int column, int role) const {

    if(m_data.empty())
        return {};

    if(column >= m_data.size() || column < 0)
        return {};

    auto column_name = m_conf->column_name(column);
    std::string key = column_name.toStdString();
    const auto itr = std::find_if(m_data.begin(), m_data.end(), [key](const value_pair& it){
        return it.first == key;
    });
    if(itr == m_data.end())
        return {};

    if(role == TABLE_DATA){
        return to_variant(itr->second->to_byte());
    }else if(role == TABLE_ITEM_READ_ONLY){
        auto ro = m_read_only.find(column_name);
        if(ro != m_read_only.end())
            return ro.value();
        else
            return false;
    }else if(role == TABLE_DATA_VALUE){
        return to_variant(itr->second->json_value());
    }else if(role == TABLE_ITEM_ROLE){
        return itr->second->role();
    }else if(role == TABLE_ITEM_SUBTYPE){
        return itr->second->data()->subtype;
    }else if(role == TABLE_ITEM_SELECT_TYPE){
        return m_conf->columns()[m_conf->column_index(column_name)].select_type;
    }else if(role == Qt::DisplayRole){
        if(itr->second->role() == editorBoolean){
            auto value = itr->second->json_value();
            if(value.is_boolean())
                return value.get<bool>();
            else if(value.is_number_integer())
                return value.get<int>() > 0;
            else
                return false;
        }else
            return itr->second->representation().c_str();
    }else if(role == Qt::DecorationRole){
        auto ico_itr = m_icon.find(column_name);
        if(ico_itr != m_icon.end())
            return ico_itr.value();
    }

    return {};
}

bool TableItem::setData(int column, const QVariant &value, int role) {
    if (column < 0 || column >= m_data.size()) return false;
    auto column_name = m_conf->column_name(column);
    if(role == Qt::EditRole){
        if(column_name == "ref")
            return false;
        //auto itr = m_data.find(column_name.toStdString());
        std::string key = column_name.toStdString();
        const auto itr = std::find_if(m_data.begin(), m_data.end(), [key](const value_pair& it){
            return it.first == key;
        });
        if(itr == m_data.end())
            return false;
        auto index = std::distance(m_data.begin(), itr);

        if(value.isValid()) {
            m_data[index] = to_value_pair(key, from_variant(value));////to_variant_p(to_data(value));
        }else{
            m_data[index] = to_value_pair(key, json(""));
        }
    }else if(role == TABLE_ITEM_READ_ONLY){
        m_read_only.insert(column_name, value.toBool());
    }else if(role == TABLE_DATA){
        if(column_name == "ref")
            return false;
        //auto itr = m_data.find(column_name.toStdString());
        std::string key = column_name.toStdString();
        const auto itr = std::find_if(m_data.begin(), m_data.end(), [key](const value_pair& it){
            return it.first == key;
        });
        if(itr == m_data.end())
            return false;
        auto index = std::distance(m_data.begin(), itr);
        m_data[index].second->from_json(from_variant(value)); //to_value_pair(key, from_variant(value));
    }else if(role == Qt::DecorationRole) {
        auto ico = qvariant_cast<QIcon>(value);
        m_icon.insert(column_name, ico);
    }
    return true;
}

int TableItem::childNumber() const {
    if (m_parentItem) return (int)m_parentItem->m_childItems.indexOf(const_cast<TableItem*>(this));
    return 0;
}

void TableItem::init(const json& data) {
    set_object(data);
}

bool TableItem::insertChildren(int position, int count, int columns)
{
    Q_UNUSED(columns);
    if (position < 0 || position > m_childItems.size()) return false;
    for (int row = 0; row < count; ++row) {
        auto data = json::object();
        auto *item = new TableItem(data, m_conf, this);
        m_childItems.insert(position, item);
    }
    return true;
}


bool TableItem::removeChildren(int position, int count)
{
    if (position < 0 || position + count > m_childItems.size()) return false;
    for (int row = 0; row < count; ++row) m_childItems.removeAt(position);
    return true;
}

bool TableItem::moveChildren(int source, int position)
{
    if(position < 0 || position >= m_childItems.size())
        return false;
    m_childItems.move(source, position);
    return true;
}


void TableItem::clearChildren()
{
    m_childItems.clear();
}

void TableItem::set_object(const json &object) {

    //std::cout << object.dump(4) << std::endl;
    if(object.empty())
        return;
    m_data = object_to_map(object);
    const std::string key = "ref";
    const auto itr = std::find_if(m_data.begin(), m_data.end(), [key](const value_pair& it){
        return it.first == key;
    });
    if(itr == m_data.end()) {
        auto var = std::make_shared<item_data>(to_byte(to_binary(QUuid::createUuid())));
        var->set_role(editor_inner_role::editorDataReference) ;
        m_data.push_back(std::make_pair(key, std::move(var)));
    }
}

json TableItem::to_object(bool lite) {
    json object = json::object();
    for (const auto& itr : m_data) {
        if(lite)
            object[itr.first] = itr.second->data()->to_json_value(); //object[itr.first] = get_value(itr.second.data);
        else{
            object[itr.first] = itr.second->to_byte(); //pre::json::to_json(itr.second);
        }

    }
    return object;
}

variant_map TableItem::to_map() const {
    return m_data;
}
