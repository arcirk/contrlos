//
// Created by admin on 12.01.2024.
//
#include "../include/treemodel.h"
#include "../../global/variant/item_data.h"

using namespace arcirk::widgets;

TreeModel::TreeModel(QObject *parent)
        : QAbstractItemModel{parent}
{
    m_conf = std::make_shared<TreeConf>();
    auto root = m_conf->predefined_object();
    rootItem = new TreeItem(root, m_conf);
    rootItem->set_mapped(true);
    rootItem->set_not_move(true);
    is_use_database = false;
    m_hierarchical_list = true;
}

TreeModel::TreeModel(const json &rootData, QObject *parent) : QAbstractItemModel{parent}
{
    m_conf = std::make_shared<TreeConf>();
    is_use_database = false;

    Q_ASSERT(rootData.is_object());

    auto _root = m_conf->restructure_facility(rootData);

    rootItem = new TreeItem(_root, m_conf);

    auto table = json::object();
    auto columns = json::array();
    auto column = header_item();
    column.name = "ref";
    column.default_type = editor_inner_role::editorText;
    column.default_value = to_nil_uuid();
    columns += pre::json::to_json(column);
    for (auto itr = _root.items().begin(); itr != _root.items().end(); ++itr) {
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
        columns += pre::json::to_json(column); //itr.key();
    }
    table["columns"] = columns;
    table["rows"] = json::array();

    form_json(table);
}
int TreeModel::rowCount(const QModelIndex &parent) const
{
    TreeItem *parentItem = getItem(parent);
    return parentItem->childCount();
}

int TreeModel::columnCount(const QModelIndex &parent) const
{
    return (int)m_conf->columns().size();
}

QVariant TreeModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid()) return {};
    if(role == Qt::SizeHintRole)
        return m_conf->size();
    TreeItem *item = getItem(index);
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
            else{
                if(item->is_group()){
                    if(!item->predefined())
                        return m_conf->default_icon(tree_rows_icons::ItemGroup);
                    else
                        return m_conf->default_icon(tree_rows_icons::ItemGroupPredefined);
                }else
                    if(!item->predefined())
                        return m_conf->default_icon(tree_rows_icons::Item);
                    else
                        return m_conf->default_icon(tree_rows_icons::ItemPredefined);
            }

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

void TreeModel::form_json(const json &table, bool resel_columns) {

    Q_ASSERT(table.is_object());
    Q_ASSERT(table.find("columns")!=table.end());
    Q_ASSERT(table.find("rows")!=table.end());

    if(resel_columns){
        beginResetModel();
        m_conf->reset_columns(table["columns"]);
        endResetModel();
    }

    for (auto itr = table["rows"].begin(); itr != table["rows"].end(); ++itr) {
        const auto& object = *itr;
        json j_parent = object.value("parent", BJson());
        QModelIndex parent = QModelIndex();
        if(!j_parent.empty()){
            auto item_ba = item_data(j_parent);
            auto uuid = QUuid::fromRfc4122(item_ba.data()->data);
            parent = find(uuid);
        }
        add(object, parent);
    }

}

QVariant TreeModel::headerData(int section, Qt::Orientation orientation, int role) const {

    if (orientation == Qt::Horizontal)
        if(role == Qt::SizeHintRole)
            return m_conf->size();

    if (orientation == Qt::Horizontal && role == Qt::DisplayRole){;
        return m_conf->column_name(section, true);
    }
//    if(role == Qt::SizeHintRole)
//        return m_conf->size();

    return QAbstractItemModel::headerData(section, orientation, role);
}

QString TreeModel::column_name(int index) const {
    Q_ASSERT(m_conf->columns().size() > index);
    Q_ASSERT(index>=0);
    return m_conf->columns()[index].name.c_str();
}

QModelIndex TreeModel::index(int row, int column, const QModelIndex &parent) const {
    if (parent.isValid() && parent.column() != 0) return QModelIndex();
    TreeItem *parentItem = getItem(parent);
    TreeItem *childItem = parentItem->child(row);
    if (childItem) return createIndex(row, column, childItem);
    else return {};
}

QModelIndex TreeModel::parent(const QModelIndex &index) const {
    if (!index.isValid()) return QModelIndex();
    TreeItem *childItem = getItem(index);
    TreeItem *parentItem = childItem->parentItem();
    if (parentItem == rootItem) return QModelIndex();
    return createIndex(parentItem->childNumber(), 0, parentItem);
}

bool TreeModel::setHeaderData(int section, Qt::Orientation orientation, const QVariant &value, int role) {
    return QAbstractItemModel::setHeaderData(section, orientation, value, role);
}

void TreeModel::headerChanged(Qt::Orientation orientation, int section) {
    emit headerDataChanged(orientation, section, section);
}

TreeModel::~TreeModel() {
    delete rootItem;
}

TreeItem *TreeModel::getItem(const QModelIndex &index) const {
    if (index.isValid()) {
        auto *item = static_cast<TreeItem*>(index.internalPointer());
        if (item){
            return item;
        }
    }
    return rootItem;
}

void TreeModel::clear()
{
    if(rowCount() == 0)
        return;
    beginResetModel();
    rootItem->clearChildren();
    endResetModel();
}

json TreeModel::to_json() {
    return to_table_model(QModelIndex());
}

json TreeModel::to_array(const QModelIndex &parent, bool hierarchy, bool group_only) {
    auto result = json::array();
    if (hierarchy) {
        to_array_recursive(parent, result, group_only);
    } else{
        for (int i = 0; i < rowCount(parent); ++i) {
            auto index = this->index(i, 0);
            auto item = getItem(index);
            if(group_only)
                if(item->is_group())
                    result += item->to_object();
            else
                result += item->to_object();
        }
    }
    return result;
}

json TreeModel::row(const QModelIndex &index, bool lite) const {
    if(!index.isValid())
        return {};
    auto item = getItem(index);
    return item->to_object(lite);
}

QModelIndex TreeModel::add(const json &object, const QModelIndex &parent) {
    TreeItem *rootItem_ = getItem(parent);
    //std::cout << rootItem_->ref().toString().toStdString() << std::endl;
    int position = rootItem_->childCount();
    if (!insertRow(position, parent)) return QModelIndex();
    TreeItem *item = getItem(index(position, 0, parent));
    if(item){
        item->set_object(object);
        rootItem_ = getItem(index(position, 0, parent).parent());
    }
    //std::cout << "item: " << item->ref().toString().toStdString() << std::endl;
    //std::cout << "parent:  " << rootItem_->ref().toString().toStdString() << std::endl;
    return index(position, 0, parent);
}

bool TreeModel::remove(const QModelIndex &index) {
    return removeRow(index.row(), index.parent());
}

bool TreeModel::move_up(const QModelIndex &index) {

    Q_ASSERT(index.isValid());
    if(index.row() < 1)
        return false;

    auto root = getItem(index.parent());

    beginMoveRows(index.parent(), index.row(), index.row(), index.parent(), index.row()-1);
    auto itemInfo = getItem(index);
    Q_ASSERT(itemInfo != nullptr);
    auto pos = itemInfo->childNumber();

    if(root != nullptr)
        root->moveChildren(pos, pos - 1);

    endMoveRows();

    return true;
}

bool TreeModel::move_down(const QModelIndex &index) {
    Q_ASSERT(index.isValid());
    if(index.row() > rowCount(index.parent()) - 2)
        return false;

    auto root = getItem(index.parent());

    beginMoveRows(index.parent(), index.row(), index.row(), index.parent(), index.row()+1);
    auto itemInfo = getItem(index);
    Q_ASSERT(itemInfo != nullptr);
    auto pos = itemInfo->childNumber();

    if(root != nullptr){
        beginMoveRows(index.parent(), pos, pos, index.parent(), (pos >= root->childCount()) ? root->childCount() : (pos + 2));
        root->moveChildren(pos, pos + 1);
    }

    endMoveRows();
    return true;
}

int TreeModel::column_index(const QString &name) const {
    return m_conf->column_index(name);
}

QList<QString> TreeModel::columns() const {
    QList<QString> cols;
    for(const auto& itr: m_conf->columns()){
        cols.append(itr.name.c_str());
    }
    return cols;
}

bool TreeModel::insertColumns(int position, int columns, const QModelIndex &parent) {
    return QAbstractItemModel::insertColumns(position, columns, parent);
}

bool TreeModel::removeColumns(int position, int columns, const QModelIndex &parent) {
    return QAbstractItemModel::removeColumns(position, columns, parent);
}

bool TreeModel::insertRows(int position, int rows, const QModelIndex &parent) {
    TreeItem *parentItem = getItem(parent);
    bool success;
    beginInsertRows(parent, position, position + rows - 1);
    success = parentItem->insertChildren(position, rows, (int)m_conf->columns().size());
    endInsertRows();
    return success;
}

bool TreeModel::removeRows(int position, int rows, const QModelIndex &parent) {
    TreeItem *parentItem = getItem(parent);
    bool success = true;
    beginRemoveRows(parent, position, position + rows - 1);
    success = parentItem->removeChildren(position, rows);
    endRemoveRows();
    return success;
}

bool TreeModel::setData(const QModelIndex &index, const QVariant &value, int role) {

    if (role != Qt::EditRole &&
        role != Qt::UserRole &&
        role != Qt::ForegroundRole &&
        role != Qt::DecorationRole &&
        role != TABLE_DATA) return false;

    TreeItem *item = getItem(index);
    return item->setData(index.column(), value, role);

}

void TreeModel::set_read_only(bool value) {
    m_conf->set_read_only(value);
}

bool TreeModel::read_only() {
    return m_conf->read_only();
}

Qt::ItemFlags TreeModel::flags(const QModelIndex &index) const {
    if (!index.isValid())
        return Qt::NoItemFlags;

    auto fl = QAbstractItemModel::flags(index);
    auto item_read_only = index.data(TABLE_ITEM_READ_ONLY).toBool();
    if(!m_conf->read_only())
        if(!item_read_only)
            fl = fl | Qt::ItemIsEditable;

    return fl;
}

[[maybe_unused]] json TreeModel::empty_data() {
    auto object = json::object();
    for(const auto& val : m_conf->columns()){
        auto var = item_data();
        var.set_role(val.default_type);
        var.set_value(val.default_value);
        object[val.name] = var.json_value();
    }
    return object;
}

void TreeModel::set_object(const QModelIndex &index, const json &object) {
    if(!index.isValid())
        return;
    auto item = getItem(index);
    item->set_object(object);
}

void TreeModel::reorder_columns(const QList<QString> &names) {
    m_conf->reorder_columns(names);
}

void TreeModel::display_icons(bool value) {
    m_conf->display_icons(value);
}

bool TreeModel::is_display_icons() const {
    return m_conf->is_display_icons();
}

void TreeModel::set_columns_aliases(const QMap<QString, QString> &aliases) {
    m_conf->set_columns_aliases(aliases);
}

QMap<QString, QString> TreeModel::columns_aliases() const {
    return m_conf->columns_aliases();
}

json TreeModel::to_array(const QString &column, const QModelIndex &parent, bool hierarchy, bool group_only) const {

    if(columns().indexOf(column) == -1)
        return {};
    auto result = json::array();
    for (int i = 0; i < rowCount(parent); ++i) {
        auto index = this->index(i, 0);
        auto item = getItem(index);
        auto object = item->to_object();
        if(group_only){
            if(item->is_group())
                result += object.value(column.toStdString(), json{});
        } else{
            result += object.value(column.toStdString(), json{});
        }
    }

    return result;
}

bool TreeModel::use_database() const {
    return is_use_database;
}

bool TreeModel::hierarchical_list() {
    return m_hierarchical_list;
}

QModelIndex TreeModel::find(const QUuid &ref, const QModelIndex &parent) const {

    for (int i = 0; i < rowCount(parent); ++i) {
        auto item = index(i, 0, parent);
        auto it = getItem(item);
        if(ref == it->ref())
            return item;
        else{
            auto ind = find(ref, item);
            if(ind.isValid())
                return ind;
        }
    }

    return QModelIndex();
}

QModelIndex TreeModel::find(int column, const QVariant &source, const QModelIndex &parent) const {

    QModelIndexList matches = match(
            index(0,column, parent),
            Qt::DisplayRole,
            source,
            -1,
            Qt::MatchRecursive);
            foreach (const QModelIndex &match, matches){
            return match;
        }
    return QModelIndex();
}

QList<QModelIndex> TreeModel::find_all(int column, const QVariant &source, const QModelIndex &parent) const {
    QList<QModelIndex> result{};
    QModelIndexList matches = match(
            index(0,column, parent),
            Qt::DisplayRole,
            source,
            -1,
            Qt::MatchRecursive);
        foreach (const QModelIndex &match, matches){
            result << match;
        }
    return result;
}

bool TreeModel::is_group(const QModelIndex &index) {
    auto item = getItem(index);
    if(!item)
        return false;
    else
        return item->is_group();
}

QList<QString> TreeModel::columns_order() const {
    return m_conf->columns_order();
}

void TreeModel::set_hierarchical_list(bool value) {
    m_hierarchical_list = value;
    emit hierarchicalListChanged(value);
}

std::vector<std::string> TreeModel::predefined_fields() const {

    auto f = std::vector<std::string>{
        arcirk::enum_synonym(ftreeRef),
        arcirk::enum_synonym(ftreeParent),
        arcirk::enum_synonym(ftreeIsGroup),
        arcirk::enum_synonym(ftreeVersion),
        arcirk::enum_synonym(ftreePredefined)
    };

    return f;
}

QUuid TreeModel::ref(const QModelIndex &index) const {
    if(!index.isValid())
        return QUuid();
    auto item = getItem(index);
    if(item == rootItem)
        return QUuid();
    return item->ref();
}

bool TreeModel::hasChildren(const QModelIndex &parent) const {
    if(m_conf->fetch_expand()){
        auto parentInfo = getItem(parent);
        Q_ASSERT(parentInfo!=0);
        return parentInfo->is_group();
    }else
        return QAbstractItemModel::hasChildren(parent);
}

bool TreeModel::canFetchMore(const QModelIndex &parent) const {
    //qDebug() << __FUNCTION__;
    if(m_conf->fetch_expand()){
        auto parentInfo = getItem(parent);
        Q_ASSERT(parentInfo != 0);
        if(parentInfo == rootItem)
            return false;
        if(parentInfo->is_group()){
            if(parentInfo->childCount() > 0)
                return false;
            else
                return !parentInfo->mapped();
        }else
            return false;
    }else
        return false;
}

void TreeModel::fetchMore(const QModelIndex &parent) {
    if(!m_conf->fetch_expand()){
        return QAbstractItemModel::fetchMore(parent);
    }

    auto parentInfo = getItem(parent);
    if(!parentInfo->mapped()){
        emit fetch(parent);
    }

    return QAbstractItemModel::fetchMore(parent);
}

bool TreeModel::predefinedItem(const QModelIndex &index) const {
    auto item = getItem(index);
    return item->predefined();
}

bool TreeModel::row_not_move(const QModelIndex &index) {
    if(index.isValid()){
        auto item = getItem(index);
        return item->not_move();
    }else
        return false;
}

void TreeModel::set_row_not_move(const QModelIndex &index, bool value) {
    if(!index.isValid())
        return;
    auto item = getItem(this->index(index.row(), 0, index.parent()));
    item->set_not_move(value);
}

json TreeModel::to_object(const QModelIndex &index, bool lite) const {
    return row(index, lite);
}

json TreeModel::to_table_model(const QModelIndex &parent, bool group_only, bool lite) {

    auto columns_j = json::array();
    for (auto& key : m_conf->columns()) {
        columns_j += key.name;
    }

    auto rows = json::array();

    to_array_recursive(parent, rows, group_only);

    return json::object({
                                {"columns" , columns_j},
                                {"rows", rows}
                        });
}

void TreeModel::to_array_recursive(const QModelIndex &parent, json& result, bool group_only) {
    for (int i = 0; i < rowCount(parent); ++i) {
        auto index = this->index(i, 0, parent);
        auto item = getItem(index);
        //result += item->to_object();
        if(!group_only) {
            result += item->to_object();
            to_array_recursive(index, result, group_only);
        }else{
            if(item->is_group()) {
                result += item->to_object();
                to_array_recursive(index, result, group_only);
            }
        }
    }
}

void TreeModel::to_array_recursive(const QString& column, const QModelIndex &parent, json& result, bool group_only) {
    if(columns().indexOf(column) == -1)
        return;
    for (int i = 0; i < rowCount(parent); ++i) {
        auto index = this->index(i, 0, parent);
        auto item = getItem(index);
        result += item->to_object();
        if(!group_only) {
            result += item->to_object().value(column.toStdString(), json{});
            to_array_recursive(column, index, result, group_only);
        }else{
            if(item->is_group()) {
                result += item->to_object().value(column.toStdString(), json{});
                to_array_recursive(column, index, result, group_only);
            }
        }
    }
}

bool TreeModel::belongsToItem(const QModelIndex &index, const QModelIndex parent)
{
    auto item = getItem(index);
    auto parentItem = getItem(parent);
    if(item == parentItem)
        return true;
    auto index_ = find(item->ref(), parent);
    return index_.isValid();
}

void TreeModel::move_to(const QModelIndex &index, const QModelIndex &new_parent)
{
    Q_ASSERT(index.isValid());
    Q_ASSERT(new_parent.isValid());

    auto itemInfo = getItem(index);
    Q_ASSERT(itemInfo != nullptr);
    auto parentInfo = getItem(new_parent);
    auto oldParentInfo = getItem(index.parent());
    Q_ASSERT(parentInfo != nullptr);

    beginMoveRows(index.parent(), index.row(), index.row(), new_parent, rowCount(new_parent));
    itemInfo->setParent(parentInfo);
    parentInfo->appendChild(itemInfo);
    oldParentInfo->removeChildren(index.row(), 1);
    endMoveRows();
}

void TreeModel::to_tree(json& result, const QModelIndex &parent) {

    if(!result.is_array())
        result = json::array();

    auto items = json::array();
    auto parentInfo = getItem(parent);
    auto object = parentInfo->to_object();

    for (int i = 0; i < rowCount(parent); ++i) {
        auto index = this->index(i, 0, parent);
        auto itemInfo = getItem(index);
        auto child = itemInfo->to_object();
        if(itemInfo->is_group()){
            to_tree(items, index);
        }else{
            child["items"] = json::array();
            items += child;
        }
    }
    object["items"] = items;
    result += object;
}

int TreeModel::first_element_position(const QModelIndex &index) {

    auto item = getItem(index);
    bool group = item->is_group();
    int position = 0;

    for (int i = 0; i < rowCount(index.parent()); ++i) {
        auto cur_index = this->index(i, 0, index.parent());
        if(group == is_group(cur_index)){
            position = cur_index.row();
            break;
        }
    }

    return position;
}

int TreeModel::last_element_position(const QModelIndex &index) {

    auto item = getItem(index);
    bool group = item->is_group();
    int position = 0;

    for (int i = 0; i < rowCount(index.parent()); ++i) {
        auto cur_index = this->index(i, 0, index.parent());
        if(group == is_group(cur_index)){
            position = cur_index.row();
        }
    }

    return position;
}
