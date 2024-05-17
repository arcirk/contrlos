#ifndef CONTROLSPROG_TREEMODEL_H
#define CONTROLSPROG_TREEMODEL_H

#include "treeconf.h"
#include "../controls_global.h"
#include <QAbstractItemModel>
#include <QList>
#include <QSize>
#include "treeitem.h"

enum tree_predefined_fields{
    ftreeRef,
    ftreeParent,
    ftreeIsGroup,
    ftreeVersion,
    ftreePredefined,
    ftreeINVALID = -1
};
NLOHMANN_JSON_SERIALIZE_ENUM(tree_predefined_fields, {
    {ftreeINVALID, nullptr}    ,
    {ftreeRef, "ref"}  ,
    {ftreeParent, "parent"}  ,
    {ftreeIsGroup, "is_group"}  ,
    {ftreePredefined, "predefined"}  ,
    {ftreeVersion, "version"}  ,
});

namespace arcirk::widgets {

    class CONTROLS_EXPORT TreeModel : public QAbstractItemModel
    {
        Q_OBJECT
        public:
            explicit TreeModel(QObject *parent = nullptr);
            explicit TreeModel(const json& rootData, QObject *parent = nullptr);

            ~TreeModel();
            [[nodiscard]] int rowCount(const QModelIndex &parent = QModelIndex()) const override;
            [[nodiscard]] int columnCount(const QModelIndex &parent = QModelIndex()) const override;

            bool hasChildren(const QModelIndex &parent = QModelIndex()) const override;

            bool canFetchMore(const QModelIndex &parent) const override;
            void fetchMore(const QModelIndex &parent) override;

            [[nodiscard]] QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
            bool setHeaderData(int section, Qt::Orientation orientation, const QVariant &value, int role = Qt::EditRole) override;
            void headerChanged(Qt::Orientation orientation, int section);

            [[nodiscard]] QVariant data(const QModelIndex &index, int role  = Qt::DisplayRole) const override;
            bool setData(const QModelIndex &index, const QVariant &value,
                         int role = Qt::EditRole) override;
            [[nodiscard]] QModelIndex index(int row, int column,
                                            const QModelIndex &parent = QModelIndex()) const override;
            [[nodiscard]] QModelIndex parent(const QModelIndex &index) const override;

            [[nodiscard]] json to_json();
            void form_json(const json& table);
            [[nodiscard]] json to_array(const QModelIndex &parent = QModelIndex(), bool hierarchy = true, bool group_only = false);
            [[nodiscard]] json to_array(const QString& column, const QModelIndex &parent = QModelIndex(), bool hierarchy = true, bool group_only = false) const;
            [[nodiscard]] json row(const QModelIndex &index, bool lite = true) const;
            [[nodiscard]] json to_object(const QModelIndex &index, bool lite = true) const;
            [[nodiscard]] json to_table_model(const QModelIndex &parent, bool group_only = false, bool lite = true);
            QModelIndex add(const json& object, const QModelIndex &parent = QModelIndex());
            void set_object(const QModelIndex &index, const json& object);
            bool remove(const QModelIndex &index);
            bool move_up(const QModelIndex &index);
            bool move_down(const QModelIndex &index);
            void move_to(const QModelIndex &index, const QModelIndex &new_parent);
            void clear();
            [[nodiscard]] QString column_name(int index) const;
            [[nodiscard]] int column_index(const QString& name) const;
            [[nodiscard]] QList<QString> columns() const;

            void set_read_only(bool value);
            bool read_only();

            [[maybe_unused]] [[maybe_unused]] json empty_data();

            void reorder_columns(const QList<QString>& names);
            QList<QString> columns_order() const;

            void display_icons(bool value);
            [[nodiscard]] bool is_display_icons() const;

            void set_columns_aliases(const QMap<QString, QString> &aliases);
            [[nodiscard]] QMap<QString, QString> columns_aliases() const;

            [[nodiscard]] TreeConf* get_conf() const {return m_conf.get();};

            [[nodiscard]] bool use_database() const;
            virtual void onRowChanged(const QModelIndex& index){ Q_UNUSED(index);};
            virtual void updateRowsPositions(){};

            bool hierarchical_list();
            void set_hierarchical_list(bool value);

            [[nodiscard]] QModelIndex find(const QUuid& ref, const QModelIndex& parent = QModelIndex()) const;
            [[nodiscard]] QModelIndex find(int column, const QVariant& source, const QModelIndex& parent = QModelIndex()) const;
            [[nodiscard]] QList<QModelIndex> find_all(int column, const QVariant& source, const QModelIndex& parent = QModelIndex()) const;

            bool is_group(const QModelIndex& index = QModelIndex());

            std::vector<std::string> predefined_fields() const;

            QUuid ref(const QModelIndex& index) const;

            bool predefinedItem(const QModelIndex& index) const;

            bool row_not_move(const QModelIndex& index);
            void set_row_not_move(const QModelIndex& index, bool value);

            bool belongsToItem(const QModelIndex& index, const QModelIndex parent);

    private:
        TreeItem* rootItem;
        std::shared_ptr<TreeConf> m_conf;
        bool m_hierarchical_list;

        [[nodiscard]] Qt::ItemFlags flags(const QModelIndex& index) const override;
        void to_array_recursive(const QModelIndex& parent, json result, bool group_only = false);
        void to_array_recursive(const QString& column, const QModelIndex& parent, json result, bool group_only = false);

    protected:
        bool is_use_database;
        [[nodiscard]] TreeItem * getItem(const QModelIndex &index) const;
        bool insertColumns(int position, int columns,const QModelIndex &parent = QModelIndex()) override;
        bool removeColumns(int position, int columns, const QModelIndex &parent = QModelIndex()) override;
        bool insertRows(int position, int rows, const QModelIndex &parent = QModelIndex()) override;
        bool removeRows(int position, int rows, const QModelIndex &parent = QModelIndex()) override;

    signals:
        void hierarchicalListChanged(bool state);
        void fetch(const QModelIndex &parent);
    };

}
#endif //CONTROLSPROG_TREEMODEL_H