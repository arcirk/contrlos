#include "treeitemmodel.h"

#ifdef USE_QUERY_BUILDER_LIB
#include "query_builder.hpp"
#include <QSqlDatabase>
#include <QSqlError>
#endif

#ifdef USE_WEBSOCKET_LIB
#include <QNetworkAccessManager>
#include <QNetworkReply>
#endif

using namespace arcirk::tree_model;

#ifdef USE_QUERY_BUILDER_LIB
using namespace arcirk::database;
#endif

TreeItemModel::TreeItemModel(QObject *parent)
    : QAbstractItemModel(parent)
{
    init_class();
}

TreeItemModel::TreeItemModel(const json &rootData, QObject *parent)
    : QAbstractItemModel(parent)
{
    init_class(rootData);
}

TreeItemModel::~TreeItemModel()
{
    delete rootItem;
}

QVariant TreeItemModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal)
        if(role == Qt::SizeHintRole)
            return m_conf->size();

    if (orientation == Qt::Horizontal && role == Qt::DisplayRole){;
        return m_conf->column_name_(section, true);
    }

    return QVariant();
}

bool TreeItemModel::setHeaderData(int section, Qt::Orientation orientation, const QVariant &value, int role)
{
    if (role != Qt::EditRole || orientation != Qt::Horizontal) return false;
    m_conf->set_header_data(section, value, role);
    emit headerDataChanged(orientation, section, section);
    return true;
}

void TreeItemModel::headerChanged(Qt::Orientation orientation, int section)
{
    emit headerDataChanged(orientation, section, section);
}

QModelIndex TreeItemModel::index(int row, int column, const QModelIndex &parent) const
{
    if (parent.isValid() && parent.column() != 0) return QModelIndex();
    TreeItem *parentItem = getItem(parent);
    TreeItem *childItem = parentItem->child(row);
    if (childItem) return createIndex(row, column, childItem);
    else return QModelIndex();
}

QModelIndex TreeItemModel::parent(const QModelIndex &index) const
{
    if (!index.isValid()) return QModelIndex();
    TreeItem *childItem = getItem(index);
    TreeItem *parentItem = childItem->parentItem();
    if (parentItem == rootItem) return QModelIndex();
    return createIndex(parentItem->childNumber(), 0, parentItem);
}

int TreeItemModel::rowCount(const QModelIndex &parent) const
{
    TreeItem *parentItem = getItem(parent);
    return parentItem->childCount();
}

int TreeItemModel::columnCount(const QModelIndex &parent) const
{
    return m_conf->columns().size();
}

bool TreeItemModel::hasChildren(const QModelIndex &parent) const
{
    if(m_conf->fetch_expand()){
        auto parentInfo = getItem(parent);
        Q_ASSERT(parentInfo!=0);
        return parentInfo->is_group();
    }else
        return QAbstractItemModel::hasChildren(parent);
}

bool TreeItemModel::canFetchMore(const QModelIndex &parent) const
{
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

void TreeItemModel::fetchMore(const QModelIndex &parent)
{
    //qDebug() << __FUNCTION__;

    if(!m_conf->fetch_expand()){
        return QAbstractItemModel::fetchMore(parent);
    }

    auto parentInfo = getItem(parent);
    if(!parentInfo->mapped()){
        if(m_conf->type_connection() != serverDirectorias)
            load_from_database(uuid_to_string(parentInfo->ref()));
        else{
            auto path = parentInfo->data(column_index("path")).toString();
            load_from_database(path);
        }
        emit fetch(parent);
    }

    return QAbstractItemModel::fetchMore(parent);
}

QVariant TreeItemModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) return QVariant();
    if(role == Qt::SizeHintRole)
        return m_conf->size();
    TreeItem *item = getItem(index);
    Q_ASSERT(item != 0);

    if (role == Qt::ForegroundRole){
        return item->data(index.column(), role);
    }else if (role == Qt::EditRole){
        return item->data(index.column());
    }else if (role == Qt::DisplayRole){
        return item->data(index.column());
    }
    else if (role == Qt::CheckStateRole){
        //        auto val = item->data(index.column());
        //        if(val.typeId() == QMetaType::Bool){
        //            return val.toBool() ? Qt::CheckState::Checked : Qt::CheckState::Unchecked;
        //        }else
        return QVariant();
        //return item->data(index.column());
    }
    else if (role == Qt::DecorationRole){
        if(index.column() == 0){
            if(m_conf->enable_rows_icons()) {
                auto object = item->to_object();
                auto is_group = item->is_group();
                if(m_conf->type_connection() != serverDirectorias){
                    auto deletion_mark = object.value("deletion_mark", 0);
                    auto icon = item->icon();//m_conf->row_icon(item->ref());
                    if(!icon.isNull()){
                        return icon;
                    }else{
                        //if(rows_icon.isNull()){
                        if(!is_group){
                            if(deletion_mark == 0)
                                return m_conf->rows_icons()[tree::item_icons_enum::Item];
                            else{
                                return m_conf->rows_icons()[tree::item_icons_enum::DeletedItem];
                            }
                        }else{
                            if(deletion_mark == 0)
                                return m_conf->rows_icons()[tree::item_icons_enum::ItemGroup];
                            else
                                return m_conf->rows_icons()[tree::item_icons_enum::DeletedItemGroup];
                        }
                    }
                }else{
                    auto path = object.value("path", "");
                    if(!path.empty()){
                        if(!is_group){
                            auto icon = item->icon();
                            if(!icon.isNull()){
                                return icon;
                            }else{
                                auto icon = system_icon(path.c_str(), QSize(16, 16));
                                item->set_icon(icon);
                                return icon;
                            }
                        }else
                            return m_conf->rows_icons()[tree::item_icons_enum::ItemGroup];
                    }else{
                        if(!is_group){
                            return m_conf->rows_icons()[tree::item_icons_enum::Item];
                        }else{
                            return m_conf->rows_icons()[tree::item_icons_enum::ItemGroup];
                        }
                    }
                }
            }else{
                auto v = index.data(Qt::DisplayRole);
                if(v.typeId() == QMetaType::Bool){
                    if(v.toBool()){
                        return m_conf->logical_icons().first;
                    }else{
                        return m_conf->logical_icons().second;
                    }
                }else
                    return QVariant();
            }
        }else{
            auto v = index.data(Qt::DisplayRole);
            if(!v.isValid())
                return QVariant();
            if(v.typeId() == QMetaType::Bool){
                if(v.toBool()){
                    return m_conf->logical_icons().first;
                }else{
                    return m_conf->logical_icons().second;
                }
            }
        }
    }else if (role >= Qt::UserRole && role <= Qt::UserRole + tree::user_roles_max()){
        auto result = item->data(index.column(), role);
        if(result.isNull()){
            return user_role_data(column_name(index.column()), (tree::user_role)role);
        }else
            return result;
    }

    return QVariant();
}

bool TreeItemModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    //    if(role == TREE_CURRENT_SELECTED_ITEM){
    //        if(index.isValid())
    //            current_widget_address_ = qMakePair(index.row(), index.column());
    //        else
    //            current_widget_address_ = qMakePair(-1, -1);
    //        return true;
    //    }

    bool is_user_data = role >= Qt::UserRole && role <= Qt::UserRole + tree::user_roles_max();

    if (role != Qt::EditRole &&
        role != Qt::UserRole &&
        role != Qt::ForegroundRole &&
        !is_user_data ) return false;

    TreeItem *item = getItem(index);
    bool result = item->setData(index.column(), value, role);
    if(role == tree::ValidateTextRole){
        auto text = index.data().toString();
        auto r = value.toString();
        if(!text.isEmpty() && !r.isEmpty() ){
            QRegularExpression rx(r);
            QValidator * validator = new QRegularExpressionValidator(rx, this);
            int pos ;
            if (validator->validate(text, pos) != QValidator::Acceptable){
                item->setData(index.column(), QColor(Qt::red), tree::TextColorRole);
            }else
                item->setData(index.column(), QVariant(), tree::TextColorRole);
            delete validator;
        }
    }

    if (result)
        emit dataChanged(index, index, {role});

    return result;
}

Qt::ItemFlags TreeItemModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;

    bool item_read_only = index.data(tree::ReadOnlyRole).toBool();

    auto fl = QAbstractItemModel::flags(index);
    fl &= ~Qt::ItemIsDragEnabled;
    fl &= ~Qt::ItemIsDropEnabled;

    if(!m_conf->read_only())
        if(!item_read_only)
            fl = fl | Qt::ItemIsEditable;

    if(m_conf->enable_drag())
        fl = fl | Qt::ItemIsDragEnabled;

    if(m_conf->enable_drop())
        fl = fl | Qt::ItemIsDropEnabled;

    auto editorType = index.data(tree::WidgetRole);
    if(editorType.isValid()){
        item_editor_widget_roles t_editor = (item_editor_widget_roles)editorType.toInt();
        if(t_editor == widgetCheckBoxRole){
            fl = fl | Qt::ItemIsUserCheckable;
        }
    }

    return fl;
    //return QAbstractItemModel::flags(index);
}

bool TreeItemModel::insertColumns(int position, int columns, const QModelIndex &parent)
{
    return QAbstractItemModel::insertColumns(position, columns, parent);
}

bool TreeItemModel::removeColumns(int position, int columns, const QModelIndex &parent)
{
    return QAbstractItemModel::removeColumns(position, columns, parent);
}

bool TreeItemModel::insertRows(int position, int rows, const QModelIndex &parent)
{
    TreeItem *parentItem = getItem(parent);
    bool success;
    beginInsertRows(parent, position, position + rows - 1);
    success = parentItem->insertChildren(position, rows, m_conf->columns().size());
    endInsertRows();
    return success;
}

bool TreeItemModel::removeRows(int position, int rows, const QModelIndex &parent)
{
    TreeItem *parentItem = getItem(parent);
    bool success = true;
    beginRemoveRows(parent, position, position + rows - 1);
    success = parentItem->removeChildren(position, rows);
    endRemoveRows();
    return success;
}

void TreeItemModel::init_class(const json& rootData)
{
    auto data = rootData;
    if(data.empty())
        data = json::object();
    data["ref"] = NIL_STRING_UUID;
    if(data.find("is_group") == data.end())
        data["is_group"] = 0;
    m_conf = std::make_shared<TreeConf>();
    m_conf->set_root_data(data);
    rootItem = new TreeItem(data, m_conf);
    m_conf->set_read_only(true);

}

TreeItem *TreeItemModel::getItem(const QModelIndex &index) const {
    if (index.isValid()) {
        TreeItem *item = static_cast<TreeItem*>(index.internalPointer());
        if (item) return item;
    }
    return rootItem;
}

void TreeItemModel::clear()
{
    if(rowCount() == 0)
        return;
    beginResetModel();
    rootItem->clearChildren();
    //row_icon.clear();
    endResetModel();
}

void TreeItemModel::reset()
{
    beginResetModel();
#ifdef USE_QUERY_BUILDER_LIB
    if((m_conf->type_connection()==root_tree_conf::sqlIteMemoryConnection ||
         m_conf->type_connection()==root_tree_conf::sqlIteConnection) && m_db.isOpen())
        reset_sql_table();
#endif
    endResetModel();
}

bool TreeItemModel::belongsToItem(const QModelIndex &index, const QModelIndex parent)
{
    auto item = getItem(index);
    auto parentItem = getItem(parent);
    if(item == parentItem)
        return true;
    auto index_ = find(item->ref(), parent);
    return index_.isValid();
}

void TreeItemModel::move_up(const QModelIndex &index)
{
    Q_ASSERT(index.isValid());
    if(index.row() < 1)
        return;

    beginMoveRows(index.parent(), index.row(), index.row(), index.parent(), index.row()-1);
    auto itemInfo = getItem(index);
    Q_ASSERT(itemInfo != 0);
    auto parentInfo = itemInfo->parentItem();
    auto pos = itemInfo->childNumber();

    if(parentInfo != 0)
        parentInfo->moveChildren(pos, pos - 1);

    endMoveRows();
}

void TreeItemModel::move_down(const QModelIndex &index)
{
    Q_ASSERT(index.isValid());
    if(index.row() > rowCount(index.parent()) - 2)
        return;

    beginMoveRows(index.parent(), index.row(), index.row(), index.parent(), index.row()+1);
    auto itemInfo = getItem(index);
    Q_ASSERT(itemInfo != 0);
    auto parentInfo = itemInfo->parentItem();
    auto pos = itemInfo->childNumber();

    if(parentInfo != 0){
        beginMoveRows(index.parent(), pos, pos, index.parent(), (pos >= parentInfo->childCount()) ? parentInfo->childCount() : (pos + 2));
        parentInfo->moveChildren(pos, pos + 1);
    }

    endMoveRows();
}

//void TreeItemModel::move_to_gui(const QModelIndex &index)
//{
//#ifndef IS_OS_ANDROID
//    auto dlg = SelectGroupDialog(this);
//    if(dlg.exec() == QDialog::Accepted){
//        auto pr = dlg.result();
//        auto parent = find(QUuid::fromString(pr.value("ref", NIL_STRING_UUID)));
//        if(parent.isValid())
//            move_to(index, parent);
//    }
//#else
//    Q_UNUSED(index);
//#endif
//}

void TreeItemModel::move_to(const QModelIndex &index, const QModelIndex &new_parent)
{
    Q_ASSERT(index.isValid());
    Q_ASSERT(new_parent.isValid());

    auto itemInfo = getItem(index);
    Q_ASSERT(itemInfo != 0);
    auto parentInfo = getItem(new_parent);
    auto oldParentInfo = getItem(index.parent());
    Q_ASSERT(parentInfo != 0);

    beginMoveRows(index.parent(), index.row(), index.row(), new_parent, rowCount(new_parent));
    itemInfo->setParent(parentInfo);
    parentInfo->appendChild(itemInfo);
    oldParentInfo->removeChildren(index.row(), 1);
    endMoveRows();
}

bool TreeItemModel::remove(const QModelIndex &index, bool upgrade_database)
{
#ifdef USE_QUERY_BUILDER_LIB
    if(upgrade_database && m_conf->is_database_changed()){
        remove_sql_data(index);
    }
#endif
    return removeRow(index.row(), index.parent());
}

[[maybe_unused]] void TreeItemModel::remove_childs(const QModelIndex &parent)
{
    while (rowCount(parent) > 0) {
        auto index = this->index(0, 0, parent);
        remove(index);
    }
}

bool TreeItemModel::remove_sql_data(const QModelIndex &index)
{
#ifdef USE_QUERY_BUILDER_LIB
    if((m_conf->type_connection() == root_tree_conf::sqlIteMemoryConnection ||
         m_conf->type_connection() == root_tree_conf::sqlIteConnection) && m_db.isOpen()){
        auto item = getItem(index);
        auto arr = json::array();
        if(item->is_group())
            arr = to_array(index.parent(), true);
        else{
            arr += to_object(index);
        }

        auto query = builder::query_builder();
        QString query_script;
        for (auto itr = arr.begin(); itr != arr.end(); ++itr) {
            std::string ref = (*itr).value(m_conf->id_column(), "");
            if(!query_script.isEmpty())
                query_script.append("\n;\n");
            if(!ref.empty())
                query_script.append(query.remove().from(m_conf->table_name().toStdString()).where(json{{m_conf->id_column(), ref}}, true).prepare().c_str());
        }

        if(!executeQueryPackade(query_script.toUtf8(), m_db))
            return false;

    }
    return true;// removeRow(index.row(), index.parent());
#else
    return false;
#endif
}

void TreeItemModel::reset_sql_table()
{
#ifdef USE_QUERY_BUILDER_LIB
    if(!m_db.isOpen())
        return;

    if(m_conf->table_name().isEmpty())
        return;

    clear();
    auto query = builder::query_builder();
    if(m_conf->user_sql_where().size() == 0){
        auto table = query.select().from(m_conf->table_name().toStdString()).to_table(m_db);
        set_table(table);
    }else{
        auto table = query.select().from(m_conf->table_name().toStdString()).where(m_conf->user_sql_where(), true).to_table(m_db);
        set_table(table);
    }
#endif

}

void TreeItemModel::get_path(const QModelIndex &index, int column, QString &current)
{
    auto item = getItem(index);
    if(item == rootItem)
        return;
    if(item->childCount() > 0)
        current.insert(0, "/");
    current.insert(0, item->data(column).toString());
    get_path(index.parent(), column, current);
}

json TreeItemModel::to_array(const QModelIndex &parent, bool childs, bool group_only) const
{

    json result = json::array();

    to_array_(parent, result, childs, group_only);

    return result;
}

void TreeItemModel::to_array_(const QModelIndex &parent, json& result, bool childs, bool group_only) const
{
    auto parent_ = index(parent.row(), 0, parent.parent());

    for (int itr = 0; itr < rowCount(parent_); ++itr) {
        auto in = index(itr, 0, parent_);
        if(in.isValid()){
            auto item = getItem(in);
            if(group_only){
                if(!item->is_group())
                    continue;
            }
            result += item->to_object();
            if(childs)
                to_array_(in, result, childs, group_only);
        }
    }

}
#ifdef USE_QUERY_BUILDER_LIB
bool TreeItemModel::connect_sql_database(const QString& table_name )
{
    if(m_conf->type_connection() == root_tree_conf::sqlIteConnection){
        m_db = QSqlDatabase::addDatabase("QSQLITE", "private.sqlite");
        m_db.setDatabaseName(m_conf->connection_string());
        if(!m_db.open()){
            auto err = m_db.lastError();
            qCritical() << err.text();
        }else{
            m_conf->set_database(&m_db);
        }

    }else if(m_conf->type_connection() == root_tree_conf::sqlIteMemoryConnection){
        m_db = QSqlDatabase::database("private.sqlite");
        m_conf->set_table_name(table_name);
        m_conf->set_database(&m_db);
    }

    return m_db.isOpen();
}
#endif

void TreeItemModel::load_from_database(const QString& parent)
{
    if(m_conf->table_name().isEmpty())
        throw NativeException("Не указана таблица!");

    if(m_conf->type_connection() == sqlIteConnection
        || m_conf->type_connection() == sqlIteMemoryConnection){
#ifdef USE_QUERY_BUILDER_LIB
        if(!m_db.isOpen())
            return;

        QString query_text = QString("select * from %1").arg(m_conf->table_name());
        if(m_conf->fetch_expand()){
            query_text.append(QString(" where parent ='%1' order by is_group DESC, [first] ASC").arg(parent));
        }

        auto arr = from_sql(m_db, query_text);
        if(arr.size() > 0){
            for (auto itr = arr.begin(); itr != arr.end(); ++itr) {
                auto row = *itr;
                auto ref = row.value("parent", NIL_STRING_UUID);
                if(ref != NIL_STRING_UUID){
                    auto parent = find(tree::to_qt_uuid(ref));
                    add(row, parent);
                    auto parentInfo = getItem(parent);
                    parentInfo->set_mapped(true);
                }else
                    add(row);

            }
        }
#endif
    }else if(m_conf->type_connection() == httpConnection){
        auto http_result = http_get(parent.toStdString());
        if(http_result.is_object()){
            auto arr = http_result["rows"];
            if(arr.size() > 0){
                for (auto itr = arr.begin(); itr != arr.end(); ++itr) {
                    auto row = *itr;
                    auto ref = row.value("parent", NIL_STRING_UUID);
                    if(ref != NIL_STRING_UUID){
                        auto parentIndex = find(to_qt_uuid(ref));
                        add(row, parentIndex);
                        auto parentInfo = getItem(parentIndex);
                        parentInfo->set_mapped(true);
                    }else
                        add(row);

                }
            }
        }
    }else if(m_conf->type_connection() == serverDirectorias){
        auto http_result = http_get_directory(parent.toStdString());
        if(http_result.is_object()){
            auto arr = http_result["rows"];
            if(arr.size() > 0){
                for (auto itr = arr.begin(); itr != arr.end(); ++itr) {
                    auto row = *itr;
                    auto ref = row.value("parent", NIL_STRING_UUID);
                    if(ref != NIL_STRING_UUID){
                        auto parentIndex = find(to_qt_uuid(ref));
                        add(row, parentIndex);
                        auto parentInfo = getItem(parentIndex);
                        parentInfo->set_mapped(true);
                    }else
                        add(row);

                }
            }
        }
    }
}

json TreeItemModel::http_get_directory(const json &parent) const{

#ifdef USE_WEBSOCKET_LIB
    std::string uuid_form_ = arcirk::uuids::nil_string_uuid();

    nlohmann::json param = {
        {"table", true},
        {"uuid_form", uuid_form_},
        {"empty_column", false},
        {"recursive", false},
        {"parent_path", parent}
    };

    auto http_ = http_param();
    http_.command = arcirk::enum_synonym(arcirk::server::server_commands::ProfileDirFileList);
    http_.param = QByteArray(param.dump().data()).toBase64().toStdString();

    QEventLoop loop;
    int httpStatus = 200;
    QByteArray httpData;
    QNetworkAccessManager httpService;

    auto finished = [&loop, &httpStatus, &httpData](QNetworkReply* reply) -> void
    {
        QVariant status_code = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
        if(status_code.isValid()){
            httpStatus = status_code.toInt();
            if(httpStatus != 200){
                qCritical() << QDateTime::currentDateTime().toString("hh:mm:ss") << __FUNCTION__ << "Error: " << httpStatus << " " + reply->errorString() ;
            }else
            {
                httpData = reply->readAll();
            }
        }
        loop.quit();

    };

    loop.connect(&httpService, &QNetworkAccessManager::finished, finished);

    QUrl url(m_conf->http_conf().host.c_str());
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QString headerData = "Token " + QByteArray(m_conf->http_conf().token.c_str()).toBase64();
    request.setRawHeader("Authorization", headerData.toLocal8Bit());

    httpService.post(request, QByteArray::fromStdString(pre::json::to_json(http_).dump()));
    loop.exec();

    if(httpStatus != 200){
        return WS_RESULT_ERROR;
    }

    if(httpData.isEmpty())
        return WS_RESULT_ERROR;

    if(httpData == "error"){
        return WS_RESULT_ERROR;
    }

    auto msg = pre::json::from_json<arcirk::server::server_response>(httpData.toStdString());

    if(msg.result.empty())
        return WS_RESULT_ERROR;

    if(msg.result == WS_RESULT_ERROR)
        return WS_RESULT_ERROR;

    auto http_result = json::parse(QByteArray::fromBase64(msg.result.data()).toStdString());

    return http_result;
#else
    return WS_RESULT_ERROR;
#endif
}


nlohmann::json TreeItemModel::http_get(const json &parent) const
{
#ifdef USE_WEBSOCKET_LIB
    if(m_conf->table_name().isEmpty())
        return WS_RESULT_ERROR;

    using namespace arcirk::database;
    auto query = builder::query_builder();

    auto where_root = json::object();
    where_root["parent"] = parent;
    std::string query_text;

    if(m_conf->user_sql_where().is_object() && m_conf->user_sql_where().size() > 0){
        auto where =  m_conf->user_sql_where();
        auto items = where.items();
        for (auto itr = items.begin(); itr != items.end(); ++itr) {
            where_root[itr.key()] = itr.value();
        }
    }
    json order{
        {"first", 0},
        {"is_group", 1}
    };

    query_text = query.select().from(m_conf->table_name().toStdString()).where(where_root, true).order_by(order).prepare();
    auto param = nlohmann::json::object();
    param["query_text"] = query_text;

    auto http_param = arcirk::http::http_param();
    http_param.command = "ExecuteSqlQuery";
    http_param.param = QByteArray(param.dump().data()).toBase64().toStdString();

    QEventLoop loop;
    int httpStatus = 200;
    QByteArray httpData;
    QNetworkAccessManager httpService;

    auto finished = [&loop, &httpStatus, &httpData](QNetworkReply* reply) -> void
    {
        QVariant status_code = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
        if(status_code.isValid()){
            httpStatus = status_code.toInt();
            if(httpStatus != 200){
                qCritical() << QDateTime::currentDateTime().toString("hh:mm:ss") << __FUNCTION__ << "Error: " << httpStatus << " " + reply->errorString() ;
            }else
            {
                httpData = reply->readAll();
            }
        }
        loop.quit();

    };

    loop.connect(&httpService, &QNetworkAccessManager::finished, finished);

    QUrl url(m_conf->http_conf().host.c_str());
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QString headerData = "Token " + QByteArray(m_conf->http_conf().token.c_str()).toBase64();
    request.setRawHeader("Authorization", headerData.toLocal8Bit());

    httpService.post(request, QByteArray::fromStdString(pre::json::to_json(http_param).dump()));
    loop.exec();

    if(httpStatus != 200){
        return WS_RESULT_ERROR;
    }

    if(httpData.isEmpty())
        return WS_RESULT_ERROR;

    if(httpData == "error"){
        return WS_RESULT_ERROR;
    }

    if(!json::accept(httpData.toStdString())){
        return WS_RESULT_ERROR;
    }

    //auto msg = pre::json::from_json<arcirk::server::server_response>(httpData.toStdString());
    auto msg = json::parse(httpData.toStdString());

    if(msg["result"].empty())
        return WS_RESULT_ERROR;

    if(msg["result"] == WS_RESULT_ERROR)
        return WS_RESULT_ERROR;

    auto http_result = nlohmann::json::parse(QByteArray::fromBase64(msg["result"].get<std::string>().data()).toStdString());

    return http_result;
#else
    return WS_RESULT_ERROR;
#endif
}

json TreeItemModel::to_object(const QModelIndex &index) const
{

    if(!index.isValid())
        return rootItem->to_object();

    TreeItem *item = getItem(index);
    Q_ASSERT(item!=0);
    return item->to_object();
}

json TreeItemModel::to_table_model(const QModelIndex &parent, bool childs, bool group_only) const
{
    auto columns_j = json::array();
    foreach (const auto& key , m_conf->columns()) {
        columns_j += key.toStdString();
    }

    auto rows = to_array(parent, childs, group_only);

    return json::object({
        {"columns" , columns_j},
        {"rows", rows}
    });
}

QModelIndex TreeItemModel::add(const json& object, const QModelIndex &parent, bool upgrade_database)
{
    auto parent_(parent);
    if(!m_conf->hierarchical_list())
        parent_ = QModelIndex();

    TreeItem *rootItem_ = getItem(parent_);
    int position = rootItem_->childCount();

    if (!insertRow(position, parent_)) return QModelIndex();

    TreeItem *item = getItem(index(position, 0, parent_));
    if(item != 0)
        item->set_object(object, upgrade_database);

    return index(position, 0, parent_);
}

void TreeItemModel::add_array(const json& arr, const QModelIndex &parent, bool upgrade_database)
{
    if(!arr.is_array())
        return;

    for (auto itr = arr.begin(); itr != arr.end(); ++itr) {
        add(*itr, parent, upgrade_database);
    }
}

void TreeItemModel::set_table(const json &table, const QModelIndex &parent, bool upgrade_database)
{
    Q_ASSERT(table.is_object());
    Q_ASSERT(table.value("columns", json{}).is_array());
    //Q_ASSERT(table.value("rows", json{}).is_array());

    auto rows = table.value("rows", nlohmann::json::array());
    for (auto itr = rows.begin(); itr != rows.end(); ++itr) {
        auto object = *itr;
        if(object.find("parent") != object.end()){
            auto parent_index = find(to_qt_uuid(object.value("parent", NIL_STRING_UUID)));
            add(*itr, parent_index, upgrade_database);
        }else
            add(*itr, parent, upgrade_database);
    }
}

void TreeItemModel::set_object(const QModelIndex &index, const nlohmann::json &object, bool upgrade_database)
{
    TreeItem *item = getItem(index);
    if(item != 0){
        item->set_object(object, upgrade_database);
    }
}

QModelIndex TreeItemModel::find(const QUuid &ref, const QModelIndex& parent) const
{
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

QModelIndex TreeItemModel::find(int column, const QVariant &source, const QModelIndex& parent) const
{
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

QList<QModelIndex> TreeItemModel::find_all(int column, const QVariant &source, const QModelIndex& parent) const
{
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

json TreeItemModel::empty_data(){
    Q_ASSERT(rootItem!=0);
    return rootItem->to_object();
}

void TreeItemModel::set_object_name(const QModelIndex &index, const QString &name)
{
    auto item = getItem(index);
    item->set_object_name(name);
}

QString TreeItemModel::object_name(const QModelIndex &index)
{
    if(!index.isValid())
        return rootItem->object_name();
    auto item = getItem(index);
    return item->object_name();
}

void TreeItemModel::set_columns_order(const QList<QString> &names)
{
    beginResetModel();
    m_conf->set_columns_order(names);
    endResetModel();
}

void TreeItemModel::set_hierarchical_list(bool value){
    m_conf->set_hierarchical_list(value);
    emit hierarchicalListChanged(value);
}

#ifdef USE_QUERY_BUILDER_LIB
void TreeItemModel::set_connection(root_tree_conf::typeConnection type, const QString &value, const QString &table_name)
{
    m_conf->set_type_connection(type);
    m_conf->set_connection_string(value);
    m_conf->set_table_name(table_name);
    if(type == root_tree_conf::typeConnection::sqlIteConnection){
        if(connect_sql_database())
            load_from_database(NIL_STRING_UUID);
    }else if(type == root_tree_conf::typeConnection::sqlIteMemoryConnection){
        if(connect_sql_database(table_name))
            load_from_database(NIL_STRING_UUID);
    }
}
#endif

void TreeItemModel::set_connection(typeConnection type, const http::http_conf &value)
{
    m_conf->set_type_connection(type);
    m_conf->set_http_conf(value.host.c_str(), value.token.c_str(), value.table.c_str());
    m_conf->set_table_name(value.table.c_str());
    if(type == typeConnection::httpConnection){
        load_from_database(NIL_STRING_UUID);
    }else     if(type == typeConnection::serverDirectorias){
        load_from_database("");
    }
}

void TreeItemModel::set_user_role_data(const QString &column, tree::user_role role, const QVariant &value)
{
    m_conf->set_user_data(column, value, role);
}

QVariant TreeItemModel::user_role_data(const QString &column, tree::user_role role) const
{
    return m_conf->user_data(column, role);
}

void TreeItemModel::set_row_image(const QModelIndex &index, const QIcon &ico)
{
    auto item = getItem(index);
    item->set_icon(ico);
}

QIcon TreeItemModel::row_image(const QModelIndex &index)
{
    auto item = getItem(index);
    return item->icon();
}

void TreeItemModel::reset_sql_data()
{
#ifdef USE_QUERY_BUILDER_LIB
    if(!m_conf->is_database_changed())
        return;
    Q_ASSERT(m_conf->get_database()!=0);
    if((m_conf->type_connection() == root_tree_conf::sqlIteMemoryConnection ||
         m_conf->type_connection() == root_tree_conf::sqlIteConnection) && m_conf->get_database()->isOpen()){

        auto where = unload_column("ref", QModelIndex(), true);
        auto query = builder::query_builder();
        QString text = query.remove().from(m_conf->table_name().toStdString()).where(json{{"ref", where}},true).prepare().c_str();
        m_conf->get_database()->exec(
            text
            );

        if(m_conf->get_database()->lastError().isValid())
            qCritical() << m_conf->get_database()->lastError().text();

        auto arr = to_array(QModelIndex(), true);
        QList<QString> m_querias;

        for (auto itr = arr.begin(); itr != arr.end(); ++itr) {
            query.clear();
            query.use(*itr);
            m_querias.append(query.insert(m_conf->table_name().toStdString(), true).prepare().c_str());
        }

        m_conf->get_database()->transaction();
        foreach (auto itr, m_querias) {
            m_conf->get_database()->exec(itr);
        }

        m_conf->get_database()->commit();

        if(m_conf->get_database()->lastError().isValid())
            qCritical() << m_conf->get_database()->lastError().text();
    }
#endif
}

json TreeItemModel::unload_column(const QString &column, const QModelIndex &parent, bool recursive)
{
    auto column_ = column_index(column);
    auto result = json::array();
    unload_column_private(column, column_, parent, result, recursive);
    return result;
}

void TreeItemModel::init_root_data(const json &rootData)
{
    if(rootData.is_array()){
        json m_data = json::object();
        for (auto itr = rootData.begin(); itr != rootData.end(); ++itr) {
            m_data[*itr] = "";
        }
        m_data["ref"] = NIL_STRING_UUID;
        m_conf->set_root_data(m_data);
    }
}

json TreeItemModel::unload_column(int column, const QModelIndex &parent, bool recursive)
{
    auto column_ = column_name(column);
    auto result = json::array();
    unload_column_private(column_, column, parent, result, recursive);
    return result;
}

void TreeItemModel::unload_column_private(const QString &column, int column_index, const QModelIndex &parent, json &result, bool recursive)
{
    Q_ASSERT(result.is_array());
    auto parentItem = getItem(parent);
    Q_ASSERT(column_index < columnCount());

    for (int i = 0; i < parentItem->childCount(); ++i) {
        auto index = this->index(i, column_index, parent);
        auto object = to_object(index);
        result += object.value(column.toStdString(), json{});
        if(recursive){
            unload_column_private(column, column_index, index, result, recursive);
        }
    }
}


void TreeItemModel::set_column_size(int section, const QSize& size)
{
    m_conf->set_section_size(section, size);
}

bool TreeItemModel::is_group(const QModelIndex& index){
    if(!index.isValid())
        return true;

    auto item = getItem(index);
    return item->is_group();
}

QUuid TreeItemModel::ref(const QModelIndex& index) const{
    if(!index.isValid())
        return QUuid();

    auto item = getItem(index);
    return item->ref();
}

void TreeItemModel::set_inner_role(const QModelIndex &index, editor_inner_role role)
{
    if(!index.isValid())
        return;

    auto item = getItem(index);
    item->set_inner_role(role);
}

editor_inner_role TreeItemModel::inner_role(const QModelIndex &index)
{
    if(!index.isValid())
        return editor_inner_role::editorInnerRoleINVALID;

    auto item = getItem(index);
    return item->inner_role();
}

QMap<QString, editor_inner_role> TreeItemModel::row_inner_roles(int row, const QModelIndex& parent) const
{
    QMap<QString, editor_inner_role> result{};
    if(row < 0 || row >= columnCount())
        return result;

    for (int i = 0; i < columnCount(); ++i) {
        auto index_ = index(row, i, parent);
        result.insert(column_name(i), (editor_inner_role)data(index_, WidgetInnerRole).toInt());
    }

    return result;
}

void TreeItemModel::set_row_inner_roles(int row, const QMap<QString, editor_inner_role>& roles, const QModelIndex &parent)
{
    if(roles.size() != columnCount())
        return;

    for (int i = 0; i < columnCount(); ++i) {
        auto index_ = index(row, i, parent);
        auto item = getItem(index_);
        item->setData(i, roles[column_name(i)], WidgetInnerRole);
    }
}

void TreeItemModel::reset_variant_roles(const QModelIndex& parent)
{
    QList<QString> m_keys{};

    auto def = m_conf->user_data_values()[tree::user_role::WidgetRole];
    for (auto itr = def.begin(); itr != def.end(); ++itr) {
        auto val = itr.value();
        if(val.isValid()){
            if((item_editor_widget_roles)val.toInt() == widgetVariantRole){
                if(m_keys.indexOf(itr.key()) == -1)
                    m_keys.append(itr.key());
            }
        }
    }

    for (int i = 0; i < rowCount(parent); ++i) {
        foreach (auto key, m_keys) {
            auto k = column_index(key);
            auto index = this->index(i, k, parent);
            if(index.isValid()){
                auto item = getItem(index);
                auto object = to_object(index);
                if(object.find(key.toStdString()) != object.end()){
                    auto val = object[key.toStdString()];
                    if(val.is_string())
                        item->setData(k, editor_inner_role::editorText, WidgetInnerRole);
                    else if(val.is_number())
                        item->setData(k, editor_inner_role::editorNumber, WidgetInnerRole);
                    else if(val.is_boolean())
                        item->setData(k, editor_inner_role::editorBoolean, WidgetInnerRole);
                    else if(val.is_array()){
                        editor_inner_role r = editor_inner_role::editorArray;
                        try {
                            ByteArray tmp = val.get<ByteArray>();
                            r = editor_inner_role::editorByteArray;
                        } catch (...) {

                        }
                        item->setData(k, r, WidgetInnerRole);
                    }else
                        item->setData(k, editor_inner_role::editorNullType, WidgetInnerRole);
                }

                reset_variant_roles(index);
            }
        }
    }
}

QString TreeItemModel::path(const QModelIndex &index, int column)
{
    if(column < 0 || column >= columnCount())
        return "/";

    auto item = getItem(index);

    if(item == rootItem)
        return "/";

    QString result;
    get_path(index, column, result);
    return result;
}
