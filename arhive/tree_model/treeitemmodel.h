#ifndef TREEITEMMODEL_H
#define TREEITEMMODEL_H

#include "tree_model_global.h"
#include "treeconf.h"
#include <QAbstractItemModel>
#include <QObject>
#include <QIcon>
#include <QHeaderView>
#include "treeitem.h"
#include <memory>

#ifdef USE_WEBSOCKET_LIB
#include "websocketclient.h"
#endif

using namespace arcirk::tree;
using namespace arcirk::widgets;
using namespace arcirk::http;

namespace arcirk::tree_model {

class TREE_MODEL_EXPORT TreeItemModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    explicit TreeItemModel(QObject *parent = nullptr);
    explicit TreeItemModel(const json& rootData, QObject *parent = nullptr);
    ~TreeItemModel();

    // Header:
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    bool setHeaderData(int section, Qt::Orientation orientation, const QVariant &value, int role = Qt::EditRole) override;
    void headerChanged(Qt::Orientation orientation, int section);

    // Basic functionality:
    QModelIndex index(int row, int column,
                      const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &index) const override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    // Fetch data dynamically:
    bool hasChildren(const QModelIndex &parent = QModelIndex()) const override;

    bool canFetchMore(const QModelIndex &parent) const override;
    void fetchMore(const QModelIndex &parent) override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    // Editable:
    bool setData(const QModelIndex &index, const QVariant &value,
                 int role = Qt::EditRole) override;

    Qt::ItemFlags flags(const QModelIndex& index) const override;

    bool insertColumns(int position, int columns,const QModelIndex &parent = QModelIndex()) override;
    bool removeColumns(int position, int columns, const QModelIndex &parent = QModelIndex()) override;
    bool insertRows(int position, int rows, const QModelIndex &parent = QModelIndex()) override;
    bool removeRows(int position, int rows, const QModelIndex &parent = QModelIndex()) override;

    TreeItem *getItem(const QModelIndex &index) const;

    int column_index(const QString &name) const {return m_conf->column_index(name);};
    QString column_name(const int &index) const {return m_conf->column_name(index);};
    void set_column_size(int section, const QSize& size);
    void set_columns_order(const QList<QString>& names);
    QList<QString> columns_order() const {return m_conf->columns_order();};
    void set_column_aliases(const QMap<QString, QString> &aliases){m_conf->set_column_aliases(aliases);};

    Q_INVOKABLE void clear();
    Q_INVOKABLE void reset();

    bool belongsToItem(const QModelIndex& index, const QModelIndex parent);

    void move_up(const QModelIndex &index);
    void move_down(const QModelIndex &index);
    //void move_to_gui(const QModelIndex &index);
    void move_to(const QModelIndex &index, const QModelIndex &new_parent);

    bool remove(const QModelIndex &index, bool upgrade_database = false);

    [[maybe_unused]] void remove_childs(const QModelIndex &parent);
    QModelIndex add(const json& object, const QModelIndex &parent = QModelIndex(), bool upgrade_database = false);
    void add_array(const json& arr, const QModelIndex &parent = QModelIndex(), bool upgrade_database = false);
    void set_table(const json& table, const QModelIndex &parent = QModelIndex(), bool upgrade_database = false);
    void set_object(const QModelIndex &index, const nlohmann::json& object, bool upgrade_database = false);

    [[nodiscard]] json to_array(const QModelIndex &parent, bool childs = false, bool group_only = false) const;
    [[nodiscard]] json to_object(const QModelIndex &index) const;
    [[nodiscard]] json to_table_model(const QModelIndex &parent, bool childs = false, bool group_only = false) const;

    [[nodiscard]] QModelIndex find(const QUuid& ref, const QModelIndex& parent = QModelIndex()) const;
    [[nodiscard]] QModelIndex find(int column, const QVariant& source, const QModelIndex& parent = QModelIndex()) const;
    [[nodiscard]] QList<QModelIndex> find_all(int column, const QVariant& source, const QModelIndex& parent = QModelIndex()) const;

    json empty_data();
    void set_object_name(const QModelIndex& index, const QString& name);
    QString object_name(const QModelIndex& index);

    void set_hierarchical_list(bool value);
    bool hierarchical_list(){return m_conf->hierarchical_list();};

    void enable_drag_and_drop(bool value){m_conf->enable_drag_and_drop(value);};
    bool enable_drag(){return m_conf->enable_drag();};
    bool enable_drop(){return m_conf->enable_drop();};
    void set_enable_drag(bool value){m_conf->set_enable_drag(value);};
    void set_enable_drop(bool value){m_conf->set_enable_drop(value);};

#ifdef USE_QUERY_BUILDER_LIB
    void set_connection(root_tree_conf::typeConnection type, const QString& value, const QString& table_name);
#endif

    void set_connection(typeConnection type, const http::http_conf& value);

#ifdef USE_QUERY_BUILDER_LIB
    void enable_database_changed(){m_conf->set_database_changed(true);};
#endif
    QPair<int, int> widget_address(){return m_widget_address;};
    void set_widget_address(const QPair<int, int> value){m_widget_address = value;};

    void set_read_only(bool value){m_conf->set_read_only(value);};
    bool read_only(){return m_conf->read_only();};

    void set_user_role_data(const QString& column, tree::user_role role, const QVariant& value);
    QVariant user_role_data(const QString& column, tree::user_role role) const;

    QMap<tree::user_role, QMap<QString, QVariant>> user_data_values() const{
        return m_conf->user_data_values();
    }
    QMap<tree::user_role, QMap<QString, QVariant>> user_data_values(const QModelIndex& index) const{
        auto u_data = m_conf->user_data_values();
        if(!index.isValid())
            return u_data;
        int count = user_roles_max() + (int)Qt::UserRole;
        int start = (int)UserRoleDef;
        for (int var = start; var < count; ++var) {
            auto itr = u_data.find((user_role)var);
            if(itr != u_data.end()){
                for (int i = 0; i < columnCount(); ++i) {
                    auto index_ = this->index(index.row(), i, index.parent());
                    if(index_.isValid()){
                        auto val = index_.data((user_role)var);
                        if(val.isValid())
                            u_data[(user_role)var][column_name(i)] = val;
                    }

                }
            }
        }


        return u_data;
    }

    void set_fetch_expand(bool value){m_conf->set_fetch_expand(value);};

    void set_rows_icon(tree::item_icons_enum state, const QIcon &icon){m_conf->set_rows_icons(state, icon);};
    QIcon rows_icon(tree::item_icons_enum state){return m_conf->rows_icon(state);};;
    void set_enable_rows_icons(bool value){m_conf->set_enable_rows_icons(value);};

    void set_row_image(const QModelIndex &index, const QIcon &ico);
    QIcon row_image(const QModelIndex &index);

    QList<QString> columnNames() const {return m_conf->columns();};

    void reset_columns(const json& cols){m_conf->reset_columns(cols);};

    const QMap<QString, QString> columns_aliases(){return m_conf->columns_aliases();}

#ifdef BADMIN_APPLICATION
    arcirk::server::server_objects server_object() const {return m_server_object;};
    void set_server_object(arcirk::server::server_objects value) {m_server_object = value;};
#endif

#ifdef USE_QUERY_BUILDER_LIB
    void set_user_sql_where(const json& value){m_conf->set_user_sql_where(value);};
#endif
    void reset_sql_data();

    json unload_column(int column, const QModelIndex& parent = QModelIndex(), bool recursive = false);
    json unload_column(const QString& column, const QModelIndex& parent = QModelIndex(), bool recursive = false);

    bool is_group(const QModelIndex& index = QModelIndex());
    QUuid ref(const QModelIndex& index = QModelIndex()) const;

    QMap<QString, QString> column_aliases_default() const{ return m_conf->column_aliases_default();};

    //внутренняя роль элемента управления TreeVariant
    void set_inner_role(const QModelIndex& index, editor_inner_role role);
        editor_inner_role inner_role(const QModelIndex& index);
    QMap<QString, editor_inner_role> row_inner_roles(int row, const QModelIndex& parent = QModelIndex()) const;
    void set_row_inner_roles(int row, const QMap<QString, editor_inner_role>& roles, const QModelIndex& parent = QModelIndex());
    void reset_variant_roles(const QModelIndex& parent = QModelIndex());
    //

    QString path(const QModelIndex& index, int column = 0);

protected:
#ifdef USE_QUERY_BUILDER_LIB
    QSqlDatabase m_db;
#endif
    std::shared_ptr<TreeConf> m_conf;

    void init_root_data(const json& rootData);

private:

    TreeItem* rootItem;

    QPair<int, int> m_widget_address;

    void init_class(const json& rootData = {});
    void to_array_(const QModelIndex &parent, json& result, bool childs = false, bool group_only = false) const;
#ifdef USE_QUERY_BUILDER_LIB
    bool connect_sql_database(const QString& table_name = "");
#endif
    void load_from_database(const QString& parent = NIL_STRING_UUID);
    json http_get(const json &parent) const;
    json http_get_directory(const json &parent) const;

    bool remove_sql_data(const QModelIndex &index);
    void reset_sql_table();

    void get_path(const QModelIndex& index, int column, QString& current);

#ifdef BADMIN_APPLICATION
    arcirk::server::server_objects m_server_object;
#endif
    void unload_column_private(const QString& column, int column_index, const QModelIndex& parent, json& result, bool recursive = false);

signals:
    void hierarchicalListChanged(bool state);
    void fetch(const QModelIndex &parent);

};


}
#endif // TREEITEMMODEL_H
