//
// Created by admin on 12.01.2024.
//
#include "../include/tablemodel.h"
#include "../../global/variant/item_data.h"

using namespace arcirk::widgets;

TableModel::TableModel(QObject *parent)
        : QAbstractItemModel{parent}
{
    m_conf = std::make_shared<TableConf>();
    rootItem = new TableItem(json::object({{"ref", ""}}), m_conf);
    is_use_database = false;
}

int TableModel::rowCount(const QModelIndex &parent) const
{
    TableItem *parentItem = getItem(parent);
    return parentItem->childCount();
}

int TableModel::columnCount(const QModelIndex &parent) const
{
    return (int)m_conf->columns().size();
}

QVariant TableModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid()) return QVariant();
    if(role == Qt::SizeHintRole)
        return m_conf->size();
    TableItem *item = getItem(index);
    Q_ASSERT(item != nullptr);
    if (role == Qt::ForegroundRole){
        return item->data(index.column(), role);
    }else if (role == Qt::EditRole){
        return item->data(index.column());
    }else if (role == Qt::DisplayRole){
        return item->data(index.column());
    }else if (role == Qt::CheckStateRole) {
        return {};
    }else if (role == Qt::DecorationRole){
        auto item_icon = item->data(index.column(), Qt::DecorationRole);
        if(index.column() == 0 && m_conf->is_display_icons()){
            if(item_icon.isValid())
                return item_icon;
            else
                return m_conf->default_icon();
        }else{
            if(item_icon.isValid())
                return item_icon;
        }
    }else if (role == TABLE_DATA){
        return item->data(index.column(), TABLE_DATA);
    }else if (role == TABLE_ITEM_READ_ONLY){
        return item->data(index.column(), TABLE_ITEM_READ_ONLY);
    }else if (role == TABLE_DATA_VALUE){
        return item->data(index.column(), TABLE_DATA_VALUE);
    }else if (role == TABLE_ITEM_ROLE){
        return item->data(index.column(), TABLE_ITEM_ROLE);
    }else if (role == TABLE_ITEM_SUBTYPE){
        return item->data(index.column(), TABLE_ITEM_SUBTYPE);
    }else if (role == TABLE_ITEM_SELECT_TYPE){
        return item->data(index.column(), TABLE_ITEM_SELECT_TYPE);
    }else{
//        auto v = index.data(Qt::DisplayRole);
//        if(!v.isValid())
//            return QVariant();
//        if(v.typeId() == QMetaType::Bool){
//            if(v.toBool()){
//                return m_conf->logical_icons().first;
//            }else{
//                return m_conf->logical_icons().second;
//            }
//        }
    }

    return {};
}

void TableModel::from_json(const json &table, bool reset_columns) {

    Q_ASSERT(table.is_object());
    Q_ASSERT(table.find("columns")!=table.end());
    Q_ASSERT(table.find("rows")!=table.end());

    if(reset_columns) {
        beginResetModel();
        m_conf->reset_columns(table["columns"]);
        endResetModel();
    }

    for (auto itr = table["rows"].begin(); itr != table["rows"].end(); ++itr) {
        const auto& object = *itr;
        add(object);
    }

}

QVariant TableModel::headerData(int section, Qt::Orientation orientation, int role) const {

    if (orientation == Qt::Horizontal)
        if(role == Qt::SizeHintRole)
            return QSize(22, 22);//m_conf->size();

    if (orientation == Qt::Horizontal && role == Qt::DisplayRole){;
        return m_conf->column_name(section, true);
    }

    return QAbstractItemModel::headerData(section, orientation, role);
}

QString TableModel::column_name(int index) const {
    Q_ASSERT(m_conf->columns().size() > index);
    Q_ASSERT(index>=0);
    return m_conf->columns()[index]->name.c_str();
}

QModelIndex TableModel::index(int row, int column, const QModelIndex &parent) const {
    if (parent.isValid() && parent.column() != 0) return QModelIndex();
    TableItem *parentItem = getItem(parent);
    TableItem *childItem = parentItem->child(row);
    if (childItem) return createIndex(row, column, childItem);
    else return {};
}

QModelIndex TableModel::parent(const QModelIndex &index) const {
    return {};
}

bool TableModel::setHeaderData(int section, Qt::Orientation orientation, const QVariant &value, int role) {
    return QAbstractItemModel::setHeaderData(section, orientation, value, role);
}

void TableModel::headerChanged(Qt::Orientation orientation, int section) {
    emit headerDataChanged(orientation, section, section);
}

TableModel::~TableModel() {
    delete rootItem;
}

TableItem *TableModel::getItem(const QModelIndex &index) const {
    if (index.isValid()) {
        auto *item = static_cast<TableItem*>(index.internalPointer());
        if (item){
            return item;
        }
    }
    return rootItem;
}

void TableModel::clear()
{
    if(rowCount() == 0)
        return;
    beginResetModel();
    rootItem->clearChildren();
    endResetModel();
}

json TableModel::to_json() const {
    auto columns_j = json::array();
    for (const auto& itr: m_conf->columns()) {
        columns_j += itr->name;
    }
    auto rows = to_array();

    return json::object({
            {"columns" , columns_j},
            {"rows", rows}
    });
}

json TableModel::to_array() const {
    auto result = json::array();
    for (int i = 0; i < rowCount(); ++i) {
        auto index = this->index(i, 0);
        auto item = getItem(index);
        result += item->to_object();
    }
    return result;
}

json TableModel::row(const QModelIndex &index, bool lite) const {
    if(!index.isValid())
        return {};
    auto item = getItem(index);
    return item->to_object(lite);
}

QModelIndex TableModel::add(const json &object) {
    TableItem *rootItem_ = getItem(QModelIndex());
    int position = rootItem_->childCount();
    if (!insertRow(position, QModelIndex())) return {};
    TableItem *item = getItem(index(position, 0, QModelIndex()));
    if(item){
        item->set_object(object);
    }
    return index(position, 0, QModelIndex());
}

bool TableModel::remove(const QModelIndex &index) {
    return removeRow(index.row(), index.parent());
}

bool TableModel::move_up(const QModelIndex &index) {

    Q_ASSERT(index.isValid());
    if(index.row() < 1)
        return false;

    beginMoveRows(index.parent(), index.row(), index.row(), index.parent(), index.row()-1);
    auto itemInfo = getItem(index);
    Q_ASSERT(itemInfo != 0);
    auto pos = itemInfo->childNumber();

    if(rootItem != 0)
        rootItem->moveChildren(pos, pos - 1);

    endMoveRows();

    return true;
}

bool TableModel::move_down(const QModelIndex &index) {
    Q_ASSERT(index.isValid());
    if(index.row() > rowCount(index.parent()) - 2)
        return false;

    beginMoveRows(index.parent(), index.row(), index.row(), index.parent(), index.row()+1);
    auto itemInfo = getItem(index);
    Q_ASSERT(itemInfo != 0);
    auto pos = itemInfo->childNumber();

    if(rootItem != 0){
        beginMoveRows(index.parent(), pos, pos, index.parent(), (pos >= rootItem->childCount()) ? rootItem->childCount() : (pos + 2));
        rootItem->moveChildren(pos, pos + 1);
    }

    endMoveRows();
    return true;
}

int TableModel::column_index(const QString &name) const {
    return m_conf->column_index(name);
}

TableModel::TableModel(const json &rootData, QObject *parent) : QAbstractItemModel{parent}
{
    m_conf = std::make_shared<TableConf>();
    is_use_database = false;

    Q_ASSERT(rootData.is_object());

    rootItem = new TableItem(rootData, m_conf);

    auto table = json::object();
    auto columns = json::array();
    auto column = header_item();
    column.name = "ref";
    column.default_type = editor_inner_role::editorText;
    column.default_value = to_byte(arcirk::to_binary(QUuid::fromString(NIL_STRING_UUID)));//to_data(json{NIL_STRING_UUID}, subtypeUUID);
    columns += column.to_json();
    for (auto itr = rootData.items().begin(); itr != rootData.items().end(); ++itr) {
        if(itr.key() == "ref")
            continue;
        column = header_item();
        column.name = itr.key();
        json val{};
        if(itr.value().is_string()){
            column.default_type = editor_inner_role::editorText;
            column.default_value =  to_byte(to_binary(itr.value(), variant_subtype::subtypeDump));// to_data(itr.value());
        }else if(itr.value().is_array()){
            column.default_type = editor_inner_role::editorByteArray;
            column.default_value = to_byte(to_binary(itr.value(), variant_subtype::subtypeArray));//to_data(itr.value(), subtypeArray);
        }else if(itr.value().is_number()){
            column.default_type = editor_inner_role::editorNumber;
            column.default_value = to_byte(to_binary(itr.value(), variant_subtype::subtypeDump));//to_data(itr.value());
        }else if(itr.value().is_boolean()){
            column.default_type = editor_inner_role::editorBoolean;
            column.default_value = to_byte(to_binary(itr.value(), variant_subtype::subtypeDump));//to_data(itr.value());
        }else if(itr.value().is_binary()){
            column.default_type = editor_inner_role::editorByteArray;
            column.default_value = to_byte(itr.value());//to_data(itr.value(), subtypeByte);
        }
        columns += column.to_json(); //itr.key();
    }
    table["columns"] = columns;
    table["rows"] = json::array();

    from_json(table);
}

QList<QString> TableModel::columns() const {
    QList<QString> cols;
    for(const auto& itr: m_conf->columns()){
        cols.append(itr->name.c_str());
    }
    return cols;
}

bool TableModel::insertColumns(int position, int columns, const QModelIndex &parent) {
    return QAbstractItemModel::insertColumns(position, columns, parent);
}

bool TableModel::removeColumns(int position, int columns, const QModelIndex &parent) {
    return QAbstractItemModel::removeColumns(position, columns, parent);
}

bool TableModel::insertRows(int position, int rows, const QModelIndex &parent) {
    TableItem *parentItem = getItem(parent);
    bool success;
    beginInsertRows(parent, position, position + rows - 1);
    success = parentItem->insertChildren(position, rows, (int)m_conf->columns().size());
    endInsertRows();
    return success;
}

bool TableModel::removeRows(int position, int rows, const QModelIndex &parent) {
    TableItem *parentItem = getItem(parent);
    bool success = true;
    beginRemoveRows(parent, position, position + rows - 1);
    success = parentItem->removeChildren(position, rows);
    endRemoveRows();
    return success;
}

bool TableModel::setData(const QModelIndex &index, const QVariant &value, int role) {

    if (role != Qt::EditRole &&
        role != Qt::UserRole &&
        role != Qt::ForegroundRole &&
        role != Qt::DecorationRole &&
        role != TABLE_DATA) return false;

    TableItem *item = getItem(index);
    return item->setData(index.column(), value, role);

}

void TableModel::set_read_only(bool value) {
    m_conf->set_read_only(value);
}

bool TableModel::read_only() {
    return m_conf->read_only();
}

Qt::ItemFlags TableModel::flags(const QModelIndex &index) const {
    if (!index.isValid())
        return Qt::NoItemFlags;

    auto fl = QAbstractItemModel::flags(index);
    auto item_read_only = index.data(TABLE_ITEM_READ_ONLY).toBool();
    if(!m_conf->read_only())
        if(!item_read_only)
            fl = fl | Qt::ItemIsEditable;

    return fl;
}

json TableModel::empty_data() {
    auto object = json::object();
    for(const auto& val : m_conf->columns()){
        auto var = item_data();
        var.set_role(val->default_type);
        //var.set_value(val->default_value);
        var.from_json(val->default_value);
        object[val->name] = var.json_value();
    }
    return object;
}

void TableModel::set_object(const QModelIndex &index, const json &object) {
    if(!index.isValid())
        return;
    auto item = getItem(index);
    item->set_object(object);
}

void TableModel::reorder_columns(const QList<QString> &names) {
    m_conf->reorder_columns(names);
}

void TableModel::display_icons(bool value) {
    m_conf->display_icons(value);
}

bool TableModel::is_display_icons() const {
    return m_conf->is_display_icons();
}

void TableModel::set_columns_aliases(const QMap<QString, QString> &aliases) {
    m_conf->set_columns_aliases(aliases);
}

QMap<QString, QString> TableModel::columns_aliases() const {
    return m_conf->columns_aliases();
}

json TableModel::to_array(const QString &column) const {

    if(columns().indexOf(column) == -1)
        return {};
    auto result = json::array();
    for (int i = 0; i < rowCount(); ++i) {
        auto index = this->index(i, 0);
        auto item = getItem(index);
        auto object = item->to_object();
        result += object.value(column.toStdString(), json{});
    }

    return result;
}

bool TableModel::use_database() const {
    return is_use_database;
}

QList<QString> TableModel::columns_order() const {
    return m_conf->columns_order();
}

QUuid TableModel::ref(const QModelIndex &index) const {
    auto item = getItem(index);
    return item->ref();
}

std::vector<std::string> TableModel::predefined_fields() const {

    auto f = std::vector<std::string>{
            arcirk::enum_synonym(fRef),
            arcirk::enum_synonym(fVersion)
    };

    return f;
}

bool TableModel::predefinedItem(const QModelIndex &index) const {
    auto item = getItem(index);
    return item->predefined();
}

