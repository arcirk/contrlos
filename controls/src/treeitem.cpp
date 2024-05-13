//
// Created by admin on 13.01.2024.
//
#include "../include/treeitem.h"

using namespace arcirk::widgets;

TreeItem::TreeItem(const json &data, std::shared_ptr<TreeConf>& conf, TreeItem *parentItem)
        : m_conf(conf)
{
    m_parentItem = parentItem;
    m_childItems = {};
    init(data);
}

TreeItem::~TreeItem() {
    qDeleteAll(m_childItems);
}

void TreeItem::appendChild(TreeItem *child) {
    m_childItems.append(child);
}

TreeItem *TreeItem::child(int row) {
    return m_childItems.value(row);
}

int TreeItem::childCount() const {
    return (int)m_childItems.count();
}

QVariant TreeItem::data(int column, int role) const {

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

bool TreeItem::setData(int column, const QVariant &value, int role) {
    if (column < 0 || column >= m_data.size()) return false;
    auto column_name = m_conf->column_name(column);
    if(role == Qt::EditRole){
        if(column_name == "ref")
            return false;
        const std::string key = column_name.toStdString();
        const auto itr = std::find_if(m_data.begin(), m_data.end(), [key](const value_pair& it){
            return it.first == key;
        });
        if(itr == m_data.end())
            return false;
        auto index = std::distance(m_data.begin(), itr);

        if(value.isValid()) {
            m_data[index] = to_value_pair(key, from_variant(value));
        }else{
            m_data[index] = to_value_pair(key, json(""));
        }
    }else if(role == TABLE_ITEM_READ_ONLY){
        m_read_only.insert(column_name, value.toBool());
    }else if(role == TABLE_DATA){
        if(column_name == "ref")
            return false;
        std::string key = column_name.toStdString();
        const auto itr = std::find_if(m_data.begin(), m_data.end(), [key](const value_pair& it){
            return it.first == key;
        });
        if(itr == m_data.end())
            return false;
        auto index = std::distance(m_data.begin(), itr);
        m_data[index].second->from_json(from_variant(value));
    }else if(role == Qt::DecorationRole) {
        auto ico = qvariant_cast<QIcon>(value);
        m_icon.insert(column_name, ico);
    }
    return true;
}

int TreeItem::childNumber() const {
    if (m_parentItem) return (int)m_parentItem->m_childItems.indexOf(const_cast<TreeItem*>(this));
    return 0;
}

void TreeItem::init(const json& data) {
    set_object(data);
}

bool TreeItem::insertChildren(int position, int count, int columns)
{
    Q_UNUSED(columns);
    if (position < 0 || position > m_childItems.size()) return false;
    for (int row = 0; row < count; ++row) {
        auto data = json::object();
        auto *item = new TreeItem(data, m_conf, this);
        m_childItems.insert(position, item);
    }
    return true;
}


bool TreeItem::removeChildren(int position, int count)
{
    if (position < 0 || position + count > m_childItems.size()) return false;
    for (int row = 0; row < count; ++row) m_childItems.removeAt(position);
    return true;
}

bool TreeItem::moveChildren(int source, int position)
{
    if(position < 0 || position >= m_childItems.size())
        return false;
    m_childItems.move(source, position);
    return true;
}


void TreeItem::clearChildren()
{
    m_childItems.clear();
}

void TreeItem::set_object(const json &object) {

    if(object.empty())
        return;

    m_data = object_to_map(object);

    std::vector<std::string> m_fields{"ref", "parent", "is_group", "row_state"};

    for (const auto& key : m_fields) {
        if(key == "ref"){
            const auto itr = std::find_if(m_data.begin(), m_data.end(), [key](const value_pair& it){
                return it.first == key;
            });
            if(itr == m_data.end()) {
                m_ref = QUuid::createUuid();
                auto var = std::make_shared<item_data>(to_byte(to_binary(m_ref)));
                var->set_role(editor_inner_role::editorDataReference) ;
                m_data.push_back(std::make_pair(key, std::move(var)));
            }else{
                auto ba = itr->second->data();
                if(ba->subtype == variant_subtype::subtypeRef){
                    m_ref = QUuid::fromRfc4122(ba->data);
                    //std::cout << m_ref.toString().toStdString() << std::endl;
                }
            }
        }else if(key == "parent"){
            const auto itr = std::find_if(m_data.begin(), m_data.end(), [key](const value_pair& it){
                return it.first == key;
            });
            if(itr == m_data.end()) {
                auto var = std::make_shared<item_data>(to_byte(to_binary(QUuid())));
                var->set_role(editor_inner_role::editorDataReference) ;
                m_data.push_back(std::make_pair(key, std::move(var)));
            }
        }else if(key == "is_group"){
            const auto itr = std::find_if(m_data.begin(), m_data.end(), [key](const value_pair& it){
                return it.first == key;
            });
            if(itr == m_data.end()) {
                auto var = std::make_shared<item_data>(to_byte(to_binary(false)));
                var->set_role(editor_inner_role::editorBoolean) ;
                m_data.push_back(std::make_pair(key, std::move(var)));
            }
        }else if(key == "row_state"){
            const auto itr = std::find_if(m_data.begin(), m_data.end(), [key](const value_pair& it){
                return it.first == key;
            });
            if(itr == m_data.end()) {
                int val = is_group() ? tree_rows_icons::ItemGroup : tree_rows_icons::Item;
                auto var = std::make_shared<item_data>(to_byte(to_binary(val)));
                var->set_role(editor_inner_role::editorNumber) ;
                m_data.push_back(std::make_pair(key, std::move(var)));
            }
        }
    }
}

json TreeItem::to_object(bool lite) {
    json object = json::object();
    for (const auto& itr : m_data) {
        if(lite)
            object[itr.first] = itr.second->data()->to_json_value();
        else{
            object[itr.first] = itr.second->to_byte();
        }

    }
    return object;
}

variant_map TreeItem::to_map() const {
    return m_data;
}

bool TreeItem::is_group() {
    const std::string key = "is_group";
    const auto itr = std::find_if(m_data.begin(), m_data.end(), [key](const value_pair& it){
        return it.first == key;
    });
    if(itr == m_data.end()) {
        return childCount() > 0;
    }else{
        auto val = itr->second->data()->to_json_value();
        if(val.is_boolean())
            return val.get<bool>();
        else if(val.is_number())
            return val.get<int>() > 0;
        else
            return childCount() > 0;
    }
}

QUuid TreeItem::ref() const {
    return m_ref;
}
